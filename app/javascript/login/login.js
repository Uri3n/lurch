
function popInElement(element){
    
    if(element.style.visibility === 'hidden') {
        element.style.visibility = 'visible';                  
        element.style.animation = 'popIn 0.3s ease';                
        
        element.addEventListener('animationend', () => {
            element.style.animation = 'none';
        }, {once:true});
    }
}


function popOutElement(element) {

    if(element.style.visibility === 'visible') {
        element.style.animation = 'popOut 0.3s ease';    
        element.style.transition = 'opacity 0.3s ease';
        
        element.addEventListener('animationend', () => {
            element.style.animation = 'none';
            element.style.transition = 'none';              
            element.style.visibility = 'hidden';    
        }, {once:true});
    }
}


async function fetchToken(username, password){
    
    const encodedCredentials = btoa(username + ':' + password);
    const response = await fetch("/verify", {
        method : "POST",
        headers : {
            Authorization : "Basic " + encodedCredentials
        }
    });

    if(!response.ok) {
        throw new Error(response.status);
    }

    const token = await response.text();
    return token;
}


async function loadMain(token) {

    const response = await fetch("/main", {
        method : "GET",
        headers : {
            Authorization : "Bearer " + token
        }
    });

    if(!response.ok){
        throw new Error(response.status);
    }

    const text = await response.text();
    const newDocument = new DOMParser().parseFromString(text, 'text/html');
    newDocument.body.setAttribute('data-temp-token', token);
    
    document.open();
    document.write(newDocument.documentElement.outerHTML);
    document.close();
}


function appendErrorNotification(msg){
    
    const notification = document.querySelector('.notification');
    notification.querySelector('p').textContent = msg.split("").filter(char => char !== "<").join(""); //let me cook
    popInElement(notification);
}


async function submitCredentials(){

    const username = document.getElementById('username').value.trim()
    const password = document.getElementById('password').value.trim();

    if(username.length === 0 || password.length === 0){
        appendErrorNotification("Empty username or password!");
        return;
    }

    try {
        const token = await fetchToken(username, password);
        await loadMain(token);
    }
    catch(error) {
        console.error('submitCredentials():', error);
        appendErrorNotification('Invalid username or password.')
    }
}


document.getElementById('submit').addEventListener('click', submitCredentials);
document.addEventListener('keydown', (event) => {
    if(event.key === 'Enter'){
        if(document.activeElement === document.getElementById('username')){
            document.getElementById('password').focus();
        }
        
        else {
            submitCredentials();
        }
    }
});


document.querySelector('.delete').addEventListener('click', (event) => {
    let parent = event.target.parentElement;

    do {
        if (parent.classList.contains('deletable-parent')) {
            popOutElement(parent);
            return;
        }

        parent = parent.parentElement;
    } while (parent);
});
