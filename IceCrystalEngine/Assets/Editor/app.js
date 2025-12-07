// ============================================================================
// IceCrystal Engine - Web Editor JavaScript
// Real-time scene inspection and manipulation
// ============================================================================

const API_BASE = window.location.origin + "/api";

// Global state
let selectedActorId = null;
let updateInterval = null;
let autoRefreshEnabled = true;
let refreshIntervalMs = 2000;
let actors = [];
let searchQuery = "";
let isEditing = false;
let editTimeout = null;
let enginePaused = false;
let componentsData = [];
let updateDebounceTimers = {};

// ============================================================================
// Initialization
// ============================================================================

document.addEventListener("DOMContentLoaded", () => {
  console.log("[WebEditor] Initializing...");

  // Check connection
  checkConnection();

  // Setup event listeners
  setupEventListeners();

  // Start auto-refresh
  if (autoRefreshEnabled) {
    startAutoRefresh();
  }

  // Load settings from localStorage
  loadSettings();

  // Check engine status
  checkEngineStatus();

  console.log("[WebEditor] Ready!");
});

// ============================================================================
// Event Listeners Setup
// ============================================================================

function setupEventListeners() {
  // Refresh button
  const refreshBtn = document.getElementById("refresh-btn");
  if (refreshBtn) {
    refreshBtn.addEventListener("click", () => {
      loadScene();
      updateStats();
    });
  }

  // Search input
  const searchInput = document.getElementById("search-input");
  if (searchInput) {
    searchInput.addEventListener("input", (e) => {
      searchQuery = e.target.value.toLowerCase();
      filterActorList();
    });
  }

  // Expand all button
  const expandAllBtn = document.getElementById("expand-all-btn");
  if (expandAllBtn) {
    expandAllBtn.addEventListener("click", () => {
      addConsoleLog("Expand all not yet implemented", "warning");
    });
  }

  // Settings button
  const settingsBtn = document.getElementById("settings-btn");
  if (settingsBtn) {
    settingsBtn.addEventListener("click", openSettings);
  }

  // Auto-refresh toggle
  const autoRefreshToggle = document.getElementById("auto-refresh-toggle");
  if (autoRefreshToggle) {
    autoRefreshToggle.addEventListener("click", toggleAutoRefresh);
  }

  // Close modal when clicking outside
  window.addEventListener("click", (e) => {
    const modal = document.getElementById("settings-modal");
    if (e.target === modal) {
      closeSettings();
    }
  });
}

// ============================================================================
// Connection & Status
// ============================================================================

async function checkConnection() {
  try {
    const response = await fetch(`${API_BASE}/stats`, {
      method: "GET",
      cache: "no-cache",
    });

    if (response.ok) {
      const data = await response.json();
      updateStatus(true);
      loadScene();
      updateStats();
      return true;
    } else {
      updateStatus(false);
      return false;
    }
  } catch (error) {
    console.error("Connection error:", error);
    updateStatus(false);
    return false;
  }
}

function updateStatus(connected) {
  const indicator = document.getElementById("status-indicator");
  const statusText = document.getElementById("status-text");

  if (connected) {
    indicator.className = "status-dot status-connected";
    statusText.textContent = "Connected";
    addConsoleLog("Connected to engine", "success");
  } else {
    indicator.className = "status-dot status-disconnected";
    statusText.textContent = "Disconnected";
    addConsoleLog("Disconnected from engine", "error");
  }

  const connectionInfo = document.getElementById("connection-info");
  if (connectionInfo) {
    connectionInfo.textContent = connected
      ? `Connected to ${window.location.host}`
      : "Disconnected";
  }
}

// ============================================================================
// Engine Pause/Resume
// ============================================================================

async function toggleEnginePause() {
  try {
    const endpoint = enginePaused ? "/engine/resume" : "/engine/pause";
    const response = await fetch(`${API_BASE}${endpoint}`, {
      method: "POST",
    });

    if (response.ok) {
      const data = await response.json();
      enginePaused = !enginePaused;
      updateEnginePauseUI();
      addConsoleLog(`Engine ${enginePaused ? "paused" : "resumed"}`, "info");

      // Refresh inspector to update disabled state
      if (selectedActorId) {
        loadActorDetails(selectedActorId);
      }
    }
  } catch (error) {
    console.error("Failed to toggle engine pause:", error);
    addConsoleLog("Failed to toggle engine pause", "error");
  }
}

