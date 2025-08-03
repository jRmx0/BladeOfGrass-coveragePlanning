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
