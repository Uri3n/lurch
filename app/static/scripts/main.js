console.log('starting...');

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



let terminalInstance = document.querySelector('.terminal-instance');
let notificationCenter = document.getElementById('notification-center');

if (notificationCenter === null || terminalInstance === null) {
    console.error('some elements are null');
}


else {

    const observer = new MutationObserver((mutations) => {
        mutations.forEach((mutation) => {
            // Check if the target element has become empty
            if (mutation.type === 'childList' && terminalInstance.children.length === 0) {
                // Handle the case where the element becomes empty
                mutation.target.innerHTML = '<img src="static/images/ui_empty_terminal.png" class="placeholder-image">'
            }
        });
    });


    const config = { childList: true };
    observer.observe(terminalInstance, config);



    const observer2 = new MutationObserver((mutations) => {
        mutations.forEach((mutation) => {
            // Check if the target element has become empty
            if (mutation.type === 'childList' && notificationCenter.children.length === 0) {
                // Handle the case where the element becomes empty
                mutation.target.innerHTML = '<img src="static/images/ui_empty_notifications.png" class="placeholder-image">'
            }
        });
    });


    const config2 = { childList: true };
    observer2.observe(notificationCenter, config2);
}


console.log('anchor element color:');
console.log((window.getComputedStyle(document.querySelector('.menu-list li a'))).color);