async function checkEngineStatus() {
  try {
    const response = await fetch(`${API_BASE}/engine/status`);
    if (response.ok) {
      const data = await response.json();
      enginePaused = data.paused;
      updateEnginePauseUI();
    }
  } catch (error) {
    console.error("Failed to check engine status:", error);
  }
}

function updateEnginePauseUI() {
  const pauseBtn = document.getElementById("pause-engine-btn");
  const pauseIcon = document.getElementById("pause-icon");
  const pauseText = document.getElementById("pause-text");

  if (pauseBtn && pauseIcon && pauseText) {
    if (enginePaused) {
      pauseIcon.textContent = "▶️";
      pauseText.textContent = "Resume";
      pauseBtn.classList.add("paused");
      pauseBtn.title = "Resume Engine";
    } else {
      pauseIcon.textContent = "⏸️";
      pauseText.textContent = "Pause";
      pauseBtn.classList.remove("paused");
      pauseBtn.title = "Pause Engine";
    }
  }
}

// ============================================================================
// Scene Loading
// ============================================================================

async function loadScene() {
  try {
    // Don't refresh if user is actively editing
    if (isEditing) {
      console.log("[WebEditor] Skipping refresh - user is editing");
      return;
    }

    const response = await fetch(`${API_BASE}/actors`, {
      method: "GET",
      cache: "no-cache",
    });

    if (!response.ok) {
      console.error("Failed to load actors");
      addConsoleLog("Failed to load scene", "error");
      return;
    }

    const data = await response.json();
    actors = data.actors || [];

    renderActorList();
    updateLastUpdateTime();

    // If an actor is selected, check if it still exists
    if (selectedActorId) {
      const actorExists = actors.some((a) => a.id === selectedActorId);
      if (!actorExists) {
        clearSelection();
      }
      // Don't reload actor details during auto-refresh to prevent jumping
    }
  } catch (error) {
    console.error("Error loading scene:", error);
    addConsoleLog(`Error loading scene: ${error.message}`, "error");
  }
}

async function updateStats() {
  try {
    const response = await fetch(`${API_BASE}/stats`);
    if (response.ok) {
      const data = await response.json();

      const actorCountEl = document.getElementById("actor-count");
      const componentCountEl = document.getElementById("component-count");

      if (actorCountEl) actorCountEl.textContent = data.actorCount || 0;
      if (componentCountEl)
        componentCountEl.textContent = data.componentCount || 0;
    }
  } catch (error) {
    console.error("Error updating stats:", error);
  }
}

function updateLastUpdateTime() {
  const lastUpdateEl = document.getElementById("last-update");
  if (lastUpdateEl) {
    const now = new Date();
    const timeStr = now.toLocaleTimeString();
    lastUpdateEl.textContent = `Last update: ${timeStr}`;
  }
}

// ============================================================================
// Actor List Rendering
// ============================================================================

function renderActorList() {
  const list = document.getElementById("actor-list");
  if (!list) return;

  list.innerHTML = "";

  const filteredActors = actors.filter((actor) => {
    if (!searchQuery) return true;
    return (
      actor.name.toLowerCase().includes(searchQuery) ||
      actor.tag.toLowerCase().includes(searchQuery)
    );
  });

  if (filteredActors.length === 0) {
    const emptyItem = document.createElement("li");
    emptyItem.className = "actor-item empty";
    emptyItem.textContent =
      searchQuery.length > 0 ? "No actors found" : "Scene is empty";
    list.appendChild(emptyItem);
    return;
  }

  filteredActors.forEach((actor) => {
    const item = document.createElement("li");
    item.className = "actor-item";
    if (actor.id === selectedActorId) {
      item.classList.add("selected");
    }

    item.onclick = () => selectActor(actor.id);

    // Icon based on tag or name
    let icon = "📦";
    if (actor.tag.toLowerCase().includes("camera")) icon = "📷";
    else if (actor.tag.toLowerCase().includes("light")) icon = "💡";

    const name = document.createElement("span");
    name.className = "actor-name";
    name.textContent = `${icon} ${actor.name}`;

    const tag = document.createElement("span");
    tag.className = "actor-tag";
    tag.textContent = actor.tag;

    item.appendChild(name);
    item.appendChild(tag);

    list.appendChild(item);
  });
}

