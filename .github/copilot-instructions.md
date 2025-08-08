# BladeOfGrass Coverage Planning - AI Coding Instructions

## Architecture Overview

This is a **hybrid C/JavaScript application** for robotic mower path planning with a unique architecture:

- **C Backend**: Mongoose webserver (`src/app/`) serves as HTTP API and static file server
- **JavaScript Frontend**: Canvas-based UI (`src/web/path_canvas/`) for interactive path planning
- **State Management**: JSON files (`src/web/path_canvas/state/`) persist boundary/obstacle/cell data

## Key Architectural Patterns

### 1. C Webserver Route Handling
The C server uses enum-based routing with dedicated handlers for each JavaScript file:
```c
// Each JS file has its own route in webserver.c
case ROUTE_PATH_BOUNDARY_MODEL_SCRIPT:
    handle_path_boundary_model_script_route(c, hm);
```
**Pattern**: When adding new JS files, add both route enum and handler function.

### 2. JavaScript Module System
Uses vanilla JS classes with manual dependency injection in `app.js`:
```javascript
const boundaryModel = new BoundaryModel();
const canvasManager = new CanvasManager(canvas, boundaryModel, obstacleModel);
const uiController = new UIController(boundaryModel, obstacleModel, canvasManager);
```
**Critical**: Script load order in `index.html` matters - models before services before controllers.

### 3. State Persistence Pattern
Data flows: UI → Models → JSON → C Server → File System
- `BoundaryModel`/`ObstacleModel`: In-memory state
- `AlgorithmService`: Coordinates model→server communication  
- `DataService`: Handles HTTP requests to C backend
- State files: `model.json` (input), `cells.json` (algorithm output)

### 4. Canvas Coordinate System
Uses world coordinates (meters) with coordinate transformation:
- Canvas pixels ↔ World meters via `CoordinateTransformer`
- Path planning operates in real-world units (0.25m path width)
- UI displays both pixel and meter coordinates

## Development Workflows

### Building and Running
```bash
make build-and-run  # Compiles C server and starts on port 8000
```
**Important**: Use root-level Makefile, not `src/app/Makefile` directly.

### Testing Patterns
- **Visual Testing**: `TestingController` provides quick environment setup buttons
  - Rectangle button: Creates test boundary + obstacle (input for algorithm)
  - Cells button: Shows hardcoded expected cells (for comparing algorithm results)
- **API Testing**: `/src/web/test/` contains JSON communication test suite
- **Algorithm Development**: Focus is on `CellularDecomposition` implementation

### Algorithm Development Focus
Current priority: Implementing proper cell decomposition algorithm
1. Input: `model.json` (boundary polygon + obstacles + path parameters)
2. Output: `cells.json` (array of polygon cells covering the boundary)
3. Testing approach: Compare algorithm output with expected hardcoded cells
4. C backend changes: Only add endpoints for new JS files if needed

## Project-Specific Conventions

### File Organization
- **Dependencies**: Embedded in `/dependencies/` (cJSON, Mongoose) - compile directly
- **State Files**: JSON format with `id`/`modelId` linking pattern
- **UI Structure**: Three-panel layout (Controls | Canvas | Data) with responsive CSS

### Error Handling
- C Server: CORS headers on all JSON responses with `Access-Control-Allow-*`
- JavaScript: Async/await pattern with try/catch in service layer
- UI: Status updates via `UIStateManager.setAlgorithmStatus()`

### Path Planning Domain
- **Boundary**: Closed polygon defining mowing area (e.g., 6m×4m rectangle)
- **Obstacles**: Array of polygons to avoid (e.g., diamond-shaped obstacle)
- **Cells**: Cellular decomposition output - array of polygon cells that tile the boundary
- **Coverage**: Path width (0.25m) + overlap (0.05m) determine coverage requirements
- **Example**: `model_example.json` → `cells.json` shows input→output transformation

## Development Philosophy

### Current Focus: Algorithm Implementation
- **Primary Goal**: Working cellular decomposition algorithm
- **Approach**: Simple, debuggable implementation over optimization
- **Testing**: Visual comparison between algorithm output and expected cells
- **C Backend**: Frozen - only add JS file endpoints if absolutely needed

## Critical Integration Points

### C↔JavaScript Communication
POST requests with JSON payloads to `/save_model` and `/save_decomposition` endpoints. C server expects specific JSON structure matching `BoundaryModel`/`ObstacleModel` properties.

### Canvas Event Handling
Mouse events → World coordinates → Model updates → Canvas redraw → UI state updates. The `CanvasManager` orchestrates this flow with help from coordinate transformation.

## Debugging Tips

- Enable browser console for JavaScript errors and algorithm output
- C server prints to console - check for JSON parsing errors
- Test JSON communication with `/test` route before debugging algorithms
- Use Testing buttons (Rectangle/Cells) to verify rendering pipeline
- Rectangle button creates input environment; Cells button shows expected output
- Compare algorithm results visually against hardcoded expected cells

## Algorithm Development Workflow

1. **Pseudocode First**: Design decomposition logic before implementation
2. **Simple Implementation**: Focus on correctness, not performance
3. **Visual Testing**: Use Rectangle→Algorithm→compare with Cells button
4. **Minimal Dependencies**: Keep implementation self-contained in `CellularDecomposition.js`

## Agent Testing Limitations

**Agent can:**
- Run the application with `make build-and-run`
- Open website at `http://localhost:8000` in Simple Browser
- View UI and monitor server logs for errors
- Implement and iterate on algorithm code

**Agent cannot:**
- Click buttons or interact with UI programmatically
- Automatically test algorithm workflows
- Draw boundaries or obstacles on canvas

**Testing requires user interaction**: Agent implements → User tests manually → Agent iterates based on feedback
