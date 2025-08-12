# Technology Stack

## Backend (C)
- **Web Framework**: Mongoose (embedded web server)
- **JSON Processing**: cJSON library
- **Build System**: Make with Windows-specific flags (`-lws2_32`)
- **Platform**: Windows (primary target)

## Frontend
- **Core**: Vanilla JavaScript (ES6 modules)
- **UI**: HTML5 Canvas for interactive drawing
- **Styling**: Plain CSS
- **Architecture**: Modular structure with services, UI controllers, and utilities

## Dependencies
- `dependencies/mongoose/` - Embedded web server
- `dependencies/cJSON/` - JSON parsing and generation
- Both vendored as source files, compiled directly into the application

## Common Commands

### Building
```bash
# Build the entire project
make build

# Clean build artifacts and temp directories
make clean

# Build and run (avoid using in automated contexts)
make build-and-run
```

### Development Workflow
```bash
# Before building, terminate any running server to avoid locked binaries
taskkill /F /IM main.exe

# Build after C code changes
make build

# Frontend-only changes don't require rebuilding
```

## Build Process
- Top-level Makefile delegates to `src/app/Makefile`
- Compiles all `.c` files in `src/app/` and `dependencies/`
- Outputs `src/app/build/main.exe`
- Automatically creates temp directories for runtime use

## Important Notes
- Only build when C files have changed
- Frontend changes don't require compilation
- Server must be terminated before rebuilding to avoid file locks
- Successful `make build` from repo root is the primary build verification