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

    async saveEnvironment(name, data) {
    const url = `/environment/InputEnvironment/save?name=${encodeURIComponent(name)}`;
        const res = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        if (!res.ok) throw new Error(`Save failed (${res.status})`);
        return res.json().catch(() => ({ status: 'ok' }));
    }

    async listSavedEnvironments() {
    const res = await fetch('/environment/InputEnvironment/saves');
        if (!res.ok) throw new Error(`List failed (${res.status})`);
        return res.json(); // array of names
    }

    async loadEnvironment(name) {
    const url = `/environment/InputEnvironment/load?name=${encodeURIComponent(name)}`;
        const res = await fetch(url);
        if (!res.ok) throw new Error(`Load failed (${res.status})`);
        return res.json();
    }

    async deleteEnvironment(name) {
    const url = `/environment/InputEnvironment/delete?name=${encodeURIComponent(name)}`;
        const res = await fetch(url, { method: 'POST' });
        if (!res.ok) throw new Error(`Delete failed (${res.status})`);
        return res.json().catch(() => ({ status: 'ok' }));
    }
}
