document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('pathCanvas');
    const boundaryModel = new BoundaryModel();
    const obstacleModel = new ObstacleModel();
    const canvasManager = new CanvasManager(canvas, boundaryModel, obstacleModel);
    new UIController(boundaryModel, obstacleModel, canvasManager);
    new TestingController(boundaryModel, obstacleModel, canvasManager);
});