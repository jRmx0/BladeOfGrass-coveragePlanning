class UIStateManager {
    constructor() {
        // UI Constants
        this.COLORS = {
            ACTIVE: '#e74c3c',
            INACTIVE: '#3498db'
        };
        
        this.BUTTON_TEXT = {
            DRAW_BOUNDARY: 'Draw Boundary',
            DRAW_OBSTACLE: 'Draw Obstacle',
            STOP_DRAWING: 'Stop Drawing'
        };
        
        this.CURSORS = {
            DEFAULT: 'default',
            CROSSHAIR: 'crosshair',
            GRABBING: 'grabbing'
        };
        
        this.CSS_CLASSES = {
            DRAWING: 'drawing'
        };
    }

    updateButtonState(elementId, isActive, activeText, inactiveText) {
        const button = document.getElementById(elementId);
        if (!button) return;
        
        button.textContent = isActive ? activeText : inactiveText;
        button.style.backgroundColor = isActive ? this.COLORS.ACTIVE : this.COLORS.INACTIVE;
    }

    updateBoundaryButton(isDrawing) {
        this.updateButtonState(
            'drawBoundary', 
            isDrawing, 
            this.BUTTON_TEXT.STOP_DRAWING, 
            this.BUTTON_TEXT.DRAW_BOUNDARY
        );
    }

    updateObstacleButton(isDrawing) {
        this.updateButtonState(
            'drawObstacle', 
            isDrawing, 
            this.BUTTON_TEXT.STOP_DRAWING, 
            this.BUTTON_TEXT.DRAW_OBSTACLE
        );
    }

    updateCanvasCursor(canvas, cursor) {
        if (canvas && canvas.style) {
            canvas.style.cursor = cursor;
        }
    }

    updateContainerDrawingState(isDrawing) {
        const container = document.querySelector('.canvas-container');
        if (!container) return;
        
        if (isDrawing) {
            container.classList.add(this.CSS_CLASSES.DRAWING);
        } else {
            container.classList.remove(this.CSS_CLASSES.DRAWING);
        }
    }

    resetAllButtons() {
        this.updateBoundaryButton(false);
        this.updateObstacleButton(false);
        this.updateContainerDrawingState(false);
    }

    updateDrawingModeUI(isDrawingBoundary, isDrawingObstacle, canvas) {
        this.updateBoundaryButton(isDrawingBoundary);
        this.updateObstacleButton(isDrawingObstacle);
        
        const isAnyDrawing = isDrawingBoundary || isDrawingObstacle;
        this.updateContainerDrawingState(isAnyDrawing);
        this.updateCanvasCursor(canvas, isAnyDrawing ? this.CURSORS.CROSSHAIR : this.CURSORS.DEFAULT);
    }

    updateDisplayInfo(boundaryPointsCount, zoomLevel) {
        const boundaryElement = document.getElementById('boundaryPoints');
        const zoomElement = document.getElementById('zoomLevel');
        
        if (boundaryElement) {
            boundaryElement.textContent = boundaryPointsCount;
        }
        
        if (zoomElement) {
            zoomElement.textContent = Math.round(zoomLevel * 100) + '%';
        }
    }

    updateMouseCoordinates(worldXMeters, worldYMeters) {
        const coordsElement = document.getElementById('canvasCoords');
        if (coordsElement) {
            coordsElement.textContent = `Mouse: (${worldXMeters}, ${worldYMeters}) m`;
        }
    }

    updateObstacleCount(obstacleCount) {
        const element = document.getElementById('obstacleCount');
        if (element) {
            element.textContent = obstacleCount;
        }
    }

    setAlgorithmStatus(status, show = true) {
        const statusContainer = document.getElementById('algorithmStatus');
        const statusText = document.getElementById('algorithmStatusText');
        
        if (statusContainer) {
            statusContainer.style.display = show ? 'flex' : 'none';
        }
        
        if (statusText) {
            statusText.textContent = status;
        }
    }

    setButtonState(buttonId, state) {
        const button = document.getElementById(buttonId);
        if (!button) return;
        
        // Remove all state classes
        button.classList.remove('loading', 'success', 'secondary', 'active');
        
        switch (state) {
            case 'loading':
                button.classList.add('loading');
                button.disabled = true;
                break;
            case 'success':
                button.classList.add('success');
                button.disabled = false;
                setTimeout(() => {
                    button.classList.remove('success');
                }, 2000);
                break;
            case 'secondary':
                button.classList.add('secondary');
                button.disabled = false;
                break;
            case 'active':
                button.classList.add('active');
                button.disabled = false;
                break;
            default:
                button.disabled = false;
        }
    }

}
