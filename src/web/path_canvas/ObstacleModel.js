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
