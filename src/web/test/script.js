let messageCounter = 1;

function addLogEntry(message, type = 'info') {
    const logContent = document.getElementById('logContent');
    const timestamp = new Date().toLocaleTimeString();
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    entry.innerHTML = `<span class="timestamp">[${timestamp}]</span> <span class="${type}">${message}</span>`;
    logContent.appendChild(entry);
    
    // Auto-scroll to bottom
    logContent.scrollTop = logContent.scrollHeight;
}

function clearLog() {
    const logContent = document.getElementById('logContent');
    logContent.innerHTML = '<div class="log-entry"><span class="timestamp">[Cleared]</span> Log cleared</div>';
}

function updateStatus(message, type) {
    const statusDiv = document.getElementById('status');
    statusDiv.textContent = message;
    statusDiv.className = `status ${type}`;
    
    // Clear status after 5 seconds
    setTimeout(() => {
        statusDiv.textContent = '';
        statusDiv.className = '';
    }, 5000);
}

async function sendMessage() {
    const button = document.getElementById('sendButton');
    button.disabled = true;
    button.textContent = 'Sending...';
    
    try {
        // Prepare JSON data to send
        const jsonData = {
            msg: `Hello from web browser! Message #${messageCounter}`,
            timestamp: new Date().toISOString(),
            counter: messageCounter
        };
        
        addLogEntry(`Sending: ${JSON.stringify(jsonData)}`, 'info');
        console.log('Sending JSON to C server:', jsonData);
        
        // Send POST request to C server
        const response = await fetch('http://localhost:8000/send', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(jsonData)
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        // Parse JSON response
        const responseData = await response.json();
        
        console.log('Response from C server:', responseData);
        addLogEntry(`Received: ${JSON.stringify(responseData)}`, 'success');
        
        updateStatus('Message sent and response received successfully!', 'success');
        messageCounter++;
        
    } catch (error) {
        console.error('Error communicating with C server:', error);
        addLogEntry(`Error: ${error.message}`, 'error');
        updateStatus(`Error: ${error.message}`, 'error');
    } finally {
        button.disabled = false;
        button.textContent = 'Send JSON Message';
    }
}

// Test server connection on page load
window.addEventListener('load', async () => {
    try {
        const response = await fetch('http://localhost:8000/test', {
            method: 'GET'
        });
        
        if (response.status === 404) {
            addLogEntry('Server is running and responding (404 expected for /test)', 'success');
            updateStatus('Server connection verified!', 'success');
        } else {
            addLogEntry('Server response received', 'success');
        }
    } catch (error) {
        addLogEntry('Cannot connect to server. Make sure the C program is running on port 8000.', 'error');
        updateStatus('Server not reachable. Start the C program first!', 'error');
    }
});
