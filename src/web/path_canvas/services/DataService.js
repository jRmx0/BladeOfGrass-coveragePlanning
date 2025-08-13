class DataService {
    constructor() {
        this.baseUrl = '';
    }

    exportToConsole(data) {
    // Intentionally no-op: avoid logging environment data in the browser console
    }

    async sendToServer(data) {
        try {
            const res = await fetch('/environment/InputEnvironment/export', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(data)
            });
            if (!res.ok) {
                throw new Error(`Server responded ${res.status}`);
            }
            const json = await res.json().catch(() => ({}));
            if (json && Array.isArray(json.event_list)) {
                console.log('Event list received:', json);
            } else {
                console.log('Export response:', json);
            }
            return json;
        } catch (err) {
            console.error('Failed to export to server:', err);
            return { status: 'error', message: String(err) };
        }
    }
}
