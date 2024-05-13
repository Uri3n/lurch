import { templates } from "./templating.js";

import { 
    fetchObjectData, 
    fetchObjectChildren, 
    fetchObjectMessages, 
    sendObjectMessage 
} from "./fetch.js";  


function popInElement(element, scrollTo){

    if(element.style.display !== 'none'){                           // element must be hidden initially for this to work
        console.error(`popInElement(): passed element: ${element}\n
                       without display set to "none".`);
        return;
    }

    setTimeout(() => {
        
        element.style.display = 'flex';                             // small delay to ensure element is loaded
        element.style.animation = 'popIn 0.3s ease';                // play "pop in" animation
        
        element.addEventListener('animationend', () => {
            element.style.animation = 'none';
        });

        if(scrollTo){
            element.scrollIntoView({behavior : 'smooth'});              // scroll towards the newly added element
        }

    }, 10);
}


function popOutElement(element) {

    element.style.animation = 'popOut 0.3s ease';
    element.style.transition = 'opacity 0.3s ease';
    
    element.addEventListener('animationend', () => {
        element.remove();
    });
}


async function appendListChildren(element){
    try {
        const children = await fetchObjectChildren(element.querySelector('a').textContent.trim().split(" ")[0]);
        const childList = document.createElement('ul');

        for(const child of children){
            const listElement = templates.listElement(child.guid, child.alias, child.type);
            childList.appendChild(listElement);    
        }

        element.appendChild(childList);
    } 
    catch(error) {
        console.error('appendListChildren(): ',error);
    }
}


function deleteListChildren(element){
    const childList = element.querySelector('ul');
    if(childList !== null) {
        childList.remove();
    }
}


export function deleteTerminalSession(guid){
    
    const sessions = document.querySelectorAll('.terminal-session');
    if(sessions !== null){
        sessions.forEach(session => {
            if(session.getAttribute('data-object-guid') === guid){
                session.remove();
            }
        });
    }
}


function deselectListElement(element){
    
    element.querySelector('a').classList.remove('is-active');
    Array.from(document.getElementById('object-icon-container').children).forEach(element => {
        
        if(element.getAttribute('data-object-type') !== 'none'){
            element.style.display = 'none';
        }

        else {
            element.style.display = 'flex';
        }
    });

    deleteListChildren(element);                                                              //remove any 'ul' elements when deselecting...
}


function selectListElement(element){
    
    const typeAttr = element.getAttribute('data-object-type');
    console.assert(typeAttr !== null, 'selectListElement(): null "data-object-type"!');

    if(typeAttr !== null){
        Array.from(document.getElementById('object-icon-container').children).forEach(element => {
            
            if(element.getAttribute('data-object-type') === typeAttr){
                element.style.display = 'flex';
            }

            else {
                element.style.display = 'none';
            }
        });

        element.querySelector('a').classList.add('is-active');
        appendListChildren(element);                                                           //add children as 'ul' element
    }
}


export function listElementClickCallback(event){
    event.stopPropagation();

    if(event.currentTarget.querySelector('a').classList.contains('is-active')){
        deselectListElement(event.currentTarget);
    }

    else {
        selectListElement(event.currentTarget);
    }
}


export function listElementDragEndCallback(event){                                              // Checks whether dropped list element is inside of a "terminal instance".
    
    event.stopPropagation();

    const dropX = event.clientX;
    const dropY = event.clientY;

    const dropzoneRect = document.querySelector('.terminal-instance').getBoundingClientRect();  // Should only be one of these elements on the page at any time

    if (dropX >= dropzoneRect.left && dropX <= dropzoneRect.right &&
        dropY >= dropzoneRect.top && dropY <= dropzoneRect.bottom) {
        
        try{
            const splitContent = event.target.textContent.trim().split(' ').filter(str => str !== '::');
            const guid = splitContent.splice(0, 1)[0];
            const alias = splitContent.join(' ');
        
            startSession(guid, alias);
        }
        catch(error){
            console.error('listElementDragEndCallback(): ', error);
        }        
    }
}


