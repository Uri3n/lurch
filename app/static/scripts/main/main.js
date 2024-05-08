import {observeElement, addDeleteButtonHandlers} from './observer.js';
import {startWebsocket} from './ws.js';
import {sendObjectMessage, fetchObjectMessages, fetchObjectData, fetchObjectChildren} from './fetch.js';
import {appendListElement, listElementClickCallback, listElementDragStartCallback, listElementDragEndCallback, terminalMenuClickCallback} from './ui.js';

//-------------------------------------------------------------------------------------------

async function main(){
    

    //---------------------------------------------------------------------------------------
    addDeleteButtonHandlers();
    observeElement(document.getElementById('notification-center'));

    document.querySelectorAll('.terminal-session').forEach( element => {
        observeElement(element);
    });

    observeElement(document.querySelector('.terminal-instance'));

    //---------------------------------------------------------------------------------------


    //
    // init root list element
    //

    const rootListElement = document.getElementById('object-menu').querySelector('.menu-list li');
    
    rootListElement.addEventListener('click', listElementClickCallback);
    rootListElement.querySelector('a').addEventListener('dragstart', listElementDragStartCallback);
    rootListElement.querySelector('a').addEventListener('dragend', listElementDragEndCallback);

    document.querySelectorAll('#terminal-menu .menu-list li').forEach(element => {
        element.addEventListener('click', terminalMenuClickCallback);
    });

}

main();