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

    drawRectangle() {
        this.boundaryModel.clear();
        this.obstacleModel.clear();
        this.boundaryModel.boundaryPoints.push({ x: -3, y: -2 });
        this.boundaryModel.boundaryPoints.push({ x: 3, y: -2 });
        this.boundaryModel.boundaryPoints.push({ x: 3, y: 2 });
        this.boundaryModel.boundaryPoints.push({ x: -3, y: 2 });

        this.obstacleModel.startNewObstacle();
        this.obstacleModel.addPointToCurrentObstacle({ x: 0, y: -1 });
        this.obstacleModel.addPointToCurrentObstacle({ x: 1, y: 0 });
        this.obstacleModel.addPointToCurrentObstacle({ x: 0, y: 1 });
        this.obstacleModel.addPointToCurrentObstacle({ x: -1, y: 0 });
        
        this.canvasManager.draw();
    }
}
