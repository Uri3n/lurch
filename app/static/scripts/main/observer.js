import { deleteButtonCallback } from "./ui.js";

//-------------------------------------------------
export const observer = new MutationObserver(mutationCallback);
//-------------------------------------------------


export function observeElement(newElement){

    const observerConfig = {
        childList : true,
        attributes: true 
    };

    observer.observe(newElement, observerConfig);
}


function mutationCallback(mutationList, observer){
    mutationList.forEach(mutation => {
        if(mutation.type === 'childList'){

            const placeholderImage = mutation.target.querySelector('.placeholder-image');
            
            if(placeholderImage !== null){
                if(mutation.target.children.length === 1){
                    placeholderImage.style.display = 'flex';
                }
                
                else {
                    placeholderImage.style.display = 'none';    
                }
            }
        }
    })
}


export function addDeleteButtonHandlers(){
    const deleteButtons = document.querySelectorAll('.delete');
    
    if(deleteButtons !== null){
        deleteButtons.forEach((button) => {
           button.addEventListener('click', deleteButtonCallback);
        });
    }
}