local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

local camera = actor
local lander = sceneManager:GetActorByTag("Lander")

-- Camera settings
local cameraDistance = 10
local mouseSensitivity = 0.15
local rotationSmoothSpeed = 0.01

-- Pitch limits
local minPitch = -89
local maxPitch = 89

-- Distance limits
local minDistance = 10
local maxDistance = 50

-- Moon center (assuming centered at origin)
local moonCenter = vec3(0, 0, 0)

-- Camera rotation state (independent of lander)
local targetYaw = 0
local targetPitch = 20
local currentYaw = 0
local currentPitch = 20

local lastMousePos = input.GetMousePosition()

while true do
    wait(0)
    
    -- Get mouse input
    local currentMousePos = input.GetMousePosition()
    local mouseDeltaX = currentMousePos.x - lastMousePos.x
    local mouseDeltaY = currentMousePos.y - lastMousePos.y
    lastMousePos = currentMousePos
    
    -- Update target rotation
    targetYaw = targetYaw + mouseDeltaX * mouseSensitivity
    targetPitch = targetPitch - mouseDeltaY * mouseSensitivity
    
    -- Clamp pitch
    if targetPitch > maxPitch then
        targetPitch = maxPitch
    elseif targetPitch < minPitch then
        targetPitch = minPitch
    end
    
    -- Update distance
    if input.scrolledUp() then
        cameraDistance = cameraDistance - .5
    elseif input.scrolledDown() then
        cameraDistance = cameraDistance + .5
    end
    
    -- Clamp camera distance
    if cameraDistance < minDistance then
        cameraDistance = minDistance
    elseif cameraDistance > maxDistance then
        cameraDistance = maxDistance
    end
    
    -- Smooth rotation interpolation
    currentYaw = currentYaw + (targetYaw - currentYaw) * rotationSmoothSpeed
    currentPitch = currentPitch + (targetPitch - currentPitch) * rotationSmoothSpeed
    
    -- Get lander position and calculate local "up" based on position, NOT lander rotation
    local landerPos = lander.transform.position
    local localUp = (landerPos - moonCenter):normalized()
    
    -- Build a stable horizontal plane perpendicular to localUp
    local worldNorth = vec3(0, 0, 1)
    if math.abs(localUp:dot(worldNorth)) > 0.9 then
        worldNorth = vec3(1, 0, 0)
    end
    
    local localEast = worldNorth:cross(localUp):normalized()
    local localNorth = localUp:cross(localEast):normalized()
    
    -- Apply camera rotations in this stable local space
    local yawRad = math.rad(currentYaw)
    local pitchRad = math.rad(currentPitch)
    
    -- Yaw rotates around localUp, pitch rotates the result up/down
    local horizontalDir = math.cos(yawRad) * localNorth + math.sin(yawRad) * localEast
    local finalDirection = math.cos(pitchRad) * horizontalDir + math.sin(pitchRad) * localUp
    finalDirection = finalDirection:normalized()
    
    -- Position camera
    local targetPosition = landerPos - finalDirection * cameraDistance
    camera.transform.position = targetPosition
    
    -- Make camera look at lander with stable up vector
    local direction = (landerPos - camera.transform.position):normalized()
    camera.transform.rotation = quat.LookRotation(direction, localUp)
end