import { observeElement } from './observer.js'; 

import {
    listElementClickCallback,
    listElementDragStartCallback,
    listElementDragEndCallback,
    deleteButtonCallback,
    terminalMenuClickCallback,
    sessionScrollCallback
} from './ui.js';



export const templates = {
    
    //
    // DOMParser.parseFromString() returns an ENTIRE html document.
    // therefore, we need to return the first child within the document's body as the new element.
    //


    terminalMessage : (headerContent, content) => {

        // temporary shit fix for this retarded word wrapping issue
        let replacementContent = '';
        for(let i = 0, k = 0; i < content.length; i++, k++){
            
            if(content[i] === '\n'){
                k = 0;
            }

            if( k > 0 && k % 100 === 0 ){
                replacementContent += '\n';
            }

            replacementContent += content[i];
        }

        const template = `<div class="terminal-instance-element">
                            <div class="message is-small">
                                <div class="message-header">
                                    <p>${headerContent}</p>
                                </div>
                                <div class="message-body">
                                    ${replacementContent}
                                </div>
                            </div>
                        </div>`;

        const parser = new DOMParser();
        const element = parser.parseFromString(template, 'text/html').body.firstChild;
        
        return element;
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
        const element = parser.parseFromString(template, 'text/html').body.firstChild;
        
        element.addEventListener('scroll', sessionScrollCallback);
        observeElement(element);
        
        return element;
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
    },


    neutralNotification : (content) => {

        const template = `<div class="notification deletable-parent"> 
                            <button class="delete"></button>
                            ${content.replace(/[<>]/g, '')}
                          </div>`;

        const parser = new DOMParser();
        const element = parser.parseFromString(template, 'text/html').body.firstChild;
        element.querySelector('.delete').addEventListener('click', deleteButtonCallback);
        
        return element;
    },


    badNotification : (content) => {

        const template = `<div class="notification deletable-parent is-danger"> 
                            <button class="delete"></button>
                            ${content.replace(/[<>]/g, '')}
                          </div>`;

        const parser = new DOMParser();
        const element = parser.parseFromString(template, 'text/html').body.firstChild;
        element.querySelector('.delete').addEventListener('click', deleteButtonCallback);
        
        return element;
    },


    goodNotification : (content) => {

        const template = `<div class="notification deletable-parent is-link"> 
                            <button class="delete"></button>
                            ${content.replace(/[<>]/g, '')}
                          </div>`;

        const parser = new DOMParser();
        const element = parser.parseFromString(template, 'text/html').body.firstChild;
        element.querySelector('.delete').addEventListener('click', deleteButtonCallback);
        
        return element;
    },

};