import { 
    appendNotification, 
    appendMessage, 
    appendListElement,
    deleteListElement,
    forceEndSession
} from "./ui.js";

const sock = new WebSocket(`ws://${window.location.hostname}:${window.location.port}/ws`);


function socketMessage(event) {
    
    try {
        const json = JSON.parse(event.data);
        const updateType = json["update-type"];

        switch(updateType){
            
            case "notification":
                appendNotification(json["body"], json["intent"]);
                break;
            
            case "message":
                appendMessage(json["body"], json["sender"], json["recipient"]);
                break;

            case "object-delete":
                deleteListElement(json["guid"]);
                forceEndSession(json["guid"]);
                break;

            case "object-create":
                appendListElement(json["guid"], json["parent"], json["alias"], json["type"]);
                break;

            default:
                throw new Error('invalid socket update type.');
        }
    }
    catch(error) {
        console.error('socketMessage():', error);
    }
}


function socketOpen() {
    console.log('successfully established websocket connection with teamserver.');
}

function socketClose() {
    console.log('websocket connection with teamserver has been closed.');
    sock.close();
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