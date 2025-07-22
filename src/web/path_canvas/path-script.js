// Robot Mower Path Planning Test Application
class PathPlanningApp {
    constructor() {
        this.canvas = document.getElementById('pathCanvas');
        this.ctx = this.canvas.getContext('2d');
        
        // State variables
        this.boundaryPoints = [];
        this.pathPoints = [];
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
        document.getElementById('calculatePath').addEventListener('click', () => this.calculatePath());
        document.getElementById('calculatePathC').addEventListener('click', () => this.calculatePathC());
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
    
    calculatePath() {
        if (this.boundaryPoints.length < 3) {
            alert('Please draw a boundary with at least 3 points first.');
            return;
        }
        
        const container = document.querySelector('.canvas-container');
        container.classList.add('calculating');
        
        // Simple timeout to simulate calculation
        setTimeout(() => {
            this.generateCoveragePath();
            container.classList.remove('calculating');
            this.updateDisplay();
            this.draw();
        }, 500);
    }
    
    async calculatePathC() {
        if (this.boundaryPoints.length < 3) {
            alert('Please draw a boundary with at least 3 points first.');
            return;
        }
        
        const btn = document.getElementById('calculatePathC');
        const originalText = btn.textContent;
        btn.disabled = true;
        btn.textContent = 'Calculating...';
        
        try {
            // Convert boundary points from pixels to meters for JSON output
            const boundaryPointsMeters = this.boundaryPoints.map(point => ({
                x: parseFloat((point.x / this.pixelsPerMeter).toFixed(3)),
                y: parseFloat((point.y / this.pixelsPerMeter).toFixed(3))
            }));
            
            // Create comprehensive JSON data for path calculation
            const pathCalculationData = {
                timestamp: new Date().toISOString(),
                settings: {
                    mowerWidth: this.mowerWidth,
                    pathOverlap: this.pathOverlap
                },
                boundary: {
                    pointCount: this.boundaryPoints.length,
                    points: boundaryPointsMeters
                }    
            };
            
            console.log('Sending path calculation data to C server:', pathCalculationData);
            
            // Send POST request to C server
            const response = await fetch('http://localhost:8000/path-calculate', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(pathCalculationData)
            });
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            // Parse JSON response
            const responseData = await response.json();
            
            console.log('Response from C server:', responseData);
            
            // Visual feedback for success
            btn.textContent = 'Path Calculated!';
            btn.style.backgroundColor = '#27ae60';
            setTimeout(() => {
                btn.textContent = originalText;
                btn.style.backgroundColor = '#3498db';
            }, 1500);
            
        } catch (error) {
            console.error('Error communicating with C server:', error);
            
            // Visual feedback for error
            btn.textContent = 'Error!';
            btn.style.backgroundColor = '#e74c3c';
            setTimeout(() => {
                btn.textContent = originalText;
                btn.style.backgroundColor = '#3498db';
            }, 1500);
        } finally {
            btn.disabled = false;
        }
    }
    
    generateCoveragePath() {
        this.pathPoints = [];
        
        if (this.boundaryPoints.length < 3) return;
        
        // Find bounding box of the boundary
        const bounds = this.getBoundingBox(this.boundaryPoints);
        
        // Generate simple back-and-forth pattern
        // Calculate spacing between path centers: mower width minus overlap
        const pathCenterSpacing = (this.mowerWidth - this.pathOverlap) * this.pixelsPerMeter;
        let y = bounds.minY + pathCenterSpacing;
        let goingRight = true;
        
        while (y < bounds.maxY) {
            const intersections = this.getHorizontalIntersections(y, this.boundaryPoints);
            
            if (intersections.length >= 2) {
                intersections.sort((a, b) => a - b);
                
                // Take the leftmost and rightmost intersections
                const startX = intersections[0];
                const endX = intersections[intersections.length - 1];
                
                if (goingRight) {
                    this.pathPoints.push({ x: startX, y: y });
                    this.pathPoints.push({ x: endX, y: y });
                } else {
                    this.pathPoints.push({ x: endX, y: y });
                    this.pathPoints.push({ x: startX, y: y });
                }
                
                goingRight = !goingRight;
            }
            
            y += pathCenterSpacing;
        }
    }
    
