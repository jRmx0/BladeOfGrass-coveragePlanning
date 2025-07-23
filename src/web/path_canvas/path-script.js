// Robot Mower Path Planning Test Application
class PathPlanningApp {
    constructor() {
        this.canvas = document.getElementById('pathCanvas');
        this.ctx = this.canvas.getContext('2d');
        
        // State variables
        this.boundaryPoints = [];
        this.isDrawingBoundary = false;
        this.isDragging = false;
        this.showGrid = true;
        
        // Canvas transform state
        this.scale = 1;
        this.offsetX = 0;
        this.offsetY = 0;
        this.lastMouseX = 0;
        this.lastMouseY = 0;
        
        // Settings (in meters)
        this.mowerWidth = 0.25;
        this.pathOverlap = 0.05;
        
        // Conversion factor: pixels to meters (100 pixels = 1 meter)
        this.pixelsPerMeter = 100;
        
        this.initializeEventListeners();
        this.setupCanvas();
        this.updateDisplay();
        this.draw();
    }
    
    initializeEventListeners() {
        // Button event listeners
        document.getElementById('drawBoundary').addEventListener('click', () => this.toggleDrawingMode());
        document.getElementById('clearCanvas').addEventListener('click', () => this.clearCanvas());
        document.getElementById('resetView').addEventListener('click', () => this.resetView());
        document.getElementById('toggleGrid').addEventListener('click', () => this.toggleGrid());
        
        // Settings input listeners
        document.getElementById('mowerWidth').addEventListener('change', (e) => {
            this.mowerWidth = parseFloat(e.target.value);
            this.updateDisplay();
        });
        
        document.getElementById('pathOverlap').addEventListener('change', (e) => {
            this.pathOverlap = parseFloat(e.target.value);
            this.updateDisplay();
        });
                
        // Canvas event listeners
        this.canvas.addEventListener('mousedown', (e) => this.handleMouseDown(e));
        this.canvas.addEventListener('mousemove', (e) => this.handleMouseMove(e));
        this.canvas.addEventListener('mouseup', (e) => this.handleMouseUp(e));
        this.canvas.addEventListener('wheel', (e) => this.handleWheel(e));
        this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        
        // Prevent default drag behavior
        this.canvas.addEventListener('dragstart', (e) => e.preventDefault());
    }
    
    setupCanvas() {
        // Set canvas size to maximize available space
        const container = this.canvas.parentElement;
        const containerRect = container.getBoundingClientRect();
        
        // Use almost all available space, leaving small margins
        this.canvas.width = containerRect.width - 20;
        this.canvas.height = containerRect.height - 20;
        
        // Set initial view to center
        this.offsetX = this.canvas.width / 2;
        this.offsetY = this.canvas.height / 2;
        
        // Add resize listener to maintain canvas size
        window.addEventListener('resize', () => this.resizeCanvas());
    }
    
    resizeCanvas() {
        const container = this.canvas.parentElement;
        const containerRect = container.getBoundingClientRect();
        
        this.canvas.width = containerRect.width - 20;
        this.canvas.height = containerRect.height - 20;
        
        this.draw();
    }
    
    toggleDrawingMode() {
        this.isDrawingBoundary = !this.isDrawingBoundary;
        const btn = document.getElementById('drawBoundary');
        const container = document.querySelector('.canvas-container');
        
        if (this.isDrawingBoundary) {
            btn.textContent = 'Stop Drawing';
            btn.style.backgroundColor = '#e74c3c';
            container.classList.add('drawing');
            this.canvas.style.cursor = 'crosshair';
        } else {
            btn.textContent = 'Draw Boundary';
            btn.style.backgroundColor = '#3498db';
            container.classList.remove('drawing');
            this.canvas.style.cursor = 'default';
        }
        
        this.updateDisplay();
    }
    
    clearCanvas() {
        this.boundaryPoints = [];
        this.pathPoints = [];
        this.isDrawingBoundary = false;
        
        // Clear server response data
        this.serverResponse = null;
        this.decompositionPoints = [];
        this.calculatedPathPoints = [];
                
        // Reset draw boundary button
        const btn = document.getElementById('drawBoundary');
        btn.textContent = 'Draw Boundary';
        btn.style.backgroundColor = '#3498db';
        
        const container = document.querySelector('.canvas-container');
        container.classList.remove('drawing');
        
        this.updateDisplay();
        this.draw();
    }
    
    resetView() {
        this.scale = 1;
        this.offsetX = this.canvas.width / 2;
        this.offsetY = this.canvas.height / 2;
        this.updateDisplay();
        this.draw();
    }
    
    toggleGrid() {
        this.showGrid = !this.showGrid;
        this.updateDisplay();
        this.draw();
    }
                            
