print("Test.lua loaded")


local vec = vec3(1, 2, 3)
print(transform.position.x, transform.position.y, transform.position.z)

local sceneManager = SceneManager:GetInstance()

wait(1000)
print("Getting Sun")

local sun = sceneManager:GetActorByTag("sun")
print(sun.name)

print("Getting light")
local light = sun:GetComponent("DirectionalLight")
print("Got light 1")
print("Got the light:" .. tostring(light.color))

light.strength = 5

while true do
    wait(5)
    if sceneManager:GetHoveredActor() ~= nil then
        local actor = sceneManager:GetHoveredActor()
        if actor.name == "Actor" then
            light.strength = light.strength + .1
        else
            light.strength = 1
        end
    end
end

-- while true do
--     wait(1000)
--     transform.position = transform.position + vec

--     if sceneManager:GetHoveredActor() ~= nil then
--         local actor = sceneManager:GetHoveredActor()
--         actor.transform:Translate(vec3(0, 1, 0))
--     end
-- end