export function listElementDragStartCallback(event){
    event.dataTransfer.setData('text/plain', 'Draggable List Element');                         //unused for now.
}


function deleteListElement_r(guid, element){

    const children = element.children;
    for(const child of children){
        
        const anchorTag = child.querySelector('a');
        const subList = child.querySelector('ul');
    
        if(anchorTag.textContent.trim().split(" ")[0] === guid) {
            child.remove();
            break;
        }

        else if(subList !== null){
            deleteListElement_r(guid, subList);
        }
    }
}


export function deleteListElement(guid){
    deleteListElement_r(guid, document.querySelector('#object-menu .menu-list'));
}


async function appendListElement_r(guid, data, element){
    
    const children = element.children;
    for(const child of children){
        
        const anchorTag = child.querySelector('a');
        const subList = child.querySelector('ul');

        if(anchorTag.textContent.trim().split(" ")[0] === data.parent && anchorTag.classList.contains('is-active')){
            const newChild = templates.listElement(guid, data.alias, data.type);
            subList.appendChild(newChild);
            break; 
        }

        else if(subList !== null && anchorTag.classList.contains('is-active')){
            appendListElement_r(guid, data, subList);
        }
    }
}   


export function appendListElement(guid, parent, alias, type){

    const data = {
        alias   : alias,
        type    : type,
        parent  : parent
    };

    appendListElement_r(guid, data, document.querySelector('#object-menu .menu-list'));
}


function selectTerminalSession(guid){

    const sessions = document.querySelectorAll('.terminal-session');
    if(guid !== null && sessions !== null){
        sessions.forEach(element => {
            if(element.getAttribute('data-object-guid') === guid){
                element.style.display = 'flex';
            }
            
            else {
                element.style.display = 'none';
            }
        });
    }
}


export function deleteButtonCallback(event){

    event.stopPropagation();
    let parent = event.target.parentElement;

    do {
        if(parent.classList.contains('deletable-parent')){
            const guid = parent.getAttribute('data-object-guid'); //this only applies to the delete buttons found on the right-hand session menu
            if(guid !== null){
                deleteTerminalSession(guid);
            }
            
            popOutElement(parent);
        }

        parent = parent.parentElement;
    } while(parent);
}


function selectTerminalMenuElement(guid){

    Array.from(document.querySelector('#terminal-menu .menu-list').children).forEach(element => {

        const elementGuid = element.getAttribute('data-object-guid');
        const anchorTag = element.querySelector('a');

        if(elementGuid !== guid && anchorTag.classList.contains('is-active')){
            anchorTag.classList.remove('is-active');
        }

        else if(!anchorTag.classList.contains('is-active') && element.getAttribute('data-object-guid') === guid){
            anchorTag.classList.add('is-active');
        }
    });
}


function sessionExists(guid){

    let exists = false;

    Array.from(document.querySelector('#terminal-menu .menu-list').children).forEach(element => {
        if(element.getAttribute('data-object-guid') === guid) {
            exists = true;
        }
    });

    return exists;
}


export function appendNotification(body, intent){

    let notification;
    const notificationCenter = document.getElementById('notification-center');

    switch(intent){
        case "good":
            notification = templates.goodNotification(body);
            break;

        case "bad":
            notification = templates.badNotification(body);
            break;
    
        default:
            notification = templates.neutralNotification(body);
            break;
    }

    notification.style.display = 'none';
    notificationCenter.appendChild(notification);
    
    popInElement(notification, true);
}


export function appendMessage(body, sender, recipient){

    const sessions = document.querySelectorAll('.terminal-session');
    if(sessions !== null){
        sessions.forEach(session => {
            if(session.getAttribute('data-object-guid') === recipient){

                const terminalMessage = templates.terminalMessage(sender, body);
                
                terminalMessage.style.display = 'none';                                 
                session.appendChild(terminalMessage);                                   
                popInElement(terminalMessage, true);

                
                try {
                    const messageIndex = parseInt(session.getAttribute('data-message-index'), 10);
                    session.setAttribute('data-message-index', (messageIndex + 1).toString());   
                }
                catch(error) {
                    console.error('appendMessage():', error);
                }
            }
        });
    }
}


