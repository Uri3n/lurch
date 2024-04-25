

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
}


document.getElementById('submit').addEventListener('click',()=> {
    
    let username = document.getElementById('username').value.trim()
    let password = document.getElementById('password').value.trim();

    if(username.length === 0 || password.length === 0){
        appendErrorNotification("empty username or password!");
    }


    const encodedCredentials = btoa(username + ':' + password);
    
    fetch("/main", {
        method: "GET",
        headers:{
            Authorization: "basic " + encodedCredentials
        }
    })
        .then(response => {
            if(!response.ok){
                throw new Error("invalid username or password!");
            }
            
            return response.text();
        })
        .then(text => {
            const newDocument = new DOMParser().parseFromString(text, 'text/html');
            
            document.open();
            document.write(newDocument.documentElement.outerHTML);
            document.close();
        })
        .catch(error => {
            appendErrorNotification(error.toString());
        });
});