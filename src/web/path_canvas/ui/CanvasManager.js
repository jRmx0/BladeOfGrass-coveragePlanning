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
    this.showCoveragePath = false; // toggle for coverage path visualization
    this.pathList = []; // last received path list from backend
    this.showMotionPlan = false; // toggle for motion plan visualization
    this.motionPlan = null; // last received motion plan from backend
        
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

    // Draw coverage path overlay (if enabled)
    if (this.showCoveragePath && this.pathList && this.pathList.length > 0 && this.cells && this.cells.length > 0) this.drawCoveragePath();

    // Draw motion plan overlay (if enabled)
    if (this.showMotionPlan && this.motionPlan && this.motionPlan.sections) this.drawMotionPlan();

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

    setPathList(pathListArray) {
        if (Array.isArray(pathListArray)) {
            this.pathList = pathListArray;
        } else {
            this.pathList = [];
        }
        this.draw();
    }

    setMotionPlan(motionPlan) {
        if (motionPlan && motionPlan.sections) {
            this.motionPlan = motionPlan;
        } else {
            this.motionPlan = null;
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
            
            // Draw cell number at the spot furthest from edges (visual center)
            const centerX = this.findCellVisualCenter(cell).x;
            const centerY = this.findCellVisualCenter(cell).y;
            
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

    findCellVisualCenter(cell) {
        // Simple approach: find the centroid of the cell's bounding rectangle
        // and then adjust it to be more central within the actual cell shape
        
        const cx1 = cell.c_begin.x * this.pixelsPerMeter;
        const cy1 = cell.c_begin.y * this.pixelsPerMeter;
        const cx2 = cell.c_end.x * this.pixelsPerMeter;
        const cy2 = cell.c_end.y * this.pixelsPerMeter;
        const fx1 = cell.f_begin.x * this.pixelsPerMeter;
        const fy1 = cell.f_begin.y * this.pixelsPerMeter;
        const fx2 = cell.f_end.x * this.pixelsPerMeter;
        const fy2 = cell.f_end.y * this.pixelsPerMeter;
        
        // Calculate bounding box
        const minX = Math.min(cx1, cx2, fx1, fx2);
        const maxX = Math.max(cx1, cx2, fx1, fx2);
        const minY = Math.min(cy1, cy2, fy1, fy2);
        const maxY = Math.max(cy1, cy2, fy1, fy2);
        
        // Get all edge points for distance calculations
        const edgePoints = [];
        
        // Add ceiling edge points
        if (cell.ceiling_edges && cell.ceiling_edges.length > 0) {
            edgePoints.push({ x: cx1, y: cy1 });
            for (const edge of cell.ceiling_edges) {
                edgePoints.push({ 
                    x: edge.end.x * this.pixelsPerMeter, 
                    y: edge.end.y * this.pixelsPerMeter 
                });
            }
            edgePoints.push({ x: cx2, y: cy2 });
        }
        
        // Add floor edge points (in reverse)
        if (cell.floor_edges && cell.floor_edges.length > 0) {
            edgePoints.push({ x: fx1, y: fy1 });
            for (let i = cell.floor_edges.length - 1; i >= 0; i--) {
                const edge = cell.floor_edges[i];
                edgePoints.push({ 
                    x: edge.end.x * this.pixelsPerMeter, 
                    y: edge.end.y * this.pixelsPerMeter 
                });
            }
            if (cell.floor_edges.length > 0) {
                const firstEdge = cell.floor_edges[0];
                edgePoints.push({ 
                    x: firstEdge.begin.x * this.pixelsPerMeter, 
                    y: firstEdge.begin.y * this.pixelsPerMeter 
                });
            }
            edgePoints.push({ x: fx2, y: fy2 });
        }
        
        // Sample grid points within bounding box and find the one furthest from edges
        const gridSize = 10; // 10x10 grid
        let bestPoint = { x: (minX + maxX) / 2, y: (minY + maxY) / 2 };
        let maxMinDistance = 0;
        
        for (let i = 0; i <= gridSize; i++) {
            for (let j = 0; j <= gridSize; j++) {
                const testX = minX + (maxX - minX) * (i / gridSize);
                const testY = minY + (maxY - minY) * (j / gridSize);
                
                // Check if point is inside the cell (simple polygon test)
                if (this.isPointInCell(testX, testY, cell)) {
                    // Find minimum distance to any edge
                    let minDistanceToEdge = Infinity;
                    
                    for (let k = 0; k < edgePoints.length; k++) {
                        const p1 = edgePoints[k];
                        const p2 = edgePoints[(k + 1) % edgePoints.length];
                        
                        const distance = this.distanceToLineSegment(testX, testY, p1.x, p1.y, p2.x, p2.y);
                        minDistanceToEdge = Math.min(minDistanceToEdge, distance);
                    }
                    
                    if (minDistanceToEdge > maxMinDistance) {
                        maxMinDistance = minDistanceToEdge;
                        bestPoint = { x: testX, y: testY };
                    }
                }
            }
        }
        
        return bestPoint;
    }

    isPointInCell(x, y, cell) {
        // Simple point-in-polygon test using ray casting
        // Build the polygon points in the correct order
        const polygon = [];
        
        // Add ceiling path
        polygon.push({ x: cell.c_begin.x * this.pixelsPerMeter, y: cell.c_begin.y * this.pixelsPerMeter });
        
        if (cell.ceiling_edges && cell.ceiling_edges.length > 0) {
            for (const edge of cell.ceiling_edges) {
                polygon.push({ x: edge.end.x * this.pixelsPerMeter, y: edge.end.y * this.pixelsPerMeter });
            }
        }
        
        polygon.push({ x: cell.c_end.x * this.pixelsPerMeter, y: cell.c_end.y * this.pixelsPerMeter });
        polygon.push({ x: cell.f_begin.x * this.pixelsPerMeter, y: cell.f_begin.y * this.pixelsPerMeter });
        
        if (cell.floor_edges && cell.floor_edges.length > 0) {
            const lastEdge = cell.floor_edges[cell.floor_edges.length - 1];
            polygon.push({ x: lastEdge.end.x * this.pixelsPerMeter, y: lastEdge.end.y * this.pixelsPerMeter });
            
            for (let i = cell.floor_edges.length - 2; i >= 0; i--) {
                const edge = cell.floor_edges[i];
                polygon.push({ x: edge.end.x * this.pixelsPerMeter, y: edge.end.y * this.pixelsPerMeter });
            }
            
            const firstEdge = cell.floor_edges[0];
            polygon.push({ x: firstEdge.begin.x * this.pixelsPerMeter, y: firstEdge.begin.y * this.pixelsPerMeter });
        }
        
        polygon.push({ x: cell.f_end.x * this.pixelsPerMeter, y: cell.f_end.y * this.pixelsPerMeter });
        
        // Ray casting algorithm
        let inside = false;
        for (let i = 0, j = polygon.length - 1; i < polygon.length; j = i++) {
            if (((polygon[i].y > y) !== (polygon[j].y > y)) &&
                (x < (polygon[j].x - polygon[i].x) * (y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
                inside = !inside;
            }
        }
        return inside;
    }

    distanceToLineSegment(px, py, x1, y1, x2, y2) {
        const dx = x2 - x1;
        const dy = y2 - y1;
        const length = Math.sqrt(dx * dx + dy * dy);
        
        if (length === 0) {
            // Point to point distance
            const dpx = px - x1;
            const dpy = py - y1;
            return Math.sqrt(dpx * dpx + dpy * dpy);
        }
        
        // Calculate the t parameter for the closest point on the line segment
        const t = Math.max(0, Math.min(1, ((px - x1) * dx + (py - y1) * dy) / (length * length)));
        
        // Find the closest point on the segment
        const closestX = x1 + t * dx;
        const closestY = y1 + t * dy;
        
        // Return distance to closest point
        const dpx = px - closestX;
        const dpy = py - closestY;
        return Math.sqrt(dpx * dpx + dpy * dpy);
    }

    drawCoveragePath() {
        if (!this.pathList || this.pathList.length === 0 || !this.cells || this.cells.length === 0) {
            return;
        }

        this.ctx.save();
        
        // Set path drawing style
        this.ctx.strokeStyle = '#2ecc71'; // Green path
        this.ctx.lineWidth = Math.max(3, 4 / this.scale);
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        // Draw path connections between cell centers
        if (this.pathList.length > 1) {
            this.ctx.beginPath();
            
            for (let i = 0; i < this.pathList.length; i++) {
                const cellIndex = this.pathList[i];
                
                if (cellIndex >= 0 && cellIndex < this.cells.length) {
                    const cell = this.cells[cellIndex];
                    const center = this.findCellVisualCenter(cell);
                    
                    if (i === 0) {
                        this.ctx.moveTo(center.x, center.y);
                    } else {
                        this.ctx.lineTo(center.x, center.y);
                    }
                }
            }
            
            this.ctx.stroke();
        }
        
        // Draw sequence numbers for ALL visits (including repeated cells)
        for (let pathIndex = 0; pathIndex < this.pathList.length; pathIndex++) {
            const cellIndex = this.pathList[pathIndex];
            
            if (cellIndex >= 0 && cellIndex < this.cells.length) {
                const cell = this.cells[cellIndex];
                const center = this.findCellVisualCenter(cell);
                
                // Calculate offset for multiple visits to the same cell
                const visitsToThisCell = this.pathList.slice(0, pathIndex + 1)
                    .map((ci, pi) => ci === cellIndex ? pi : -1)
                    .filter(pi => pi !== -1);
                const visitNumber = visitsToThisCell.length;
                
                // Offset position for multiple visits (smaller radius)
                const offsetRadius = Math.max(15, 20 / this.scale);
                const angle = (visitNumber - 1) * (Math.PI / 3); // 60-degree increments
                const offsetX = Math.cos(angle) * offsetRadius * (visitNumber > 1 ? 0.6 : 0);
                const offsetY = Math.sin(angle) * offsetRadius * (visitNumber > 1 ? 0.6 : 0);
                
                const drawX = center.x + offsetX;
                const drawY = center.y + offsetY;
                
                // Draw small sequence number similar to vertex numbers
                this.ctx.save();
                
                // Small background circle
                const radius = Math.max(6, 8 / this.scale);
                this.ctx.fillStyle = visitNumber > 1 ? '#ffa425ff' : '#2e50ccff'; // orange for revisits, blue for first visit
                this.ctx.beginPath();
                this.ctx.arc(drawX, drawY, radius, 0, 2 * Math.PI);
                this.ctx.fill();
                
                // White outline
                this.ctx.strokeStyle = '#ffffff';
                this.ctx.lineWidth = Math.max(1, 1.5 / this.scale);
                this.ctx.stroke();
                
                // Small sequence number text
                this.ctx.fillStyle = '#ffffff';
                this.ctx.font = `bold ${Math.max(8, Math.round(10 / this.scale))}px Arial, sans-serif`;
                this.ctx.textAlign = 'center';
                this.ctx.textBaseline = 'middle';
                this.ctx.fillText((pathIndex + 1).toString(), drawX, drawY);
                
                this.ctx.restore();
                
                // Draw direction arrow to next cell
                if (pathIndex < this.pathList.length - 1) {
                    const nextCellIndex = this.pathList[pathIndex + 1];
                    if (nextCellIndex >= 0 && nextCellIndex < this.cells.length) {
                        const nextCell = this.cells[nextCellIndex];
                        const nextCenter = this.findCellVisualCenter(nextCell);
                        
                        // Calculate next position with offset
                        const nextVisitsToThisCell = this.pathList.slice(0, pathIndex + 2)
                            .map((ci, pi) => ci === nextCellIndex ? pi : -1)
                            .filter(pi => pi !== -1);
                        const nextVisitNumber = nextVisitsToThisCell.length;
                        const nextAngle = (nextVisitNumber - 1) * (Math.PI / 3);
                        const nextOffsetX = Math.cos(nextAngle) * offsetRadius * (nextVisitNumber > 1 ? 0.7 : 0);
                        const nextOffsetY = Math.sin(nextAngle) * offsetRadius * (nextVisitNumber > 1 ? 0.7 : 0);
                        
                        const nextDrawX = nextCenter.x + nextOffsetX;
                        const nextDrawY = nextCenter.y + nextOffsetY;
                        
                        this.drawArrow(drawX, drawY, nextDrawX, nextDrawY);
                    }
                }
                
                // Draw start marker for first cell (larger green ring)
                if (pathIndex === 0) {
                    this.ctx.save();
                    this.ctx.strokeStyle = '#008618ff';
                    this.ctx.lineWidth = Math.max(2, 2.5 / this.scale);
                    this.ctx.beginPath();
                    this.ctx.arc(drawX, drawY, Math.max(8, 10 / this.scale), 0, 2 * Math.PI);
                    this.ctx.stroke();
                    this.ctx.restore();
                }
                
                // Draw end marker for last cell (smaller purple ring)
                if (pathIndex === this.pathList.length - 1) {
                    this.ctx.save();
                    this.ctx.strokeStyle = '#c11010ff';
                    this.ctx.lineWidth = Math.max(2, 2.5 / this.scale);
                    this.ctx.beginPath();
                    this.ctx.arc(drawX, drawY, Math.max(8, 10 / this.scale), 0, 2 * Math.PI);
                    this.ctx.stroke();
                    this.ctx.restore();
                }
            }
        }
        
        this.ctx.restore();
    }

    drawMotionPlan() {
        if (!this.motionPlan || !this.motionPlan.sections || this.motionPlan.sections.length === 0) {
            return;
        }

        this.ctx.save();
        
        // Draw coverage motion (boustrophedon patterns)
        this.ctx.strokeStyle = '#3c78e7ff'; // Red for coverage paths
        this.ctx.lineWidth = Math.max(2, 3 / this.scale);
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        for (let i = 0; i < this.motionPlan.sections.length; i++) {
            const section = this.motionPlan.sections[i];
            
            // Draw coverage path as a continuous line
            if (section.coverage && section.coverage.length > 1) {
                this.ctx.beginPath();
                
                // Start the path at the first point
                const firstPoint = section.coverage[0];
                const firstX = firstPoint.x * this.pixelsPerMeter;
                const firstY = firstPoint.y * this.pixelsPerMeter;
                this.ctx.moveTo(firstX, firstY);
                
                // Connect all subsequent points
                for (let j = 1; j < section.coverage.length; j++) {
                    const point = section.coverage[j];
                    const x = point.x * this.pixelsPerMeter;
                    const y = point.y * this.pixelsPerMeter;
                    this.ctx.lineTo(x, y);
                }
                
                this.ctx.stroke();
            }
        }
        
        // Draw navigation paths between sections
        this.ctx.strokeStyle = '#3498db'; // Blue for navigation paths
        this.ctx.lineWidth = Math.max(1, 2 / this.scale);
        this.ctx.setLineDash([5, 5]); // Dashed line for navigation
        
        for (let i = 0; i < this.motionPlan.sections.length; i++) {
            const section = this.motionPlan.sections[i];
            
            // Draw navigation points
            if (section.navigation && section.navigation.length > 1) {
                this.ctx.beginPath();
                
                for (let j = 0; j < section.navigation.length; j++) {
                    const point = section.navigation[j];
                    const x = point.x * this.pixelsPerMeter;
                    const y = point.y * this.pixelsPerMeter;
                    
                    if (j === 0) {
                        this.ctx.moveTo(x, y);
                    } else {
                        this.ctx.lineTo(x, y);
                    }
                }
                
                this.ctx.stroke();
            }
        }
        
        // Reset line dash
        this.ctx.setLineDash([]);
        
        // Draw section start/end points
        this.ctx.fillStyle = '#27ae60'; // Green for start points
        for (let i = 0; i < this.motionPlan.sections.length; i++) {
            const section = this.motionPlan.sections[i];
            
            if (section.coverage && section.coverage.length > 0) {
                // Mark start of coverage
                const startPoint = section.coverage[0];
                const startX = startPoint.x * this.pixelsPerMeter;
                const startY = startPoint.y * this.pixelsPerMeter;
                
                this.ctx.beginPath();
                this.ctx.arc(startX, startY, Math.max(3, 4 / this.scale), 0, 2 * Math.PI);
                this.ctx.fill();
                
                // Mark end of coverage
                if (section.coverage.length > 1) {
                    const endPoint = section.coverage[section.coverage.length - 1];
                    const endX = endPoint.x * this.pixelsPerMeter;
                    const endY = endPoint.y * this.pixelsPerMeter;
                    
                    this.ctx.fillStyle = '#e67e22'; // Orange for end points
                    this.ctx.beginPath();
                    this.ctx.arc(endX, endY, Math.max(3, 4 / this.scale), 0, 2 * Math.PI);
                    this.ctx.fill();
                    this.ctx.fillStyle = '#27ae60'; // Reset to green
                }
            }
        }
        
        this.ctx.restore();
    }

    drawArrow(x1, y1, x2, y2) {
        const headLength = Math.max(8, 12 / this.scale);
        const angle = Math.atan2(y2 - y1, x2 - x1);
        
        // Shorten the arrow to not overlap with cell centers
        const distance = Math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2);
        const shortenBy = Math.max(15, 20 / this.scale);
        const factor = Math.max(0, (distance - shortenBy) / distance);
        
        const endX = x1 + (x2 - x1) * factor;
        const endY = y1 + (y2 - y1) * factor;
        
        this.ctx.save();
        this.ctx.strokeStyle = '#27ae60';
        this.ctx.fillStyle = '#27ae60';
        this.ctx.lineWidth = Math.max(2, 3 / this.scale);
        
        // Draw arrow head
        this.ctx.beginPath();
        this.ctx.moveTo(endX, endY);
        this.ctx.lineTo(endX - headLength * Math.cos(angle - Math.PI / 6), 
                       endY - headLength * Math.sin(angle - Math.PI / 6));
        this.ctx.lineTo(endX - headLength * Math.cos(angle + Math.PI / 6), 
                       endY - headLength * Math.sin(angle + Math.PI / 6));
        this.ctx.closePath();
        this.ctx.fill();
        
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
