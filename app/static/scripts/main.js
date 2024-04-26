

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
}

main();