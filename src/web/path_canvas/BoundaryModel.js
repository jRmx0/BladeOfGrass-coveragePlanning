class BoundaryModel {
    constructor() {
        this.boundaryPoints = [];
        this.pathWidth = 0.25;
        this.pathOverlap = 0.05;
        this.id = `model`;
    }
    clear() {
        this.boundaryPoints = [];
        this.id = `model`;
    }
}
