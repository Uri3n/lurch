import * as ui from './ui.js'
import * as req from './fetch.js';
import { templates } from './templating.js';



export function dragoverCallback(event) {

    const X = event.clientX;
    const Y = event.clientY;
    const terminal = document.querySelector('.terminal-instance');
    const filedrop = document.querySelector('.file-drop-icon');

    if(ui.isWithinBoundingRect(X, Y, terminal) === true) {
        terminal.style.opacity = 0.3;
        filedrop.style.opacity = 1;
    }
    else {
        terminal.style.opacity = 1;
        filedrop.style.opacity = 0;
    }
}


export function terminalDropCallback(event) {

    event.preventDefault();    
    ui.focusInputElement();
    const sessionGuid = ui.currentSessionGuid();

    //
    // if a file was dropped, send it to /objects/upload
    //
    
    if(event.dataTransfer.types.includes('Files') && sessionGuid !== null) {
        
        const droppedFile = event.dataTransfer.files[0];
        const reader = new FileReader();

        reader.onload = async function(event) {
            
            try {
                const arrayBuff = event.target.result;                                  // convert file to ArrayBuffer
                const blob = new Blob([arrayBuff]);                                     // convert ArrayBuffer to blob
                const extension = droppedFile.name.split('.').pop().toLowerCase();      // extract file extension
                
                if(extension === null || extension === "") {                            // ????
                    throw new Error('no file extension.');
                }

                await req.uploadFile(sessionGuid, blob, extension);    
            }
            catch(error) {
                console.error('terminalDropCallback(): ', error);
            }                     
        }

        reader.readAsArrayBuffer(droppedFile);
    }
}


export async function sessionScrollCallback(event){
    
    event.stopPropagation();
    
    const session = event.target;
    let msgIndex = parseInt(session.getAttribute('data-message-index'), 10);
    const guid = session.getAttribute('data-object-guid');

    if(event.target.scrollTop === 0 && msgIndex !== null && guid !== null){
        try {

            const messages = await req.fetchObjectMessages(guid, msgIndex.toString());            

            for(const message of messages) {

                const messageElement = templates.terminalMessage(message.sender, message.body) 
                messageElement.style.display = 'none';
                ui.popInElement(messageElement, false);
                
                session.prepend(messageElement);
                msgIndex++;
            }
        } 
        catch(error) {
            console.error('sessionScrollCallback():',error);
        }
        finally {
            session.setAttribute('data-message-index', msgIndex.toString());
            console.log('Message Index:', session.getAttribute('data-message-index'));
        }
    }
}


export function terminalMenuClickCallback(event){
    
    event.stopPropagation();
    const guid = event.currentTarget.getAttribute('data-object-guid');
    
    ui.selectTerminalSession(guid);
    ui.selectTerminalMenuElement(guid);
}


export function deleteButtonCallback(event){

    event.stopPropagation();
    let parent = event.target.parentElement;

    do {
        if(parent.classList.contains('deletable-parent')){
            const guid = parent.getAttribute('data-object-guid');                           //this only applies to the delete buttons 
                                                                                            //found on the right-hand session menu
            if(guid !== null){
                ui.deleteTerminalSession(guid);
            }
            
            ui.popOutElement(parent);
        }

        parent = parent.parentElement;
    } while(parent);
}


export function listElementDragEndCallback(event){                                          
    
    event.stopPropagation();

    const dropX = event.clientX;
    const dropY = event.clientY;
    
    const terminal = document.querySelector('.terminal-instance');                          
    const inputElement = document.getElementById('terminal-input');
    
    let splitContent;
    let guid;
    let alias;


    try{
        splitContent = event.target.textContent.trim().split(' ').filter(str => str !== '::');
        guid = splitContent.splice(0, 1)[0];
        alias = splitContent.join(' ');
    }
    catch(error){
        console.error('listElementDragEndCallback(): ', error);
    }     


    if (ui.isWithinBoundingRect(dropX, dropY, terminal)) {                                                  
        ui.startSession(guid, alias);
        inputElement.focus();
    }

    else if(ui.isWithinBoundingRect(dropX, dropY, inputElement)) {                                          
        if(inputElement.value.length > 0 && inputElement.value.charAt(inputElement.value.length - 1) !== ' ') {
            inputElement.value += ' ';
        }

        inputElement.value += `"${guid}"`;
        inputElement.focus();
    }
}


export function listElementDragStartCallback(event){
    event.dataTransfer.setData('text/plain', 'Draggable List Element');                                     //unused.
}


export function listElementClickCallback(event){
    event.stopPropagation();

    if(event.currentTarget.querySelector('a').classList.contains('is-active')){
        ui.deselectListElement(event.currentTarget);
    }

    else {
        ui.selectListElement(event.currentTarget);
    }
}


export async function keyDownCallback(event){
    
    if(event.key === 'Enter' && ui.isInputSelected()){
        
        const sessions = document.querySelectorAll('.terminal-session');
        const messageContent = ui.currentInputContent();
        
        if(sessions !== null && messageContent.length > 0){
            for(const session of sessions){
                
                const guid = session.getAttribute('data-object-guid');
                if(session.style.display !== 'none' && guid !== null){
                    
                    try {
                        await req.sendObjectMessage(guid, messageContent);
                    }
                    
                    catch(error) {
                        console.error('keyDownCallback():', error);
                    }
                }
            }
        }

        ui.clearInputContent();
    }
}