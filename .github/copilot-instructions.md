# Copilot instructions for BladeOfGrass-coveragePlanning

Purpose: Fast-start guide for AI agents. Keep edits small, follow existing patterns, and prefer minimal, isolated changes.

## Big picture
- C backend (Mongoose) serves a vanilla JS canvas app for coverage planning.
- Backend: `src/app/` (entry `main.c`) starts Mongoose and dispatches routes in `webserver.c` via `get_route_type` → handler functions. JSON is parsed with cJSON. Coverage logic lives in `src/app/coverage_path_planning/**` and is triggered by a POST export endpoint.
- Frontend: `src/web/path_canvas/` contains `index.html`, `app.js`, and modules under `environment/`, `services/`, and `ui/`. Boundary polygon is CW; obstacle polygons are CCW.

## Build workflow (Windows)
- Top-level Makefile: `make build` builds `src/app/build/main.exe` with `-lws2_32`; `make clean` resets build and temp dirs. A `run` target exists but agents should not use it.
- App Makefile compiles: `main.c`, `webserver.c`, coverage files under `coverage_path_planning/**`, plus vendored `dependencies/{cJSON,mongoose}/*.c`. Add new C sources to `SRC` here.
- Important: Before `make build`, terminate any running server to avoid locked binaries: `taskkill /F /IM main.exe` (or stop the active Task / Ctrl+C).
- Agent constraints: Only build when C files changed; skip builds for frontend-only edits. Do not run the server or open a browser. Treat a successful `make build` from repo root as the check.

## Backend patterns (Mongoose + cJSON)
- Init: `webserver_init` → `mg_http_listen(mgr, listen_url, webserver_event_handler, NULL)`; `main.c` polls `mg_mgr`.
- Routing: `webserver_event_handler` switches on `get_route_type(hm->uri)` and calls handlers. Routes include static assets and JSON endpoints.
- Static files: `mg_http_serve_file(c, hm, "../../web/path_canvas/...", &opts)`; test page uses `../../web/test/...`. Paths are relative to `src/app/webserver.c`.
- JSON examples:
  - `/send` → `handle_send_route`: parse body with cJSON, reply JSON with CORS headers.
  - `/environment/InputEnvironment/export` → `handle_path_input_environment_export_route`: logs prettified JSON, calls `coverage_path_planning_process(body_str)`, replies `{ "status":"ok" }` with CORS.
- Gotcha: Ensure enum names and handler prototypes in `webserver.h` match `webserver.c` (e.g., export route uses `ROUTE_PATH_INPUT_ENVIRONMENT_EXPORT` and `handle_path_input_environment_export_route`). Keep CORS headers identical to existing handlers.

## Frontend structure & conventions
- Key files: `environment/InputEnvironment.js`, `services/{CoordinateTransformer,DataService}.js`, `ui/{CanvasManager,UIController,UIStateManager}.js`, `utils/testing.js`, `app.js`.
- Interaction (from `UIController`): left-click adds points; right-click completes/cancels (≥3 points); ESC cancels; delete mode toggles obstacle deletion. `CanvasManager` manages pan/zoom; `CoordinateTransformer` converts screen↔world.

## Routes and aliases (examples)
- `/` or `/path-index.html` or `/index.html` → `index.html`
- `/path-styles.css` or `/styles.css` → `styles.css`; `/path-script.js` or `/app.js` → `app.js`
- Module routes: `/environment/InputEnvironment.js`, `/services/{CoordinateTransformer,DataService}.js`, `/ui/{CanvasManager,UIController,UIStateManager}.js`, `/utils/testing.js` (alias `/testing.js`).
- Test page: `/test`, `/test-styles.css`, `/test-script.js`.

Questions or gaps? If any section is unclear (especially route naming alignment in `webserver.h` vs `webserver.c` or coverage hook expectations), tell me and I’ll refine this doc.
