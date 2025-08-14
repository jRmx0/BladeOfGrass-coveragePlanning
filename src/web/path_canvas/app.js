document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('pathCanvas');
    const inputEnvironment = new InputEnvironment();
    const canvasManager = new CanvasManager(canvas, inputEnvironment);
    const uiController = new UIController(inputEnvironment, canvasManager);
});