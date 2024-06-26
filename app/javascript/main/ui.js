import * as transitions from './transitions.js';
import { templates } from "./templating.js";

import { 
    fetchObjectChildren, 
    fetchObjectMessages, 
} from "./fetch.js";  


export async function appendListChildren(element){
    
    try {
        const children  = await fetchObjectChildren(element.querySelector('a').textContent.trim().split(" ")[0]);
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


export function deleteListChildren(element){
    const childList = element.querySelector('ul');
    if(childList !== null) {
        childList.remove();
    }
}


export function deleteTerminalSession(guid){
    
    const sessions = document.querySelectorAll('.terminal-session');
    let previousGuid = null;
    let currentGuid  = null;
    let removed      = false;

    if(sessions !== null){
        for(const session of sessions) {
            currentGuid = session.getAttribute('data-object-guid');
            if(currentGuid === guid) {
                session.remove();
                removed = true;
                break;
            }

            previousGuid = currentGuid;
        }

        //
        // if there's a session that's before the one we just removed, select it.
        // if the one we just removed was the first one, select the new first one.
        //

        if(removed) {
            const first = document.querySelector('.terminal-session');

            if(previousGuid !== null) {
                selectTerminalSession(previousGuid);
                selectTerminalMenuElement(previousGuid); 
            }
            else if(first !== null){
                selectTerminalSession(first.getAttribute('data-object-guid'));
                selectTerminalMenuElement(first.getAttribute('data-object-guid'));
            }
        }
    }
}


export function deselectListElement(element){
    
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


export function selectListElement(element){
    
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


export function isWithinBoundingRect(X, Y, element){

    const boundingRect = element.getBoundingClientRect();
    return (X >= boundingRect.left && X <= boundingRect.right &&
           Y >= boundingRect.top && Y <= boundingRect.bottom);
}


function deleteListElement_r(guid, element){

    const children = element.children;
    for(const child of children){
        
        const anchorTag = child.querySelector('a');
        const subList   = child.querySelector('ul');
    
        if(anchorTag.textContent.trim().split(" ")[0] === guid) {
            transitions.fadeOutElement(child, {remove: true});                                 // delete the list element
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
        
        let anchorTag = child.querySelector('a');
        let subList   = child.querySelector('ul');

        if(anchorTag.textContent.trim().split(" ")[0] === data.parent && anchorTag.classList.contains('is-active')){
            
            const newChild = templates.listElement(guid, data.alias, data.type);
            
            if(subList !== null) {
                subList.appendChild(newChild);
            } else {
                subList = document.createElement('ul');
                subList.appendChild(newChild);
                child.appendChild(subList);
            }

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


export function selectTerminalSession(guid){

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


export function selectTerminalMenuElement(guid){

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


export function sessionExists(guid){

    let exists = false;

    Array.from(document.querySelector('#terminal-menu .menu-list').children).forEach(element => {
        if(element.getAttribute('data-object-guid') === guid) {
            exists = true;
        }
    });

    return exists;
}


export function currentSessionGuid() {

    const sessions = document.querySelectorAll('.terminal-session');
    
    if(sessions !== null) {
        for(const session of sessions) {           
            const guid = session.getAttribute('data-object-guid');
            if(session.style.display !== 'none' && guid !== null) {
                return guid;
            }
        }
    }

    return null;
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
        
        case "neutral":
            notification = templates.neutralNotification(body);
            break;

        default:
            notification = templates.neutralNotification(body);
            break;
    }

    notification.style.display = 'none';
    notificationCenter.appendChild(notification);
    
    transitions.popInElement(notification, true);
}


export function appendMessage(body, sender, recipient, timestamp){

    const sessions = document.querySelectorAll('.terminal-session');
    if(sessions !== null){
        sessions.forEach(session => {
            if(session.getAttribute('data-object-guid') === recipient){

                const terminalMessage = templates.terminalMessage(sender, body, timestamp);
                
                terminalMessage.style.display = 'none';                                 
                session.appendChild(terminalMessage);                                   
                transitions.popInElement(terminalMessage, true);
                
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


export function shuffleNotifications(elementToRemove, sibling) {

    const removedHeight = elementToRemove.offsetHeight;
    const removedMargin = window.getComputedStyle(elementToRemove).marginBottom.replace('px','');

    elementToRemove.remove();

    if(!sibling) {
        return;
    }
    
    
    sibling.style.marginTop = String(parseInt(removedHeight,10) + parseInt(removedMargin,10)) + 'px';
    sibling.style.animation = 'notificationAdjust 0.2s ease-in-out';
    
    sibling.addEventListener('animationend', () => {
        
        sibling.style.animation = 'none';
        sibling.style.marginTop = 0;
    
    }, { once: true });
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
                newSession.appendChild(templates.terminalMessage(messages[i].sender, messages[i].body, messages[i].timestamp));
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
            //--
        }
    }
}


// This function does nothing if there is no active session with this GUID.
export function forceEndSession(guid){

    const sessions = document.querySelectorAll('.terminal-session');
    const terminalMenuElements = document.querySelectorAll('#terminal-menu .menu-list li');
    let previous_session = null;
    let found = false;

    
    if(sessions !== null){
        for(let session of sessions) {
            const curr_guid = session.getAttribute('data-object-guid');
            if(curr_guid === guid){
                session.remove();
                found = true;
                break;
            }

            previous_session = curr_guid;
        }
    }

    if(!found) {
        return;
    }


    if(terminalMenuElements !== null){
        for(let element of terminalMenuElements) {
            if(element.getAttribute('data-object-guid') === guid){
                element.remove();
                break;
            }
        }
    }


    if(previous_session !== null) {
        selectTerminalSession(previous_session);
        selectTerminalMenuElement(previous_session);
    }
    else if(document.querySelector('.terminal-session') !== null){
        previous_session = document.querySelector('.terminal-session').getAttribute('data-object-guid');
        selectTerminalSession(previous_session);
        selectTerminalMenuElement(previous_session);
    }
}


export function isInputSelected(){
    const activeElement = document.activeElement;
    return activeElement !== null && activeElement.getAttribute('id') === 'terminal-input'; 
}

export function currentInputContent(){
    return document.getElementById('terminal-input').value.trim();
}

export function clearInputContent(){
    document.getElementById('terminal-input').value = '';
}

export function focusInputElement() {
    document.getElementById('terminal-input').focus();
}


export function isTerminalFullscreened() {
    const fullscreen = document.getElementById('terminal-container').getAttribute('data-is-fullscreen');
    return fullscreen !== null && fullscreen !== undefined && fullscreen === 'true';
}

export function fullscreenTerminalSession() {

    const terminalInstance  = document.querySelector('.terminal-instance');
    const terminalContainer = document.getElementById('terminal-container');
    const terminalMenu      = document.getElementById('terminal-menu');
    const objectContainer   = document.getElementById('object-container'); 
    const terminalInput     = document.getElementById('terminal-input');

    if(isTerminalFullscreened() === true) {
        console.warn('Attempted to fullscreen terminal window while already fullscreened.');
        return;
    }


    transitions.fadeOutElement(terminalMenu,    { disableDisplay : true });
    transitions.fadeOutElement(objectContainer, { disableDisplay : true, callback : () => {
        
        terminalContainer.style.marginTop   = '45vh';
        terminalInstance.style.width        = '100%';
        terminalInput.style.flexGrow        = '1';
        
        terminalInstance.addEventListener('transitionend', () => {
            terminalContainer.style.animation = 'terminalContainerTopExtend 0.5s ease-in-out';
            terminalContainer.addEventListener('animationend', () => {
                
                terminalContainer.style.animation   = 'none';
                terminalContainer.style.marginTop   = 0;
                terminalContainer.style.height      = '90vh';
            
            },{ once: true });
        },{ once: true });
    }});

    terminalContainer.setAttribute('data-is-fullscreen', 'true');
}


export function minimizeTerminalFullscreen() {
    
    const terminalInstance  = document.querySelector('.terminal-instance');
    const terminalContainer = document.getElementById('terminal-container');
    const terminalMenu      = document.getElementById('terminal-menu');
    const objectContainer   = document.getElementById('object-container'); 
    const terminalInput     = document.getElementById('terminal-input');

    if(isTerminalFullscreened() === false) {
        console.warn('Attempted to minimize terminal window while not fullscreened.');
        return;
    }


    terminalContainer.style.animation = 'terminalContainerTopShrink 0.5s ease-in-out';
    terminalContainer.addEventListener('animationend', () => {
       
        terminalContainer.style.animation   = 'none';
        terminalContainer.style.marginTop   = '45vh';
        terminalContainer.style.height      = '55vh';

        terminalInstance.style.width = '80%';
        terminalInput.style.flexGrow = '0';

        terminalInstance.addEventListener('transitionend', () => {
            
            terminalContainer.style.marginTop = 0;
            transitions.fadeInElement(terminalMenu);
            transitions.fadeInElement(objectContainer);

        }, { once: true });
    },{ once: true });

    terminalContainer.setAttribute('data-is-fullscreen', 'false');
}