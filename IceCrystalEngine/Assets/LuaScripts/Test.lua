print("Test.lua loaded")


local vec = vec3(1, 2, 3)
print(transform.position.x, transform.position.y, transform.position.z)

local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

wait(1000)
print("Getting Sun")

local sun = sceneManager:GetActorByTag("sun")
print(sun.name)

print("Getting light")
local light = sun:GetComponent("DirectionalLight")
print("Got light")

while true do
    wait(1)
    if input.GetKeyDown(Key.E) then
        light.strength = 5
    end
    if input.GetKeyDown(Key.Q) then
        light.strength = 1
    end
end