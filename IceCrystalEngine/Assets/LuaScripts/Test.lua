print("Test.lua loaded")


local vec = vec3(1, 2, 3)
print(transform.position.x, transform.position.y, transform.position.z)

local sceneManager = SceneManager:GetInstance()

while true do
    wait(1000)
    transform.position = transform.position + vec

    if sceneManager:GetHoveredActor() ~= nil then
        local actor = sceneManager:GetHoveredActor()
        actor.transform:Translate(vec3(0, 1, 0))
    end
end