function filterActorList() {
  renderActorList();
}

// ============================================================================
// Actor Selection
// ============================================================================

function selectActor(actorId) {
  selectedActorId = actorId;
  renderActorList(); // Update selection highlight
  loadActorDetails(actorId, false);
  addConsoleLog(`Selected actor: ${actorId}`, "info");
}

async function loadActorDetails(actorId, skipComponentReload = false) {
  try {
    const response = await fetch(`${API_BASE}/actors/${actorId}`);
    if (!response.ok) {
      console.error("Failed to load actor details");
      return;
    }

    const actorSelectionResponse = await fetch(
      `${API_BASE}/editor/select/${actorId}`,
    );

    if (!actorSelectionResponse) {
      console.error("Failed to load actor details(Selecting Actor)");
    }

    const actor = await response.json();
    renderInspector(actor);

    // Only load components when not skipping reload (i.e., when explicitly selecting actor)
    if (!skipComponentReload) {
      await loadComponents(actorId);
    }
  } catch (error) {
    console.error("Error loading actor details:", error);
    addConsoleLog(`Error loading actor details: ${error.message}`, "error");
  }
}

// ============================================================================
// Inspector Rendering
// ============================================================================

function renderInspector(actor) {
  const inspector = document.getElementById("inspector-content");
  if (!inspector) return;

  const t = actor.transform;
  const disabled = !enginePaused ? "disabled" : "";
  const disabledStyle = !enginePaused
    ? "opacity: 0.5; cursor: not-allowed;"
    : "";

  inspector.innerHTML = `
        <div class="property-group">
            <h3>🎭 Actor Properties</h3>
            ${!enginePaused ? '<div class="warning-banner">⚠️ Pause the engine to edit properties</div>' : ""}
            <div class="property">
                <label>Name</label>
                <input type="text" value="${escapeHtml(actor.name)}"
                       data-actor-id="${actor.id}"
                       data-property="name"
                       onfocus="startEditing()"
                       onblur="stopEditing()"
                       onchange="updateActorProperty(this)"
                       ${disabled}
                       style="${disabledStyle}">
            </div>
            <div class="property">
                <label>Tag</label>
                <input type="text" value="${escapeHtml(actor.tag)}"
                       data-actor-id="${actor.id}"
                       data-property="tag"
                       onfocus="startEditing()"
                       onblur="stopEditing()"
                       onchange="updateActorProperty(this)"
                       ${disabled}
                       style="${disabledStyle}">
            </div>
            <div class="property">
                <label>ID</label>
                <input type="text" value="${actor.id}" readonly style="opacity: 0.5; cursor: not-allowed;">
            </div>
        </div>

        <div class="property-group">
            <h3>📐 Transform</h3>

            <div class="property">
                <label>Position</label>
                <div class="vector-group">
                    <div class="vector-input">
                        <label>X</label>
                        <input type="number" step="0.1" value="${t.position.x.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="position"
                               data-axis="x"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                    <div class="vector-input">
                        <label>Y</label>
                        <input type="number" step="0.1" value="${t.position.y.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="position"
                               data-axis="y"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                    <div class="vector-input">
                        <label>Z</label>
                        <input type="number" step="0.1" value="${t.position.z.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="position"
                               data-axis="z"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                </div>
            </div>

            <div class="property">
                <label>Rotation (Degrees)</label>
                <div class="vector-group">
                    <div class="vector-input">
                        <label>X</label>
                        <input type="number" step="1" value="${t.rotation.x.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="rotation"
                               data-axis="x"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                    <div class="vector-input">
                        <label>Y</label>
                        <input type="number" step="1" value="${t.rotation.y.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="rotation"
                               data-axis="y"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                    <div class="vector-input">
                        <label>Z</label>
                        <input type="number" step="1" value="${t.rotation.z.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="rotation"
                               data-axis="z"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                </div>
            </div>

            <div class="property">
                <label>Scale</label>
                <div class="vector-group">
                    <div class="vector-input">
                        <label>X</label>
                        <input type="number" step="0.1" value="${t.scale.x.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="scale"
                               data-axis="x"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                    <div class="vector-input">
                        <label>Y</label>
                        <input type="number" step="0.1" value="${t.scale.y.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="scale"
                               data-axis="y"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                    <div class="vector-input">
                        <label>Z</label>
                        <input type="number" step="0.1" value="${t.scale.z.toFixed(2)}"
                               data-actor-id="${actor.id}"
                               data-transform="scale"
                               data-axis="z"
                               onfocus="startEditing()"
                               onblur="stopEditing()"
                               oninput="debouncedUpdateTransform(this)"
                               ${disabled}
                               style="${disabledStyle}">
                    </div>
                </div>
            </div>
        </div>

        <div class="property-group">
            <h3>🔧 Components</h3>
            ${!enginePaused ? '<div class="warning-banner">⚠️ Pause the engine to edit components</div>' : '<div class="info-note">✏️ Component editing enabled (paused)</div>'}
            <div id="components-container">
                <div class="info-item">⏳ Loading components...</div>
            </div>
        </div>
    `;
}

