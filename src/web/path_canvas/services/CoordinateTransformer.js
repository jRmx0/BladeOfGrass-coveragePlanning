class CoordinateTransformer {
    constructor(canvasManager) {
        this.canvasManager = canvasManager;
    }

    screenToWorld(screenX, screenY) {
        const worldX = (screenX - this.canvasManager.offsetX) / this.canvasManager.scale;
        const worldY = (screenY - this.canvasManager.offsetY) / this.canvasManager.scale;
        return { x: worldX, y: worldY };
    }

    screenToWorldMeters(screenX, screenY, precision = 2) {
        const world = this.screenToWorld(screenX, screenY);
        const worldXMeters = parseFloat((world.x / this.canvasManager.pixelsPerMeter).toFixed(precision));
        const worldYMeters = parseFloat((world.y / this.canvasManager.pixelsPerMeter).toFixed(precision));
        return { x: worldXMeters, y: worldYMeters };
    }

    getMousePositionFromEvent(event) {
        const rect = this.canvasManager.canvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        return { x: mouseX, y: mouseY };
    }

    eventToWorldMeters(event, precision = 2) {
        const mousePos = this.getMousePositionFromEvent(event);
        return this.screenToWorldMeters(mousePos.x, mousePos.y, precision);
    }

    eventToWorldMetersFormatted(event, precision = 2) {
        const worldMeters = this.eventToWorldMeters(event, precision);
        return {
            x: worldMeters.x,
            y: worldMeters.y,
            xFormatted: worldMeters.x.toFixed(precision),
            yFormatted: worldMeters.y.toFixed(precision)
        };
    }

    calculateZoomTransform(mouseX, mouseY, zoomFactor, currentScale) {
        const minScale = 0.1;
        const maxScale = 5.0;
        
        const newScale = Math.max(minScale, Math.min(maxScale, currentScale * zoomFactor));
        
        // Calculate world position before zoom
        const worldX = (mouseX - this.canvasManager.offsetX) / currentScale;
        const worldY = (mouseY - this.canvasManager.offsetY) / currentScale;
        
        // Calculate new offsets to keep mouse position constant in world space
        const newOffsetX = mouseX - worldX * newScale;
        const newOffsetY = mouseY - worldY * newScale;
        
        return {
            scale: newScale,
            offsetX: newOffsetX,
            offsetY: newOffsetY
        };
    }
}