    getBoundingBox(points) {
        if (points.length === 0) return { minX: 0, maxX: 0, minY: 0, maxY: 0 };
        
        let minX = points[0].x, maxX = points[0].x;
        let minY = points[0].y, maxY = points[0].y;
        
        for (const point of points) {
            minX = Math.min(minX, point.x);
            maxX = Math.max(maxX, point.x);
            minY = Math.min(minY, point.y);
            maxY = Math.max(maxY, point.y);
        }
        
        return { minX, maxX, minY, maxY };
    }
    
    getHorizontalIntersections(y, boundary) {
        const intersections = [];
        
        for (let i = 0; i < boundary.length; i++) {
            const p1 = boundary[i];
            const p2 = boundary[(i + 1) % boundary.length];
            
            // Check if the line segment crosses the horizontal line
            if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
                // Calculate intersection point
                const t = (y - p1.y) / (p2.y - p1.y);
                const x = p1.x + t * (p2.x - p1.x);
                intersections.push(x);
            }
        }
        
        return intersections;
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
        
        // Draw path
        if (this.pathPoints.length > 0) {
            this.drawPath();
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
    
    drawPath() {
        if (this.pathPoints.length < 2) return;
        
        this.ctx.strokeStyle = '#e74c3c';
        this.ctx.lineWidth = 2 / this.scale;
        this.ctx.beginPath();
        
        for (let i = 0; i < this.pathPoints.length; i += 2) {
            if (i + 1 < this.pathPoints.length) {
                this.ctx.moveTo(this.pathPoints[i].x, this.pathPoints[i].y);
                this.ctx.lineTo(this.pathPoints[i + 1].x, this.pathPoints[i + 1].y);
            }
        }
        
        this.ctx.stroke();
        
        // Draw path direction arrows
        this.ctx.fillStyle = '#c0392b';
        for (let i = 0; i < this.pathPoints.length; i += 2) {
            if (i + 1 < this.pathPoints.length) {
                const p1 = this.pathPoints[i];
                const p2 = this.pathPoints[i + 1];
                const midX = (p1.x + p2.x) / 2;
                const midY = (p1.y + p2.y) / 2;
                
                // Draw small arrow
                const angle = Math.atan2(p2.y - p1.y, p2.x - p1.x);
                const arrowSize = 5 / this.scale;
                
                this.ctx.save();
                this.ctx.translate(midX, midY);
                this.ctx.rotate(angle);
                this.ctx.beginPath();
                this.ctx.moveTo(-arrowSize, -arrowSize/2);
                this.ctx.lineTo(0, 0);
                this.ctx.lineTo(-arrowSize, arrowSize/2);
                this.ctx.stroke();
                this.ctx.restore();
            }
        }
    }
    
    calculateArea() {
        if (this.boundaryPoints.length < 3) return 0;
        
        let area = 0;
        for (let i = 0; i < this.boundaryPoints.length; i++) {
            const j = (i + 1) % this.boundaryPoints.length;
            area += this.boundaryPoints[i].x * this.boundaryPoints[j].y;
            area -= this.boundaryPoints[j].x * this.boundaryPoints[i].y;
        }
        // Convert from square pixels to square meters
        const areaPixels = Math.abs(area) / 2;
        return areaPixels / (this.pixelsPerMeter * this.pixelsPerMeter);
    }
    
    calculatePathLength() {
        let length = 0;
        for (let i = 0; i < this.pathPoints.length; i += 2) {
            if (i + 1 < this.pathPoints.length) {
                const p1 = this.pathPoints[i];
                const p2 = this.pathPoints[i + 1];
                const dx = p2.x - p1.x;
                const dy = p2.y - p1.y;
                length += Math.sqrt(dx * dx + dy * dy);
            }
        }
        // Convert from pixels to meters
        return length / this.pixelsPerMeter;
    }
    
    updateDisplay() {     
        // Update boundary info
        document.getElementById('boundaryPoints').textContent = this.boundaryPoints.length;
        document.getElementById('areaSize').textContent = this.calculateArea().toFixed(2);
        document.getElementById('pathLength').textContent = this.calculatePathLength().toFixed(2);
        
        document.getElementById('zoomLevel').textContent = Math.round(this.scale * 100) + '%';
    }
}

// Initialize the application when the page loads
document.addEventListener('DOMContentLoaded', () => {
    new PathPlanningApp();
});