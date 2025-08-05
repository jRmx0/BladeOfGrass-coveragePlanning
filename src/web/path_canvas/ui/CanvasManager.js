class CanvasManager {
    constructor(canvas, boundaryModel, obstacleModel) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.boundaryModel = boundaryModel;
        this.obstacleModel = obstacleModel;
        this.scale = 1;
        this.offsetX = 0;
        this.offsetY = 0;
        this.lastMouseX = 0;
        this.lastMouseY = 0;
        this.isDragging = false;
        this.showGrid = true;
        this.showCells = true;
        this.showCellNumbers = true;
        this.pixelsPerMeter = 100;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.cells = [];
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
        if (this.boundaryModel.boundaryPoints.length > 0) this.drawBoundary();
        if (this.obstacleModel.obstacles.length > 0) this.drawObstacles();
        if (this.cells.length > 0 && this.showCells) this.drawCells();
        this.ctx.restore();
    }

    drawObstacles() {
        for (const obstacle of this.obstacleModel.obstacles) {
            if (obstacle.length === 0) continue;
            // Draw obstacle polygon
            if (obstacle.length >= 2) {
                this.ctx.strokeStyle = '#e67e22';
                this.ctx.lineWidth = 2 / this.scale;
                this.ctx.beginPath();
                this.ctx.moveTo(obstacle[0].x * this.pixelsPerMeter, obstacle[0].y * this.pixelsPerMeter);
                for (let i = 1; i < obstacle.length; i++) {
                    this.ctx.lineTo(obstacle[i].x * this.pixelsPerMeter, obstacle[i].y * this.pixelsPerMeter);
                }
                if (obstacle.length > 2) {
                    this.ctx.closePath();
                    this.ctx.fillStyle = 'rgba(230, 126, 34, 0.15)';
                    this.ctx.fill();
                }
                this.ctx.stroke();
            }
            // Draw obstacle points
            this.ctx.fillStyle = '#e67e22';
            for (const point of obstacle) {
                this.ctx.beginPath();
                this.ctx.arc(point.x * this.pixelsPerMeter, point.y * this.pixelsPerMeter, 3 / this.scale, 0, Math.PI * 2);
                this.ctx.fill();
            }
        }
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
        const points = this.boundaryModel.boundaryPoints;
        if (points.length === 0) return;
        if (points.length >= 2) {
            this.ctx.strokeStyle = '#3498db';
            this.ctx.lineWidth = 3 / this.scale;
            this.ctx.beginPath();
            this.ctx.moveTo(points[0].x * this.pixelsPerMeter, points[0].y * this.pixelsPerMeter);
            for (let i = 1; i < points.length; i++) {
                this.ctx.lineTo(points[i].x * this.pixelsPerMeter, points[i].y * this.pixelsPerMeter);
            }
            if (points.length > 2) {
                this.ctx.closePath();
                this.ctx.fillStyle = 'rgba(52, 152, 219, 0.1)';
                this.ctx.fill();
            }
            this.ctx.stroke();
        }
        this.ctx.fillStyle = '#2980b9';
        for (const point of points) {
            this.ctx.beginPath();
            this.ctx.arc(point.x * this.pixelsPerMeter, point.y * this.pixelsPerMeter, 4 / this.scale, 0, Math.PI * 2);
            this.ctx.fill();
        }
    }
    drawCells() {
        this.ctx.strokeStyle = '#2ecc71';
        this.ctx.lineWidth = 2 / this.scale;
        this.ctx.setLineDash([8, 4]);
        
        // Add subtle fill for better visibility
        this.ctx.fillStyle = 'rgba(46, 204, 113, 0.08)';
        
        for (let cellIndex = 0; cellIndex < this.cells.length; cellIndex++) {
            const cell = this.cells[cellIndex];
            
            // Draw cell polygon
            this.ctx.beginPath();
            this.ctx.moveTo(cell[0].x * this.pixelsPerMeter, cell[0].y * this.pixelsPerMeter);
            for (let i = 1; i < cell.length; i++) {
                this.ctx.lineTo(cell[i].x * this.pixelsPerMeter, cell[i].y * this.pixelsPerMeter);
            }
            this.ctx.closePath();
            this.ctx.fill();
            this.ctx.stroke();
            
            // Calculate cell center (centroid) and draw number if enabled
            if (this.showCellNumbers) {
                const center = this.calculateCellCenter(cell);
                this.drawCellNumber(center.x, center.y, cellIndex + 1);
            }
        }
        this.ctx.setLineDash([]);
    }
    
    calculateCellCenter(cell) {
        // Find the point inside the polygon that is furthest from all edges
        // This is better than centroid for label placement
        return this.findOptimalLabelPosition(cell);
    }
    
    findOptimalLabelPosition(polygon) {
        // Find bounding box
        let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
        for (const point of polygon) {
            minX = Math.min(minX, point.x);
            minY = Math.min(minY, point.y);
            maxX = Math.max(maxX, point.x);
            maxY = Math.max(maxY, point.y);
        }
        
        // Grid-based approach to find the point with maximum distance to edges
        const gridSize = 0.1; // 10cm resolution
        let bestPoint = { x: (minX + maxX) / 2, y: (minY + maxY) / 2 };
        let maxDistance = 0;
        
        // Sample points in a grid within the bounding box
        for (let x = minX; x <= maxX; x += gridSize) {
            for (let y = minY; y <= maxY; y += gridSize) {
                const testPoint = { x, y };
                
                // Check if point is inside the polygon
                if (this.isPointInPolygon(testPoint, polygon)) {
                    // Calculate minimum distance to any edge
                    const minDistToEdge = this.getMinDistanceToPolygonEdges(testPoint, polygon);
                    
                    if (minDistToEdge > maxDistance) {
                        maxDistance = minDistToEdge;
                        bestPoint = testPoint;
                    }
                }
            }
        }
        
        return {
            x: bestPoint.x * this.pixelsPerMeter,
            y: bestPoint.y * this.pixelsPerMeter
        };
    }
    
    isPointInPolygon(point, polygon) {
        let inside = false;
        for (let i = 0, j = polygon.length - 1; i < polygon.length; j = i++) {
            if (((polygon[i].y > point.y) !== (polygon[j].y > point.y)) &&
                (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
                inside = !inside;
            }
        }
        return inside;
    }
    
    getMinDistanceToPolygonEdges(point, polygon) {
        let minDistance = Infinity;
        
        for (let i = 0; i < polygon.length; i++) {
            const j = (i + 1) % polygon.length;
            const edge = { start: polygon[i], end: polygon[j] };
            const distance = this.getDistanceToLineSegment(point, edge);
            minDistance = Math.min(minDistance, distance);
        }
        
        return minDistance;
    }
    
    getDistanceToLineSegment(point, edge) {
        const { start, end } = edge;
        const A = point.x - start.x;
        const B = point.y - start.y;
        const C = end.x - start.x;
        const D = end.y - start.y;
        
        const dot = A * C + B * D;
        const lenSq = C * C + D * D;
        
        if (lenSq === 0) {
            // Start and end are the same point
            return Math.sqrt(A * A + B * B);
        }
        
        let param = dot / lenSq;
        
        let xx, yy;
        if (param < 0) {
            xx = start.x;
            yy = start.y;
        } else if (param > 1) {
            xx = end.x;
            yy = end.y;
        } else {
            xx = start.x + param * C;
            yy = start.y + param * D;
        }
        
        const dx = point.x - xx;
        const dy = point.y - yy;
        return Math.sqrt(dx * dx + dy * dy);
    }
    
    drawCellNumber(x, y, number) {
        // Save context
        this.ctx.save();
        
        // Set font properties (scale with zoom)
        const fontSize = Math.max(12, 16 / this.scale);
        this.ctx.font = `bold ${fontSize}px Arial`;
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        
        // Draw background circle for better visibility
        const text = number.toString();
        const textMetrics = this.ctx.measureText(text);
        const padding = 4 / this.scale;
        const radius = Math.max(textMetrics.width / 2 + padding, fontSize / 2 + padding);
        
        // Background circle
        this.ctx.fillStyle = 'rgba(255, 255, 255, 0.9)';
        this.ctx.beginPath();
        this.ctx.arc(x, y, radius, 0, Math.PI * 2);
        this.ctx.fill();
        
        // Border for the circle
        this.ctx.strokeStyle = '#2ecc71';
        this.ctx.lineWidth = 1.5 / this.scale;
        this.ctx.setLineDash([]);
        this.ctx.stroke();
        
        // Draw number text (flip text back to readable orientation)
        this.ctx.fillStyle = '#2c3e50';
        this.ctx.fillText(text, x, y); 
        
        // Restore context
        this.ctx.restore();
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
    toggleCells() {
        this.showCells = !this.showCells;
        this.draw();
    }
    clearCellsOnly() {
        this.cells = [];
        this.draw();
    }
}
