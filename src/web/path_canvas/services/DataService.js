class DataService {
    constructor() {
        this.baseUrl = '';
    }

    async sendToServer(endpoint, data) {
        try {
            const response = await fetch(endpoint, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data),
            });
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const result = await response.json();
            console.log('Server response:', result);
            return result;
        } catch (error) {
            console.error('Error sending data to server:', error);
            throw error;
        }
    }

    async saveModel(modelData) {
        try {
            return await this.sendToServer('/save-model', modelData);
        } catch (error) {
            alert('Error saving model data.');
            throw error;
        }
    }

    async saveDecomposition(decompositionData) {
        try {
            return await this.sendToServer('/save-decomposition', decompositionData);
        } catch (error) {
            alert('Error saving decomposition data.');
            throw error;
        }
    }

    exportToConsole(data) {
        console.log(JSON.stringify(data, null, 2));
    }
}
