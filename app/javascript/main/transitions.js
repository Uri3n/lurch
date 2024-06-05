export function fadeInElement(element){

    element.style.animation = 'fadeIn 0.2s ease';
    if(element.style.display === 'none') {
        element.style.display = 'flex';
    }

    element.addEventListener('animationend', () => {
        element.style.opacity = 100;
        element.style.animation = 'none';
    },{ once: true });
}


export function fadeOutElement(element, config){

    element.style.animation = 'fadeOut 0.3s ease';
    
    element.addEventListener('animationend', () => {
        element.style.animation = 'none';
        
        if(config) {
            if(config.disableDisplay) {
                element.style.display = 'none';
            }
    
            if(config.disableOpacity) {
                element.style.opacity = '0';
            }
    
            if(config.remove) {
                element.remove();
            }
    
            if(config.callback) {
                config.callback();
            }
        }

    },{ once: true });
}


export function popInElement(element, scrollTo){

    if(element.style.display !== 'none'){                           // element must be hidden initially for this to work
        console.error(`popInElement(): passed element: ${element}\n
                       without display set to "none".`);
        return;
    }

    setTimeout(() => {
        
        element.style.opacity = 0;
        element.style.display = 'flex';                             // small delay to ensure element is loaded
        element.style.animation = 'popIn 0.1s ease';                // play "pop in" animation
        
        element.addEventListener('animationend', () => {
            fadeInElement(element);
        },{ once: true });

        if(scrollTo){
            element.scrollIntoView({behavior : 'smooth'});          // scroll towards the newly added element
        }

    }, 10);
}


export function popOutElement(element, remove) {

    element.style.animation  = 'popOut 0.3s ease';
    element.style.transition = 'opacity 0.3s ease';
    
    element.addEventListener('animationend', () => {
        if(remove) {
            element.remove();
        } else {
            element.style.animation  = 'none';
            element.style.transition = 'none';
        }
    });
}