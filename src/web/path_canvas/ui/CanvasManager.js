class CanvasManager {
    constructor(canvas, inputEnvironment) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.inputEnvironment = inputEnvironment;
        this.scale = 1;
        this.offsetX = 0;
        this.offsetY = 0;
        this.lastMouseX = 0;
        this.lastMouseY = 0;
        this.isDragging = false;
        this.showGrid = true;
        this.pixelsPerMeter = 100;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.currentObstacle = null; // Track current obstacle being drawn
    this.showVertexNumbers = false; // debug toggle
    this.vertexScale = 1; // scales point radius when showVertexNumbers is on
        
        // Notification system
        this.notification = null;
        this.notificationTimer = null;
        
        this.setupCanvas();
    }
    setupCanvas() {
        const container = this.canvas.parentElement;
        const containerRect = container.getBoundingClientRect();
        this.canvas.width = containerRect.width - 20;
        this.canvas.height = containerRect.height - 20;
        this.offsetX = this.canvas.width / 2;
        this.offsetY = this.canvas.height / 2;
        window.addEventListener('resize', () => this.resizeCanvas());
    }
    resizeCanvas() {
        const container = this.canvas.parentElement;
        const containerRect = container.getBoundingClientRect();
        this.canvas.width = containerRect.width - 20;
        this.canvas.height = containerRect.height - 20;
        this.draw();
    }
    draw() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.save();
        this.ctx.translate(this.offsetX, this.offsetY);
        this.ctx.scale(this.scale, this.scale);
        if (this.showGrid) this.drawGrid();
        
        const boundaryVertices = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
        if (boundaryVertices && boundaryVertices.length > 0) this.drawBoundary();
        
        if (this.inputEnvironment.obstaclePolygonList.length > 0) this.drawObstacles();
        
        // Draw current obstacle being built
        if (this.currentObstacle) this.drawCurrentObstacle();
        
        // Draw notification if present
        if (this.notification) this.drawNotification();
        
        this.ctx.restore();
    }

    drawObstacles() {
        for (const obstaclePolygon of this.inputEnvironment.obstaclePolygonList) {
            const vertices = obstaclePolygon.polygonVertexListCcw;
            if (!vertices || vertices.length === 0) continue;
            
            // Draw obstacle polygon
            if (vertices.length >= 2) {
                this.ctx.strokeStyle = '#e74c3c';
                this.ctx.lineWidth = 2 / this.scale;
                this.ctx.beginPath();
                this.ctx.moveTo(vertices[0].x * this.pixelsPerMeter, vertices[0].y * this.pixelsPerMeter);
                for (let i = 1; i < vertices.length; i++) {
                    this.ctx.lineTo(vertices[i].x * this.pixelsPerMeter, vertices[i].y * this.pixelsPerMeter);
                }
                if (vertices.length > 2) {
                    this.ctx.closePath();
                    this.ctx.fillStyle = 'rgba(231, 76, 60, 0.15)';
                    this.ctx.fill();
                }
                this.ctx.stroke();
            }
            // Draw obstacle points
            this.ctx.fillStyle = '#c0392b';
            const r = this.getPointRadius(false);
            for (const point of vertices) {
                this.ctx.beginPath();
                this.ctx.arc(point.x * this.pixelsPerMeter, point.y * this.pixelsPerMeter, r, 0, Math.PI * 2);
                this.ctx.fill();
            }

            // Draw obstacle vertex numbers (if enabled)
            if (this.showVertexNumbers) this.drawVertexLabels(vertices, '#c0392b');
        }
    }

    drawCurrentObstacle() {
        if (!this.currentObstacle) return;
        
        const vertices = this.currentObstacle.polygonVertexListCcw;
        if (!vertices || vertices.length === 0) return;
        
        // Draw current obstacle with same style as boundary but in red
        if (vertices.length >= 2) {
            this.ctx.strokeStyle = '#e74c3c'; // Same red as finished obstacles
            this.ctx.lineWidth = 3 / this.scale; // Same thickness as boundary
            this.ctx.beginPath();
            this.ctx.moveTo(vertices[0].x * this.pixelsPerMeter, vertices[0].y * this.pixelsPerMeter);
            for (let i = 1; i < vertices.length; i++) {
                this.ctx.lineTo(vertices[i].x * this.pixelsPerMeter, vertices[i].y * this.pixelsPerMeter);
            }
            // Close the polygon if we have 3+ vertices (preview of final shape)
            if (vertices.length > 2) {
                this.ctx.closePath();
                this.ctx.fillStyle = 'rgba(231, 76, 60, 0.15)'; // Same fill as finished obstacles
                this.ctx.fill();
            }
            this.ctx.stroke();
        }
        
        // Draw current obstacle points with same style as boundary
        this.ctx.fillStyle = '#c0392b';
        const r = this.getPointRadius(true);
        for (const point of vertices) {
            this.ctx.beginPath();
            this.ctx.arc(point.x * this.pixelsPerMeter, point.y * this.pixelsPerMeter, r, 0, Math.PI * 2);
            this.ctx.fill();
        }

        // Draw current obstacle vertex numbers (if enabled)
        if (this.showVertexNumbers) this.drawVertexLabels(vertices, '#c0392b');
    }

    drawGrid() {
        const gridSize = 50;
        const bounds = {
            left: Math.floor((-this.offsetX / this.scale) / gridSize) * gridSize,
            top: Math.floor((-this.offsetY / this.scale) / gridSize) * gridSize,
            right: Math.ceil((this.canvas.width - this.offsetX) / this.scale / gridSize) * gridSize,
            bottom: Math.ceil((this.canvas.height - this.offsetY) / this.scale / gridSize) * gridSize
        };
        this.ctx.strokeStyle = '#e0e0e0';
        this.ctx.lineWidth = 1 / this.scale;
        this.ctx.beginPath();
        for (let x = bounds.left; x <= bounds.right; x += gridSize) {
            this.ctx.moveTo(x, bounds.top);
            this.ctx.lineTo(x, bounds.bottom);
        }
        for (let y = bounds.top; y <= bounds.bottom; y += gridSize) {
            this.ctx.moveTo(bounds.left, y);
            this.ctx.lineTo(bounds.right, y);
        }
        this.ctx.stroke();
        this.ctx.strokeStyle = '#c0c0c0';
        this.ctx.lineWidth = 2 / this.scale;
        this.ctx.beginPath();
        this.ctx.moveTo(-10, 0);
        this.ctx.lineTo(10, 0);
        this.ctx.moveTo(0, -10);
        this.ctx.lineTo(0, 10);
        this.ctx.stroke();
    }
    drawBoundary() {
        const points = this.inputEnvironment.boundaryPolygon.polygonVertexListCw;
        if (!points || points.length === 0) return;
        
        // Determine if we're currently drawing (show highlighted) or finished (show normal)
        const isCurrentlyDrawing = this.isDrawingBoundary;
        
        if (points.length >= 2) {
            this.ctx.strokeStyle = '#27ae60';
            this.ctx.lineWidth = 3 / this.scale;
            this.ctx.beginPath();
            this.ctx.moveTo(points[0].x * this.pixelsPerMeter, points[0].y * this.pixelsPerMeter);
            for (let i = 1; i < points.length; i++) {
                this.ctx.lineTo(points[i].x * this.pixelsPerMeter, points[i].y * this.pixelsPerMeter);
            }
            if (points.length > 2) {
                this.ctx.closePath();
                this.ctx.fillStyle = 'rgba(39, 174, 96, 0.1)';
                this.ctx.fill();
            }
            this.ctx.stroke();
        }
        
        // Draw points with different size/style based on drawing state
        if (isCurrentlyDrawing) {
            // Larger, more prominent points while drawing
            this.ctx.fillStyle = '#229954';
            const r = this.getPointRadius(true);
            for (const point of points) {
                this.ctx.beginPath();
                this.ctx.arc(point.x * this.pixelsPerMeter, point.y * this.pixelsPerMeter, r, 0, Math.PI * 2);
                this.ctx.fill();
            }
        } else {
            // Medium-sized points when completed (visible but not highlighted)
            this.ctx.fillStyle = '#27ae60';
            const r = this.getPointRadius(false);
            for (const point of points) {
                this.ctx.beginPath();
                this.ctx.arc(point.x * this.pixelsPerMeter, point.y * this.pixelsPerMeter, r, 0, Math.PI * 2);
                this.ctx.fill();
            }
        }

        // Draw boundary vertex numbers (if enabled)
        if (this.showVertexNumbers) this.drawVertexLabels(points, '#1e8449');
    }

    // Helper: draw vertex index labels centered inside each point
    drawVertexLabels(vertices, color = '#000000') {
        if (!vertices || vertices.length === 0) return;

    // Slightly larger font to fit inside the vertex dot; scales with zoom
    const fontPx = Math.max(7, Math.round(10 / this.scale));

        // Configure text drawing styles: center/middle to place inside the dot
        this.ctx.font = `${fontPx}px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif`;
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';

        for (let i = 0; i < vertices.length; i++) {
            const v = vertices[i];
            const x = v.x * this.pixelsPerMeter;
            const y = v.y * this.pixelsPerMeter;
            const label = String(i + 1);

            // Dark outline for contrast against vertex fill
            this.ctx.lineWidth = Math.max(1, 2 / this.scale);
            this.ctx.strokeStyle = 'rgba(0,0,0,0.6)';
            this.ctx.strokeText(label, x, y);

            // White text inside the colored dot for readability
            this.ctx.fillStyle = '#ffffff';
            this.ctx.fillText(label, x, y);
        }
    }

    getPointRadius(isDrawing) {
        // Base radii
        const base = isDrawing ? 5 : 3.5; // previous sizes
        const scaleFactor = this.showVertexNumbers ? 1.8 : 1.0; // enlarge when showing numbers
        return (base * scaleFactor) / this.scale;
    }

    drawNotification() {
        if (!this.notification) return;
        
        // Save current context
        this.ctx.save();
        
        // Reset transformations for UI elements
        this.ctx.setTransform(1, 0, 0, 1, 0, 0);
        
        const padding = 16;
        const cornerRadius = 8;
        const maxWidth = 280;
        const iconSize = 16;
        
        // Set font for measuring
        this.ctx.font = 'bold 13px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif';
        
        // Measure text
        const textMetrics = this.ctx.measureText(this.notification);
        const textWidth = Math.min(textMetrics.width, maxWidth);
        const textHeight = 16;
        
        // Calculate notification box dimensions
        const boxWidth = textWidth + padding * 2 + iconSize + 8; // Extra space for icon
        const boxHeight = Math.max(textHeight + padding * 2, 48);
        
        // Position in top-right corner with margin
        const x = this.canvas.width - boxWidth - 20;
        const y = 20;
        
        // Draw shadow
        this.ctx.fillStyle = 'rgba(0, 0, 0, 0.15)';
        this.ctx.beginPath();
        this.ctx.roundRect(x + 2, y + 2, boxWidth, boxHeight, cornerRadius);
        this.ctx.fill();
        
        // Draw background with gradient
        const gradient = this.ctx.createLinearGradient(x, y, x, y + boxHeight);
        gradient.addColorStop(0, 'rgba(45, 55, 72, 0.98)');
        gradient.addColorStop(1, 'rgba(26, 32, 44, 0.98)');
        this.ctx.fillStyle = gradient;
        this.ctx.beginPath();
        this.ctx.roundRect(x, y, boxWidth, boxHeight, cornerRadius);
        this.ctx.fill();
        
        // Draw subtle border
        this.ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
        this.ctx.lineWidth = 1;
        this.ctx.stroke();
        
        // Draw info icon (i)
        const iconX = x + padding;
        const iconY = y + boxHeight / 2;
        
        // Icon background circle
        this.ctx.fillStyle = 'rgba(59, 130, 246, 0.9)'; // Blue
        this.ctx.beginPath();
        this.ctx.arc(iconX + iconSize/2, iconY, iconSize/2, 0, Math.PI * 2);
        this.ctx.fill();
        
        // Icon text "i"
        this.ctx.fillStyle = '#ffffff';
        this.ctx.font = 'bold 11px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        this.ctx.fillText('i', iconX + iconSize/2, iconY);
        
        // Draw main text
        this.ctx.fillStyle = '#e2e8f0';
        this.ctx.font = '13px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif';
        this.ctx.textAlign = 'left';
        this.ctx.textBaseline = 'middle';
        this.ctx.fillText(this.notification, iconX + iconSize + 8, iconY, maxWidth);
        
        // Restore context
        this.ctx.restore();
    }

    showNotification(message, duration = 3000) {
        this.notification = message;
        
        // Clear existing timer
        if (this.notificationTimer) {
            clearTimeout(this.notificationTimer);
        }
        
        // Set timer to hide notification
        this.notificationTimer = setTimeout(() => {
            this.notification = null;
            this.draw();
        }, duration);
        
        this.draw();
    }

    resetView() {
        this.scale = 1;
        this.offsetX = this.canvas.width / 2;
        this.offsetY = this.canvas.height / 2;
        this.draw();
    }
    toggleGrid() {
        this.showGrid = !this.showGrid;
        this.draw();
    }
}
