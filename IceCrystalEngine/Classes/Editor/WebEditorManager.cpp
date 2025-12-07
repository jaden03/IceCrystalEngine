#include <Ice/Editor/WebEditorManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h>
#include <Ice/Core/Component.h>
#include <Ice/Components/Physics/RigidBody.h>
#include <Ice/Components/Physics/BoxCollider.h>
#include <Ice/Components/Physics/SphereCollider.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Camera.h>
#include <Ice/Components/Light.h>
#include <Ice/Components/Freecam.h>
#include <Ice/Components/LuaExecutor.h>
#include <Ice/Core/PhysicsManager.h>
#include <Ice/Utils/FileUtil.h>
#include <JSON/json.h>

// Disable SSL/TLS and compression support - we only need plain HTTP for local development
#define CPPHTTPLIB_OPENSSL_SUPPORT
#undef CPPHTTPLIB_OPENSSL_SUPPORT

#define CPPHTTPLIB_ZLIB_SUPPORT
#undef CPPHTTPLIB_ZLIB_SUPPORT

#define CPPHTTPLIB_BROTLI_SUPPORT
#undef CPPHTTPLIB_BROTLI_SUPPORT

#include <httplib/httplib.h>

#include <iostream>
#include <sstream>
#include <fstream>

using json = nlohmann::json;

// Forward declarations for default content
std::string GetDefaultEditorHTML();
std::string GetDefaultEditorCSS();
std::string GetDefaultEditorJS();

WebEditorManager::WebEditorManager()
    : server(nullptr)
    , serverThread(nullptr)
    , running(false)
    , port(8080)
    , editorEnabled(false)
    , enginePaused(false)
    , selectedActor(nullptr)
{
    editorPath = FileUtil::AssetDir + "Editor/";
}

WebEditorManager::~WebEditorManager()
{
    Stop();
}

void WebEditorManager::Start(int p)
{
    if (running)
    {
        std::cout << "[WebEditor] Already running on port " << port << std::endl;
        return;
    }

    port = p;
    server = new httplib::Server();
    
    SetupRoutes();
    
    running = true;
    editorEnabled = true;

    // Start server in separate thread
    serverThread = new std::thread(&WebEditorManager::ServerThread, this);

    std::cout << "[WebEditor] Starting web editor on http://localhost:" << port << std::endl;
    std::cout << "[WebEditor] Open http://localhost:" << port << "/editor in your browser" << std::endl;
}

void WebEditorManager::Stop()
{
    if (!running)
        return;

    running = false;
    editorEnabled = false;

    if (server)
    {
        server->stop();
    }

    if (serverThread && serverThread->joinable())
    {
        serverThread->join();
        delete serverThread;
        serverThread = nullptr;
    }

    if (server)
    {
        delete server;
        server = nullptr;
    }

    std::cout << "[WebEditor] Web editor stopped" << std::endl;
}

void WebEditorManager::ServerThread()
{
    if (!server->listen("0.0.0.0", port))
    {
        std::cerr << "[WebEditor] Failed to start server on port " << port << std::endl;
        running = false;
    }
}

void WebEditorManager::SetupRoutes()
{
    // CORS middleware
    server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        
        if (req.method == "OPTIONS")
        {
            res.status = 200;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Serve static files (editor HTML/JS/CSS)
    server->Get("/editor", [this](const httplib::Request&, httplib::Response& res) {
        std::ifstream file(editorPath + "index.html");
        if (file.good())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        }
        else
        {
            res.set_content(GetDefaultEditorHTML(), "text/html");
        }
        res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
        res.set_header("Pragma", "no-cache");
        res.set_header("Expires", "0");
    });

    server->Get("/editor/style.css", [this](const httplib::Request&, httplib::Response& res) {
        std::ifstream file(editorPath + "style.css");
        if (file.good())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/css");
        }
        else
        {
            res.set_content(GetDefaultEditorCSS(), "text/css");
        }
        res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
        res.set_header("Pragma", "no-cache");
        res.set_header("Expires", "0");
    });

    server->Get("/editor/app.js", [this](const httplib::Request&, httplib::Response& res) {
        std::ifstream file(editorPath + "app.js");
        if (file.good())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "application/javascript");
        }
        else
        {
            res.set_content(GetDefaultEditorJS(), "application/javascript");
        }
        res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
        res.set_header("Pragma", "no-cache");
        res.set_header("Expires", "0");
    });

    // API Routes
    server->Get("/api/scene", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandleGetScene(), "application/json");
    });

    server->Get("/api/actors", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandleGetActors(), "application/json");
    });

    server->Get(R"(/api/actors/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        int actorId = std::stoi(req.matches[1]);
        res.set_content(HandleGetActor(actorId), "application/json");
    });

    server->Post(R"(/api/actors/(\d+)/transform)", [this](const httplib::Request& req, httplib::Response& res) {
        int actorId = std::stoi(req.matches[1]);
        res.set_content(HandleUpdateActorTransform(actorId, req.body), "application/json");
    });

    server->Post(R"(/api/actors/(\d+)/property)", [this](const httplib::Request& req, httplib::Response& res) {
        int actorId = std::stoi(req.matches[1]);
        res.set_content(HandleUpdateActorProperty(actorId, req.body), "application/json");
    });

    server->Get(R"(/api/actors/(\d+)/components)", [this](const httplib::Request& req, httplib::Response& res) {
        int actorId = std::stoi(req.matches[1]);
        res.set_content(HandleGetComponents(actorId), "application/json");
    });

    server->Post(R"(/api/components/(\d+)/property)", [this](const httplib::Request& req, httplib::Response& res) {
        int componentId = std::stoi(req.matches[1]);
        res.set_content(HandleUpdateComponentProperty(componentId, req.body), "application/json");
    });

    server->Post(R"(/api/components/(\d+)/enabled)", [this](const httplib::Request& req, httplib::Response& res) {
        int componentId = std::stoi(req.matches[1]);
        res.set_content(HandleToggleComponentEnabled(componentId, req.body), "application/json");
    });

    server->Get("/api/stats", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandleGetSceneStats(), "application/json");
    });

    server->Post("/api/engine/pause", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandlePauseEngine(), "application/json");
    });

    server->Post("/api/engine/resume", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(HandleResumeEngine(), "application/json");
    });

    server->Get("/api/engine/status", [this](const httplib::Request&, httplib::Response& res) {
        json status = {
            {"paused", enginePaused.load()},
        };
        res.set_content(status.dump(), "application/json");
    });

    server->Get( R"(/api/editor/select/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        int actorId = std::stoi(req.matches[1]);
        try {
            std::cout << "Running Editor Select " << std::endl;
            SetSelectedActor(actorId);
            res.set_content(json({{"success", true}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // Health check
    server->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"message", "IceCrystalEngine Web Editor is running"}
        };
        res.set_content(response.dump(), "application/json");
    });
}

