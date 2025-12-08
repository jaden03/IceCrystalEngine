print("Game Manager Running...")

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

local rb = actor:GetComponent("RigidBody");
local light = sceneManager:GetActorByTag("engineLight"):GetComponent("PointLight")
print(light.owner.name)

-- Attitude control settings
local torqueStrength = 5000  -- Adjust for responsiveness
local maxAngularVelocity = 5  -- Limit rotation speed

while true do
    wait(1)

    -- Lander Controls
    if input.GetKey(Key.Space) then
        rb:AddForce(transform.up * 10000)
        light.enabled = true
    else
        light.enabled = false
    end
    
    local horizontal = input.GetAxis("horizontal") -- value between -1 and 1 using a and d
    local vertical = input.GetAxis("vertical")     -- value between -1 and 1 using w and s

    -- Calculate torque based on input
    -- Pitch (W/S) - rotates around right axis
    local pitchTorque = transform.right * (vertical * torqueStrength)
    
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