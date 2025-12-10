print("Lander Controller Running...")

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

local rb = actor:GetComponent("RigidBody");
local light = sceneManager:GetActorByTag("engineLight"):GetComponent("PointLight")

-- Attitude control settings
local torqueStrength = 1000
local engineStrength = 1500

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

RunService.Update(function(dt)
    -- SAS Toggle
    if input.GetKeyDown(Key.T) then
        sasEnabled = not sasEnabled
        sasToggleImage.enabled = sasEnabled
    end

    -- Lander Controls
    if input.GetKey(Key.Space) and fuel > 0 then
        rb:AddForce(transform.up * engineStrength)
        light.strength = lerp(light.strength, 1, dt * 10)
        enginePlume.transform.scale = vec3.lerp(enginePlume.transform.scale, vec3(1, 1, 1), dt * 10)
        fuel = fuel - .005;
    else
        light.strength = lerp(light.strength, 0, dt * 10)
        enginePlume.transform.scale = vec3.lerp(enginePlume.transform.scale, vec3(1, 0, 1), dt * 10)
    end
    
    if inRefuelZone and fuel < 100 then
        fuel = fuel + 0.1;
    end
    
    -- Update fuel bar UI
    fuelBar.transform:SetScale(fuelBarStartingScale * (fuel / 100), fuelBar.transform.scale.y, fuelBar.transform.scale.z)
    
    local horizontal = input.GetAxis("horizontal") -- value between -1 and 1 using a and d
    local vertical = input.GetAxis("vertical")     -- value between -1 and 1 using w and s

    -- Calculate torque based on input
    -- Pitch (W/S) - rotates around right axis
    local pitchTorque = transform.right * (-vertical * torqueStrength)
    
    -- Roll (A/D) - rotates around forward axis
    local rollTorque = transform.forward * (-horizontal * torqueStrength)
    
    -- Yaw (Q/E) - rotates around up axis
    local yaw = 0
    if input.GetKey(Key.Q) then
        yaw = 1
    elseif input.GetKey(Key.E) then
        yaw = -1
    end
    local yawTorque = transform.up * (yaw * torqueStrength)
    
    -- Apply combined torque
    local totalTorque = pitchTorque + rollTorque + yawTorque
    rb:AddTorque(totalTorque)
    
     if sasEnabled and vec3.length(totalTorque) < 1 then
        rb:SetAngularVelocity(rb:GetAngularVelocity() * .995)
     end
end)