void WebEditorManager::Update()
{
    if (!running)
        return;

    // Process any queued messages for broadcasting
    std::lock_guard<std::mutex> lock(messageMutex);
    if (!messageQueue.empty())
    {
        // In a full implementation, this would send via WebSocket
        // For now, messages are queued and can be polled
        messageQueue.clear();
    }
}

std::string WebEditorManager::GetURL() const
{
    return "http://localhost:" + std::to_string(port) + "/editor";
}

// ============================================================================
// API Handlers
// ============================================================================

std::string WebEditorManager::HandleGetScene()
{
    return SerializeScene();
}

std::string WebEditorManager::HandleGetActors()
{
    return SerializeActorList();
}

std::string WebEditorManager::HandleGetActor(int actorId)
{
    auto& sceneManager = SceneManager::GetInstance();
    
    for (Actor* actor : *sceneManager.GetActors())
    {
        if (reinterpret_cast<intptr_t>(actor) == actorId)
        {
            return SerializeActor(actor);
        }
    }

    json error = {
        {"error", "Actor not found"},
        {"actorId", actorId}
    };
    return error.dump();
}

std::string WebEditorManager::HandleUpdateActorTransform(int actorId, const std::string& body)
{
    auto& sceneManager = SceneManager::GetInstance();
    
    try
    {
        json data = json::parse(body);
        
        for (Actor* actor : *sceneManager.GetActors())
        {
            if (reinterpret_cast<intptr_t>(actor) == actorId)
            {
                if (data.contains("position"))
                {
                    glm::vec3 pos(
                        data["position"]["x"].get<float>(),
                        data["position"]["y"].get<float>(),
                        data["position"]["z"].get<float>()
                    );
                    actor->transform->SetPosition(pos);
                }

                if (data.contains("rotation"))
                {
                    glm::vec3 rot(
                        data["rotation"]["x"].get<float>(),
                        data["rotation"]["y"].get<float>(),
                        data["rotation"]["z"].get<float>()
                    );
                    // SetRotation properly handles eulerAngles and quaternion conversion
                    actor->transform->SetRotation(rot);
                }

                if (data.contains("scale"))
                {
                    glm::vec3 scale(
                        data["scale"]["x"].get<float>(),
                        data["scale"]["y"].get<float>(),
                        data["scale"]["z"].get<float>()
                    );
                    actor->transform->SetScale(scale);
                }

                BroadcastTransformChanged(actor);

                json response = {
                    {"success", true},
                    {"message", "Transform updated"},
                    {"actor", json::parse(SerializeActor(actor))}
                };
                return response.dump();
            }
        }
    }
    catch (const json::exception& e)
    {
        json error = {
            {"error", "Invalid JSON"},
            {"message", e.what()}
        };
        return error.dump();
    }

    json error = {
        {"error", "Actor not found"},
        {"actorId", actorId}
    };
    return error.dump();
}

std::string WebEditorManager::HandleUpdateActorProperty(int actorId, const std::string& body)
{
    auto& sceneManager = SceneManager::GetInstance();
    
    try
    {
        json data = json::parse(body);
        
        for (Actor* actor : *sceneManager.GetActors())
        {
            if (reinterpret_cast<intptr_t>(actor) == actorId)
            {
                if (data.contains("name"))
                {
                    actor->name = data["name"].get<std::string>();
                }

                if (data.contains("tag"))
                {
                    actor->tag = data["tag"].get<std::string>();
                }

                BroadcastActorUpdated(actor);

                json response = {
                    {"success", true},
                    {"message", "Actor property updated"}
                };
                return response.dump();
            }
        }
    }
    catch (const json::exception& e)
    {
        json error = {
            {"error", "Invalid JSON"},
            {"message", e.what()}
        };
        return error.dump();
    }

    json error = {
        {"error", "Actor not found"},
        {"actorId", actorId}
    };
    return error.dump();
}

