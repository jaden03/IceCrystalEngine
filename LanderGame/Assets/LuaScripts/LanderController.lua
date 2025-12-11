print("Lander Controller Running...")

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

local rb = actor:GetComponent("RigidBody");
local light = sceneManager:GetActorByTag("engineLight"):GetComponent("PointLight")

-- Attitude control settings
local torqueStrength = 15000
local engineStrength = 20000

-- Fuel
local fuel = 100;
local fuelBar = sceneManager:GetActorByTag("fuelBarFG")
local fuelBarStartingScale = fuelBar.transform.scale.x;

local inRefuelZone = false
rb.OnTriggerEntered = function(other)
    if other.owner.tag == "refuelTrigger" then
        inRefuelZone = true
    end
end
rb.OnTriggerExited = function(other)
    if other.owner.tag == "refuelTrigger" then
        inRefuelZone = false
    end
end

-- SAS
local sasEnabled = false
local sasToggleImage = sceneManager:GetActorByTag("sasToggleFG"):GetComponent("RawImage")

-- Plume
local enginePlume = sceneManager:GetActorByTag("enginePlume")

function lerp(a, b, t)
    return a + (b - a) * t
end

-- Cache input state for FixedUpdate
local isThrusting = false
local horizontalInput = 0
local verticalInput = 0
local yawInput = 0

-- Handle input and visuals
RunService.Update(function(dt)
    -- SAS Toggle
    if input.GetKeyDown(Key.T) then
        sasEnabled = not sasEnabled
        sasToggleImage.enabled = sasEnabled
    end

    -- Cache input for physics
    isThrusting = input.GetKey(Key.Space) and fuel > 0
    horizontalInput = input.GetAxis("horizontal")
    verticalInput = input.GetAxis("vertical")
    
    yawInput = 0
    if input.GetKey(Key.Q) then
        yawInput = 1
    elseif input.GetKey(Key.E) then
        yawInput = -1
    end

    -- Visual feedback (safe to do in Update)
    if isThrusting then
        light.strength = lerp(light.strength, 1, dt * 10)
        enginePlume.transform.scale = vec3.lerp(enginePlume.transform.scale, vec3(1, 1, 1), dt * 10)
    else
        light.strength = lerp(light.strength, 0, dt * 10)
        enginePlume.transform.scale = vec3.lerp(enginePlume.transform.scale, vec3(1, 0, 1), dt * 10)
    end
    
    -- Update fuel bar UI
    fuelBar.transform:SetScale(fuelBarStartingScale * (fuel / 100), fuelBar.transform.scale.y, fuelBar.transform.scale.z)
    
    -- Fuel
    if isThrusting then
        fuel = fuel - 0.005
    end
    if inRefuelZone and fuel < 100 then
        fuel = fuel + 0.1
    end
end)

-- Handle physics
RunService.FixedUpdate(function(fixedDt)
    -- Thrust
    if isThrusting then
        rb:AddForce(transform.up * engineStrength)
    end

    -- Calculate torque based on cached input
    local pitchTorque = transform.right * (-verticalInput * torqueStrength)
    local rollTorque = transform.forward * (-horizontalInput * torqueStrength)
    local yawTorque = transform.up * (yawInput * torqueStrength)
    
    -- Apply combined torque
    local totalTorque = pitchTorque + rollTorque + yawTorque
    rb:AddTorque(totalTorque)
    
    -- SAS dampening
    if sasEnabled and vec3.length(totalTorque) < 1 then
        rb:SetAngularVelocity(rb:GetAngularVelocity() * 0.95)
    end
end)