// ============================================================================
// Editing State Management
// ============================================================================

function startEditing() {
  isEditing = true;
  if (editTimeout) {
    clearTimeout(editTimeout);
  }
  console.log("[WebEditor] Started editing");
}

function stopEditing() {
  // Delay clearing the editing flag to allow onChange to fire
  editTimeout = setTimeout(() => {
    isEditing = false;
    console.log("[WebEditor] Stopped editing");
  }, 500);
}

// ============================================================================
// Actor Property Updates
// ============================================================================

async function updateActorProperty(input) {
  if (!enginePaused) {
    addConsoleLog("Cannot edit while engine is running", "warning");
    return;
  }

  const actorId = parseInt(input.dataset.actorId);
  const property = input.dataset.property;
  const value = input.value;

  try {
    const body = JSON.stringify({
      property: property,
      value: value,
    });

    const response = await fetch(`${API_BASE}/actors/${actorId}/property`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: body,
    });

    const result = await response.json();

    if (result.success) {
      addConsoleLog(`Updated ${property} to ${value}`, "success");

      // Update local actor data
      const actor = actors.find((a) => a.id === actorId);
      if (actor) {
        actor[property] = value;
        renderActorList(); // Re-render to show updated name/tag
      }
    } else {
      addConsoleLog(`Failed to update ${property}`, "error");
    }
  } catch (error) {
    console.error("Error updating actor property:", error);
    addConsoleLog(`Error updating property: ${error.message}`, "error");
  }
}

function debouncedUpdateTransform(input) {
  const key = `transform_${input.dataset.actorId}_${input.dataset.transform}_${input.dataset.axis}`;

  // Clear existing timer for this input
  if (updateDebounceTimers[key]) {
    clearTimeout(updateDebounceTimers[key]);
  }

  // Set new timer
  updateDebounceTimers[key] = setTimeout(() => {
    updateTransform(input);
    delete updateDebounceTimers[key];
  }, 300); // 300ms debounce
}

async function updateTransform(input) {
  if (!enginePaused) {
    addConsoleLog("Cannot edit while engine is running", "warning");
    return;
  }

  const actorId = parseInt(input.dataset.actorId);
  const transformType = input.dataset.transform;
  const axis = input.dataset.axis;
  const value = parseFloat(input.value);

  try {
    // Get current actor
    const response = await fetch(`${API_BASE}/actors/${actorId}`);
    const actor = await response.json();

    // Update the specific axis
    actor.transform[transformType][axis] = value;

    // Send complete transform update
    const updateResponse = await fetch(
      `${API_BASE}/actors/${actorId}/transform`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(actor.transform),
      },
    );

    const result = await updateResponse.json();

    if (result.success) {
      addConsoleLog(
        `Updated ${transformType}.${axis} to ${value.toFixed(2)}`,
        "success",
      );
    } else {
      addConsoleLog(`Failed to update transform`, "error");
    }
  } catch (error) {
    console.error("Error updating transform:", error);
    addConsoleLog(`Error updating transform: ${error.message}`, "error");
  }
}

// ============================================================================
// Component Loading and Display
// ============================================================================

async function loadComponents(actorId) {
  try {
    const response = await fetch(`${API_BASE}/actors/${actorId}/components`);
    if (!response.ok) {
      console.error("Failed to load components");
      return;
    }

    const data = await response.json();
    componentsData = data.components || [];

    renderComponents();
  } catch (error) {
    console.error("Error loading components:", error);
    addConsoleLog(`Error loading components: ${error.message}`, "error");
  }
}