std::string WebEditorManager::HandleGetComponents(int actorId)
{
    auto& sceneManager = SceneManager::GetInstance();
    
    for (Actor* actor : *sceneManager.GetActors())
    {
        if (reinterpret_cast<intptr_t>(actor) == actorId)
        {
            json components = json::array();
            
            for (Component* comp : *actor->components)
            {
                components.push_back(json::parse(SerializeComponent(comp)));
            }

            json response = {
                {"actorId", actorId},
                {"components", components}
            };
            return response.dump();
        }
    }

    json error = {
        {"error", "Actor not found"},
        {"actorId", actorId}
    };
    return error.dump();
}

std::string WebEditorManager::HandleUpdateComponentProperty(int componentId, const std::string& body)
{
    try
    {
        json data = json::parse(body);
        std::string property = data["property"];
        
        // Find the component by ID across all actors
        auto& sceneManager = SceneManager::GetInstance();
        for (Actor* actor : *sceneManager.GetActors())
        {
            for (Component* comp : *actor->components)
            {
                if (reinterpret_cast<intptr_t>(comp) == componentId)
                {
                    // Update based on component type
                    if (RigidBody* rb = dynamic_cast<RigidBody*>(comp))
                    {
                        if (property == "mass")
                            rb->mass = data["value"];
                        else if (property == "isKinematic")
                            rb->SetKinematic(data["value"]);
                        
                        return json({{"success", true}, {"message", "RigidBody property updated"}}).dump();
                    }
                    else if (BoxCollider* bc = dynamic_cast<BoxCollider*>(comp))
                    {
                        if (property == "size.x")
                            bc->size.x = data["value"];
                        else if (property == "size.y")
                            bc->size.y = data["value"];
                        else if (property == "size.z")
                            bc->size.z = data["value"];
                        
                        return json({{"success", true}, {"message", "BoxCollider property updated"}}).dump();
                    }
                    else if (SphereCollider* sc = dynamic_cast<SphereCollider*>(comp))
                    {
                        if (property == "radius")
                            sc->radius = data["value"];
                        
                        return json({{"success", true}, {"message", "SphereCollider property updated"}}).dump();
                    }
                    else if (Camera* cam = dynamic_cast<Camera*>(comp))
                    {
                        if (property == "fieldOfView")
                            cam->fieldOfView = data["value"];
                        else if (property == "nearClippingPlane")
                            cam->nearClippingPlane = data["value"];
                        else if (property == "farClippingPlane")
                            cam->farClippingPlane = data["value"];
                        
                        return json({{"success", true}, {"message", "Camera property updated"}}).dump();
                    }
                    else if (DirectionalLight* dl = dynamic_cast<DirectionalLight*>(comp))
                    {
                        if (property == "color.r")
                            dl->color.r = data["value"];
                        else if (property == "color.g")
                            dl->color.g = data["value"];
                        else if (property == "color.b")
                            dl->color.b = data["value"];
                        else if (property == "strength")
                            dl->strength = data["value"];
                        else if (property == "castShadows")
                            dl->castShadows = data["value"];
                        else if (property == "shadowMapResolution")
                            dl->shadowMapResolution = data["value"];
                        
                        return json({{"success", true}, {"message", "DirectionalLight property updated"}}).dump();
                    }
                    else if (PointLight* pl = dynamic_cast<PointLight*>(comp))
                    {
                        if (property == "color.r")
                            pl->color.r = data["value"];
                        else if (property == "color.g")
                            pl->color.g = data["value"];
                        else if (property == "color.b")
                            pl->color.b = data["value"];
                        else if (property == "strength")
                            pl->strength = data["value"];
                        else if (property == "radius")
                            pl->radius = data["value"];
                        
                        return json({{"success", true}, {"message", "PointLight property updated"}}).dump();
                    }
                    else if (SpotLight* sl = dynamic_cast<SpotLight*>(comp))
                    {
                        if (property == "color.r")
                            sl->color.r = data["value"];
                        else if (property == "color.g")
                            sl->color.g = data["value"];
                        else if (property == "color.b")
                            sl->color.b = data["value"];
                        else if (property == "strength")
                            sl->strength = data["value"];
                        else if (property == "distance")
                            sl->distance = data["value"];
                        else if (property == "angle")
                            sl->angle = data["value"];
                        else if (property == "castShadows")
                            sl->castShadows = data["value"];
                        else if (property == "shadowMapResolution")
                            sl->shadowMapResolution = data["value"];
                        
                        return json({{"success", true}, {"message", "SpotLight property updated"}}).dump();
                    }
                    else if (Freecam* fc = dynamic_cast<Freecam*>(comp))
                    {
                        if (property == "speed")
                            fc->speed = data["value"];
                        else if (property == "sensitivity")
                            fc->sensitivity = data["value"];
                        else if (property == "requireRightClick")
                            fc->requireRightClick = data["value"];
                        
                        return json({{"success", true}, {"message", "Freecam property updated"}}).dump();
                    }
                    
                    return json({{"error", "Property not found or not editable"}, {"property", property}}).dump();
                }
            }
        }
        
        return json({{"error", "Component not found"}, {"componentId", componentId}}).dump();
    }
    catch (const std::exception& e)
    {
        return json({{"error", e.what()}}).dump();
    }
}

std::string WebEditorManager::HandleToggleComponentEnabled(int componentId, const std::string& body)
{
    try
    {
        json data = json::parse(body);
        bool enabled = data["enabled"];
        
        // Find the component by ID across all actors
        auto& sceneManager = SceneManager::GetInstance();
        for (Actor* actor : *sceneManager.GetActors())
        {
            for (Component* comp : *actor->components)
            {
                if (reinterpret_cast<intptr_t>(comp) == componentId)
                {
                    comp->SetEnabled(enabled);
                    return json({
                        {"success", true}, 
                        {"message", enabled ? "Component enabled" : "Component disabled"},
                        {"enabled", enabled}
                    }).dump();
                }
            }
        }
        
        return json({{"error", "Component not found"}, {"componentId", componentId}}).dump();
    }
    catch (const std::exception& e)
    {
        return json({{"error", e.what()}}).dump();
    }
}

