class TestingController {
    constructor(boundaryModel, obstacleModel, canvasManager, uiController) {
        this.boundaryModel = boundaryModel;
        this.obstacleModel = obstacleModel;
        this.canvasManager = canvasManager;
        this.uiController = uiController;
        this.initUI();
    }

    initUI() {
        document.getElementById('drawRectangle').addEventListener('click', () => this.drawRectangle());
        document.getElementById('drawPredefinedCells').addEventListener('click', () => this.drawPredefinedCells());
    }

    // rectangle boundary with diamond obstacle in the middle
    drawRectangle() {
        // Clear existing cells first
        this.canvasManager.cells = [];
        
        // Clear boundary and obstacles
        this.boundaryModel.clear();
        this.obstacleModel.clear();
        
        const rectanglePoints = [
            { x: -3, y: -2 },
            { x: 3, y: -2 },
            { x: 3, y: 2 },
            { x: -3, y: 2 }
        ];
        rectanglePoints.forEach(point => this.boundaryModel.boundaryPoints.push(point));

        this.obstacleModel.startNewObstacle();
        this.obstacleModel.addPointToCurrentObstacle({ x: 0, y: -1 });
        this.obstacleModel.addPointToCurrentObstacle({ x: 1, y: 0 });
        this.obstacleModel.addPointToCurrentObstacle({ x: 0, y: 1 });
        this.obstacleModel.addPointToCurrentObstacle({ x: -1, y: 0 });
        
        // Update display through UIController to refresh all UI elements
        if (this.uiController && this.uiController.updateDisplay) {
            this.uiController.updateDisplay();
        }
        
        this.canvasManager.draw();
    }

    // Draw predefined cells for testing visualization
    drawPredefinedCells() {
        // Clear existing cells first
        this.canvasManager.cells = [];
        
        // Define some sample cells for testing
        const predefinedCells = [
            [
                { x: -3, y: -2 },
                { x: -3, y: 2 },
                { x: -1, y: 2 },
                { x: -1, y: -2 }
            ],
            [
                { x: -1, y: 0 },
                { x: 0, y: -1 },
                { x: 1, y: 0 },
                { x: 1, y: -2 },
                { x: -1, y: -2 }
            ],
            [
                { x: -1, y: 0 },
                { x: -1, y: 2 },
                { x: 1, y: 2 },
                { x: 1, y: 0 },
                { x: 0, y: 1 }
            ],
            [
                { x: 1, y: -2 },
                { x: 1, y: 2 },
                { x: 3, y: 2 },
                { x: 3, y: -2 }
            ]
        ];
        
        // Set the cells in canvas manager
        this.canvasManager.cells = predefinedCells;
        
        // Update display through UIController to refresh all UI elements
        if (this.uiController && this.uiController.updateDisplay) {
            this.uiController.updateDisplay();
        }
        
        this.canvasManager.draw();
    }
}