function renderComponents() {
  const container = document.getElementById("components-container");
  if (!container) return;

  if (componentsData.length === 0) {
    container.innerHTML = '<div class="info-item">No components</div>';
    return;
  }

  const disabled = !enginePaused ? "disabled" : "";
  const disabledStyle = !enginePaused
    ? "opacity: 0.5; cursor: not-allowed;"
    : "";

  let html = "";

  componentsData.forEach((comp) => {
    const enabledClass = comp.enabled ? "" : "component-disabled";
    html += `<div class="component-item ${enabledClass}">`;
    html += `<div class="component-header">`;
    html += `<span>${getComponentIcon(comp.type)} ${comp.type}</span>`;
    html += `<label class="component-toggle" title="${comp.enabled ? "Disable" : "Enable"} component">`;
    html += `<input type="checkbox" ${comp.enabled ? "checked" : ""}
             onchange="toggleComponentEnabled(${comp.id}, this.checked)"
             ${disabled}>`;
    html += `<span class="toggle-slider"></span>`;
    html += `</label>`;
    html += `</div>`;

    if (comp.properties && Object.keys(comp.properties).length > 0) {
      html += `<div class="component-properties">`;
      html += renderComponentProperties(comp, disabled, disabledStyle);
      html += `</div>`;
    }

    html += `</div>`;
  });

  container.innerHTML = html;
}

function renderComponentProperties(comp, disabled, disabledStyle) {
  let html = "";

  for (const [key, value] of Object.entries(comp.properties)) {
    if (typeof value === "object" && value !== null && !Array.isArray(value)) {
      // Handle nested objects (like vectors, colors)
      html += `<div class="property">`;
      html += `<label>${formatPropertyName(key)}</label>`;
      html += `<div class="vector-group">`;

      for (const [subKey, subValue] of Object.entries(value)) {
        html += `<div class="vector-input">`;
        html += `<label>${subKey.toUpperCase()}</label>`;
        html += `<input type="number" step="0.01" value="${typeof subValue === "number" ? subValue.toFixed(2) : subValue}"
                 data-component-id="${comp.id}"
                 data-property="${key}.${subKey}"
                 oninput="debouncedUpdateComponentProperty(this)"
                 onfocus="startEditing()"
                 onblur="stopEditing()"
                 ${disabled}
                 style="${disabledStyle}">`;
        html += `</div>`;
      }

      html += `</div></div>`;
    } else if (typeof value === "boolean") {
      // Handle boolean properties
      html += `<div class="property">`;
      html += `<label>${formatPropertyName(key)}</label>`;
      html += `<input type="checkbox" ${value ? "checked" : ""}
               data-component-id="${comp.id}"
               data-property="${key}"
               onchange="updateComponentProperty(this)"
               ${disabled}
               style="${disabledStyle}">`;
      html += `</div>`;
    } else if (typeof value === "number") {
      // Handle numeric properties
      html += `<div class="property">`;
      html += `<label>${formatPropertyName(key)}</label>`;
      html += `<input type="number" step="0.1" value="${value}"
               data-component-id="${comp.id}"
               data-property="${key}"
               oninput="debouncedUpdateComponentProperty(this)"
               onfocus="startEditing()"
               onblur="stopEditing()"
               ${disabled}
               style="${disabledStyle}">`;
      html += `</div>`;
    } else if (typeof value === "string") {
      // Handle string properties
      html += `<div class="property">`;
      html += `<label>${formatPropertyName(key)}</label>`;
      html += `<input type="text" value="${escapeHtml(value)}"
               data-component-id="${comp.id}"
               data-property="${key}"
               oninput="debouncedUpdateComponentProperty(this)"
               onfocus="startEditing()"
               onblur="stopEditing()"
               ${disabled}
               style="${disabledStyle}">`;
      html += `</div>`;
    }
  }

  return html;
}

function getComponentIcon(type) {
  const icons = {
    RigidBody: "⚙️",
    BoxCollider: "📦",
    SphereCollider: "🔵",
    Camera: "📷",
    DirectionalLight: "☀️",
    PointLight: "💡",
    SpotLight: "🔦",
    Freecam: "🎮",
    Renderer: "🎨",
    LuaExecutor: "📜",
  };

  return icons[type] || "🔧";
}