std::string WebEditorManager::HandleAddComponent(int actorId, const std::string& body)
{
    json response = {
        {"success", false},
        {"message", "Component adding not yet implemented"}
    };
    return response.dump();
}

std::string WebEditorManager::HandleRemoveComponent(int actorId, int componentId)
{
    json response = {
        {"success", false},
        {"message", "Component removal not yet implemented"}
    };
    return response.dump();
}

std::string WebEditorManager::HandleGetSceneStats()
{
    auto& sceneManager = SceneManager::GetInstance();
    
    int actorCount = sceneManager.GetActors()->size();
    int componentCount = 0;
    
    for (Actor* actor : *sceneManager.GetActors())
    {
        componentCount += actor->components->size();
    }

    json stats = {
        {"actorCount", actorCount},
        {"componentCount", componentCount},
        {"running", running.load()}
    };
    
    return stats.dump();
}

std::string WebEditorManager::HandlePauseEngine()
{
    PauseEngine();
    
    json response = {
        {"success", true},
        {"message", "Engine paused"},
        {"paused", true}
    };
    
    return response.dump();
}

std::string WebEditorManager::HandleResumeEngine()
{
    ResumeEngine();
    
    json response = {
        {"success", true},
        {"message", "Engine resumed"},
        {"paused", false}
    };
    
    return response.dump();
}

// ============================================================================
// Serialization
// ============================================================================

std::string WebEditorManager::SerializeActor(Actor* actor)
{
    json j = {
        {"id", reinterpret_cast<intptr_t>(actor)},
        {"name", actor->name},
        {"tag", actor->tag},
        {"transform", json::parse(SerializeTransform(actor))},
        {"componentCount", actor->components->size()}
    };
    
    return j.dump();
}

std::string WebEditorManager::SerializeActorList()
{
    auto& sceneManager = SceneManager::GetInstance();
    json actorArray = json::array();
    
    for (Actor* actor : *sceneManager.GetActors())
    {
        actorArray.push_back(json::parse(SerializeActor(actor)));
    }
    
    json response = {
        {"actors", actorArray},
        {"count", actorArray.size()}
    };
    
    return response.dump();
}

std::string WebEditorManager::SerializeTransform(Actor* actor)
{
    Transform* t = actor->transform;
    
    // Convert quaternion to euler angles for easier editing
    glm::vec3 euler = glm::eulerAngles(t->rotation);
    
    json j = {
        {"position", {
            {"x", t->position.x},
            {"y", t->position.y},
            {"z", t->position.z}
        }},
        {"rotation", {
            {"x", glm::degrees(euler.x)},
            {"y", glm::degrees(euler.y)},
            {"z", glm::degrees(euler.z)}
        }},
        {"scale", {
            {"x", t->scale.x},
            {"y", t->scale.y},
            {"z", t->scale.z}
        }}
    };
    
    return j.dump();
}

std::string WebEditorManager::SerializeComponent(Component* component)
{
    json j = {
        {"id", reinterpret_cast<intptr_t>(component)},
        {"type", "Component"},
        {"enabled", component->enabled},
        {"properties", json::object()}
    };
    
    // Try to identify component type and serialize its properties
    if (RigidBody* rb = dynamic_cast<RigidBody*>(component))
    {
        j["type"] = "RigidBody";
        j["properties"] = {
            {"mass", rb->mass},
            {"isKinematic", rb->IsKinematic()},
            {"isActive", rb->IsActive()},
            {"linearVelocity", {
                {"x", rb->GetLinearVelocity().x},
                {"y", rb->GetLinearVelocity().y},
                {"z", rb->GetLinearVelocity().z}
            }},
            {"angularVelocity", {
                {"x", rb->GetAngularVelocity().x},
                {"y", rb->GetAngularVelocity().y},
                {"z", rb->GetAngularVelocity().z}
            }}
        };
    }
    else if (BoxCollider* bc = dynamic_cast<BoxCollider*>(component))
    {
        j["type"] = "BoxCollider";
        j["properties"] = {
            {"size", {
                {"x", bc->size.x},
                {"y", bc->size.y},
                {"z", bc->size.z}
            }}
        };
    }
    else if (SphereCollider* sc = dynamic_cast<SphereCollider*>(component))
    {
        j["type"] = "SphereCollider";
        j["properties"] = {
            {"radius", sc->radius}
        };
    }
    else if (Camera* cam = dynamic_cast<Camera*>(component))
    {
        j["type"] = "Camera";
        j["properties"] = {
            {"fieldOfView", cam->fieldOfView},
            {"nearClippingPlane", cam->nearClippingPlane},
            {"farClippingPlane", cam->farClippingPlane}
        };
    }
    else if (DirectionalLight* dl = dynamic_cast<DirectionalLight*>(component))
    {
        j["type"] = "DirectionalLight";
        j["properties"] = {
            {"color", {
                {"r", dl->color.r},
                {"g", dl->color.g},
                {"b", dl->color.b}
            }},
            {"strength", dl->strength},
            {"castShadows", dl->castShadows},
            {"shadowMapResolution", dl->shadowMapResolution},
            {"cascadeCount", dl->cascadeCount}
        };
    }
    else if (PointLight* pl = dynamic_cast<PointLight*>(component))
    {
        j["type"] = "PointLight";
        j["properties"] = {
            {"color", {
                {"r", pl->color.r},
                {"g", pl->color.g},
                {"b", pl->color.b}
            }},
            {"strength", pl->strength},
            {"radius", pl->radius}
        };
    }
    else if (SpotLight* sl = dynamic_cast<SpotLight*>(component))
    {
        j["type"] = "SpotLight";
        j["properties"] = {
            {"color", {
                {"r", sl->color.r},
                {"g", sl->color.g},
                {"b", sl->color.b}
            }},
            {"strength", sl->strength},
            {"distance", sl->distance},
            {"angle", sl->angle},
            {"castShadows", sl->castShadows},
            {"shadowMapResolution", sl->shadowMapResolution}
        };
    }
    else if (Freecam* fc = dynamic_cast<Freecam*>(component))
    {
        j["type"] = "Freecam";
        j["properties"] = {
            {"speed", fc->speed},
            {"sensitivity", fc->sensitivity},
            {"requireRightClick", fc->requireRightClick}
        };
    }
    else if (LuaExecutor* le = dynamic_cast<LuaExecutor*>(component))
    {
        j["type"] = "LuaExecutor";
        j["properties"] = {
            {"scriptPath", le->filePath}
        };
    }
    else if (Renderer* r = dynamic_cast<Renderer*>(component))
    {
        j["type"] = "Renderer";
        j["properties"] = {
            {"meshPath", "Mesh data"},
            {"materialPath", "Material data"}
        };
    }
    
    return j.dump();
}

