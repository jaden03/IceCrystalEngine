print("Test.lua loaded")

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

wait(1000)
local rb = actor:GetComponent("RigidBody")

local force = 3
while true do
    wait(1)
    
    local addedVel = vec3(0,0,0)
    if input.GetKey(Key.W) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
                addedVel = addedVel + (vec3(0, 0, 1) * force)
            end
        end
    end
    
    if input.GetKey(Key.A) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
            addedVel = addedVel + (vec3(1, 0, 0) * force)
            end
        end
    end
    
    if input.GetKey(Key.D) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
            addedVel = addedVel + (vec3(-1, 0, 0) * force)
            end
        end
    end
    
    if input.GetKey(Key.S) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
            addedVel = addedVel + (vec3(0, 0, -1) * force)
            end
        end
    end
    
    if input.GetKey(Key.E) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
            addedVel = addedVel + (vec3(0, 1, 0) * force)
            end
        end
    end
    
    if input.GetKey(Key.Q) then
        if sceneManager:GetHoveredActor() ~= nil then
            if sceneManager:GetHoveredActor() == actor then
                addedVel = addedVel + (vec3(0, -1, 0) * force)
            end
        end
    end
    
    if (addedVel ~= vec3(0,0,0)) then
        rb:SetLinearVelocity(addedVel)
    end
end