function formatPropertyName(name) {
  // Convert camelCase to Title Case
  return name
    .replace(/([A-Z])/g, " $1")
    .replace(/^./, (str) => str.toUpperCase())
    .trim();
}

function debouncedUpdateComponentProperty(input) {
  const key = `component_${input.dataset.componentId}_${input.dataset.property}`;

  if (updateDebounceTimers[key]) {
    clearTimeout(updateDebounceTimers[key]);
  }

  updateDebounceTimers[key] = setTimeout(() => {
    updateComponentProperty(input);
    delete updateDebounceTimers[key];
  }, 300);
}

async function updateComponentProperty(input) {
  if (!enginePaused) {
    addConsoleLog("Cannot edit while engine is running", "warning");
    return;
  }

  const componentId = parseInt(input.dataset.componentId);
  const property = input.dataset.property;
  let value;

  if (input.type === "checkbox") {
    value = input.checked;
  } else if (input.type === "number") {
    value = parseFloat(input.value);
  } else {
    value = input.value;
  }

  try {
    const body = JSON.stringify({
      property: property,
      value: value,
    });

    const response = await fetch(
      `${API_BASE}/components/${componentId}/property`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: body,
      },
    );

    const result = await response.json();

    if (result.success || result.message) {
      addConsoleLog(`Updated ${property} to ${value}`, "success");

      // Update local data
      const component = componentsData.find((c) => c.id === componentId);
      if (component && component.properties) {
        const propertyParts = property.split(".");
        if (propertyParts.length === 2) {
          if (!component.properties[propertyParts[0]]) {
            component.properties[propertyParts[0]] = {};
          }
          component.properties[propertyParts[0]][propertyParts[1]] = value;
        } else {
          component.properties[property] = value;
        }
      }
    } else if (result.error) {
      addConsoleLog(`Failed to update: ${result.error}`, "error");
    }
  } catch (error) {
    console.error("Error updating component:", error);
    addConsoleLog(`Error: ${error.message}`, "error");
  }
}

async function toggleComponentEnabled(componentId, enabled) {
  if (!enginePaused) {
    addConsoleLog(
      "Cannot toggle components while engine is running",
      "warning",
    );
    // Revert the checkbox
    const checkbox = event.target;
    checkbox.checked = !enabled;
    return;
  }

  try {
    const response = await fetch(
      `${API_BASE}/components/${componentId}/enabled`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ enabled: enabled }),
      },
    );

    const result = await response.json();

    if (result.success) {
      addConsoleLog(`Component ${enabled ? "enabled" : "disabled"}`, "success");

      // Update local data
      const component = componentsData.find((c) => c.id === componentId);
      if (component) {
        component.enabled = enabled;
        renderComponents(); // Re-render to update visual state
      }
    } else {
      addConsoleLog(`Failed to toggle component: ${result.error}`, "error");
      // Revert the checkbox
      const checkbox = event.target;
      checkbox.checked = !enabled;
    }
  } catch (error) {
    console.error("Error toggling component:", error);
    addConsoleLog(`Error: ${error.message}`, "error");
    // Revert the checkbox
    const checkbox = event.target;
    checkbox.checked = !enabled;
  }
}

// ============================================================================
// Auto-Refresh
// ============================================================================

function startAutoRefresh() {
  if (updateInterval) {
    clearInterval(updateInterval);
  }

  updateInterval = setInterval(() => {
    loadScene();
    updateStats();
  }, refreshIntervalMs);

  autoRefreshEnabled = true;
  updateAutoRefreshUI();
  addConsoleLog(`Auto-refresh started (${refreshIntervalMs}ms)`, "info");
}

function stopAutoRefresh() {
  if (updateInterval) {
    clearInterval(updateInterval);
    updateInterval = null;
  }

  autoRefreshEnabled = false;
  updateAutoRefreshUI();
  addConsoleLog("Auto-refresh stopped", "info");
}

function toggleAutoRefresh() {
  if (autoRefreshEnabled) {
    stopAutoRefresh();
  } else {
    startAutoRefresh();
  }
}

function updateAutoRefreshUI() {
  const statusEl = document.getElementById("auto-refresh-status");
  if (statusEl) {
    statusEl.textContent = autoRefreshEnabled ? "ON" : "OFF";
  }

  const toggleBtn = document.getElementById("auto-refresh-toggle");
  if (toggleBtn) {
    toggleBtn.style.fontWeight = autoRefreshEnabled ? "bold" : "normal";
  }
}