std::string WebEditorManager::SerializeScene()
{
    auto& sceneManager = SceneManager::GetInstance();
    json actorArray = json::array();
    
    for (Actor* actor : *sceneManager.GetActors())
    {
        json actorData = json::parse(SerializeActor(actor));
        
        // Add components info
        json components = json::array();
        for (Component* comp : *actor->components)
        {
            components.push_back(json::parse(SerializeComponent(comp)));
        }
        actorData["components"] = components;
        
        actorArray.push_back(actorData);
    }
    
    json scene = {
        {"name", "Main Scene"},
        {"actors", actorArray}
    };
    
    return scene.dump();
}

// ============================================================================
// Broadcasting
// ============================================================================

void WebEditorManager::BroadcastActorCreated(Actor* actor)
{
    if (!editorEnabled)
        return;

    json message = {
        {"type", "actorCreated"},
        {"data", json::parse(SerializeActor(actor))}
    };
    
    BroadcastMessage(message.dump());
}

void WebEditorManager::BroadcastActorDeleted(int actorId)
{
    if (!editorEnabled)
        return;

    json message = {
        {"type", "actorDeleted"},
        {"data", {{"actorId", actorId}}}
    };
    
    BroadcastMessage(message.dump());
}

void WebEditorManager::BroadcastActorUpdated(Actor* actor)
{
    if (!editorEnabled)
        return;

    json message = {
        {"type", "actorUpdated"},
        {"data", json::parse(SerializeActor(actor))}
    };
    
    BroadcastMessage(message.dump());
}

void WebEditorManager::BroadcastTransformChanged(Actor* actor)
{
    if (!editorEnabled)
        return;

    json message = {
        {"type", "transformChanged"},
        {"data", {
            {"actorId", reinterpret_cast<intptr_t>(actor)},
            {"transform", json::parse(SerializeTransform(actor))}
        }}
    };
    
    BroadcastMessage(message.dump());
}

void WebEditorManager::BroadcastComponentAdded(Actor* actor, Component* component)
{
    if (!editorEnabled)
        return;

    json message = {
        {"type", "componentAdded"},
        {"data", {
            {"actorId", reinterpret_cast<intptr_t>(actor)},
            {"component", json::parse(SerializeComponent(component))}
        }}
    };
    
    BroadcastMessage(message.dump());
}

void WebEditorManager::BroadcastComponentRemoved(Actor* actor, int componentId)
{
    if (!editorEnabled)
        return;

    json message = {
        {"type", "componentRemoved"},
        {"data", {
            {"actorId", reinterpret_cast<intptr_t>(actor)},
            {"componentId", componentId}
        }}
    };
    
    BroadcastMessage(message.dump());
}

void WebEditorManager::BroadcastMessage(const std::string& message)
{
    std::lock_guard<std::mutex> lock(messageMutex);
    messageQueue.push_back(message);
}

// ============================================================================
// Engine Control
// ============================================================================

void WebEditorManager::PauseEngine()
{
    enginePaused = true;
    std::cout << "[WebEditor] Engine paused - Physics and Lua updates stopped" << std::endl;
}

