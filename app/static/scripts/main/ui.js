import { fetchObjectData, fetchObjectChildren } from "./fetch.js";  
import { templates } from "./templating.js";


async function appendListChildren(element){
    try {
        const children = await fetchObjectChildren(element.querySelector('a').textContent.trim().split(" ")[0]);
        const childList = document.createElement('ul');

        for(const child of children){
            const listElement = templates.listElement(child.guid, child.alias, child.type);
            childList.appendChild(listElement);    
        }

        element.appendChild(childList);

    } catch(error) {
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


export async function appendListElement(guid){
    try {
        const json = await fetchObjectData(guid);
        await appendListElement_r(guid, json, document.querySelector('#object-menu .menu-list'));

    } catch(error){
        console.error("appendListElement(): ",error);
        return;
    }
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
            
            parent.remove();
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


export function terminalMenuClickCallback(event){
    
    event.stopPropagation();
    const guid = event.currentTarget.getAttribute('data-object-guid');
    
    selectTerminalSession(guid);
    selectTerminalMenuElement(guid);
}


export function startSession(guid, alias){

    if(sessionExists(guid)){
        selectTerminalMenuElement(guid);
        selectTerminalSession(guid);
    } 
    
    else {
        const newSession = templates.terminalSession(guid);
        const menuElement = templates.terminalMenuElement(guid, alias);

        document.querySelector('#terminal-menu .menu-list').appendChild(menuElement);
        document.querySelector('.terminal-instance').appendChild(newSession);

        selectTerminalMenuElement(guid);
        selectTerminalSession(guid);
    }
}