export function terminalMenuClickCallback(event){
    
    event.stopPropagation();
    const guid = event.currentTarget.getAttribute('data-object-guid');
    
    selectTerminalSession(guid);
    selectTerminalMenuElement(guid);
}


export async function sessionScrollCallback(event){
    
    event.stopPropagation();
    
    const session = event.target;
    let msgIndex = parseInt(session.getAttribute('data-message-index'), 10);
    const guid = session.getAttribute('data-object-guid');

    if(event.target.scrollTop === 0 && msgIndex !== null && guid !== null){
        try {

            const messages = await fetchObjectMessages(guid, msgIndex.toString());            

            for(const message of messages) {

                const messageElement = templates.terminalMessage(message.sender, message.body) 
                messageElement.style.display = 'none';
                popInElement(messageElement, false);
                
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


export async function startSession(guid, alias){

    if(sessionExists(guid)){
        selectTerminalMenuElement(guid);
        selectTerminalSession(guid);
    } 
    
    else {

        const newSession = templates.terminalSession(guid);
        const menuElement = templates.terminalMenuElement(guid, alias);
        let msgIndex = 0;

        try {
            const messages = await fetchObjectMessages(guid, msgIndex);

            for(let i = messages.length - 1; i >= 0; i--){
                newSession.appendChild(templates.terminalMessage(messages[i].sender, messages[i].body));
                msgIndex++;
            }
        } 
        catch(error) {
            console.log(`${guid}: no messages found.`);
        }
        finally {
            newSession.setAttribute('data-message-index', msgIndex.toString());
        }

        document.querySelector('#terminal-menu .menu-list').appendChild(menuElement);
        document.querySelector('.terminal-instance').appendChild(newSession);
        
        selectTerminalMenuElement(guid);
        selectTerminalSession(guid);

        // Scroll down to the last message.
        try {
            newSession.lastChild.scrollIntoView({ behavior: "smooth" });
        } 
        catch(error) {
            //-
        }
    }
}


// This function does nothing if there is no active session with this GUID.
export function forceEndSession(guid){

    const sessions = document.querySelectorAll('.terminal-session');
    const terminalMenuElements = document.querySelectorAll('#terminal-menu .menu-list li');

    if(sessions !== null){
        sessions.forEach(session => {
            if(session.getAttribute('data-object-guid') === guid){
                session.remove();
            }
        });
    }

    if(terminalMenuElements !== null){
        terminalMenuElements.forEach(element => {
            if(element.getAttribute('data-object-guid') === guid){
                element.remove();
            }
        });
    }
}


function isInputSelected(){
    const activeElement = document.activeElement;
    return activeElement !== null && activeElement.getAttribute('id') === 'terminal-input'; 
}

function currentInputContent(){
    return document.getElementById('terminal-input').value.trim();
}

function clearInputContent(){
    document.getElementById('terminal-input').value = '';
}


export async function keyDownCallback(event){
    
    if(event.key === 'Enter' && isInputSelected()){
        
        const sessions = document.querySelectorAll('.terminal-session');
        const messageContent = currentInputContent();
        
        if(sessions !== null && messageContent.length > 0){
            for(const session of sessions){
                
                const guid = session.getAttribute('data-object-guid');
                if(session.style.display !== 'none' && guid !== null){
                    
                    try {
                        await sendObjectMessage(guid, messageContent);
                    }
                    
                    catch(error) {
                        console.error('keyDownCallback():', error);
                        appendNotification(`Invalid access level for object: ${guid}`, "bad");
                    }
                }
            }
        }

        clearInputContent();
    }
}