void WebEditorManager::ResumeEngine()
{
    enginePaused = false;
    
    // Sync all transforms back to physics bodies before resuming
    // This prevents physics from overwriting editor changes on the next Update()
    SceneManager& sceneManager = SceneManager::GetInstance();
    auto actors = sceneManager.GetActors();
    
    for (Actor* actor : *actors)
    {
        if (!actor) continue;
        
        // Find RigidBody component and sync transform to physics
        auto components = actor->components;
        for (Component* comp : *components)
        {
            RigidBody* rb = dynamic_cast<RigidBody*>(comp);
            JPH::Body* body = rb ? rb->GetBody() : nullptr;
            if (body)
            {
                // Get current transform position/rotation
                glm::vec3 pos = actor->transform->position;
                glm::quat rot = actor->transform->rotation;
                
                // Convert to Jolt format
                JPH::Vec3 joltPos(pos.x, pos.y, pos.z);
                JPH::Quat joltRot(rot.x, rot.y, rot.z, rot.w);
                
                // Use BodyInterface for all operations to ensure thread safety
                JPH::BodyInterface& bodyInterface = PhysicsManager::GetInstance().GetSystem().GetBodyInterface();
                JPH::BodyID bodyID = body->GetID();
                
                // Update position and rotation (don't activate yet)
                bodyInterface.SetPositionAndRotation(bodyID, joltPos, joltRot, JPH::EActivation::DontActivate);
                
                // Reset velocities to prevent sudden movements (using BodyInterface methods)
                bodyInterface.SetLinearAndAngularVelocity(bodyID, JPH::Vec3::sZero(), JPH::Vec3::sZero());
                
                // Now activate if it's a dynamic body
                if (!body->IsStatic())
                {
                    bodyInterface.ActivateBody(bodyID);
                }
            }
        }
    }
    
    std::cout << "[WebEditor] Engine resumed - transforms synced to physics" << std::endl;
}

void WebEditorManager::SetEnginePaused(bool paused)
{
    enginePaused = paused;
}

void WebEditorManager::SetSelectedActor(int actorId)
{
    auto& sceneManager = SceneManager::GetInstance();
    selectedActor = nullptr;
    
    for (Actor* actor : *sceneManager.GetActors())
    {
        if (reinterpret_cast<intptr_t>(actor) == actorId)
        {
            selectedActor = actor;
            std::cout << "[WebEditor] Selected actor: " << actor->name << " (ID: " << actorId << ")" << std::endl;
            break;
        }
    }
    
    if (!selectedActor)
    {
        std::cout << "[WebEditor] No actor found with ID: " << actorId << std::endl;
    }
}

Actor* WebEditorManager::GetSelectedActor() const
{
    return selectedActor;
}

void WebEditorManager::HandleWebSocketMessage(const std::string& message)
{
    // Handle incoming WebSocket messages from clients
    try
    {
        json data = json::parse(message);
        
        // Handle pause/resume commands
        if (data.contains("command"))
        {
            std::string cmd = data["command"].get<std::string>();
            if (cmd == "pause")
            {
                PauseEngine();
            }
            else if (cmd == "resume")
            {
                ResumeEngine();
            }
        }
    }
    catch (const json::exception& e)
    {
        std::cerr << "[WebEditor] WebSocket message parse error: " << e.what() << std::endl;
    }
}

// ============================================================================
// Default embedded HTML/CSS/JS (if files don't exist)
// ============================================================================

std::string GetDefaultEditorHTML()
{
    return R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IceCrystal Engine - Web Editor</title>
    <link rel="stylesheet" href="/editor/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>🧊 IceCrystal Engine</h1>
            <div class="header-controls">
                <button id="pause-btn" class="btn btn-pause">⏸ Pause</button>
                <button id="resume-btn" class="btn btn-resume" style="display:none;">▶ Resume</button>
            </div>
            <div class="status">
                <span id="status-indicator" class="status-dot"></span>
                <span id="status-text">Connecting...</span>
            </div>
        </header>

        <div class="content">
            <aside class="sidebar">
                <h2>Scene Hierarchy</h2>
                <div class="stats">
                    <div>Actors: <span id="actor-count">0</span></div>
                    <div>Components: <span id="component-count">0</span></div>
                </div>
                <button id="refresh-btn" class="btn">🔄 Refresh</button>
                <button id="test-select-btn" class="btn" style="background:#dc2626;">🧪 Test Select</button>
                <ul id="actor-list" class="actor-list"></ul>
            </aside>

            <main class="inspector">
                <h2>Inspector</h2>
                <div id="inspector-content" class="inspector-empty">
                    Select an actor to inspect
                </div>
            </main>
        </div>
    </div>
    <script src="/editor/app.js"></script>
</body>
</html>
)HTML";
}

std::string GetDefaultEditorCSS()
{
    return R"CSS(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: #1e1e1e;
    color: #d4d4d4;
    height: 100vh;
    overflow: hidden;
}

.container {
    display: flex;
    flex-direction: column;
    height: 100vh;
}

header {
    background: #252526;
    padding: 15px 20px;
    border-bottom: 1px solid #3c3c3c;
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 20px;
}

.header-controls {
    display: flex;
    gap: 10px;
}

h1 {
    font-size: 20px;
    color: #4fc3f7;
}

.status {
    display: flex;
    align-items: center;
    gap: 8px;
}

.status-dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: #f44336;
    animation: pulse 2s infinite;
}

.status-dot.connected {
    background: #4caf50;
}

@keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
}

.content {
    display: flex;
    flex: 1;
    overflow: hidden;
}

.sidebar {
    width: 300px;
    background: #252526;
    border-right: 1px solid #3c3c3c;
    padding: 20px;
    overflow-y: auto;
}

.sidebar h2 {
    font-size: 16px;
    margin-bottom: 15px;
    color: #4fc3f7;
}

.stats {
    background: #2d2d30;
    padding: 10px;
    border-radius: 4px;
    margin-bottom: 15px;
    font-size: 14px;
}

.stats div {
    margin: 5px 0;
}

.btn {
    width: 100%;
    padding: 8px;
    background: #0e639c;
    color: white;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-size: 14px;
    margin-bottom: 15px;
}

.btn:hover {
    background: #1177bb;
}

.btn-pause {
    background: #d97706;
}

.btn-pause:hover {
    background: #ea580c;
}

