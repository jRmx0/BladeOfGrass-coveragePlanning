# Copilot instructions for BladeOfGrass-coveragePlanning

Purpose: Help AI agents be productive fast by capturing how this repo actually works (architecture, workflows, and patterns). Prefer small, isolated edits that fit the existing style.

## Big picture
- C HTTP server (Mongoose) serves a vanilla JS canvas app for coverage planning.
- Backend: `src/app/` (entry `main.c`) starts Mongoose, maps URIs → handlers, optionally parses JSON with cJSON.
- Frontend: `src/web/path_canvas/` contains `index.html`, drawing logic, and UI controllers. Boundary polygon is CW; obstacle polygons are CCW.

## Build & run (Windows)
- Top-level Makefile:
  - `make build` → builds `src/app/build/main.exe` using `gcc` and `-lws2_32`.
  - `make run` → runs the server; default URL: `http://localhost:8000` (set in `src/app/main.c`).
  - `make build-and-run` and `make clean` available.
- App Makefile (`src/app/Makefile`): compiles `main.c`, `webserver.c`, vendored `dependencies/cJSON/cJSON.c`, `dependencies/mongoose/mongoose.c`. Add new C sources to `SRC` here.

### Agent constraints (CI/local automation)
- Only run `make build` when C sources are added or modified (e.g., files under `src/app/*.c` or vendored deps in `dependencies/**.c`). For frontend-only changes (JS/HTML/CSS), skip the build.
- Do NOT run `make run` or `make build-and-run`.
- Do NOT open a web browser; a human will do runtime/UI checks.
- When C files change, treat a successful `make build` from the repo root as the verification step.

## Backend patterns (Mongoose + cJSON)
- `webserver.h`:
  - Declares `route_type_t` enum, handler prototypes, and includes `mongoose.h`.
- `webserver.c`:
  - `webserver_init` → `mg_http_listen(mgr, listen_url, webserver_event_handler, NULL)`.
  - `webserver_event_handler` → on `MG_EV_HTTP_MSG`, switch on `get_route_type(hm->uri)` and dispatch.
  - Static files served with `mg_http_serve_file` and opts `{ .root_dir = "../../web/path_canvas" }` (or `../../web/test`).
  - JSON example: `handle_send_route` copies `hm->body`, parses via cJSON, and replies JSON with CORS headers.
- Add a static route:
  1) Add enum + handler prototype in `webserver.h`.
  2) Map URI in `get_route_type` (e.g., `"/services/MyTool.js"`).
  3) Implement handler calling `mg_http_serve_file` with a `../../web/...` path.
- Add a JSON API:
  1) Add enum + `get_route_type` mapping (e.g., `"/compute"`).
  2) In handler: parse body with cJSON, `mg_http_reply` with CORS (copy from `handle_send_route`).

## Frontend structure & conventions
- Files: `environment/InputEnvironment.js`, `ui/CanvasManager.js`, `ui/UIController.js`, `ui/UIStateManager.js`, `services/{CoordinateTransformer,DataService}.js`, `utils/testing.js`, `app.js`.
- State model (`InputEnvironment`):
  - Boundary CW (`BoundaryPolygon.polygonVertexListCw`), obstacles CCW (`ObstaclePolygon.polygonVertexListCcw`). Private fields (`#...`) maintained internally.
  - `json` getter returns serializable state; `DataService.exportToConsole` logs it.
- Interaction rules (from `UIController`):
  - Left click adds a vertex; right click completes/cancels (boundary requires ≥3 points; obstacles complete at ≥3). ESC cancels current mode.
  - Delete mode toggles obstacle deletion; click inside/near vertex to delete. `UIStateManager` updates button states and cursor.
  - `CanvasManager` handles pan/zoom (`pixelsPerMeter = 100`); `CoordinateTransformer` converts screen↔world.

## Pages & quick test
- `/` → `src/web/path_canvas/index.html` (serves specific JS/CSS via explicit routes).
- `/test` → `src/web/test/test-index.html` and its assets.
- Human smoke test: run the server, open `/`, click “Rectangle” (from `utils/testing.js`), then “Export Data” to see JSON in browser console. Agents won’t perform this step.

## Gotchas
- Paths in `mg_http_serve_file` are relative to `src/app/webserver.c`; use the `../../web/...` pattern as in existing handlers.
- Route aliases exist (e.g., `/index.html` and `/path-index.html` both map to the same). Keep consistency if adding new aliases.
- Maintain polygon orientation: boundary CW, obstacles CCW. Rendering assumes this for fills/closures.
- Keep CORS headers in JSON replies (copy from `handle_send_route`) to support browser fetches.
