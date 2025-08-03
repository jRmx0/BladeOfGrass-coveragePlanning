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
        document.getElementById('resetView').addEventListener('click', () => this.canvasManager.resetView());
        document.getElementById('toggleGrid').addEventListener('click', () => this.canvasManager.toggleGrid());
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
    }
    
    runBoustrophedon() {
        try {
            this.algorithmService.processAndVisualizeBoustrophedon(
                this.boundaryModel, 
                this.obstacleModel, 
                this.canvasManager
            );
        } catch (error) {
            console.error('Boustrophedon algorithm failed:', error);
        }
    }

    getModelData() {
        return this.algorithmService.createModelData(this.boundaryModel, this.obstacleModel);
    }
}
