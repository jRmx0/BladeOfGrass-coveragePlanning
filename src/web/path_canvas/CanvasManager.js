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
        if (this.cells.length > 0) this.drawCells();
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
        this.ctx.lineWidth = 3 / this.scale;
        this.ctx.setLineDash([5, 5]);
        for (const cell of this.cells) {
            this.ctx.beginPath();
            this.ctx.moveTo(cell[0].x * this.pixelsPerMeter, cell[0].y * this.pixelsPerMeter);
            for (let i = 1; i < cell.length; i++) {
                this.ctx.lineTo(cell[i].x * this.pixelsPerMeter, cell[i].y * this.pixelsPerMeter);
            }
            this.ctx.closePath();
            this.ctx.stroke();
        }
        this.ctx.setLineDash([]);
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
