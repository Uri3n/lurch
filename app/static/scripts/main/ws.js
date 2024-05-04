
const sock = new WebSocket(`ws://${window.location.hostname}:${window.location.port}/ws`);


function socketMessage(event) {
    console.log('websocket message: ', event);
}

function socketOpen() {
    console.log('successfully established websocket connection with teamserver.');
    g_Sock.send('wuzpoppin');
}

function socketClose() {
    console.log('websocket connection with teamserver has been closed.');
    g_Sock.close();
}

function socketError(error) {
    console.error('websocket error: ', error);
}


export function startWebsocket(){

    sock.onopen = socketOpen;
    sock.onclose = socketClose;
    sock.onmessage = socketMessage;
    sock.onerror = socketError;
}