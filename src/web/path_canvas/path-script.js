class ObstacleModel {
    constructor() {
        this.obstacles = []; // Array of arrays of points
    }
    startNewObstacle() {
        this.obstacles.push([]);
    }
    addPointToCurrentObstacle(point) {
        if (this.obstacles.length === 0) this.startNewObstacle();
        this.obstacles[this.obstacles.length - 1].push(point);
    }
    clear() {
        this.obstacles = [];
    }
}

class BoundaryModel {
    constructor() {
        this.boundaryPoints = [];
        this.pathWidth = 0.25;
        this.pathOverlap = 0.05;
        this.id = `model-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    }
    clear() {
        this.boundaryPoints = [];
        this.id = `model-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    }
}

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

class UIController {
    constructor(boundaryModel, obstacleModel, canvasManager) {
        this.boundaryModel = boundaryModel;
        this.obstacleModel = obstacleModel;
        this.canvasManager = canvasManager;
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.initUI();
    }
    initUI() {
        document.getElementById('drawBoundary').addEventListener('click', () => this.toggleDrawingMode('boundary'));
        // Add obstacle button if present
        const obstacleBtn = document.getElementById('drawObstacle');
        if (obstacleBtn) {
            obstacleBtn.addEventListener('click', () => this.toggleDrawingMode('obstacle'));
        }
        document.getElementById('clearCanvas').addEventListener('click', () => this.clearCanvas());
        document.getElementById('resetView').addEventListener('click', () => this.canvasManager.resetView());
        document.getElementById('toggleGrid').addEventListener('click', () => this.canvasManager.toggleGrid());
        document.getElementById('exportData').addEventListener('click', () => this.exportData());
        document.getElementById('pathWidth').addEventListener('change', (e) => {
            this.boundaryModel.pathWidth = parseFloat(e.target.value);
            this.updateDisplay();
        });
        document.getElementById('pathOverlap').addEventListener('change', (e) => {
            this.boundaryModel.pathOverlap = parseFloat(e.target.value);
            this.updateDisplay();
        });
        this.canvasManager.canvas.addEventListener('mousedown', (e) => this.handleMouseDown(e));
        this.canvasManager.canvas.addEventListener('mousemove', (e) => this.handleMouseMove(e));
        this.canvasManager.canvas.addEventListener('mouseup', (e) => this.handleMouseUp(e));
        this.canvasManager.canvas.addEventListener('wheel', (e) => this.handleWheel(e));
        this.canvasManager.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        this.canvasManager.canvas.addEventListener('dragstart', (e) => e.preventDefault());
        this.updateDisplay();
        this.canvasManager.draw();
        document.getElementById('boustrophedonCellularDecomposition').addEventListener('click', () => this.runBoustrophedon());
    }

