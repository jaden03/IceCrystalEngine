print("Lander Controller Running...")

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

local rb = actor:GetComponent("RigidBody");
local light = sceneManager:GetActorByTag("engineLight"):GetComponent("PointLight")

-- Attitude control settings
local torqueStrength = 5000  -- Adjust for responsiveness
local maxAngularVelocity = 5 -- Limit rotation speed

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

while true do
    wait(1)

    -- Lander Controls
    if input.GetKey(Key.Space) and fuel > 0 then
        rb:AddForce(transform.up * 10000)
        light.enabled = true

        fuel = fuel - .01;
    else
        light.enabled = false
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
end