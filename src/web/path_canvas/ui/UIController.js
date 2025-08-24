class UIController {
    constructor(inputEnvironment, canvasManager) {
        this.inputEnvironment = inputEnvironment;
        this.canvasManager = canvasManager;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.isDeletingObstacle = false;
    this.isPanKeyActive = false; // hold Space to pan while in any mode
        
        // Initialize service dependencies
        this.dataService = new DataService();
        this.uiStateManager = new UIStateManager();
        this.coordinateTransformer = new CoordinateTransformer(canvasManager);
        
        this.initUI();
    }
    initUI() {
        document.getElementById('drawBoundary').addEventListener('click', () => this.toggleDrawingMode('boundary'));
        // Add boundary delete button if present
        const deleteBoundaryBtn = document.getElementById('deleteBoundary');
        if (deleteBoundaryBtn) {
            deleteBoundaryBtn.addEventListener('click', () => { this.cancelAllModes(); this.deleteBoundary(); });
        }
        // Add obstacle button if present
        const obstacleBtn = document.getElementById('drawObstacle');
        if (obstacleBtn) {
            obstacleBtn.addEventListener('click', () => this.toggleDrawingMode('obstacle'));
        }
        // Add delete obstacle button if present
        const deleteBtn = document.getElementById('deleteObstacle');
        if (deleteBtn) {
            deleteBtn.addEventListener('click', () => this.toggleDeleteMode());
        }
        // Add vertex numbers toggle (switch) if present
        const toggleVertexSwitch = document.getElementById('toggleVertexNumbers');
        const toggleEventsSwitch = document.getElementById('toggleEvents');
        const toggleCellsSwitch = document.getElementById('toggleCells');
        const toggleCoveragePathSwitch = document.getElementById('toggleCoveragePath');
        if (toggleVertexSwitch) {
            toggleVertexSwitch.addEventListener('change', (e) => {
                // If enabling vertex numbers, disable events and cells
                if (e.target.checked) {
                    if (toggleEventsSwitch && toggleEventsSwitch.checked) {
                        toggleEventsSwitch.checked = false;
                        this.canvasManager.showEvents = false;
                    }
                    if (toggleCellsSwitch && toggleCellsSwitch.checked) {
                        toggleCellsSwitch.checked = false;
                        this.canvasManager.showCells = false;
                    }
                }
                this.toggleVertexNumbers(e.target.checked);
            });
        }
        if (toggleEventsSwitch) {
            toggleEventsSwitch.addEventListener('change', (e) => {
                // If enabling events, disable vertex numbers and cells
                if (e.target.checked) {
                    if (toggleVertexSwitch && toggleVertexSwitch.checked) {
                        toggleVertexSwitch.checked = false;
                        this.toggleVertexNumbers(false);
                    }
                    if (toggleCellsSwitch && toggleCellsSwitch.checked) {
                        toggleCellsSwitch.checked = false;
                        this.canvasManager.showCells = false;
                    }
                }
                this.canvasManager.showEvents = !!e.target.checked;
                this.canvasManager.draw();
            });
        }
        if (toggleCellsSwitch) {
            toggleCellsSwitch.addEventListener('change', (e) => {
                // If enabling cells, disable vertex numbers and events
                if (e.target.checked) {
                    if (toggleVertexSwitch && toggleVertexSwitch.checked) {
                        toggleVertexSwitch.checked = false;
                        this.toggleVertexNumbers(false);
                    }
                    if (toggleEventsSwitch && toggleEventsSwitch.checked) {
                        toggleEventsSwitch.checked = false;
                        this.canvasManager.showEvents = false;
                    }
                }
                this.canvasManager.showCells = !!e.target.checked;
                this.canvasManager.draw();
            });
        }
        if (toggleCoveragePathSwitch) {
            toggleCoveragePathSwitch.addEventListener('change', (e) => {
                this.canvasManager.showCoveragePath = !!e.target.checked;
                this.canvasManager.draw();
            });
        }
        document.getElementById('clearCanvas').addEventListener('click', () => { this.cancelAllModes(); this.clearCanvas(); });
        document.getElementById('resetView').addEventListener('click', () => { this.cancelAllModes(); this.handleResetView(); });
        document.getElementById('toggleGrid').addEventListener('click', () => { this.cancelAllModes(); this.handleToggleGrid(); });
    const runBCD = document.getElementById('runBCD');
    if (runBCD) runBCD.addEventListener('click', () => { this.cancelAllModes(); this.runBCD(); });
    const saveBtn = document.getElementById('saveEnv');
    if (saveBtn) saveBtn.addEventListener('click', () => { this.cancelAllModes(); this.openSaveModal(); });
    const loadBtn = document.getElementById('loadEnv');
    if (loadBtn) loadBtn.addEventListener('click', () => { this.cancelAllModes(); this.openLoadModal(); });
        document.getElementById('pathWidth').addEventListener('change', (e) => {
            this.cancelAllModes();
            this.inputEnvironment.pathWidth = parseFloat(e.target.value);
            this.updateDisplay();
        });
        document.getElementById('pathOverlap').addEventListener('change', (e) => {
            this.cancelAllModes();
            this.inputEnvironment.pathOverlap = parseFloat(e.target.value);
            this.updateDisplay();
        });
        this.canvasManager.canvas.addEventListener('mousedown', (e) => this.handleMouseDown(e));
        this.canvasManager.canvas.addEventListener('mousemove', (e) => this.handleMouseMove(e));
        this.canvasManager.canvas.addEventListener('mouseup', (e) => this.handleMouseUp(e));
        this.canvasManager.canvas.addEventListener('wheel', (e) => this.handleWheel(e));
        this.canvasManager.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        this.canvasManager.canvas.addEventListener('dragstart', (e) => e.preventDefault());
        
    // Keyboard: ESC cancel; Space enables temporary pan mode
    document.addEventListener('keydown', (e) => this.handleKeyDown(e));
    document.addEventListener('keyup', (e) => this.handleKeyUp(e));
        
        this.updateDisplay();
        this.canvasManager.draw();
    }

    toggleVertexNumbers(isOn) {
        // If param not provided (legacy), flip; otherwise set from switch state
        if (typeof isOn === 'boolean') {
            this.canvasManager.showVertexNumbers = isOn;
        } else {
            this.canvasManager.showVertexNumbers = !this.canvasManager.showVertexNumbers;
        }
        this.updateDisplay();
        this.canvasManager.draw();
    }

    openSaveModal() {
        const modal = document.getElementById('saveModal');
        const input = document.getElementById('saveFileName');
        const confirm = document.getElementById('saveConfirm');
        const cancel = document.getElementById('saveCancel');
        if (!modal) return;
        modal.classList.remove('hidden');
        if (input) input.value = '';
        const close = () => modal.classList.add('hidden');
        const onSave = async () => {
            const name = (input?.value || '').trim();
            if (!name) { this.canvasManager.showNotification('Enter a file name.'); return; }
            try {
                await this.dataService.saveEnvironment(name, this.inputEnvironment.json);
                this.canvasManager.showNotification('Environment saved.');
                close();
            } catch (e) {
                this.canvasManager.showNotification('Save failed.');
            }
        };
        confirm?.addEventListener('click', onSave, { once: true });
        cancel?.addEventListener('click', () => close(), { once: true });
    }

    async openLoadModal() {
        const modal = document.getElementById('loadModal');
        const list = document.getElementById('loadList');
        const cancel = document.getElementById('loadCancel');
        if (!modal || !list) return;
        modal.classList.remove('hidden');
        list.innerHTML = 'Loading...';
        try {
            const items = await this.dataService.listSavedEnvironments();
            if (!Array.isArray(items) || items.length === 0) {
                list.innerHTML = '<div class="item">No saved environments.</div>';
            } else {
                list.innerHTML = '';
                for (const name of items) {
                    const row = document.createElement('div');
                    row.className = 'item';
                    const span = document.createElement('span');
                    span.textContent = name;
                    const loadBtn = document.createElement('button');
                    loadBtn.className = 'control-btn small';
                    loadBtn.textContent = 'Load';
                    loadBtn.addEventListener('click', async () => {
                        try {
                            const data = await this.dataService.loadEnvironment(name);
                            this.applyEnvironmentJSON(data);
                            this.canvasManager.showNotification('Environment loaded.');
                            modal.classList.add('hidden');
                        } catch (e) {
                            this.canvasManager.showNotification('Load failed.');
                        }
                    });
                    const delBtn = document.createElement('button');
                    delBtn.className = 'control-btn small cancel';
                    delBtn.textContent = 'Delete';
                    delBtn.addEventListener('click', async () => {
                        try {
                            await this.dataService.deleteEnvironment(name);
                            this.canvasManager.showNotification('Deleted.');
                            // Refresh list
                            this.openLoadModal();
                        } catch (e) {
                            this.canvasManager.showNotification('Delete failed.');
                        }
                    });
                    const actions = document.createElement('div');
                    actions.className = 'item-actions';
                    actions.appendChild(loadBtn);
                    actions.appendChild(delBtn);
                    row.appendChild(span);
                    row.appendChild(actions);
                    list.appendChild(row);
                }
            }
        } catch (e) {
            list.innerHTML = '<div class="item">Failed to load list.</div>';
        }
        cancel?.addEventListener('click', () => modal.classList.add('hidden'), { once: true });
    }

    applyEnvironmentJSON(json) {
        if (!json) return;
        // Reset environment
        const newEnv = new InputEnvironment();
        // Keep provided id if present
        if (typeof json.id === 'number') newEnv.id = json.id;
        if (typeof json.pathWidth === 'number') newEnv.pathWidth = json.pathWidth;
        if (typeof json.pathOverlap === 'number') newEnv.pathOverlap = json.pathOverlap;
        // Boundary
        if (Array.isArray(json.boundary)) {
            newEnv.boundaryPolygon.clear();
            for (const v of json.boundary) {
                if (v && typeof v.x === 'number' && typeof v.y === 'number') {
                    newEnv.boundaryPolygon.insertPolygonVertex(new PolygonVertex(v.x, v.y));
                }
            }
        }
        // Obstacles
        newEnv.obstaclePolygonList = [];
        if (Array.isArray(json.obstacles)) {
            for (const poly of json.obstacles) {
                if (!Array.isArray(poly)) continue;
                const obs = new ObstaclePolygon();
                for (const v of poly) {
                    if (v && typeof v.x === 'number' && typeof v.y === 'number') {
                        obs.insertPolygonVertex(new PolygonVertex(v.x, v.y));
                    }
                }
                if (obs.polygonVertexListCcw?.length > 0) newEnv.obstaclePolygonList.push(obs);
            }
        }
        this.inputEnvironment = newEnv;
        this.canvasManager.inputEnvironment = newEnv;
        // Clear transient state
        this.canvasManager.currentObstacle = null;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.isDeletingObstacle = false;
        this.canvasManager.isDrawingBoundary = false;
        this.canvasManager.isDrawingObstacle = false;
        
        // Clear event markers and reset toggle state
        this.clearEventsOverlay();
        this.clearCellsOverlay();
        this.clearCoveragePathOverlay();
        
        // Update UI
        this.uiStateManager.resetAllButtons();
        this.updateDeleteModeUI();
        this.updateDisplay();
        this.canvasManager.draw();
    }

    runBCD() {
        // Cancel delete mode when exporting data
        if (this.isDeletingObstacle) {
            this.isDeletingObstacle = false;
            this.updateDeleteModeUI();
        }
        this.runBCDExport();
    }

    runBCDExport() {
        const data = this.inputEnvironment.json;
        this.dataService.exportToConsole(data);
        // Also send to server
        this.dataService.sendToServer(data).then((resp) => {
            let ok = resp && resp.status !== 'error';
            // Store events if present
            if (resp && Array.isArray(resp.event_list)) {
                this.canvasManager.setEvents(resp.event_list);
                // Enable events toggle
                const es = document.getElementById('toggleEvents');
                if (es) es.disabled = false;
            }
            // Store cells if present
            if (resp && Array.isArray(resp.cell_list)) {
                this.canvasManager.setCells(resp.cell_list);
                // Enable cells toggle
                const cs = document.getElementById('toggleCells');
                if (cs) cs.disabled = false;
            }
            // Store path list if present
            if (resp && Array.isArray(resp.path_list)) {
                this.canvasManager.setPathList(resp.path_list);
                // Enable coverage path toggle
                const ps = document.getElementById('toggleCoveragePath');
                if (ps) ps.disabled = false;
            }
            const msg = ok ? 'BCD run complete.' : 'BCD run failed.';
            this.canvasManager.showNotification(msg);
        });
    }

    handleResetView() {
        // Cancel delete mode when resetting view
        if (this.isDeletingObstacle) {
            this.isDeletingObstacle = false;
            this.updateDeleteModeUI();
        }
        this.canvasManager.resetView();
    }

    handleToggleGrid() {
        // Cancel delete mode when toggling grid
        if (this.isDeletingObstacle) {
            this.isDeletingObstacle = false;
            this.updateDeleteModeUI();
        }
        this.canvasManager.toggleGrid();
    }

    deleteBoundary() {
        // Cancel delete mode when deleting boundary
        if (this.isDeletingObstacle) {
            this.isDeletingObstacle = false;
            this.updateDeleteModeUI();
        }
        
        // Check if boundary exists
        const currentVertices = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
        if (currentVertices && currentVertices.length > 0) {
            // Clear the boundary
            this.inputEnvironment.boundaryPolygon.clear();
            
            // If currently drawing boundary, exit drawing mode
            if (this.isDrawingBoundary) {
                this.isDrawingBoundary = false;
                this.canvasManager.isDrawingBoundary = false;
                this.uiStateManager.updateDrawingModeUI(false, this.isDrawingObstacle, this.canvasManager.canvas);
            }

            // Clear any existing BCD events, since geometry changed
            this.clearEventsOverlay();
            this.clearCellsOverlay();
            this.clearCoveragePathOverlay();
            
            this.canvasManager.showNotification('Boundary deleted.');
            this.updateDisplay();
            this.canvasManager.draw();
        } else {
            this.canvasManager.showNotification('No boundary to delete.');
        }
    }

    cancelAllModes() {
        // Cancel all drawing and deletion modes
        if (this.isDrawingBoundary || this.isDrawingObstacle || this.isDeletingObstacle) {
            this.isDrawingBoundary = false;
            this.isDrawingObstacle = false;
            this.isDeletingObstacle = false;
            this.canvasManager.isDrawingBoundary = false;
            this.canvasManager.isDrawingObstacle = false;
            this.canvasManager.currentObstacle = null;
            
            // Update UI to reflect no active modes
            this.uiStateManager.updateDrawingModeUI(false, false, this.canvasManager.canvas);
            this.updateDeleteModeUI();
            
            this.updateDisplay();
            this.canvasManager.draw();
        }
    }

    toggleDrawingMode(mode) {
        // Automatically cancel delete mode when entering any drawing mode
        if (this.isDeletingObstacle) {
            this.isDeletingObstacle = false;
            this.updateDeleteModeUI();
        }
        
        if (mode === 'boundary') {
            if (this.isDrawingBoundary) {
                // Check if boundary has at least 3 points before stopping
                const currentVertices = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
                if (currentVertices && currentVertices.length >= 3) {
                    this.isDrawingBoundary = false;
                    this.canvasManager.isDrawingBoundary = false;
                } else if (currentVertices && currentVertices.length > 0) {
                    // User wants to cancel incomplete boundary
                    this.cancelCurrentBoundary();
                    this.canvasManager.showNotification('Boundary requires at least 3 points to complete.');
                    this.isDrawingBoundary = false;
                    this.canvasManager.isDrawingBoundary = false;
                } else {
                    // No points, just exit drawing mode
                    this.isDrawingBoundary = false;
                    this.canvasManager.isDrawingBoundary = false;
                }
            } else {
                this.isDrawingBoundary = true;
                this.canvasManager.isDrawingBoundary = true;
            }
            this.isDrawingObstacle = false;
            this.canvasManager.isDrawingObstacle = false;
        } else if (mode === 'obstacle') {
            // Button always finishes and exits obstacle drawing completely
            if (this.isDrawingObstacle) {
                if (this.canvasManager.currentObstacle && 
                    this.canvasManager.currentObstacle.polygonVertexListCcw?.length >= 3) {
                    // Finish current obstacle and exit drawing mode
                    this.finishCurrentObstacle();
                    this.isDrawingObstacle = false;
                } else if (this.canvasManager.currentObstacle && 
                           this.canvasManager.currentObstacle.polygonVertexListCcw?.length > 0) {
                    // Cancel incomplete obstacle and exit
                    this.cancelCurrentObstacle();
                    this.canvasManager.showNotification('Obstacle requires at least 3 points to complete.');
                    this.isDrawingObstacle = false;
                } else {
                    // No points, just exit drawing mode
                    this.isDrawingObstacle = false;
                }
            } else {
                // Start drawing obstacles
                this.isDrawingObstacle = true;
            }
            
            this.isDrawingBoundary = false;
            
            // Clear any existing current obstacle when exiting mode
            if (!this.isDrawingObstacle) {
                this.canvasManager.currentObstacle = null;
            }
        }
        
        // Sync state with canvas manager
        this.canvasManager.isDrawingBoundary = this.isDrawingBoundary;
        this.canvasManager.isDrawingObstacle = this.isDrawingObstacle;
        
        // Update UI using state manager
        this.uiStateManager.updateDrawingModeUI(
            this.isDrawingBoundary, 
            this.isDrawingObstacle, 
            this.canvasManager.canvas
        );
        
        this.updateDisplay();
        this.canvasManager.draw();
    }

    toggleDeleteMode() {
        this.isDeletingObstacle = !this.isDeletingObstacle;
        
        // Exit any drawing modes when entering delete mode
        if (this.isDeletingObstacle) {
            this.isDrawingBoundary = false;
            this.isDrawingObstacle = false;
            this.canvasManager.isDrawingBoundary = false;
            this.canvasManager.isDrawingObstacle = false;
            this.canvasManager.currentObstacle = null;
        }
        
        // Update UI state
        this.updateDeleteModeUI();
        this.updateDisplay();
        this.canvasManager.draw();
    }

    updateDeleteModeUI() {
        const deleteBtn = document.getElementById('deleteObstacle');
        if (deleteBtn) {
            if (this.isDeletingObstacle) {
                deleteBtn.classList.add('cancel');
                deleteBtn.classList.remove('active');
                deleteBtn.title = 'Cancel Delete Mode';
            } else {
                deleteBtn.classList.remove('cancel');
                deleteBtn.classList.remove('active');
                deleteBtn.title = 'Delete Obstacle';
            }
        }
        
        // Update canvas delete state
        this.uiStateManager.updateContainerDeleteState(this.isDeletingObstacle);
        
        // Reset other drawing buttons
        this.uiStateManager.updateDrawingModeUI(false, false, this.canvasManager.canvas);
        
        // Set cursor AFTER drawing UI update to prevent it from being overridden
        if (this.isDeletingObstacle) {
            this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, 'crosshair');
        }
    }

    clearCanvas() {
    // Destroy current environment and create a new one
    const newEnv = new InputEnvironment();
    this.inputEnvironment = newEnv;
    this.canvasManager.inputEnvironment = newEnv;
        this.canvasManager.currentObstacle = null;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.isDeletingObstacle = false;
        this.canvasManager.isDrawingBoundary = false;
        this.canvasManager.isDrawingObstacle = false;

        // Clear event markers and reset toggle state
        this.clearEventsOverlay();
        this.clearCellsOverlay();
        this.clearCoveragePathOverlay();
        
        // Reset UI using state manager
        this.uiStateManager.resetAllButtons();
        this.updateDeleteModeUI();
        this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, this.uiStateManager.CURSORS.DEFAULT);
        
    this.updateDisplay();
        this.canvasManager.draw();
    }


    handleMouseDown(e) {
        const mousePos = this.coordinateTransformer.getMousePositionFromEvent(e);
        const worldMeters = this.coordinateTransformer.screenToWorldMeters(mousePos.x, mousePos.y);
        
        // Allow panning with middle mouse button or while holding Space
        const panRequested = (e.button === 1) || this.isPanKeyActive;
        if (panRequested) {
            this.canvasManager.isDragging = true;
            this.canvasManager.lastMouseX = mousePos.x;
            this.canvasManager.lastMouseY = mousePos.y;
            this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, this.uiStateManager.CURSORS.GRABBING);
            return; // don't process draw/delete on pan start
        }

        if (this.isDeletingObstacle) {
            // Handle obstacle deletion
            this.handleObstacleDeletion(worldMeters);
        } else if (this.isDrawingBoundary || this.isDrawingObstacle) {
            // Unified drawing logic for both boundary and obstacles
            this.addVertexToCurrentPolygon(worldMeters, e.button);
            this.updateDisplay();
            this.canvasManager.draw();
        } else {
            // Only start dragging if not in any special mode
            this.canvasManager.isDragging = true;
            this.canvasManager.lastMouseX = mousePos.x;
            this.canvasManager.lastMouseY = mousePos.y;
            this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, this.uiStateManager.CURSORS.GRABBING);
        }
    }

    addVertexToCurrentPolygon(worldMeters, mouseButton) {
        if (this.isDrawingBoundary) {
            // Handle right click to finish boundary (if it has at least 3 points)
            if (mouseButton === 2) {
                const currentVertices = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
                if (currentVertices && currentVertices.length >= 3) {
                    // Finish boundary drawing
                    this.isDrawingBoundary = false;
                    this.canvasManager.isDrawingBoundary = false;
                    this.uiStateManager.updateDrawingModeUI(false, this.isDrawingObstacle, this.canvasManager.canvas);
                } else if (currentVertices && currentVertices.length > 0) {
                    // Cancel incomplete boundary
                    this.cancelCurrentBoundary();
                    this.canvasManager.showNotification('Boundary requires at least 3 points to complete.');
                    this.uiStateManager.updateDrawingModeUI(false, this.isDrawingObstacle, this.canvasManager.canvas);
                }
                return;
            }
            
            // Only add vertex on left click
            if (mouseButton === 0) {
                // Enforce unique x across all points
                const uniqueX = this.ensureUniqueX(worldMeters.x);
                const vertex = new PolygonVertex(uniqueX, worldMeters.y);
                this.inputEnvironment.boundaryPolygon.insertPolygonVertex(vertex);
            }
        } else if (this.isDrawingObstacle) {
            // Handle right click for obstacle: finish if ≥3 vertices and continue drawing, exit if <3 vertices
            if (mouseButton === 2) {
                if (this.canvasManager.currentObstacle && 
                    this.canvasManager.currentObstacle.polygonVertexListCcw?.length >= 3) {
                    // Finish current obstacle and continue drawing mode for next obstacle
                    this.finishCurrentObstacle();
                    this.canvasManager.currentObstacle = null; // Ready for next obstacle
                    // Stay in drawing mode (don't change this.isDrawingObstacle)
                } else if (this.canvasManager.currentObstacle && 
                           this.canvasManager.currentObstacle.polygonVertexListCcw?.length > 0) {
                    // Cancel incomplete obstacle and exit drawing mode
                    this.cancelCurrentObstacle();
                    this.canvasManager.showNotification('Obstacle drawing cancelled.');
                    this.isDrawingObstacle = false;
                    this.canvasManager.isDrawingObstacle = false;
                    this.uiStateManager.updateDrawingModeUI(this.isDrawingBoundary, false, this.canvasManager.canvas);
                } else {
                    // No vertices drawn, exit drawing mode
                    this.isDrawingObstacle = false;
                    this.canvasManager.isDrawingObstacle = false;
                    this.uiStateManager.updateDrawingModeUI(this.isDrawingBoundary, false, this.canvasManager.canvas);
                }
                return;
            }
            
            // Only add vertex on left click
            if (mouseButton === 0) {
                // Ensure we have a current obstacle
                if (!this.canvasManager.currentObstacle) {
                    this.canvasManager.currentObstacle = new ObstaclePolygon();
                }
                
                // Add vertex to current obstacle
                // Enforce unique x across all points
                const uniqueX = this.ensureUniqueX(worldMeters.x);
                const vertex = new PolygonVertex(uniqueX, worldMeters.y);
                this.canvasManager.currentObstacle.insertPolygonVertex(vertex);
            }
        }
    }

    handleObstacleDeletion(worldMeters) {
        // Find which obstacle was clicked (if any)
        const clickedObstacleIndex = this.findObstacleAtPoint(worldMeters);
        
        if (clickedObstacleIndex !== -1) {
            // Remove the obstacle
            this.inputEnvironment.obstaclePolygonList.splice(clickedObstacleIndex, 1);
            // Clear any existing BCD events, since geometry changed
            this.clearEventsOverlay();
            this.clearCellsOverlay();
            this.clearCoveragePathOverlay();
            this.canvasManager.showNotification('Obstacle deleted.');
            this.updateDisplay();
            this.canvasManager.draw();
        } else {
            this.canvasManager.showNotification('Click on an obstacle to delete it.');
        }
    }

    findObstacleAtPoint(worldMeters) {
        const tolerance = 0.5; // 0.5 meter tolerance for clicking
        
        for (let i = 0; i < this.inputEnvironment.obstaclePolygonList.length; i++) {
            const obstacle = this.inputEnvironment.obstaclePolygonList[i];
            const vertices = obstacle.polygonVertexListCcw;
            
            if (vertices && vertices.length >= 3) {
                // Check if point is inside the obstacle polygon
                if (this.isPointInPolygon(worldMeters, vertices)) {
                    return i;
                }
                
                // Also check if point is near any vertex (for easier clicking)
                for (const vertex of vertices) {
                    const distance = Math.sqrt(
                        Math.pow(worldMeters.x - vertex.x, 2) + 
                        Math.pow(worldMeters.y - vertex.y, 2)
                    );
                    if (distance <= tolerance) {
                        return i;
                    }
                }
            }
        }
        
        return -1; // No obstacle found
    }

    isPointInPolygon(point, vertices) {
        let inside = false;
        
        for (let i = 0, j = vertices.length - 1; i < vertices.length; j = i++) {
            const xi = vertices[i].x;
            const yi = vertices[i].y;
            const xj = vertices[j].x;
            const yj = vertices[j].y;
            
            if (((yi > point.y) !== (yj > point.y)) &&
                (point.x < (xj - xi) * (point.y - yi) / (yj - yi) + xi)) {
                inside = !inside;
            }
        }
        
        return inside;
    }

    handleMouseMove(e) {
        const mousePos = this.coordinateTransformer.getMousePositionFromEvent(e);
        const worldMetersFormatted = this.coordinateTransformer.eventToWorldMetersFormatted(e);
        
        // Update mouse coordinate display
        this.uiStateManager.updateMouseCoordinates(worldMetersFormatted.xFormatted, worldMetersFormatted.yFormatted);
        
    // Handle canvas panning (allowed in any mode when dragging)
    if (this.canvasManager.isDragging) {
            const deltaX = mousePos.x - this.canvasManager.lastMouseX;
            const deltaY = mousePos.y - this.canvasManager.lastMouseY;
            
            this.canvasManager.offsetX += deltaX;
            this.canvasManager.offsetY += deltaY;
            this.canvasManager.lastMouseX = mousePos.x;
            this.canvasManager.lastMouseY = mousePos.y;
            this.canvasManager.draw();
        }
    }
    handleMouseUp(e) {
        this.canvasManager.isDragging = false;
        let cursor;
        if (this.isDrawingBoundary || this.isDrawingObstacle) {
            cursor = this.uiStateManager.CURSORS.CROSSHAIR;
        } else if (this.isDeletingObstacle) {
            cursor = 'crosshair';
        } else {
            cursor = this.uiStateManager.CURSORS.DEFAULT;
        }
        this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, cursor);
    }
    handleWheel(e) {
        e.preventDefault();
        const mousePos = this.coordinateTransformer.getMousePositionFromEvent(e);
        const zoomFactor = e.deltaY > 0 ? 0.9 : 1.1;
        
        const zoomTransform = this.coordinateTransformer.calculateZoomTransform(
            mousePos.x, 
            mousePos.y, 
            zoomFactor, 
            this.canvasManager.scale
        );
        
        this.canvasManager.scale = zoomTransform.scale;
        this.canvasManager.offsetX = zoomTransform.offsetX;
        this.canvasManager.offsetY = zoomTransform.offsetY;
        
        this.updateDisplay();
        this.canvasManager.draw();
    }

    handleKeyDown(e) {
        // Handle ESC key to cancel current drawing or delete mode
        if (e.key === ' ' || e.code === 'Space') {
            // Space enables temporary pan mode
            this.isPanKeyActive = true;
            // Do not prevent default to allow scrolling prevention handled elsewhere
            return;
        }
        if (e.key === 'Escape' || e.keyCode === 27) {
            if (this.isDeletingObstacle) {
                // Exit delete mode
                this.isDeletingObstacle = false;
                this.updateDeleteModeUI();
                this.canvasManager.showNotification('Delete mode cancelled.');
            } else if (this.isDrawingBoundary) {
                const currentVertices = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
                if (currentVertices && currentVertices.length > 0) {
                    this.cancelCurrentBoundary();
                    this.canvasManager.showNotification('Boundary drawing cancelled.');
                    this.uiStateManager.updateDrawingModeUI(false, this.isDrawingObstacle, this.canvasManager.canvas);
                } else {
                    // No vertices, just exit drawing mode
                    this.isDrawingBoundary = false;
                    this.canvasManager.isDrawingBoundary = false;
                    this.uiStateManager.updateDrawingModeUI(false, this.isDrawingObstacle, this.canvasManager.canvas);
                }
                this.updateDisplay();
                this.canvasManager.draw();
            } else if (this.isDrawingObstacle) {
                if (this.canvasManager.currentObstacle && 
                    this.canvasManager.currentObstacle.polygonVertexListCcw?.length > 0) {
                    this.cancelCurrentObstacle();
                    this.canvasManager.showNotification('Obstacle drawing cancelled.');
                    this.uiStateManager.updateDrawingModeUI(this.isDrawingBoundary, false, this.canvasManager.canvas);
                } else {
                    // No vertices, just exit drawing mode
                    this.isDrawingObstacle = false;
                    this.canvasManager.isDrawingObstacle = false;
                    this.canvasManager.currentObstacle = null;
                    this.uiStateManager.updateDrawingModeUI(this.isDrawingBoundary, false, this.canvasManager.canvas);
                }
                this.updateDisplay();
                this.canvasManager.draw();
            }
            
            // Prevent default ESC behavior
            e.preventDefault();
        }
    }

    handleKeyUp(e) {
        if (e.key === ' ' || e.code === 'Space') {
            this.isPanKeyActive = false;
            // Restore cursor when not dragging
            if (!this.canvasManager.isDragging) {
                let cursor;
                if (this.isDrawingBoundary || this.isDrawingObstacle) cursor = this.uiStateManager.CURSORS.CROSSHAIR;
                else if (this.isDeletingObstacle) cursor = 'crosshair';
                else cursor = this.uiStateManager.CURSORS.DEFAULT;
                this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, cursor);
            }
        }
    }

    updateDisplay() {
        const boundaryVertices = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
        const boundaryPointsCount = boundaryVertices ? boundaryVertices.length : 0;
        
        this.uiStateManager.updateDisplayInfo(
            boundaryPointsCount, 
            this.canvasManager.scale
        );
        
        // Update obstacle count
        const obstacleCount = this.inputEnvironment.obstaclePolygonList.filter(obs => 
            obs.polygonVertexListCcw && obs.polygonVertexListCcw.length > 0
        ).length;
        this.uiStateManager.updateObstacleCount(obstacleCount);
    }
    finishCurrentObstacle() {
        if (this.canvasManager.currentObstacle && 
            this.canvasManager.currentObstacle.polygonVertexListCcw?.length > 0) {
            // Add the completed obstacle to the environment
            this.inputEnvironment.obstaclePolygonList.push(this.canvasManager.currentObstacle);
            this.canvasManager.currentObstacle = null;
        }
    }

    cancelCurrentBoundary() {
        // Clear the incomplete boundary without saving
        this.inputEnvironment.boundaryPolygon.clear();
        this.isDrawingBoundary = false;
        this.canvasManager.isDrawingBoundary = false;
    }

    cancelCurrentObstacle() {
        // Clear the incomplete obstacle without saving
        this.canvasManager.currentObstacle = null;
        this.isDrawingObstacle = false;
        this.canvasManager.isDrawingObstacle = false;
    }

    // Helper: clear BCD event overlay and disable toggle until next run
    clearEventsOverlay() {
        this.canvasManager.setEvents([]);
        this.canvasManager.showEvents = false;
        const eventsToggle = document.getElementById('toggleEvents');
        if (eventsToggle) {
            eventsToggle.checked = false;
            eventsToggle.disabled = true; // disabled until next export provides events
        }
    }

    // Helper: clear BCD cells overlay and disable toggle until next run
    clearCellsOverlay() {
        this.canvasManager.setCells([]);
        this.canvasManager.showCells = false;
        const cellsToggle = document.getElementById('toggleCells');
        if (cellsToggle) {
            cellsToggle.checked = false;
            cellsToggle.disabled = true; // disabled until next export provides cells
        }
    }

    // Helper: clear coverage path overlay and disable toggle until next run
    clearCoveragePathOverlay() {
        this.canvasManager.setPathList([]);
        this.canvasManager.showCoveragePath = false;
        const pathToggle = document.getElementById('toggleCoveragePath');
        if (pathToggle) {
            pathToggle.checked = false;
            pathToggle.disabled = true; // disabled until next export provides path
        }
    }

    // Ensure no two points in environment share the same x value.
    // If duplicate is found, move minimally to the right until unique.
    ensureUniqueX(x) {
        const EPS = 1e-6; // equality tolerance
        const STEP = 1e-4; // minimal right shift per attempt (meters)
        const xs = this.collectAllXValues();
        let candidate = x;
        let moved = false;
        let guard = 0;
        const isDup = (val) => xs.some(v => Math.abs(v - val) <= EPS);
        while (isDup(candidate) && guard < 10000) {
            candidate += STEP;
            moved = true;
            guard++;
        }
        if (moved) {
            // Per requirement, message text must be exactly this
            this.canvasManager.showNotification('Point moved to the right.');
        }
        return candidate;
    }

    // Collect all existing x values from boundary, all obstacles, and the current obstacle (if any)
    collectAllXValues() {
        const xs = [];
        const b = this.inputEnvironment?.boundaryPolygon?.polygonVertexListCw || [];
        for (const v of b) if (v && typeof v.x === 'number') xs.push(v.x);
        const obsList = this.inputEnvironment?.obstaclePolygonList || [];
        for (const obs of obsList) {
            const verts = obs?.polygonVertexListCcw || [];
            for (const v of verts) if (v && typeof v.x === 'number') xs.push(v.x);
        }
        const curr = this.canvasManager?.currentObstacle?.polygonVertexListCcw || [];
        for (const v of curr) if (v && typeof v.x === 'number') xs.push(v.x);
        return xs;
    }
}
