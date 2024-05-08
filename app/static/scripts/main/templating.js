import {listElementClickCallback, listElementDragStartCallback, listElementDragEndCallback, deleteButtonCallback, terminalMenuClickCallback} from './ui.js';
import { observeElement } from './observer.js'; 

export const templates = {
    
    //
    // DOMParser.parseFromString() returns an ENTIRE html document.
    // therefore, we need to return the first child within the document's body as the new element.
    //


    notification : (content) => {
        const template = `<div class="notification deletable-parent"> 
                            <button class="delete"></button>
                            ${content.replace(/[<>]/g, '')}
                          </div>`;

        const parser = new DOMParser();
        const doc = parser.parseFromString(template, 'text/html');
        
        return doc.body.firstChild;
    },


    terminalMessage : (headerContent, content) => {
        const template = `<div class="terminal-instance-element message deletable-parent">
                            <div class="message-header">
                                <p>${headerContent.replace(/[<>]/g, '')}</p>
                                <button class="delete" aria-label="delete"></button>
                            </div>
                            <div class="message-body">
                                ${content.replace(/[<>]/g, '')}
                            </div>
                        </div>`;

        const parser = new DOMParser();
        const doc = parser.parseFromString(template, 'text/html');

        return doc.body.firstChild;
    },


    terminalMenuElement : (guid, alias) => {

        const template = `<li data-object-guid="${guid}" class="deletable-parent">
                            <a>${alias}</a> 
                            <button class="delete" aria-label="delete"> 
                          </li>`;

        const parser = new DOMParser();
        const doc = parser.parseFromString(template, 'text/html');

        doc.querySelector('li').addEventListener('click', terminalMenuClickCallback);
        doc.querySelector('.delete').addEventListener('click', deleteButtonCallback);

        return doc.body.firstChild;
    },


    terminalSession : (guid) => {
        
        const template = `<div class="terminal-session" data-object-guid="${guid}" style="display:none;">
                            <img src="static/assets/ui_empty_terminal.png" class="placeholder-image">
                          </div>`
    
        const parser = new DOMParser();
        const doc = parser.parseFromString(template, 'text/html');
        observeElement(doc);
        
        return doc.body.firstChild; 
    },


    listElement : (guid, alias, type) => {

        const li = document.createElement('li');
        const a = document.createElement('a');
    
        li.setAttribute('data-object-type', type);
        li.addEventListener('click', listElementClickCallback);
        
        a.textContent = `${guid} :: ${alias}`
        a.setAttribute('draggable', 'true');
        a.addEventListener('dragstart', listElementDragStartCallback);
        a.addEventListener('dragend', listElementDragEndCallback);
    
        li.appendChild(a);
        return li;
    }

};