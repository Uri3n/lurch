import {observeElement, addDeleteButtonHandlers} from './observer.js';
import {startWebsocket} from './ws.js';
import { setToken } from './fetch.js';
import * as cb from './callbacks.js';

//-------------------------------------------------------------------------------------------


function main(){

    const token = document.querySelector('body').getAttribute('data-temp-token');
    document.querySelector('body').removeAttribute('data-temp-token');
    setToken(token);


    //
    // init root list element
    //

    const rootListElement = document.getElementById('object-menu').querySelector('.menu-list li');
    
    rootListElement.addEventListener('click',                           cb.listElementClickCallback);
    rootListElement.querySelector('a').addEventListener('dragstart',    cb.listElementDragStartCallback);
    rootListElement.querySelector('a').addEventListener('dragend',      cb.listElementDragEndCallback);


    //
    // init websocket
    //

    startWebsocket(token);


    //
    // submit field event listener
    //

    document.addEventListener('keydown', cb.keyDownCallback);


    //
    // initial mutation observer targets
    //

    observeElement(document.querySelector('.terminal-instance'));
    observeElement(document.getElementById('notification-center'));


    //
    // init drag & drop
    //

    ['drop', 'dragover', 'dragleave', 'dragenter'].forEach(eventName => {
        document.body.addEventListener(eventName, (event) => {
            event.preventDefault();
        });
    });

    document.body.addEventListener('drop', () => {
        const terminal = document.querySelector('.terminal-instance');
        const filedrop = document.querySelector('.file-drop-icon');
        terminal.style.opacity = 1;
        filedrop.style.opacity = 0;
    });

    document.querySelector('.terminal-instance').addEventListener('drop', cb.terminalDropCallback);
    document.body.addEventListener('dragover', cb.dragoverCallback);

}

main();