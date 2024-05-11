import {observeElement, addDeleteButtonHandlers} from './observer.js';
import {startWebsocket} from './ws.js';

import {
    sendObjectMessage, 
    fetchObjectMessages, 
    fetchObjectData, 
    fetchObjectChildren
} from './fetch.js';

import {
    appendListElement, 
    listElementClickCallback, 
    listElementDragStartCallback, 
    listElementDragEndCallback, 
    terminalMenuClickCallback,
    keyDownCallback
} from './ui.js';

//-------------------------------------------------------------------------------------------

function main(){

    //
    // initial mutation observer targets
    //

    observeElement(document.querySelector('.terminal-instance'));
    observeElement(document.getElementById('notification-center'));


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

    startWebsocket();


    //
    // submit field event listener
    //

    document.addEventListener('keydown', keyDownCallback);
    addDeleteButtonHandlers();
}

main();