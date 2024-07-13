

export function isWithinBoundingRect(X, Y, element){

    const boundingRect = element.getBoundingClientRect();
    return (X >= boundingRect.left && X <= boundingRect.right &&
           Y >= boundingRect.top && Y <= boundingRect.bottom);
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