.btn-resume {
    background: #059669;
}

.btn-resume:hover {
    background: #10b981;
}

.actor-list {
    list-style: none;
}

.actor-item {
    padding: 8px 12px;
    margin: 4px 0;
    background: #2d2d30;
    border-radius: 4px;
    cursor: pointer;
    transition: background 0.2s;
    font-size: 14px;
}

.actor-item:hover {
    background: #37373d;
}

.actor-item.selected {
    background: #094771;
}

.inspector {
    flex: 1;
    padding: 20px;
    overflow-y: auto;
}

.inspector h2 {
    font-size: 16px;
    margin-bottom: 20px;
    color: #4fc3f7;
}

.inspector-empty {
    text-align: center;
    color: #808080;
    margin-top: 50px;
}

.property-group {
    background: #2d2d30;
    padding: 15px;
    border-radius: 4px;
    margin-bottom: 15px;
}

.property-group h3 {
    font-size: 14px;
    margin-bottom: 10px;
    color: #569cd6;
}

.property {
    margin: 10px 0;
}

.property label {
    display: block;
    font-size: 12px;
    color: #9cdcfe;
    margin-bottom: 4px;
}

.property input {
    width: 100%;
    padding: 6px;
    background: #3c3c3c;
    border: 1px solid #555;
    color: #d4d4d4;
    border-radius: 4px;
    font-size: 13px;
}

.property input:focus {
    outline: none;
    border-color: #007acc;
}

.vector-group {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
    gap: 8px;
}

.vector-input {
    display: flex;
    flex-direction: column;
}

.vector-input label {
    font-size: 11px;
    color: #858585;
}
)CSS";
}

std::string GetDefaultEditorJS()
{
    return R"JS(
const API_BASE = 'http://localhost:8080/api';
let selectedActorId = null;
let updateInterval = null;

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    checkConnection();
    document.getElementById('refresh-btn').addEventListener('click', loadScene);
    document.getElementById('pause-btn').addEventListener('click', pauseEngine);
    document.getElementById('resume-btn').addEventListener('click', resumeEngine);
    document.getElementById('test-select-btn').addEventListener('click', testSelectEndpoint);
    startAutoRefresh();
    updateEngineStatus();
});

async function checkConnection() {
    try {
        const response = await fetch(`${API_BASE}/health`);
        if (response.ok) {
            updateStatus(true, 'Connected');
            loadScene();
        } else {
            updateStatus(false, 'Connection Error');
        }
    } catch (error) {
        updateStatus(false, 'Disconnected');
    }
}

function updateStatus(connected, text) {
    const indicator = document.getElementById('status-indicator');
    const statusText = document.getElementById('status-text');
    
    indicator.classList.toggle('connected', connected);
    statusText.textContent = text;
}

async function loadScene() {
    try {
        const response = await fetch(`${API_BASE}/actors`);
        const data = await response.json();
        
        renderActorList(data.actors);
        updateStats(data.count);
        
        if (selectedActorId) {
            loadActorDetails(selectedActorId);
        }
    } catch (error) {
        console.error('Failed to load scene:', error);
    }
}

async function updateStats(actorCount) {
    try {
        const response = await fetch(`${API_BASE}/stats`);
        const data = await response.json();
        
        document.getElementById('actor-count').textContent = data.actorCount;
        document.getElementById('component-count').textContent = data.componentCount;
    } catch (error) {
        console.error('Failed to update stats:', error);
    }
}

function renderActorList(actors) {
    console.log('Rendering actor list, count:', actors.length);
    const list = document.getElementById('actor-list');
    list.innerHTML = '';
    
    actors.forEach(actor => {
        const item = document.createElement('li');
        item.className = 'actor-item';
        if (actor.id === selectedActorId) {
            item.classList.add('selected');
        }
        
        item.textContent = `${actor.name} [${actor.tag}]`;
        item.onclick = (e) => {
            console.log('Actor clicked:', actor.name, 'ID:', actor.id);
            selectActor(actor.id, e);
        };
        
        list.appendChild(item);
    });
    console.log('Actor list rendered');
}

async function selectActor(actorId, event) {
    console.log('=== selectActor called ===');
    console.log('Actor ID:', actorId);
    console.log('Event:', event);
    
    selectedActorId = actorId;
    
    // Update UI immediately
    document.querySelectorAll('.actor-item').forEach(item => {
        item.classList.remove('selected');
    });
    if (event && event.target) {
        event.target.classList.add('selected');
        console.log('UI updated for selection');
    }
    
    // Load actor details
    console.log('Loading actor details...');
    loadActorDetails(actorId);
    
    // Notify engine of selected actor for gizmo rendering
    console.log('Notifying engine of selection...');
    try {
        const url = `${API_BASE}/editor/select`;
        console.log('Fetch URL:', url);
        console.log('Fetch body:', JSON.stringify({ actorId: actorId }));
        
        const response = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ actorId: actorId })
        });
        
        console.log('Response status:', response.status);
        const result = await response.json();
        console.log('Selection response:', result);
    } catch (error) {
        console.error('!!! Failed to notify engine of selection !!!');
        console.error('Error details:', error);
    }
    console.log('=== selectActor complete ===');
}

async function testSelectEndpoint() {
    console.log('=== TESTING SELECT ENDPOINT ===');
    const testActorId = 12345; // dummy ID
    try {
        const url = `${API_BASE}/editor/select`;
        console.log('Test fetch URL:', url);
        const response = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ actorId: testActorId })
        });
        console.log('Test response status:', response.status);
        const result = await response.json();
        console.log('Test response data:', result);
    } catch (error) {
        console.error('Test FAILED:', error);
    }
    console.log('=== TEST COMPLETE ===');
}