    handleMouseDown(e) {
        const rect = this.canvas.getBoundingClientRect();
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;
        
        // Convert to world coordinates
        const worldX = (mouseX - this.offsetX) / this.scale;
        const worldY = (mouseY - this.offsetY) / this.scale;
        
        if (this.isDrawingBoundary) {
            // Add point to boundary
            this.boundaryPoints.push({ x: worldX, y: worldY });
            this.updateDisplay();
            this.draw();
        } else {
            // Start panning
            this.isDragging = true;
            this.lastMouseX = mouseX;
            this.lastMouseY = mouseY;
            this.canvas.style.cursor = 'grabbing';
        }
    }
    
    handleMouseMove(e) {
        const rect = this.canvas.getBoundingClientRect();
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;
        
        // Update mouse coordinates display
        const worldX = (mouseX - this.offsetX) / this.scale;
        const worldY = (mouseY - this.offsetY) / this.scale;
        const worldXMeters = (worldX / this.pixelsPerMeter).toFixed(2);
        const worldYMeters = (worldY / this.pixelsPerMeter).toFixed(2);
        document.getElementById('canvasCoords').textContent = `Mouse: (${worldXMeters}, ${worldYMeters}) m`;
        
        if (this.isDragging && !this.isDrawingBoundary) {
            // Pan the view
            const deltaX = mouseX - this.lastMouseX;
            const deltaY = mouseY - this.lastMouseY;
            
            this.offsetX += deltaX;
            this.offsetY += deltaY;
            
            this.lastMouseX = mouseX;
            this.lastMouseY = mouseY;
            
            this.draw();
        }
    }
    
    handleMouseUp(e) {
        this.isDragging = false;
        this.canvas.style.cursor = this.isDrawingBoundary ? 'crosshair' : 'default';
    }
    
    handleWheel(e) {
        e.preventDefault();
        
        const rect = this.canvas.getBoundingClientRect();
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;
        
        // Zoom in/out
        const zoomFactor = e.deltaY > 0 ? 0.9 : 1.1;
        const newScale = Math.max(0.1, Math.min(5, this.scale * zoomFactor));
        
        // Adjust offset to zoom towards mouse position
        const worldX = (mouseX - this.offsetX) / this.scale;
        const worldY = (mouseY - this.offsetY) / this.scale;
        
        this.scale = newScale;
        this.offsetX = mouseX - worldX * this.scale;
        this.offsetY = mouseY - worldY * this.scale;
        
        this.updateDisplay();
        this.draw();
    }
    
    draw() {
        // Clear canvas
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        
        // Save context
        this.ctx.save();
        
        // Apply transform
        this.ctx.translate(this.offsetX, this.offsetY);
        this.ctx.scale(this.scale, this.scale);
        
        // Draw grid
        if (this.showGrid) {
            this.drawGrid();
        }
        
        // Draw boundary
        if (this.boundaryPoints.length > 0) {
            this.drawBoundary();
        }
                
        // Restore context
        this.ctx.restore();
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
        
        // Vertical lines
        for (let x = bounds.left; x <= bounds.right; x += gridSize) {
            this.ctx.moveTo(x, bounds.top);
            this.ctx.lineTo(x, bounds.bottom);
        }
        
        // Horizontal lines
        for (let y = bounds.top; y <= bounds.bottom; y += gridSize) {
            this.ctx.moveTo(bounds.left, y);
            this.ctx.lineTo(bounds.right, y);
        }
        
        this.ctx.stroke();
        
        // Draw origin
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
        if (this.boundaryPoints.length === 0) return;
        
        // Draw boundary lines only if we have 2 or more points
        if (this.boundaryPoints.length >= 2) {
            this.ctx.strokeStyle = '#3498db';
            this.ctx.lineWidth = 3 / this.scale;
            this.ctx.beginPath();
            
            this.ctx.moveTo(this.boundaryPoints[0].x, this.boundaryPoints[0].y);
            for (let i = 1; i < this.boundaryPoints.length; i++) {
                this.ctx.lineTo(this.boundaryPoints[i].x, this.boundaryPoints[i].y);
            }
            
            // Close the boundary if we have more than 2 points
            if (this.boundaryPoints.length > 2) {
                this.ctx.closePath();
                this.ctx.fillStyle = 'rgba(52, 152, 219, 0.1)';
                this.ctx.fill();
            }
            
            this.ctx.stroke();
        }
        
        // Always draw boundary points (even if there's only one)
        this.ctx.fillStyle = '#2980b9';
        for (const point of this.boundaryPoints) {
            this.ctx.beginPath();
            this.ctx.arc(point.x, point.y, 4 / this.scale, 0, Math.PI * 2);
            this.ctx.fill();
        }
    }
            
    updateDisplay() {     
        // Update boundary info
        document.getElementById('boundaryPoints').textContent = this.boundaryPoints.length;
        document.getElementById('zoomLevel').textContent = Math.round(this.scale * 100) + '%';
    }
}

// Initialize the application when the page loads
document.addEventListener('DOMContentLoaded', () => {
    new PathPlanningApp();
});