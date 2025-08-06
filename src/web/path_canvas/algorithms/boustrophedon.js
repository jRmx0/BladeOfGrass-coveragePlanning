class BoustrophedonDecomposition {
    constructor(model) {
        this.id = "cells";
        this.modelId = model.id;
        this.boundary = model.boundaryPoints;
        this.obstacles = model.obstacles;
        this.EPSILON = 1e-10;
        this.cellIdCounter = 0;
    }

    decompose() {
        try {
            console.log('Starting boustrophedon decomposition...');
            console.log('Boundary:', this.boundary);
            console.log('Obstacles:', this.obstacles);

            // Special case: no obstacles means entire boundary is one cell
            if (!this.obstacles || this.obstacles.length === 0) {
                console.log('No obstacles detected - creating single cell for entire boundary');
                const singleCell = [...this.boundary]; // Copy boundary points directly
                
                return {
                    id: this.id,
                    modelId: this.modelId,
                    cells: [singleCell]
                };
            }

            // Generate events from polygons (only obstacles create IN/OUT events)
            const events = this.createEventsList(this.boundary, this.obstacles);
            console.log('Generated events:', events);

            // Sort events by x-coordinate
            this.sortEventsByX(events);
            console.log('Sorted events:', events);

            // Initialize cell tracking
            const finishedCells = [];
            const currentCells = [];

            // Initialize first cell (leftmost region)
            const bounds = this.computeEnvironmentBounds(this.boundary, this.obstacles);
            console.log('Environment bounds:', bounds);
            
            const firstCell = this.initializeFirstCell(bounds);
            currentCells.push(firstCell);

            // Process each event in order
            for (const event of events) {
                console.log(`Processing ${event.type} event at (${event.x}, ${event.y})`);
                this.processEvent(event, currentCells, finishedCells);
                console.log(`Current cells count: ${currentCells.length}, Finished cells count: ${finishedCells.length}`);
            }

            // Finalize remaining cells
            for (const cell of currentCells) {
                this.finalizeCellAtX(cell, bounds.maxX);
                finishedCells.push(cell);
            }
            currentCells.length = 0;

            console.log(`Total cells generated: ${finishedCells.length}`);
            
            // Convert cells to the expected format
            const cellPolygons = finishedCells.map(cell => cell.polygon).filter(polygon => polygon && polygon.length > 0);
            
            return {
                id: this.id,
                modelId: this.modelId,
                cells: cellPolygons
            };

        } catch (error) {
            console.error('Error in boustrophedon decomposition:', error);
            return {
                id: this.id,
                modelId: this.modelId,
                cells: []
            };
        }
    }

    // Event generation functions
    createEventsList(boundary, obstacles) {
        const events = [];
        
        // Only process obstacle polygons for IN/OUT events
        // Boundary defines the workspace area, not an obstacle
        for (const obstacle of obstacles) {
            this.processObstaclePolygon(obstacle, events);
        }
        
        // Process boundary polygon for CEILING/FLOOR events (workspace boundaries)
        this.processBoundaryPolygon(boundary, events);
        
        return events;
    }

    processObstaclePolygon(polygon, events) {
        if (!polygon || polygon.length < 3) return;

        // Find leftmost vertex (IN event)
        const leftmostIndex = this.findLeftmostVertexIndex(polygon);
        const leftmostVertex = polygon[leftmostIndex];
        
        const inEvent = this.createEvent("IN", leftmostVertex.x, leftmostVertex.y, polygon, leftmostIndex);
        events.push(inEvent);
        
        // Find rightmost vertex (OUT event)
        const rightmostIndex = this.findRightmostVertexIndex(polygon);
        const rightmostVertex = polygon[rightmostIndex];
        
        const outEvent = this.createEvent("OUT", rightmostVertex.x, rightmostVertex.y, polygon, rightmostIndex);
        events.push(outEvent);
        
        // Create CEILING events (vertices between IN and OUT)
        let currentIndex = (leftmostIndex + 1) % polygon.length;
        while (currentIndex !== rightmostIndex) {
            const vertex = polygon[currentIndex];
            const ceilingEvent = this.createEvent("CEILING", vertex.x, vertex.y, polygon, currentIndex);
            events.push(ceilingEvent);
            currentIndex = (currentIndex + 1) % polygon.length;
        }
        
        // Create FLOOR events (vertices between OUT and IN)
        currentIndex = (rightmostIndex + 1) % polygon.length;
        while (currentIndex !== leftmostIndex) {
            const vertex = polygon[currentIndex];
            const floorEvent = this.createEvent("FLOOR", vertex.x, vertex.y, polygon, currentIndex);
            events.push(floorEvent);
            currentIndex = (currentIndex + 1) % polygon.length;
        }
    }

    processBoundaryPolygon(boundary, events) {
        if (!boundary || boundary.length < 3) return;

        // For boundary, all vertices are either CEILING or FLOOR events
        // We need to determine which vertices form the top and bottom of the boundary
        for (let i = 0; i < boundary.length; i++) {
            const vertex = boundary[i];
            const prevVertex = boundary[(i - 1 + boundary.length) % boundary.length];
            const nextVertex = boundary[(i + 1) % boundary.length];
            
            // Determine if this vertex is part of ceiling or floor based on local geometry
            const isTopVertex = this.isVertexOnTopBoundary(vertex, prevVertex, nextVertex, boundary);
            
            if (isTopVertex) {
                const ceilingEvent = this.createEvent("CEILING", vertex.x, vertex.y, boundary, i);
                events.push(ceilingEvent);
            } else {
                const floorEvent = this.createEvent("FLOOR", vertex.x, vertex.y, boundary, i);
                events.push(floorEvent);
            }
        }
    }

    isVertexOnTopBoundary(vertex, prevVertex, nextVertex, boundary) {
        // Simple heuristic: if vertex y is above average y of polygon, it's likely on top
        const avgY = boundary.reduce((sum, v) => sum + v.y, 0) / boundary.length;
        return vertex.y > avgY;
    }

    // Event processing
    processEvent(event, currentCells, finishedCells) {
        switch (event.type) {
            case "IN":
                this.processInEvent(event, currentCells, finishedCells);
                break;
            case "OUT":
                this.processOutEvent(event, currentCells, finishedCells);
                break;
            case "CEILING":
                this.processCeilingEvent(event, currentCells);
                break;
            case "FLOOR":
                this.processFloorEvent(event, currentCells);
                break;
        }
    }

    processInEvent(event, currentCells, finishedCells) {
        // Find cell that contains this event
        const containingCell = this.findCellContainingPoint(event.y, currentCells);
        if (!containingCell) {
            console.warn('No containing cell found for IN event at', event.y);
            return;
        }

        // Close the containing cell
        this.closeCellAtX(containingCell, event.x);
        const cellIndex = currentCells.indexOf(containingCell);
        currentCells.splice(cellIndex, 1);
        finishedCells.push(containingCell);

        // Create two new cells: bottom and top
        const bottomCell = this.createBottomCell(containingCell, event);
        const topCell = this.createTopCell(containingCell, event);

        currentCells.push(bottomCell);
        currentCells.push(topCell);
    }

    processOutEvent(event, currentCells, finishedCells) {
        // Find the two cells that will merge
        const [bottomCell, topCell] = this.findCellsForMerge(event, currentCells);
        if (!bottomCell || !topCell) {
            console.warn('Could not find cells to merge for OUT event');
            return;
        }

        // Close both cells
        this.closeCellAtX(bottomCell, event.x);
        this.closeCellAtX(topCell, event.x);
        
        const bottomIndex = currentCells.indexOf(bottomCell);
        const topIndex = currentCells.indexOf(topCell);
        currentCells.splice(Math.max(bottomIndex, topIndex), 1);
        currentCells.splice(Math.min(bottomIndex, topIndex), 1);
        
        finishedCells.push(bottomCell);
        finishedCells.push(topCell);

        // Create merged cell
        const mergedCell = this.createMergedCell(bottomCell, topCell, event);
        currentCells.push(mergedCell);
    }

    processCeilingEvent(event, currentCells) {
        // Update ceiling of affected cells
        const affectedCells = this.findCellsAffectedByCeiling(event, currentCells);
        for (const cell of affectedCells) {
            this.updateCellCeiling(cell, event);
        }
    }

    processFloorEvent(event, currentCells) {
        // Update floor of affected cells
        const affectedCells = this.findCellsAffectedByFloor(event, currentCells);
        for (const cell of affectedCells) {
            this.updateCellFloor(cell, event);
        }
    }

    // Cell creation and management
    createBottomCell(parentCell, inEvent) {
        const bottomCell = this.createNewCell();
        bottomCell.leftBoundary = inEvent.x;
        
        // Floor edges come from parent cell
        bottomCell.floorEdges = [...parentCell.floorEdges];
        
        // Ceiling edges come from the obstacle's floor edge
        const obstacleFloorEdge = this.getObstacleFloorEdge(inEvent);
        bottomCell.ceilingEdges = obstacleFloorEdge ? [obstacleFloorEdge] : [];
        
        return bottomCell;
    }

    createTopCell(parentCell, inEvent) {
        const topCell = this.createNewCell();
        topCell.leftBoundary = inEvent.x;
        
        // Floor edges come from the obstacle's ceiling edge
        const obstacleCeilingEdge = this.getObstacleCeilingEdge(inEvent);
        topCell.floorEdges = obstacleCeilingEdge ? [obstacleCeilingEdge] : [];
        
        // Ceiling edges come from parent cell
        topCell.ceilingEdges = [...parentCell.ceilingEdges];
        
        return topCell;
    }

    createMergedCell(bottomCell, topCell, outEvent) {
        const mergedCell = this.createNewCell();
        mergedCell.leftBoundary = outEvent.x;
        
        // Floor comes from bottom cell, ceiling comes from top cell
        mergedCell.floorEdges = [...bottomCell.floorEdges];
        mergedCell.ceilingEdges = [...topCell.ceilingEdges];
        
        return mergedCell;
    }

    // Helper functions
    findLeftmostVertexIndex(polygon) {
        let leftmostIndex = 0;
        for (let i = 1; i < polygon.length; i++) {
            if (polygon[i].x < polygon[leftmostIndex].x) {
                leftmostIndex = i;
            }
        }
        return leftmostIndex;
    }

    findRightmostVertexIndex(polygon) {
        let rightmostIndex = 0;
        for (let i = 1; i < polygon.length; i++) {
            if (polygon[i].x > polygon[rightmostIndex].x) {
                rightmostIndex = i;
            }
        }
        return rightmostIndex;
    }

    createEvent(type, x, y, polygon, vertexIndex) {
        return {
            type,
            x,
            y,
            polygon,
            vertexIndex
        };
    }

    sortEventsByX(events) {
        const priority = { "IN": 1, "CEILING": 2, "FLOOR": 3, "OUT": 4 };
        events.sort((a, b) => {
            if (Math.abs(a.x - b.x) < this.EPSILON) {
                return priority[a.type] - priority[b.type];
            }
            return a.x - b.x;
        });
    }

    computeEnvironmentBounds(boundary, obstacles) {
        const allVertices = [...boundary];
        for (const obstacle of obstacles) {
            allVertices.push(...obstacle);
        }
        
        const minX = Math.min(...allVertices.map(v => v.x));
        const maxX = Math.max(...allVertices.map(v => v.x));
        const minY = Math.min(...allVertices.map(v => v.y));
        const maxY = Math.max(...allVertices.map(v => v.y));
        
        return { minX, maxX, minY, maxY };
    }

    initializeFirstCell(bounds) {
        const firstCell = this.createNewCell();
        firstCell.leftBoundary = bounds.minX - this.EPSILON;
        
        // Create boundary edges from the actual boundary polygon
        // For no obstacles case, the entire boundary forms the cell
        const boundaryEdges = this.getBoundaryEdges(this.boundary);
        
        // Separate boundary edges into floor and ceiling
        const { floorEdges, ceilingEdges } = this.separateBoundaryEdges(boundaryEdges, bounds);
        
        firstCell.floorEdges = floorEdges;
        firstCell.ceilingEdges = ceilingEdges;
        
        return firstCell;
    }

    getBoundaryEdges(boundary) {
        const edges = [];
        for (let i = 0; i < boundary.length; i++) {
            const startPoint = boundary[i];
            const endPoint = boundary[(i + 1) % boundary.length];
            edges.push({
                startPoint: { x: startPoint.x, y: startPoint.y },
                endPoint: { x: endPoint.x, y: endPoint.y }
            });
        }
        return edges;
    }

    separateBoundaryEdges(edges, bounds) {
        const avgY = (bounds.minY + bounds.maxY) / 2;
        const floorEdges = [];
        const ceilingEdges = [];
        
        for (const edge of edges) {
            const avgEdgeY = (edge.startPoint.y + edge.endPoint.y) / 2;
            if (avgEdgeY <= avgY) {
                floorEdges.push(edge);
            } else {
                ceilingEdges.push(edge);
            }
        }
        
        // Ensure we have at least some edges for floor and ceiling
        if (floorEdges.length === 0) {
            // Create artificial floor edge
            floorEdges.push({
                startPoint: { x: bounds.minX, y: bounds.minY },
                endPoint: { x: bounds.maxX, y: bounds.minY }
            });
        }
        
        if (ceilingEdges.length === 0) {
            // Create artificial ceiling edge  
            ceilingEdges.push({
                startPoint: { x: bounds.minX, y: bounds.maxY },
                endPoint: { x: bounds.maxX, y: bounds.maxY }
            });
        }
        
        return { floorEdges, ceilingEdges };
    }

    createNewCell() {
        return {
            id: `cell_${this.cellIdCounter++}`,
            leftBoundary: 0,
            rightBoundary: 0,
            floorEdges: [],
            ceilingEdges: [],
            polygon: []
        };
    }

    findCellContainingPoint(y, currentCells) {
        for (const cell of currentCells) {
            const yRange = this.getCellYRange(cell);
            if (y >= yRange.min - this.EPSILON && y <= yRange.max + this.EPSILON) {
                return cell;
            }
        }
        return null;
    }

    getCellYRange(cell) {
        const floorY = this.getMinYOfEdges(cell.floorEdges);
        const ceilingY = this.getMaxYOfEdges(cell.ceilingEdges);
        return { min: floorY, max: ceilingY };
    }

    getMinYOfEdges(edges) {
        if (edges.length === 0) return 0;
        let minY = Infinity;
        for (const edge of edges) {
            minY = Math.min(minY, edge.startPoint.y, edge.endPoint.y);
        }
        return minY;
    }

    getMaxYOfEdges(edges) {
        if (edges.length === 0) return 0;
        let maxY = -Infinity;
        for (const edge of edges) {
            maxY = Math.max(maxY, edge.startPoint.y, edge.endPoint.y);
        }
        return maxY;
    }

    findCellsForMerge(outEvent, currentCells) {
        const candidateCells = [];
        
        for (const cell of currentCells) {
            const yRange = this.getCellYRange(cell);
            if (Math.abs(yRange.min - outEvent.y) < this.EPSILON || 
                Math.abs(yRange.max - outEvent.y) < this.EPSILON) {
                candidateCells.push(cell);
            }
        }
        
        if (candidateCells.length === 2) {
            const range0 = this.getCellYRange(candidateCells[0]);
            const range1 = this.getCellYRange(candidateCells[1]);
            
            if (range0.min < range1.min) {
                return [candidateCells[0], candidateCells[1]]; // bottom, top
            } else {
                return [candidateCells[1], candidateCells[0]]; // bottom, top
            }
        }
        
        console.warn('Could not find exactly 2 cells for merge');
        return [null, null];
    }

    closeCellAtX(cell, x) {
        cell.rightBoundary = x;
        this.finalizeCellPolygon(cell);
    }

    finalizeCellAtX(cell, x) {
        this.closeCellAtX(cell, x);
    }

    finalizeCellPolygon(cell) {
        const vertices = [];
        
        // For boundary-based cells (no obstacles), the cell should match the boundary shape
        if (cell.floorEdges.length === 0 && cell.ceilingEdges.length === 0) {
            // Fallback to simple rectangle
            vertices.push({ x: cell.leftBoundary, y: -2 });
            vertices.push({ x: cell.rightBoundary, y: -2 });
            vertices.push({ x: cell.rightBoundary, y: 2 });
            vertices.push({ x: cell.leftBoundary, y: 2 });
        } else {
            // Build polygon from floor and ceiling edges
            this.buildPolygonFromEdges(cell, vertices);
        }
        
        cell.polygon = this.cleanPolygonVertices(vertices);
    }

    buildPolygonFromEdges(cell, vertices) {
        // For cells that span the entire workspace (no obstacles case)
        if (Math.abs(cell.leftBoundary - (this.computeEnvironmentBounds(this.boundary, this.obstacles).minX - this.EPSILON)) < this.EPSILON &&
            cell.rightBoundary === this.computeEnvironmentBounds(this.boundary, this.obstacles).maxX) {
            
            // This cell covers the entire boundary - use the boundary polygon directly
            for (const point of this.boundary) {
                vertices.push({ x: point.x, y: point.y });
            }
            return;
        }

        // Standard cell polygon construction for partial cells
        try {
            // Add bottom-left corner
            const leftFloorY = this.computeYIntersection(cell.leftBoundary, cell.floorEdges);
            vertices.push({ x: cell.leftBoundary, y: leftFloorY });
            
            // Add floor edge intersections within cell bounds
            this.addEdgeIntersections(cell.floorEdges, cell.leftBoundary, cell.rightBoundary, vertices, false);
            
            // Add bottom-right corner
            const rightFloorY = this.computeYIntersection(cell.rightBoundary, cell.floorEdges);
            vertices.push({ x: cell.rightBoundary, y: rightFloorY });
            
            // Add top-right corner
            const rightCeilingY = this.computeYIntersection(cell.rightBoundary, cell.ceilingEdges);
            vertices.push({ x: cell.rightBoundary, y: rightCeilingY });
            
            // Add ceiling edge intersections within cell bounds (reverse order)
            this.addEdgeIntersections(cell.ceilingEdges, cell.rightBoundary, cell.leftBoundary, vertices, true);
            
            // Add top-left corner
            const leftCeilingY = this.computeYIntersection(cell.leftBoundary, cell.ceilingEdges);
            vertices.push({ x: cell.leftBoundary, y: leftCeilingY });
            
        } catch (error) {
            console.warn('Error building polygon from edges, using fallback:', error);
            // Fallback to simple rectangle
            vertices.length = 0;
            const bounds = this.computeEnvironmentBounds(this.boundary, this.obstacles);
            vertices.push({ x: cell.leftBoundary, y: bounds.minY });
            vertices.push({ x: cell.rightBoundary, y: bounds.minY });
            vertices.push({ x: cell.rightBoundary, y: bounds.maxY });
            vertices.push({ x: cell.leftBoundary, y: bounds.maxY });
        }
    }

    addEdgeIntersections(edges, startX, endX, vertices, reverse) {
        const intersections = [];
        
        for (const edge of edges) {
            // Find vertices of this edge that fall within the x range
            if (this.isPointInXRange(edge.startPoint, startX, endX)) {
                intersections.push(edge.startPoint);
            }
            if (this.isPointInXRange(edge.endPoint, startX, endX)) {
                intersections.push(edge.endPoint);
            }
        }
        
        // Sort intersections by x coordinate
        intersections.sort((a, b) => reverse ? b.x - a.x : a.x - b.x);
        
        // Remove duplicates and add to vertices
        for (let i = 0; i < intersections.length; i++) {
            const point = intersections[i];
            const lastVertex = vertices[vertices.length - 1];
            
            if (!lastVertex || 
                Math.abs(point.x - lastVertex.x) > this.EPSILON || 
                Math.abs(point.y - lastVertex.y) > this.EPSILON) {
                vertices.push({ x: point.x, y: point.y });
            }
        }
    }

    isPointInXRange(point, startX, endX) {
        const minX = Math.min(startX, endX);
        const maxX = Math.max(startX, endX);
        return point.x >= minX - this.EPSILON && point.x <= maxX + this.EPSILON;
    }

    computeYIntersection(x, edges) {
        for (const edge of edges) {
            const minX = Math.min(edge.startPoint.x, edge.endPoint.x);
            const maxX = Math.max(edge.startPoint.x, edge.endPoint.x);
            
            if (x >= minX - this.EPSILON && x <= maxX + this.EPSILON) {
                if (Math.abs(edge.endPoint.x - edge.startPoint.x) < this.EPSILON) {
                    // Vertical edge - return average y or closest y
                    return (edge.startPoint.y + edge.endPoint.y) / 2;
                } else {
                    const t = (x - edge.startPoint.x) / (edge.endPoint.x - edge.startPoint.x);
                    return edge.startPoint.y + t * (edge.endPoint.y - edge.startPoint.y);
                }
            }
        }
        
        // If no direct intersection found, find closest edge
        let closestY = 0;
        let minDistance = Infinity;
        
        for (const edge of edges) {
            const edgeCenterX = (edge.startPoint.x + edge.endPoint.x) / 2;
            const edgeCenterY = (edge.startPoint.y + edge.endPoint.y) / 2;
            const distance = Math.abs(x - edgeCenterX);
            
            if (distance < minDistance) {
                minDistance = distance;
                closestY = edgeCenterY;
            }
        }
        
        if (minDistance < Infinity) {
            return closestY;
        }
        
        console.warn('No intersection or close edge found for x =', x);
        return 0;
    }

    cleanPolygonVertices(vertices) {
        // Remove duplicate consecutive vertices
        const cleaned = [];
        for (let i = 0; i < vertices.length; i++) {
            const current = vertices[i];
            const next = vertices[(i + 1) % vertices.length];
            
            if (Math.abs(current.x - next.x) > this.EPSILON || 
                Math.abs(current.y - next.y) > this.EPSILON) {
                cleaned.push(current);
            }
        }
        return cleaned;
    }

    // Placeholder functions for edge management (simplified)
    getObstacleFloorEdge(inEvent) {
        const polygon = inEvent.polygon;
        const currentIndex = inEvent.vertexIndex;
        const nextIndex = (currentIndex + 1) % polygon.length;
        
        return {
            startPoint: polygon[currentIndex],
            endPoint: polygon[nextIndex]
        };
    }

    getObstacleCeilingEdge(inEvent) {
        const polygon = inEvent.polygon;
        const currentIndex = inEvent.vertexIndex;
        const prevIndex = (currentIndex - 1 + polygon.length) % polygon.length;
        
        return {
            startPoint: polygon[prevIndex],
            endPoint: polygon[currentIndex]
        };
    }

    findCellsAffectedByCeiling(event, currentCells) {
        // Simplified: return cells that might be affected
        return currentCells.filter(cell => {
            const yRange = this.getCellYRange(cell);
            return event.y >= yRange.min - this.EPSILON && event.y <= yRange.max + this.EPSILON;
        });
    }

    findCellsAffectedByFloor(event, currentCells) {
        // Simplified: return cells that might be affected
        return currentCells.filter(cell => {
            const yRange = this.getCellYRange(cell);
            return event.y >= yRange.min - this.EPSILON && event.y <= yRange.max + this.EPSILON;
        });
    }

    updateCellCeiling(cell, event) {
        // Simplified ceiling update
        const edge = this.getObstacleCeilingEdge(event);
        if (edge) {
            cell.ceilingEdges.push(edge);
        }
    }

    updateCellFloor(cell, event) {
        // Simplified floor update
        const edge = this.getObstacleFloorEdge(event);
        if (edge) {
            cell.floorEdges.push(edge);
        }
    }
}
