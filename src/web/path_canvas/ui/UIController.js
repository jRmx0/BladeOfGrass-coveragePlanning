class UIController {
    constructor(boundaryModel, obstacleModel, canvasManager) {
        this.boundaryModel = boundaryModel;
        this.obstacleModel = obstacleModel;
        this.canvasManager = canvasManager;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        
        // Initialize service dependencies
        this.dataService = new DataService();
        this.uiStateManager = new UIStateManager();
        this.coordinateTransformer = new CoordinateTransformer(canvasManager);
        this.algorithmService = new AlgorithmService(this.dataService);
        
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
        document.getElementById('clearCells').addEventListener('click', () => this.clearCellsOnly());
        document.getElementById('resetView').addEventListener('click', () => this.canvasManager.resetView());
        document.getElementById('toggleGrid').addEventListener('click', () => this.canvasManager.toggleGrid());
        document.getElementById('showCells').addEventListener('change', (e) => this.toggleCellVisibility(e.target.checked));
        document.getElementById('showCellNumbers').addEventListener('change', (e) => this.toggleCellNumbers(e.target.checked));
        document.getElementById('exportData').addEventListener('click', () => this.exportData());
        document.getElementById('pathWidth').addEventListener('change', (e) => {
            this.boundaryModel.pathWidth = parseFloat(e.target.value);
            this.updateDisplay();
        });
        document.getElementById('pathOverlap').addEventListener('change', (e) => {
            this.boundaryModel.pathOverlap = parseFloat(e.target.value);
            this.updateDisplay();
        });
        this.canvasManager.canvas.addEventListener('mousedown', (e) => this.handleMouseDown(e));
        this.canvasManager.canvas.addEventListener('mousemove', (e) => this.handleMouseMove(e));
        this.canvasManager.canvas.addEventListener('mouseup', (e) => this.handleMouseUp(e));
        this.canvasManager.canvas.addEventListener('wheel', (e) => this.handleWheel(e));
        this.canvasManager.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        this.canvasManager.canvas.addEventListener('dragstart', (e) => e.preventDefault());
        this.updateDisplay();
        this.canvasManager.draw();
        document.getElementById('boustrophedonCellularDecomposition').addEventListener('click', () => this.runBoustrophedon());
        
        // Initialize checkbox state
        document.getElementById('showCells').checked = this.canvasManager.showCells;
        document.getElementById('showCellNumbers').checked = this.canvasManager.showCellNumbers;
        
        // Initialize clear cells button state
        this.uiStateManager.updateClearCellsButton(this.canvasManager.cells.length > 0);
    }

    async exportData() {
        try {
            await this.algorithmService.exportModelAndDecomposition(this.boundaryModel, this.obstacleModel);
        } catch (error) {
            console.error('Export failed:', error);
        }
    }

    exportModelToConsole() {
        const data = this.algorithmService.createModelData(this.boundaryModel, this.obstacleModel);
        this.dataService.exportToConsole(data);
    }
    toggleDrawingMode(mode) {
        if (mode === 'boundary') {
            this.isDrawingBoundary = !this.isDrawingBoundary;
            this.isDrawingObstacle = false;
        } else if (mode === 'obstacle') {
            this.isDrawingObstacle = !this.isDrawingObstacle;
            this.isDrawingBoundary = false;
            if (this.isDrawingObstacle) this.obstacleModel.startNewObstacle();
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
    }
    clearCanvas() {
        this.boundaryModel.clear();
        this.obstacleModel.clear();
        this.canvasManager.cells = [];
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

    clearCellsOnly() {
        this.canvasManager.clearCellsOnly();
        this.updateDisplay();
    }

    toggleCellVisibility(show) {
        this.canvasManager.showCells = show;
        this.canvasManager.draw();
    }

    toggleCellNumbers(show) {
        this.canvasManager.showCellNumbers = show;
        this.canvasManager.draw();
    }

    calculateCoverageArea() {
        // Simple approximation based on cell count and average cell size
        // This could be improved with actual polygon area calculation
        const cellCount = this.canvasManager.cells.length;
        const estimatedAreaPerCell = 1.5; // m² per cell (rough estimate)
        return cellCount * estimatedAreaPerCell;
    }
    handleMouseDown(e) {
        const mousePos = this.coordinateTransformer.getMousePositionFromEvent(e);
        const worldMeters = this.coordinateTransformer.screenToWorldMeters(mousePos.x, mousePos.y);
        
        if (this.isDrawingBoundary) {
            this.boundaryModel.boundaryPoints.push({ x: worldMeters.x, y: worldMeters.y });
            this.updateDisplay();
            this.canvasManager.draw();
        } else if (this.isDrawingObstacle) {
            this.obstacleModel.addPointToCurrentObstacle({ x: worldMeters.x, y: worldMeters.y });
            this.updateDisplay();
            this.canvasManager.draw();
        } else {
            this.canvasManager.isDragging = true;
            this.canvasManager.lastMouseX = mousePos.x;
            this.canvasManager.lastMouseY = mousePos.y;
            this.uiStateManager.updateCanvasCursor(this.canvasManager.canvas, this.uiStateManager.CURSORS.GRABBING);
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
    updateDisplay() {
        this.uiStateManager.updateDisplayInfo(
            this.boundaryModel.boundaryPoints.length, 
            this.canvasManager.scale
        );
        
        // Update cell information
        const cellCount = this.canvasManager.cells.length;
        const coverageArea = this.calculateCoverageArea();
        this.uiStateManager.updateCellInfo(cellCount, coverageArea);
        
        // Update clear cells button state based on whether cells exist
        this.uiStateManager.updateClearCellsButton(cellCount > 0);
        
        // Update obstacle count
        const obstacleCount = this.obstacleModel.obstacles.filter(obs => obs.length > 0).length;
        this.uiStateManager.updateObstacleCount(obstacleCount);
    }
    
    runBoustrophedon() {
        // Validate that we have boundary points
        if (this.boundaryModel.boundaryPoints.length < 3) {
            alert('Please draw a boundary with at least 3 points before generating path cells.');
            return;
        }

        try {
            // Set loading state
            this.uiStateManager.setButtonState('boustrophedonCellularDecomposition', 'loading');
            this.uiStateManager.setAlgorithmStatus('Generating path cells...', true);
            
            // Use setTimeout to allow UI to update before running algorithm
            setTimeout(() => {
                try {
                    this.algorithmService.processAndVisualizeBoustrophedon(
                        this.boundaryModel, 
                        this.obstacleModel, 
                        this.canvasManager
                    );
                    
                    // Success state
                    this.uiStateManager.setButtonState('boustrophedonCellularDecomposition', 'success');
                    this.uiStateManager.setAlgorithmStatus(`Generated ${this.canvasManager.cells.length} path cells`, true);
                    
                    // Update display with new cell information
                    this.updateDisplay();
                    
                    // Hide status after 3 seconds
                    setTimeout(() => {
                        this.uiStateManager.setAlgorithmStatus('', false);
                    }, 3000);
                    
                } catch (error) {
                    console.error('Boustrophedon algorithm failed:', error);
                    this.uiStateManager.setButtonState('boustrophedonCellularDecomposition', 'normal');
                    this.uiStateManager.setAlgorithmStatus('Error: Failed to generate path cells', true);
                    alert('Failed to generate path cells. Please check your boundary and try again.');
                    
                    // Hide error status after 5 seconds
                    setTimeout(() => {
                        this.uiStateManager.setAlgorithmStatus('', false);
                    }, 5000);
                }
            }, 100);
            
        } catch (error) {
            console.error('Boustrophedon algorithm failed:', error);
            this.uiStateManager.setButtonState('boustrophedonCellularDecomposition', 'normal');
            this.uiStateManager.setAlgorithmStatus('Error: Failed to generate path cells', true);
            alert('Failed to generate path cells. Please check your boundary and try again.');
        }
    }

    getModelData() {
        return this.algorithmService.createModelData(this.boundaryModel, this.obstacleModel);
    }
}
