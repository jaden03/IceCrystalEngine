print("Test.lua loaded")

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

wait(1000)
local rb = actor:GetComponent("RigidBody")

while true do
    wait(1)
    --if input.GetKeyDown(Key.E) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
                rb:SetLinearVelocity(vec3(0, 1, 0)) 
            end
        end
    --end
end