class AlgorithmService {
    constructor(dataService) {
        this.dataService = dataService;
    }

    createModelData(boundaryModel, obstacleModel) {
        return {
            id: boundaryModel.id,
            pathWidth: boundaryModel.pathWidth,
            pathOverlap: boundaryModel.pathOverlap,
            boundaryPoints: boundaryModel.boundaryPoints,
            obstacles: obstacleModel.obstacles
        };
    }

    runBoustrophedonDecomposition(modelData) {
        try {
            const decomposition = new BoustrophedonDecomposition(modelData);
            return decomposition.decompose();
        } catch (error) {
            console.error('Error running Boustrophedon decomposition:', error);
            throw new Error('Failed to run path decomposition algorithm');
        }
    }

    async exportModelAndDecomposition(boundaryModel, obstacleModel) {
        try {
            const modelData = this.createModelData(boundaryModel, obstacleModel);
            
            // Save model
            await this.dataService.saveModel(modelData);
            
            // Compute and save decomposition
            const decompositionResult = this.runBoustrophedonDecomposition(modelData);
            await this.dataService.saveDecomposition(decompositionResult);
            
            return {
                model: modelData,
                decomposition: decompositionResult
            };
        } catch (error) {
            console.error('Error in export process:', error);
            throw error;
        }
    }

    processAndVisualizeBoustrophedon(boundaryModel, obstacleModel, canvasManager) {
        try {
            const modelData = this.createModelData(boundaryModel, obstacleModel);
            const decompositionResult = this.runBoustrophedonDecomposition(modelData);
            
            // Update canvas with results
            canvasManager.cells = decompositionResult.cells;
            canvasManager.draw();
            
            return decompositionResult;
        } catch (error) {
            alert('Error running Boustrophedon algorithm: ' + error.message);
            throw error;
        }
    }
}
