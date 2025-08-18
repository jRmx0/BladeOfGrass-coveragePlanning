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
    this.showEvents = false; // debug toggle for events
    this.events = []; // last received events from backend
    this.showCells = false; // debug toggle for cells
    this.cells = []; // last received cells from backend
    this.showCells = false; // debug toggle for cells
    this.cells = []; // last received cells from backend
        
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
        
    // Draw cells overlay (if enabled)
    if (this.showCells && this.cells && this.cells.length > 0) this.drawCells();
        
    // Draw events overlay (if enabled)
    if (this.showEvents && this.events && this.events.length > 0) this.drawEvents();

    // Draw notification if present
        if (this.notification) this.drawNotification();
        
        this.ctx.restore();
    }

    setEvents(eventsArray) {
        if (Array.isArray(eventsArray)) {
            this.events = eventsArray;
        } else {
            this.events = [];
        }
        this.draw();
    }

    setCells(cellsArray) {
        if (Array.isArray(cellsArray)) {
            this.cells = cellsArray;
        } else {
            this.cells = [];
        }
        this.draw();
    }

    drawEvents() {
        const r = Math.max(3, 4 / this.scale);
        for (const ev of this.events) {
            if (!ev || !ev.vertex) continue;
            const x = ev.vertex.x * this.pixelsPerMeter;
            const y = ev.vertex.y * this.pixelsPerMeter;

            // Color per event type
            const t = ev.event_type || '';
            let color = '#8e44ad'; // default purple
            if (t === 'IN' || t === 'B_IN' || t === 'SIDE_IN' || t === 'B_SIDE_IN') color = '#2ecc71';
            else if (t === 'OUT' || t === 'B_OUT' || t === 'SIDE_OUT' || t === 'B_SIDE_OUT') color = '#e74c3c';
            else if (t === 'FLOOR') color = '#3498db';
            else if (t === 'CEILING') color = '#f1c40f';
            else if (t === 'B_INIT') color = '#2ecc71';
            else if (t === 'B_DEINIT') color = '#e74c3c';

            // Draw marker
            this.ctx.fillStyle = color;
            this.ctx.beginPath();
            this.ctx.arc(x, y, r, 0, Math.PI * 2);
            this.ctx.fill();

            // Optional: small label
            this.ctx.font = `${Math.max(7, Math.round(9 / this.scale))}px Segoe UI, Arial`;
            this.ctx.textAlign = 'left';
            this.ctx.textBaseline = 'top';
            this.ctx.fillStyle = color;
            this.ctx.fillText(t, x + r + 2, y + r + 2);
        }
    }

    drawCells() {
        this.ctx.save();
        
        // Set drawing properties for cells
        this.ctx.strokeStyle = '#000000'; // Black border for cells
        this.ctx.lineWidth = Math.max(1, 2 / this.scale);
        this.ctx.fillStyle = 'rgba(0, 0, 0, 0.05)'; // Very light fill for visibility
        
        for (let i = 0; i < this.cells.length; i++) {
            const cell = this.cells[i];
            if (!cell) continue;
            
            // Draw cell boundary as a polygon following the corrected edge construction:
            // c_begin -> ceiling_edges[0].end -> ceiling_edges[i] -> ceiling_edges[last].begin, c_end -> 
            // c_end, f_begin -> f_begin, floor_edges[last].end -> floor_edges[i] -> floor_edges[0].begin, f_end -> f_end, c_begin
            this.ctx.beginPath();
            
            // Start from ceiling begin
            const cx1 = cell.c_begin.x * this.pixelsPerMeter;
            const cy1 = cell.c_begin.y * this.pixelsPerMeter;
            this.ctx.moveTo(cx1, cy1);
            
            // Draw ceiling edges: c_begin, ceiling_edges[0].end -> ceiling_edges[i] -> ceiling_edges[last].begin, c_end
            if (cell.ceiling_edges && cell.ceiling_edges.length > 0) {
                // Move to first ceiling edge end
                const firstEdge = cell.ceiling_edges[0];
                this.ctx.lineTo(firstEdge.end.x * this.pixelsPerMeter, firstEdge.end.y * this.pixelsPerMeter);
                
                // Draw through all ceiling edges
                for (let j = 1; j < cell.ceiling_edges.length; j++) {
                    const edge = cell.ceiling_edges[j];
                    this.ctx.lineTo(edge.end.x * this.pixelsPerMeter, edge.end.y * this.pixelsPerMeter);
                }
            }
            
            // Draw to ceiling end: ceiling_edges[last].begin, c_end
            const cx2 = cell.c_end.x * this.pixelsPerMeter;
            const cy2 = cell.c_end.y * this.pixelsPerMeter;
            this.ctx.lineTo(cx2, cy2);
            
            // Draw vertical connection: c_end, f_begin
            const fx1 = cell.f_begin.x * this.pixelsPerMeter;
            const fy1 = cell.f_begin.y * this.pixelsPerMeter;
            this.ctx.lineTo(fx1, fy1);
            
            // Draw floor edges in reverse: f_begin, floor_edges[last].end -> floor_edges[i] -> floor_edges[0].begin, f_end
            if (cell.floor_edges && cell.floor_edges.length > 0) {
                // Move to last floor edge end (traverse in reverse)
                const lastEdge = cell.floor_edges[cell.floor_edges.length - 1];
                this.ctx.lineTo(lastEdge.end.x * this.pixelsPerMeter, lastEdge.end.y * this.pixelsPerMeter);
                
                // Draw through all floor edges in reverse order
                for (let j = cell.floor_edges.length - 2; j >= 0; j--) {
                    const edge = cell.floor_edges[j];
                    this.ctx.lineTo(edge.end.x * this.pixelsPerMeter, edge.end.y * this.pixelsPerMeter);
                }
                
                // Draw to first floor edge begin: floor_edges[0].begin
                const firstEdge = cell.floor_edges[0];
                this.ctx.lineTo(firstEdge.begin.x * this.pixelsPerMeter, firstEdge.begin.y * this.pixelsPerMeter);
            }
            
            // Draw to floor end: floor_edges[0].begin, f_end
            const fx2 = cell.f_end.x * this.pixelsPerMeter;
            const fy2 = cell.f_end.y * this.pixelsPerMeter;
            this.ctx.lineTo(fx2, fy2);
            
            // Close the path: f_end -> c_begin
            this.ctx.closePath();
            
            // Fill and stroke the cell
            this.ctx.fill();
            this.ctx.stroke();
            
            // Draw cell number in the center
            const centerX = (cx1 + cx2 + fx1 + fx2) / 4;
            const centerY = (cy1 + cy2 + fy1 + fy2) / 4;
            
            this.ctx.save();
            this.ctx.fillStyle = '#000000';
            this.ctx.font = `bold ${Math.max(12, Math.round(16 / this.scale))}px Arial, sans-serif`;
            this.ctx.textAlign = 'center';
            this.ctx.textBaseline = 'middle';
            this.ctx.fillText(cell.cell_number.toString(), centerX, centerY);
            this.ctx.restore();
        }
        
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
