class BoustrophedonDecomposition {
    constructor(model) {
        this.modelId = model.id;
        this.boundary = model.boundaryPoints;
        this.obstacles = model.obstacles;
        this.pathWidth = model.pathWidth;
    }

    decompose() {
        const events = [];
        this.boundary.forEach(p => events.push({ x: p.x, type: 'boundary' }));
        this.obstacles.forEach(obstacle => {
            obstacle.forEach(p => events.push({ x: p.x, type: 'obstacle' }));
        });

        events.sort((a, b) => a.x - b.x);

        const cells = [];
        for (let i = 0; i < events.length - 1; i++) {
            const x_start = events[i].x;
            const x_end = events[i + 1].x;
            if (x_start === x_end) continue;

            const mid_x = (x_start + x_end) / 2;
            
            const y_intervals = this.getYSpan(mid_x);

            y_intervals.forEach(interval => {
                cells.push([
                    { x: x_start, y: interval.min },
                    { x: x_end, y: interval.min },
                    { x: x_end, y: interval.max },
                    { x: x_start, y: interval.max }
                ]);
            });
        }

        return {
            modelId: this.modelId,
            decompositionId: `decomp-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`,
            cells: cells
        };
    }

    getYSpan(x) {
        const boundary_intersections = this.getIntersections(this.boundary, x);
        if (boundary_intersections.length < 2) return [];

        const y_min_boundary = Math.min(...boundary_intersections);
        const y_max_boundary = Math.max(...boundary_intersections);

        let obstacle_intersections = [];
        this.obstacles.forEach(obstacle => {
            obstacle_intersections.push(...this.getIntersections(obstacle, x));
        });
        
        obstacle_intersections = obstacle_intersections.filter(y => y > y_min_boundary && y < y_max_boundary);
        obstacle_intersections.sort((a, b) => a - b);

        const all_ys = [y_min_boundary, ...obstacle_intersections, y_max_boundary];
        const intervals = [];
        for (let i = 0; i < all_ys.length; i += 2) {
            if (i + 1 < all_ys.length) {
                intervals.push({ min: all_ys[i], max: all_ys[i+1] });
            }
        }
        return intervals;
    }

    getIntersections(polygon, x) {
        const intersections = [];
        for (let i = 0; i < polygon.length; i++) {
            const p1 = polygon[i];
            const p2 = polygon[(i + 1) % polygon.length];

            if (p1.x === p2.x) continue; 

            if (Math.min(p1.x, p2.x) <= x && Math.max(p1.x, p2.x) > x) {
                const y = p1.y + (p2.y - p1.y) * (x - p1.x) / (p2.x - p1.x);
                intersections.push(y);
            }
        }
        return intersections;
    }
}
