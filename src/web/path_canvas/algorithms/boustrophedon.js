class BoustrophedonDecomposition {
    constructor(model) {
        this.modelId = model.id;
        this.boundary = model.boundaryPoints;
        this.obstacles = model.obstacles;
        this.pathWidth = model.pathWidth;
    }
}
