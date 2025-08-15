document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('pathCanvas');
    const inputEnvironment = new InputEnvironment();
    const canvasManager = new CanvasManager(canvas, inputEnvironment);
    const uiController = new UIController(inputEnvironment, canvasManager);
    
    // Right panel collapse functionality
    const rightPanel = document.getElementById('rightPanel');
    const collapseBtn = document.getElementById('collapseRightPanel');
    
    if (rightPanel && collapseBtn) {
        collapseBtn.addEventListener('click', () => {
            rightPanel.classList.toggle('collapsed');
            
            // Update button text/icon based on state
            if (rightPanel.classList.contains('collapsed')) {
                collapseBtn.textContent = '→';
                collapseBtn.title = 'Expand Panel';
            } else {
                collapseBtn.textContent = '←';
                collapseBtn.title = 'Collapse Panel';
            }
            
            // Update canvas after panel transition completes
            setTimeout(() => {
                canvasManager.resizeCanvas();
            }, 300); // Match the CSS transition duration
        });
    }
});