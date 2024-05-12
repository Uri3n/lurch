import {observeElement, addDeleteButtonHandlers} from './observer.js';
import {startWebsocket} from './ws.js';
import { setToken } from './fetch.js';

import { 
    listElementClickCallback, 
    listElementDragStartCallback, 
    listElementDragEndCallback, 
    terminalMenuClickCallback,
    keyDownCallback
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
}

main();