    async sendDataToServer(endpoint, data) {
        try {
            const response = await fetch(endpoint, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data),
            });
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            const result = await response.json();
            console.log('Server response:', result);
        } catch (error) {
            console.error('Error sending data to server:', error);
            alert('Error saving data.');
        }
    }

    exportData() {
        const data = this.getModelData();
        this.sendDataToServer('/save-model', data);
        // Also compute and save decomposition
        const decomposition = new BoustrophedonDecomposition(data);
        const decompositionResult = decomposition.decompose();
        this.sendDataToServer('/save-decomposition', decompositionResult);
    }

    exportModelToConsole() {
        const data = this.getModelData();
        console.log(JSON.stringify(data, null, 2));
    }
    toggleDrawingMode(mode) {
        if (mode === 'boundary') {
            this.isDrawingBoundary = !this.isDrawingBoundary;
            this.isDrawingObstacle = false;
        } else if (mode === 'obstacle') {
            this.isDrawingObstacle = !this.isDrawingObstacle;
            this.isDrawingBoundary = false;
            if (this.isDrawingObstacle) this.obstacleModel.startNewObstacle();
        }
        this.canvasManager.isDrawingBoundary = this.isDrawingBoundary;
        this.canvasManager.isDrawingObstacle = this.isDrawingObstacle;
        // UI feedback
        const btn = document.getElementById('drawBoundary');
        const obstacleBtn = document.getElementById('drawObstacle');
        const container = document.querySelector('.canvas-container');
        if (this.isDrawingBoundary) {
            btn.textContent = 'Stop Drawing';
            btn.style.backgroundColor = '#e74c3c';
            container.classList.add('drawing');
            this.canvasManager.canvas.style.cursor = 'crosshair';
            if (obstacleBtn) {
                obstacleBtn.textContent = 'Draw Obstacle';
                obstacleBtn.style.backgroundColor = '#3498db';
            }
        } else if (this.isDrawingObstacle) {
            if (obstacleBtn) {
                obstacleBtn.textContent = 'Stop Drawing';
                obstacleBtn.style.backgroundColor = '#e74c3c';
            }
            btn.textContent = 'Draw Boundary';
            btn.style.backgroundColor = '#3498db';
            container.classList.add('drawing');
            this.canvasManager.canvas.style.cursor = 'crosshair';
        } else {
            btn.textContent = 'Draw Boundary';
            btn.style.backgroundColor = '#3498db';
            if (obstacleBtn) {
                obstacleBtn.textContent = 'Draw Obstacle';
                obstacleBtn.style.backgroundColor = '#3498db';
            }
            container.classList.remove('drawing');
            this.canvasManager.canvas.style.cursor = 'default';
        }
        this.updateDisplay();
    }
    clearCanvas() {
        this.boundaryModel.clear();
        this.obstacleModel.clear();
        this.isDrawingBoundary = false;
        this.isDrawingObstacle = false;
        this.canvasManager.isDrawingBoundary = false;
        this.canvasManager.isDrawingObstacle = false;
        const btn = document.getElementById('drawBoundary');
        btn.textContent = 'Draw Boundary';
        btn.style.backgroundColor = '#3498db';
        const obstacleBtn = document.getElementById('drawObstacle');
        if (obstacleBtn) {
            obstacleBtn.textContent = 'Draw Obstacle';
            obstacleBtn.style.backgroundColor = '#3498db';
        }
        const container = document.querySelector('.canvas-container');
        container.classList.remove('drawing');
        this.updateDisplay();
        this.canvasManager.draw();
    }
    handleMouseDown(e) {
        const rect = this.canvasManager.canvas.getBoundingClientRect();
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;
        const worldX = parseFloat(((mouseX - this.canvasManager.offsetX) / this.canvasManager.scale / this.canvasManager.pixelsPerMeter).toFixed(2));
        const worldY = parseFloat(((mouseY - this.canvasManager.offsetY) / this.canvasManager.scale / this.canvasManager.pixelsPerMeter).toFixed(2));
        if (this.isDrawingBoundary) {
            this.boundaryModel.boundaryPoints.push({ x: worldX, y: worldY });
            this.updateDisplay();
            this.canvasManager.draw();
        } else if (this.isDrawingObstacle) {
            this.obstacleModel.addPointToCurrentObstacle({ x: worldX, y: worldY });
            this.updateDisplay();
            this.canvasManager.draw();
        } else {
            this.canvasManager.isDragging = true;
            this.canvasManager.lastMouseX = mouseX;
            this.canvasManager.lastMouseY = mouseY;
            this.canvasManager.canvas.style.cursor = 'grabbing';
        }
    }
    handleMouseMove(e) {
        const rect = this.canvasManager.canvas.getBoundingClientRect();
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;
        const worldX = (mouseX - this.canvasManager.offsetX) / this.canvasManager.scale;
        const worldY = (mouseY - this.canvasManager.offsetY) / this.canvasManager.scale;
        const worldXMeters = (worldX / this.canvasManager.pixelsPerMeter).toFixed(2);
        const worldYMeters = (worldY / this.canvasManager.pixelsPerMeter).toFixed(2);
        document.getElementById('canvasCoords').textContent = `Mouse: (${worldXMeters}, ${worldYMeters}) m`;
        if (this.canvasManager.isDragging && !this.isDrawingBoundary && !this.isDrawingObstacle) {
            const deltaX = mouseX - this.canvasManager.lastMouseX;
            const deltaY = mouseY - this.canvasManager.lastMouseY;
            this.canvasManager.offsetX += deltaX;
            this.canvasManager.offsetY += deltaY;
            this.canvasManager.lastMouseX = mouseX;
            this.canvasManager.lastMouseY = mouseY;
            this.canvasManager.draw();
        }
    }
    handleMouseUp(e) {
        this.canvasManager.isDragging = false;
        this.canvasManager.canvas.style.cursor = (this.isDrawingBoundary || this.isDrawingObstacle) ? 'crosshair' : 'default';
    }
    handleWheel(e) {
        e.preventDefault();
        const rect = this.canvasManager.canvas.getBoundingClientRect();
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;
        const zoomFactor = e.deltaY > 0 ? 0.9 : 1.1;
        const newScale = Math.max(0.1, Math.min(5, this.canvasManager.scale * zoomFactor));
        const worldX = (mouseX - this.canvasManager.offsetX) / this.canvasManager.scale;
        const worldY = (mouseY - this.canvasManager.offsetY) / this.canvasManager.scale;
        this.canvasManager.scale = newScale;
        this.canvasManager.offsetX = mouseX - worldX * this.canvasManager.scale;
        this.canvasManager.offsetY = mouseY - worldY * this.canvasManager.scale;
        this.updateDisplay();
        this.canvasManager.draw();
    }
    updateDisplay() {
        document.getElementById('boundaryPoints').textContent = this.boundaryModel.boundaryPoints.length;
        document.getElementById('zoomLevel').textContent = Math.round(this.canvasManager.scale * 100) + '%';
    }
    runBoustrophedon() {
        const model = this.getModelData();
        const decomposition = new BoustrophedonDecomposition(model);
        const decompositionResult = decomposition.decompose();
        this.canvasManager.cells = decompositionResult.cells;
        this.canvasManager.draw();
    }

    getModelData() {
        return {
            id: this.boundaryModel.id,
            pathWidth: this.boundaryModel.pathWidth,
            pathOverlap: this.boundaryModel.pathOverlap,
            boundaryPoints: this.boundaryModel.boundaryPoints,
            obstacles: this.obstacleModel.obstacles
        };
    }
}

document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('pathCanvas');
    const boundaryModel = new BoundaryModel();
    const obstacleModel = new ObstacleModel();
    const canvasManager = new CanvasManager(canvas, boundaryModel, obstacleModel);
    new UIController(boundaryModel, obstacleModel, canvasManager);
    new TestingController(boundaryModel, obstacleModel, canvasManager);
});