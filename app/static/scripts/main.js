
//-------------------------------------------------

const g_Observer = new MutationObserver(mutationCallback);

//-------------------------------------------------


function observeElement(newElement){

    const observerConfig = {
        childList : true,
        attributes: true //may need this later
    };

    g_Observer.observe(newElement, observerConfig);
}


function mutationCallback(mutationList, observer){
    mutationList.forEach(mutation => {
        if(mutation.type === 'childList' && mutation.target.children.length === 0){
            
            const targetId = mutation.target.getAttribute('id');
            if(targetId !== null && targetId === 'notification-center'){    //Short circuit evaluated, should be OK.
                mutation.target.innerHTML = '<img src="static/assets/ui_empty_notifications.png" class="placeholder-image">'
            }

            else if(mutation.target.classList.contains('terminal-instance')){
                mutation.target.innerHTML = '<img src="static/assets/ui_empty_terminal.png" class="placeholder-image">'
            }
        }
    })
}


function addDeleteButtonHandlers(){
    const deleteButtons = document.querySelectorAll('.delete');
    
    if(deleteButtons !== null){
        deleteButtons.forEach((button) => {
           button.addEventListener('click', () => {
                let parent = button.parentElement;

                do{
                    if(parent.classList.contains('deletable-parent')){
                        parent.remove();
                    }

                    parent = parent.parentElement;
                } while(parent);
           }); 
        });
    }
}


function main(){
    
    addDeleteButtonHandlers();
    observeElement(document.getElementById('notification-center'));

    document.querySelectorAll('.terminal-instance').forEach( element => {
        observeElement(element);
    });

}

main();