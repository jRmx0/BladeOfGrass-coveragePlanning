# Project Structure

## Root Level
- `Makefile` - Top-level build orchestration
- `.gitignore` - Excludes build artifacts and IDE files
- `docs/` - Documentation and diagrams

## Source Code (`src/`)

### Backend (`src/app/`)
- `main.c` - Application entry point, Mongoose manager polling
- `webserver.c` - HTTP request routing and handler implementations
- `webserver.h` - Route definitions and handler prototypes
- `Makefile` - C compilation rules and source file management
- `build/` - Compiled executable output
- `coverage_path_planning/` - Core algorithm implementations
  - `coverage_path_planning.c/.h` - Main coverage logic
  - `boustrophedon_cellular_decomposition/` - Specific algorithm implementation
- `misc/` - Utility functions (e.g., `fjson.c`)

### Frontend (`src/web/`)

#### Main Application (`src/web/path_canvas/`)
- `index.html` - Main application page
- `app.js` - Application entry point and module coordination
- `styles.css` - Application styling
- `environment/` - Environment definition modules
  - `InputEnvironment.js` - Boundary and obstacle management
- `services/` - Core application services
  - `CoordinateTransformer.js` - Screen/world coordinate conversion
  - `DataService.js` - API communication
- `ui/` - User interface controllers
  - `CanvasManager.js` - Canvas rendering and interaction
  - `UIController.js` - User input handling
  - `UIStateManager.js` - Application state management
- `utils/` - Utility modules
  - `testing.js` - Testing utilities

#### Test Interface (`src/web/test/`)
- `test-index.html` - Test page
- `test-script.js` - Test functionality
- `test-styles.css` - Test page styling

## Dependencies (`dependencies/`)
- `cJSON/` - JSON parsing library (vendored)
- `mongoose/` - Web server library (vendored)

## Conventions

### File Organization
- C source files grouped by functionality
- Frontend modules organized by responsibility (services, ui, utils)
- Static assets served relative to `src/app/webserver.c`

### Naming Patterns
- C files: lowercase with underscores (`coverage_path_planning.c`)
- JavaScript modules: PascalCase (`InputEnvironment.js`)
- Route handlers: `handle_*_route` pattern
- Enum routes: `ROUTE_*` pattern

### Path Conventions
- Static file paths relative to `src/app/webserver.c`
- Main app: `../../web/path_canvas/`
- Test page: `../../web/test/`
- Route aliases support multiple URL patterns for same resource