async function pauseEngine() {
    try {
        await fetch(`${API_BASE}/engine/pause`, { method: 'POST' });
        document.getElementById('pause-btn').style.display = 'none';
        document.getElementById('resume-btn').style.display = 'block';
        console.log('Engine paused');
    } catch (error) {
        console.error('Failed to pause engine:', error);
    }
}

async function resumeEngine() {
    try {
        await fetch(`${API_BASE}/engine/resume`, { method: 'POST' });
        document.getElementById('pause-btn').style.display = 'block';
        document.getElementById('resume-btn').style.display = 'none';
        console.log('Engine resumed');
    } catch (error) {
        console.error('Failed to resume engine:', error);
    }
}

async function updateEngineStatus() {
    try {
        const response = await fetch(`${API_BASE}/engine/status`);
        const data = await response.json();
        if (data.paused) {
            document.getElementById('pause-btn').style.display = 'none';
            document.getElementById('resume-btn').style.display = 'block';
        } else {
            document.getElementById('pause-btn').style.display = 'block';
            document.getElementById('resume-btn').style.display = 'none';
        }
    } catch (error) {
        console.error('Failed to get engine status:', error);
    }
}

async function loadActorDetails(actorId) {
    try {
        const response = await fetch(`${API_BASE}/actors/${actorId}`);
        const actor = await response.json();
        
        if (actor.error) {
            document.getElementById('inspector-content').innerHTML = 
                `<div class="inspector-empty">Error: ${actor.error}</div>`;
            return;
        }
        
        renderInspector(actor);
    } catch (error) {
        console.error('Failed to load actor details:', error);
    }
}

function renderInspector(actor) {
    const inspector = document.getElementById('inspector-content');
    inspector.innerHTML = `
        <div class="property-group">
            <h3>Actor Properties</h3>
            <div class="property">
                <label>Name</label>
                <input type="text" value="${actor.name}" 
                       onchange="updateActorProperty(${actor.id}, 'name', this.value)">
            </div>
            <div class="property">
                <label>Tag</label>
                <input type="text" value="${actor.tag}"
                       onchange="updateActorProperty(${actor.id}, 'tag', this.value)">
            </div>
        </div>

        <div class="property-group">
            <h3>Transform</h3>
            <div class="property">
                <label>Position</label>
                <div class="vector-group">
                    <div class="vector-input">
                        <label>X</label>
                        <input type="number" step="0.1" value="${actor.transform.position.x.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'position', 'x', this.value)">
                    </div>
                    <div class="vector-input">
                        <label>Y</label>
                        <input type="number" step="0.1" value="${actor.transform.position.y.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'position', 'y', this.value)">
                    </div>
                    <div class="vector-input">
                        <label>Z</label>
                        <input type="number" step="0.1" value="${actor.transform.position.z.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'position', 'z', this.value)">
                    </div>
                </div>
            </div>

            <div class="property">
                <label>Rotation</label>
                <div class="vector-group">
                    <div class="vector-input">
                        <label>X</label>
                        <input type="number" step="1" value="${actor.transform.rotation.x.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'rotation', 'x', this.value)">
                    </div>
                    <div class="vector-input">
                        <label>Y</label>
                        <input type="number" step="1" value="${actor.transform.rotation.y.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'rotation', 'y', this.value)">
                    </div>
                    <div class="vector-input">
                        <label>Z</label>
                        <input type="number" step="1" value="${actor.transform.rotation.z.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'rotation', 'z', this.value)">
                    </div>
                </div>
            </div>

            <div class="property">
                <label>Scale</label>
                <div class="vector-group">
                    <div class="vector-input">
                        <label>X</label>
                        <input type="number" step="0.1" value="${actor.transform.scale.x.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'scale', 'x', this.value)">
                    </div>
                    <div class="vector-input">
                        <label>Y</label>
                        <input type="number" step="0.1" value="${actor.transform.scale.y.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'scale', 'y', this.value)">
                    </div>
                    <div class="vector-input">
                        <label>Z</label>
                        <input type="number" step="0.1" value="${actor.transform.scale.z.toFixed(2)}"
                               onchange="updateTransform(${actor.id}, 'scale', 'z', this.value)">
                    </div>
                </div>
            </div>
        </div>

        <div class="property-group">
            <h3>Components</h3>
            <div>Component Count: ${actor.componentCount}</div>
        </div>
    `;
}

async function updateActorProperty(actorId, property, value) {
    try {
        const body = {};
        body[property] = value;
        
        await fetch(`${API_BASE}/actors/${actorId}/property`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(body)
        });
    } catch (error) {
        console.error('Failed to update property:', error);
    }
}

async function updateTransform(actorId, type, axis, value) {
    try {
        const response = await fetch(`${API_BASE}/actors/${actorId}`);
        const actor = await response.json();
        
        actor.transform[type][axis] = parseFloat(value);
        
        await fetch(`${API_BASE}/actors/${actorId}/transform`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(actor.transform)
        });
    } catch (error) {
        console.error('Failed to update transform:', error);
    }
}

function startAutoRefresh() {
    updateInterval = setInterval(() => {
        loadScene();
        updateEngineStatus();
    }, 2000); // Refresh every 2 seconds
}

function stopAutoRefresh() {
    if (updateInterval) {
        clearInterval(updateInterval);
    }
}
)JS";
}