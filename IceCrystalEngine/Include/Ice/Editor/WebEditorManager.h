#pragma once

#ifndef WEB_EDITOR_MANAGER_H
#define WEB_EDITOR_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>

// Forward declarations
namespace httplib {
    class Server;
}

class Actor;
class Component;

struct WebSocketClient {
    int id;
    std::string sessionId;
    bool connected;
};

class WebEditorManager
{
public:
    static WebEditorManager& GetInstance()
    {
        static WebEditorManager instance;
        return instance;
    }

    // Start/Stop the web server
    void Start(int port = 8080);
    void Stop();
    bool IsRunning() const { return running; }

    // Update method to be called from main loop for WebSocket updates
    void Update();

    // Pause/Resume engine updates (physics, Lua, etc)
    void PauseEngine();
    void ResumeEngine();
    bool IsEnginePaused() const { return enginePaused; }
    void SetEnginePaused(bool paused);

    // Selected actor tracking for gizmo rendering
    void SetSelectedActor(int actorId);
    Actor* GetSelectedActor() const;
    bool HasSelectedActor() const { return selectedActor != nullptr; }

    // Broadcast events to connected clients
    void BroadcastActorCreated(Actor* actor);
    void BroadcastActorDeleted(int actorId);
    void BroadcastActorUpdated(Actor* actor);
    void BroadcastTransformChanged(Actor* actor);
    void BroadcastComponentAdded(Actor* actor, Component* component);
    void BroadcastComponentRemoved(Actor* actor, int componentId);

    // Configuration
    int GetPort() const { return port; }
    std::string GetURL() const;

private:
    WebEditorManager();
    ~WebEditorManager();

    WebEditorManager(WebEditorManager const&) = delete;
    void operator=(WebEditorManager const&) = delete;

    // HTTP Server thread
    void ServerThread();
    
    // Setup HTTP routes
    void SetupRoutes();
    
    // API Endpoints
    std::string HandleGetScene();
    std::string HandleGetActors();
    std::string HandleGetActor(int actorId);
    std::string HandleUpdateActorTransform(int actorId, const std::string& body);
    std::string HandleUpdateActorProperty(int actorId, const std::string& body);
    std::string HandleGetComponents(int actorId);
    std::string HandleUpdateComponentProperty(int componentId, const std::string& body);
    std::string HandleToggleComponentEnabled(int componentId, const std::string& body);
    std::string HandleAddComponent(int actorId, const std::string& body);
    std::string HandleRemoveComponent(int actorId, int componentId);
    std::string HandleGetSceneStats();
    std::string HandlePauseEngine();
    std::string HandleResumeEngine();

    // WebSocket handlers
    void HandleWebSocketMessage(const std::string& message);
    void BroadcastMessage(const std::string& message);

    // Serialization helpers
    std::string SerializeActor(Actor* actor);
    std::string SerializeActorList();
    std::string SerializeTransform(Actor* actor);
    std::string SerializeComponent(Component* component);
    std::string SerializeScene();

    // Member variables
    httplib::Server* server;
    std::thread* serverThread;
    std::atomic<bool> running;
    int port;

    // WebSocket clients (simplified - using polling for now)
    std::vector<WebSocketClient> clients;
    std::mutex clientsMutex;
    
    // Message queue for broadcasting
    std::vector<std::string> messageQueue;
    std::mutex messageMutex;

    // Editor state
    bool editorEnabled;
    std::string editorPath;
    std::atomic<bool> enginePaused;
    Actor* selectedActor;
};

#endif