// ============================================================================
// Quick Actions
// ============================================================================

function clearSelection() {
  if (selectedActorId === null) {
    addConsoleLog("No actor selected", "warning");
    return;
  }

  selectedActorId = null;
  componentsData = [];
  renderActorList();

  const inspector = document.getElementById("inspector-content");
  if (inspector) {
    inspector.innerHTML = `
            <div class="inspector-empty">
                <div class="empty-state">
                    <div class="empty-icon">📋</div>
                    <p>Select an actor to inspect</p>
                </div>
            </div>
        `;
  }

  addConsoleLog("Selection cleared", "info");
}

function focusSelected() {
  if (!selectedActorId) {
    addConsoleLog("No actor selected to focus", "warning");
    return;
  }

  addConsoleLog("Focus selected not yet implemented", "warning");
}

async function exportScene() {
  try {
    const response = await fetch(`${API_BASE}/scene`);
    const scene = await response.json();

    const dataStr = JSON.stringify(scene, null, 2);
    const dataUri =
      "data:application/json;charset=utf-8," + encodeURIComponent(dataStr);

    const exportFileDefaultName = "scene.json";

    const linkElement = document.createElement("a");
    linkElement.setAttribute("href", dataUri);
    linkElement.setAttribute("download", exportFileDefaultName);
    linkElement.click();

    addConsoleLog("Scene exported successfully", "success");
  } catch (error) {
    console.error("Error exporting scene:", error);
    addConsoleLog(`Error exporting scene: ${error.message}`, "error");
  }
}

// ============================================================================
// Console Output
// ============================================================================

function addConsoleLog(message, type = "info") {
  const consoleOutput = document.getElementById("console-output");
  if (!consoleOutput) return;

  const line = document.createElement("div");
  line.className = `console-line console-${type}`;

  const timestamp = new Date().toLocaleTimeString();
  line.textContent = `[${timestamp}] ${message}`;

  consoleOutput.appendChild(line);
  consoleOutput.scrollTop = consoleOutput.scrollHeight;

  // Keep only last 50 messages
  while (consoleOutput.children.length > 50) {
    consoleOutput.removeChild(consoleOutput.firstChild);
  }
}

function clearConsole() {
  const consoleOutput = document.getElementById("console-output");
  if (consoleOutput) {
    consoleOutput.innerHTML = "";
  }
  addConsoleLog("Console cleared", "info");
}

// ============================================================================
// Settings Modal
// ============================================================================

function openSettings() {
  const modal = document.getElementById("settings-modal");
  if (modal) {
    modal.style.display = "flex";
  }
}

function closeSettings() {
  const modal = document.getElementById("settings-modal");
  if (modal) {
    modal.style.display = "none";
  }
}

function saveSettings() {
  const refreshInterval = document.getElementById("refresh-interval");
  const apiPort = document.getElementById("api-port");
  const showDebug = document.getElementById("show-debug-info");

  const settings = {
    refreshInterval: parseInt(refreshInterval.value),
    apiPort: parseInt(apiPort.value),
    showDebug: showDebug.checked,
  };

  localStorage.setItem("webEditorSettings", JSON.stringify(settings));

  refreshIntervalMs = settings.refreshInterval;

  if (autoRefreshEnabled) {
    stopAutoRefresh();
    startAutoRefresh();
  }

  closeSettings();
  addConsoleLog("Settings saved", "success");
}

function loadSettings() {
  const settings = localStorage.getItem("webEditorSettings");

  if (settings) {
    try {
      const parsed = JSON.parse(settings);

      if (parsed.refreshInterval) {
        refreshIntervalMs = parsed.refreshInterval;
        const refreshInput = document.getElementById("refresh-interval");
        if (refreshInput) refreshInput.value = parsed.refreshInterval;
      }
    } catch (error) {
      console.error("Error loading settings:", error);
    }
  }
}

// ============================================================================
// Utility Functions
// ============================================================================

function escapeHtml(unsafe) {
  const map = {
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#039;",
  };
  return unsafe.replace(/[&<>"']/g, (m) => map[m]);
}

// Modal close on outside click
const modal = document.getElementById("settings-modal");
if (modal) {
  window.onclick = function (event) {
    if (event.target === modal) {
      closeSettings();
    }
  };
}
