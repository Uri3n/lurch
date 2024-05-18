import {observeElement, addDeleteButtonHandlers} from './observer.js';
import {startWebsocket} from './ws.js';
import { setToken } from './fetch.js';

import { 
    listElementClickCallback, 
    listElementDragStartCallback, 
    listElementDragEndCallback, 
    terminalMenuClickCallback,
    keyDownCallback,
    terminalDragEnterCallback,
    terminalDragLeaveCallback,
    terminalDropCallback,
    dragoverCallback
} from './ui.js';



//-------------------------------------------------------------------------------------------


function main(){

    //
    // store in-memory auth token
    //

    const token = document.querySelector('body').getAttribute('data-temp-token');
    document.querySelector('body').removeAttribute('data-temp-token');
    setToken(token);


    //
    // init root list element
    //

    const rootListElement = document.getElementById('object-menu').querySelector('.menu-list li');
    
    rootListElement.addEventListener('click', listElementClickCallback);
    rootListElement.querySelector('a').addEventListener('dragstart', listElementDragStartCallback);
    rootListElement.querySelector('a').addEventListener('dragend', listElementDragEndCallback);


    //
    // init websocket
    //

    startWebsocket(token);


    //
    // submit field event listener
    //

    document.addEventListener('keydown', keyDownCallback);


    //
    // initial mutation observer targets
    //

    observeElement(document.querySelector('.terminal-instance'));
    observeElement(document.getElementById('notification-center'));


    //
    // init terminal drag & drop
    //

    const preventDefaults = (event) => {
        event.preventDefault();
    };
    
    ['drop', 'dragover', 'dragleave', 'dragenter'].forEach(eventName => {
        document.body.addEventListener(eventName, preventDefaults);
    });
     

    document.querySelector('.terminal-instance').addEventListener('drop', terminalDropCallback);
    document.body.addEventListener('dragover', dragoverCallback);
    document.body.addEventListener('drop', () => {
        console.log('fired');
        const terminal = document.querySelector('.terminal-instance');
        const filedrop = document.querySelector('.file-drop-icon');
        terminal.style.opacity = 1;
        filedrop.style.opacity = 0;
    });
}

main();