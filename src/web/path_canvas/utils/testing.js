class TestingController {
    constructor(boundaryModel, obstacleModel, canvasManager) {
        this.boundaryModel = boundaryModel;
        this.obstacleModel = obstacleModel;
        this.canvasManager = canvasManager;
        this.initUI();
    }

    initUI() {
        document.getElementById('drawRectangle').addEventListener('click', () => this.drawRectangle());
    }

    // rectangle boundary with diamond obstacle in the middle
    drawRectangle() {
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
        
        this.canvasManager.draw();
    }
}
