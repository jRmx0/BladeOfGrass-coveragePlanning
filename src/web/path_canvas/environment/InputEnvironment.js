// Default Settings
const DEFAULT_PATH_WIDTH = 0.25;
const DEFAULT_PATH_OVERLAP = 0.05;

class InputEnvironment {
    constructor () {
        this.id = Math.floor(1000 + Math.random() * 9000);  // 4 digit random number
        this.boundaryPolygon = new BoundaryPolygon();
        this.obstaclePolygonList = [];

        // Settings
        this.pathWidth = DEFAULT_PATH_WIDTH;
        this.pathOverlap = DEFAULT_PATH_OVERLAP;
    }
    clear() {
        this.boundaryPolygon.clear();
        this.obstaclePolygonList = [];
    }
}

class BoundaryPolygon {
    #polygonVertexListInserted;
    #polygonVertexListCcw; // CW
    #polygonEdgeList; // CW

    constructor () {
        this.#polygonVertexListInserted = [];
        this.#polygonEdgeList = [];
    }

    insertPolygonVertex(vertex) {
        this.#polygonVertexListInserted.push(vertex);
        this.#polygonVertexListCcw = [...this.#polygonVertexListInserted];

        if ((this.#polygonVertexListInserted.length >= 3) && (!this.#isCW(this.#polygonVertexListInserted))) {
            this.#polygonVertexListCcw.reverse();
        }

        this.#updateEdgeList();
    }

    #isCW(polygonVertexList) {
        let sum = 0;
        for (let i = 0; i < polygonVertexList.length; i++) {
            const v1 = polygonVertexList[i];
            const v2 = polygonVertexList[(i + 1) % polygonVertexList.length];
            sum += (v2.x - v1.x) * (v2.y + v1.y);
        }
        return sum < 0; 
    }

    #updateEdgeList() {
        this.#polygonEdgeList = [];

        for (let i = 0; i < this.#polygonVertexListCcw.length; i++) {
            const start = this.#polygonVertexListCcw[i];
            const end = this.#polygonVertexListCcw[(i + 1) % this.#polygonVertexListCcw.length];
            this.#polygonEdgeList.push(new PolygonEdge(start, end));
        }
    }

    clear() {
        this.#polygonVertexListInserted = [];
        this.#polygonEdgeList = [];
    }
}

class ObstaclePolygon {
    #polygonVertexListInserted;
    #polygonVertexListCcw; // CCW
    #polygonEdgeList; // CCW

    constructor() {
        this.#polygonVertexListInserted = [];
        this.#polygonEdgeList = [];
    }

    insertPolygonVertex(vertex) {
        this.#polygonVertexListInserted.push(vertex);
        this.#polygonVertexListCcw = [...this.#polygonVertexListInserted];

        if ((this.#polygonVertexListInserted.length >= 3) && (!this.#isCCW(this.#polygonVertexListInserted))) {
                this.#polygonVertexListCcw.reverse();
        }

        this.#updateEdgeList();
    }

    #isCCW(polygonVertexList) {
        // Check if the polygon vertex list is in counter-clockwise order
        let sum = 0;
        for (let i = 0; i < polygonVertexList.length; i++) {
            const v1 = polygonVertexList[i];
            const v2 = polygonVertexList[(i + 1) % polygonVertexList.length];
            sum += (v2.x - v1.x) * (v2.y + v1.y);
        }
        return sum > 0; 
    }

    #updateEdgeList() {
        this.#polygonEdgeList = [];

        for (let i = 0; i < this.#polygonVertexListCcw.length; i++) {
            const start = this.#polygonVertexListCcw[i];
            const end = this.#polygonVertexListCcw[(i + 1) % this.#polygonVertexListCcw.length];
            this.#polygonEdgeList.push(new PolygonEdge(start, end));
        }
    }

    clear() {
        this.#polygonVertexListInserted = [];
        this.#polygonVertexListCcw = [];
        this.#polygonEdgeList = [];
    }
}

class PolygonVertex {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
}

class PolygonEdge {
    constructor(polygonVertexBegin, polygonVertexEnd) {
        this.polygonVertexBegin = polygonVertexBegin;
        this.polygonVertexEnd = polygonVertexEnd;
    }
}
