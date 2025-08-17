# Copilot instructions for BladeOfGrass-coveragePlanning

Purpose: Fast-start guide for AI agents. Keep edits small, follow existing patterns, and prefer minimal, isolated changes.

## Big picture
- C backend (Mongoose) serves a vanilla JS canvas app for coverage planning.
- Backend: `src/app/` (entry `main.c`) starts Mongoose and dispatches routes in `webserver.c` via `get_route_type` → handler functions. JSON is parsed with cJSON. Coverage logic lives in `src/app/coverage_path_planning/**` and is triggered by a POST export endpoint (Run BCD).
- Frontend: `src/web/path_canvas/` contains `index.html`, `app.js`, and modules under `environment/`, `services/`, and `ui/`. Boundary polygon is CW; obstacle polygons are CCW.

## Coverage Path Planning Core
- Entry point: `coverage_path_planning_process(json_str)` in `coverage_path_planning.c` receives environment JSON, orchestrates BCD algorithm.
- BCD implementation: `boustrophedon_cellular_decomposition/` contains cell computation, coverage planning, and event list building modules.
- Data types: `point_t`, `polygon_edge_t`, `polygon_winding_t` (CW=boundary, CCW=obstacle), `polygon_type_t` defined in `coverage_path_planning.h`.
- Output: Event list JSON with coverage path coordinates returned to frontend for visualization.

## Build workflow (Windows)
- Top-level Makefile: `make build` builds `src/app/build/main.exe` with `-lws2_32`; `make clean` resets build and temp dirs. `make build-and-run` builds and runs in one step (but agents should avoid the run part).
- App Makefile compiles: `main.c`, `webserver.c`, coverage files under `coverage_path_planning/**`, plus vendored `dependencies/{cJSON,mongoose}/*.c`. Add new C sources to `SRC` here.
- Important: Before `make build`, terminate any running server to avoid locked binaries: `taskkill /F /IM main.exe`.
- Agent constraints: Only build when C files changed; skip builds for frontend-only edits. Do not run the server or open a browser. Treat a successful `make build` from repo root as the check.

## File System & Data Flow
- Save files: User environments stored as JSON in `src/save_files/<name>.json` via save API endpoints.
- Temp directories: `temp/env_in/` and `temp/dec_out/` used by coverage algorithms (cleaned/recreated by `make clean`).
- Dependencies: Vendored libraries in `dependencies/` - cJSON for JSON parsing, Mongoose for web server, cvector for dynamic arrays.

## Backend patterns (Mongoose + cJSON)
- Init: `webserver_init` → `mg_http_listen(mgr, listen_url, webserver_event_handler, NULL)`; `main.c` polls `mg_mgr`.
- Routing: `webserver_event_handler` switches on `get_route_type(hm->uri)` and calls handlers. Routes include static assets and JSON endpoints.
- Static files: `mg_http_serve_file(c, hm, "../../web/path_canvas/...", &opts)`; test page uses `../../web/test/...`. Paths are relative to `src/app/webserver.c`.
- JSON examples:
  - `/send` → `handle_send_route`: parse body with cJSON, reply JSON with CORS headers.
  - `/environment/InputEnvironment/export` → `handle_path_input_environment_export_route`: consumes environment JSON, calls `coverage_path_planning_process(body_str)`, replies event list JSON with CORS.
  - Saves API:
  - `POST /environment/InputEnvironment/save?name=<file>` → `handle_path_input_environment_save_route`: writes JSON file to `src/save_files/<file>.json` (sanitized name).
  - `GET /environment/InputEnvironment/saves` → `handle_path_input_environment_saves_list_route`: JSON array of saved names (without .json).
  - `GET /environment/InputEnvironment/load?name=<file>` → `handle_path_input_environment_load_route`: returns saved JSON.
  - `POST /environment/InputEnvironment/delete?name=<file>` → `handle_path_input_environment_delete_route`: deletes saved file.
- Gotcha: Ensure enum names and handler prototypes in `webserver.h` match `webserver.c` (e.g., export route uses `ROUTE_PATH_INPUT_ENVIRONMENT_EXPORT` and `handle_path_input_environment_export_route`). Keep CORS headers identical to existing handlers.

## Frontend structure & conventions
- Key files: `environment/InputEnvironment.js`, `services/{CoordinateTransformer,DataService}.js`, `ui/{CanvasManager,UIController,UIStateManager}.js`, `app.js`.
- Main init: `app.js` creates instances and wires up right panel collapse functionality with canvas resize handling.
- Interaction (from `UIController`):
  - Left-click adds points; right-click completes/cancels (≥3 points) for boundary/obstacles.
  - ESC cancels current mode.
  - Delete mode toggles obstacle deletion.
  - Panning is available at any time with middle-mouse drag or hold Space + drag.
  - `CanvasManager` manages pan/zoom; `CoordinateTransformer` converts screen↔world.

## Routes and aliases (examples)
- `/` or `/path-index.html` or `/index.html` → `index.html`
- `/path-styles.css` or `/styles.css` → `styles.css`; `/path-script.js` or `/app.js` → `app.js`
- Module routes: `/environment/InputEnvironment.js`, `/services/{CoordinateTransformer,DataService}.js`, `/ui/{CanvasManager,UIController,UIStateManager}.js`.
- Test page: `/test`, `/test-styles.css`, `/test-script.js`.

## UI overview
- Left panel: Draw Boundary, Draw Obstacle, Run BCD, Clear Canvas, Save, Load.
- Save opens a filename prompt; Load lists saved environments with Load/Delete.
- Run BCD posts the current environment to `/environment/InputEnvironment/export` and shows returned events (toggle via Event Markers switch).

Questions or gaps? If any section is unclear (especially route naming alignment in `webserver.h` vs `webserver.c` or coverage hook expectations), tell me and I’ll refine this doc.
