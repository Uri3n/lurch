

function appendErrorNotification(msg){
    
    msg = msg.split("").filter(char => char !== "<").join("");

    const notification = document.createElement('div');
    notification.classList.add('notification', 'is-danger', 'deletable-parent');
    notification.innerHTML = `
        <button class="delete"></button>                     
        ${msg}
    `;
    
    
    //Delete existing notification if one exists
    document.querySelectorAll('.notification').forEach((element) => {
        element.remove();
    });

    document.getElementById('login-container').prepend(notification);

    // ensure notification is deleted when button is clicked
    document.querySelectorAll('.delete').forEach((element) => {
        element.addEventListener('click', () => {
            let parent = element.parentElement;
    
            do {
                if (parent.classList.contains('deletable-parent')) {
                    parent.remove();
                    return;
                }
    
                parent = parent.parentElement;
            } while (parent);
        });
    });
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

async function submitCredentials(){

    const username = document.getElementById('username').value.trim()
    const password = document.getElementById('password').value.trim();

    if(username.length === 0 || password.length === 0){
        appendErrorNotification("empty username or password!");
    }

    try {
        const token = await fetchToken(username, password);
        await loadMain(token);
    }
    catch(error) {
        console.error('submitCredentials():', error);
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
})
