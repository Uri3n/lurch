import {observeElement, addDeleteButtonHandlers} from './observer.js';
import {startWebsocket} from './ws.js';
import {sendObjectMessage, fetchObjectMessages, fetchObjectData, fetchObjectChildren} from './fetch.js';

//-------------------------------------------------------------------------------------------



async function main(){
    
    addDeleteButtonHandlers();
    observeElement(document.getElementById('notification-center'));

    document.querySelectorAll('.terminal-instance').forEach( element => {
        observeElement(element);
    });


}

main();