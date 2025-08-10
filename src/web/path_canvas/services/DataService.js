class DataService {
    constructor() {
        this.baseUrl = '';
    }

    exportToConsole(data) {
        console.log('Environment Data:');
        console.log(JSON.stringify(data, null, 2));
    }
}
