class UIController {
    constructor(inputEnvironment, canvasManager) {
        this.inputEnvironment = inputEnvironment;
        this.canvasManager = canvasManager;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        
        // Initialize service dependencies
        this.dataService = new DataService();
        this.uiStateManager = new UIStateManager();
        this.coordinateTransformer = new CoordinateTransformer(canvasManager);
        
        this.initUI();
    }
    initUI() {
        document.getElementById('drawBoundary').addEventListener('click', () => this.toggleDrawingMode('boundary'));
        // Add obstacle button if present
        const obstacleBtn = document.getElementById('drawObstacle');
        if (obstacleBtn) {
            obstacleBtn.addEventListener('click', () => this.toggleDrawingMode('obstacle'));
        }
        document.getElementById('clearCanvas').addEventListener('click', () => this.clearCanvas());
        document.getElementById('resetView').addEventListener('click', () => this.canvasManager.resetView());
        document.getElementById('toggleGrid').addEventListener('click', () => this.canvasManager.toggleGrid());
        document.getElementById('exportData').addEventListener('click', () => this.exportData());
        document.getElementById('pathWidth').addEventListener('change', (e) => {
            this.inputEnvironment.pathWidth = parseFloat(e.target.value);
            this.updateDisplay();
        });
        document.getElementById('pathOverlap').addEventListener('change', (e) => {
            this.inputEnvironment.pathOverlap = parseFloat(e.target.value);
            this.updateDisplay();
        });
        this.canvasManager.canvas.addEventListener('mousedown', (e) => this.handleMouseDown(e));
        this.canvasManager.canvas.addEventListener('mousemove', (e) => this.handleMouseMove(e));
        this.canvasManager.canvas.addEventListener('mouseup', (e) => this.handleMouseUp(e));
        this.canvasManager.canvas.addEventListener('wheel', (e) => this.handleWheel(e));
        this.canvasManager.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        this.canvasManager.canvas.addEventListener('dragstart', (e) => e.preventDefault());
        
        // Add keyboard event listener for ESC key
        document.addEventListener('keydown', (e) => this.handleKeyDown(e));
        
        this.updateDisplay();
        this.canvasManager.draw();
    }

    exportData() {
        this.exportEnvironmentToConsole();
    }

    exportEnvironmentToConsole() {
        const data = this.inputEnvironment.json;
        this.dataService.exportToConsole(data);
    }
    toggleDrawingMode(mode) {
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
            // If stopping obstacle drawing, finish the current obstacle first
            if (this.isDrawingObstacle) {
                if (this.canvasManager.currentObstacle && 
                    this.canvasManager.currentObstacle.polygonVertexListCcw?.length >= 3) {
                    this.finishCurrentObstacle();
                } else if (this.canvasManager.currentObstacle && 
                           this.canvasManager.currentObstacle.polygonVertexListCcw?.length > 0) {
                    // User wants to cancel incomplete obstacle
                    this.cancelCurrentObstacle();
                    this.canvasManager.showNotification('Obstacle requires at least 3 points to complete.');
                    this.isDrawingObstacle = false;
                } else {
                    // No points, just exit drawing mode
                    this.isDrawingObstacle = false;
                }
            } else {
                this.isDrawingObstacle = true;
            }
            
            this.isDrawingBoundary = false;
            
            // Clear any existing current obstacle when toggling mode off
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
    clearCanvas() {
        this.inputEnvironment.clear();
        this.canvasManager.currentObstacle = null;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.canvasManager.isDrawingBoundary = false;
        this.canvasManager.isDrawingObstacle = false;
        
        // Reset UI using state manager
        this.uiStateManager.resetAllButtons();
        this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, this.uiStateManager.CURSORS.DEFAULT);
        
        this.updateDisplay();
        this.canvasManager.draw();
    }


    handleMouseDown(e) {
        const mousePos = this.coordinateTransformer.getMousePositionFromEvent(e);
        const worldMeters = this.coordinateTransformer.screenToWorldMeters(mousePos.x, mousePos.y);
        
        if (this.isDrawingBoundary || this.isDrawingObstacle) {
            // Unified drawing logic for both boundary and obstacles
            this.addVertexToCurrentPolygon(worldMeters, e.button);
            this.updateDisplay();
            this.canvasManager.draw();
        } else {
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
                const vertex = new PolygonVertex(worldMeters.x, worldMeters.y);
                this.inputEnvironment.boundaryPolygon.insertPolygonVertex(vertex);
            }
        } else if (this.isDrawingObstacle) {
            // Handle right click to finish obstacle (if it has at least 3 points)
            if (mouseButton === 2) {
                if (this.canvasManager.currentObstacle && 
                    this.canvasManager.currentObstacle.polygonVertexListCcw?.length >= 3) {
                    this.finishCurrentObstacle();
                    this.isDrawingObstacle = false;
                    this.canvasManager.isDrawingObstacle = false;
                    this.uiStateManager.updateDrawingModeUI(this.isDrawingBoundary, false, this.canvasManager.canvas);
                } else if (this.canvasManager.currentObstacle && 
                           this.canvasManager.currentObstacle.polygonVertexListCcw?.length > 0) {
                    // Cancel incomplete obstacle
                    this.cancelCurrentObstacle();
                    this.canvasManager.showNotification('Obstacle requires at least 3 points to complete.');
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
                const vertex = new PolygonVertex(worldMeters.x, worldMeters.y);
                this.canvasManager.currentObstacle.insertPolygonVertex(vertex);
            }
        }
    }
    handleMouseMove(e) {
        const mousePos = this.coordinateTransformer.getMousePositionFromEvent(e);
        const worldMetersFormatted = this.coordinateTransformer.eventToWorldMetersFormatted(e);
        
        // Update mouse coordinate display
        this.uiStateManager.updateMouseCoordinates(worldMetersFormatted.xFormatted, worldMetersFormatted.yFormatted);
        
        // Handle canvas panning
        if (this.canvasManager.isDragging && !this.isDrawingBoundary && !this.isDrawingObstacle) {
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
        const cursor = (this.isDrawingBoundary || this.isDrawingObstacle) ? 
            this.uiStateManager.CURSORS.CROSSHAIR : 
            this.uiStateManager.CURSORS.DEFAULT;
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
        // Handle ESC key to cancel current drawing
        if (e.key === 'Escape' || e.keyCode === 27) {
            if (this.isDrawingBoundary) {
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
}
