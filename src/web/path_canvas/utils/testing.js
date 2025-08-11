class TestingController {
    constructor(inputEnvironment, canvasManager, uiController) {
        this.inputEnvironment = inputEnvironment;
        this.canvasManager = canvasManager;
        this.uiController = uiController;
        this.initUI();
    }

    initUI() {
        document.getElementById('drawRectangle').addEventListener('click', () => {
            // Cancel any active drawing/deletion modes before running test
            if (this.uiController && this.uiController.cancelAllModes) {
                this.uiController.cancelAllModes();
            }
            this.drawRectangle();
        });
    }

    // rectangle boundary with diamond obstacle in the middle
    drawRectangle() {
    // Always use the current environment (in case it was replaced)
    const env = this.canvasManager?.inputEnvironment || this.inputEnvironment;
    this.inputEnvironment = env;
    // Clear boundary and obstacles on the current environment
    env.clear();
        
        const rectanglePoints = [
            { x: -3, y: -2 },
            { x: 3, y: -2 },
            { x: 3, y: 2 },
            { x: -3, y: 2 }
        ];
        
        // Add boundary vertices using the new structure
        rectanglePoints.forEach(point => {
            const vertex = new PolygonVertex(point.x, point.y);
            env.boundaryPolygon.insertPolygonVertex(vertex);
        });

        // Create and add obstacle
        const obstaclePolygon = new ObstaclePolygon();
        const obstaclePoints = [
            { x: 0, y: -1 },
            { x: 1, y: 0 },
            { x: 0, y: 1 },
            { x: -1, y: 0 }
        ];
        
        obstaclePoints.forEach(point => {
            const vertex = new PolygonVertex(point.x, point.y);
            obstaclePolygon.insertPolygonVertex(vertex);
        });
        
    env.obstaclePolygonList.push(obstaclePolygon);
        
        // Update display through UIController to refresh all UI elements
        if (this.uiController && this.uiController.updateDisplay) {
            this.uiController.updateDisplay();
        }
        
    this.canvasManager.draw();
    }
}
