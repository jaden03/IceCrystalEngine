local sceneManager = SceneManager:GetInstance()
local input = Input:GetInstance()

local camera = actor
local lander = sceneManager:GetActorByTag("Lander")

-- Camera settings
local cameraDistance = 10
local mouseSensitivity = 0.15
local rotationSmoothSpeed = 0.01

-- Rotation state
local targetYaw = 0
local targetPitch = 20
local currentYaw = 0
local currentPitch = 20

-- Position state
local currentPosition = camera.transform.position

-- Pitch limits
local minPitch = -89
local maxPitch = 89

local lastMousePos = input.GetMousePosition()

while true do
    wait(0)
    
    -- Get mouse input
    local currentMousePos = input.GetMousePosition()
    local mouseDeltaX = currentMousePos.x - lastMousePos.x
    local mouseDeltaY = currentMousePos.y - lastMousePos.y
    lastMousePos = currentMousePos
    
    -- Update target rotation (fixed mouseY direction)
    targetYaw = targetYaw - mouseDeltaX * mouseSensitivity
    targetPitch = targetPitch + mouseDeltaY * mouseSensitivity  -- Changed to +
    
    -- Clamp pitch
    if targetPitch > maxPitch then
        targetPitch = maxPitch
    elseif targetPitch < minPitch then
        targetPitch = minPitch
    end
    
    -- Smooth rotation interpolation
    currentYaw = currentYaw + (targetYaw - currentYaw) * rotationSmoothSpeed
    currentPitch = currentPitch + (targetPitch - currentPitch) * rotationSmoothSpeed
    
    -- Calculate target camera position offset from lander
    local yawRad = math.rad(currentYaw)
    local pitchRad = math.rad(currentPitch)
    
    local offset = vec3(
        math.cos(pitchRad) * math.sin(yawRad) * cameraDistance,
        math.sin(pitchRad) * cameraDistance,
        math.cos(pitchRad) * math.cos(yawRad) * cameraDistance
    )
    
    -- Calculate target position
    local landerPos = lander.transform.position
    local targetPosition = landerPos + offset

    camera.transform.position = targetPosition
    
    -- Make camera look at lander
    local direction = (landerPos - camera.transform.position):normalized()
    camera.transform.rotation = quat.LookRotation(direction)
end