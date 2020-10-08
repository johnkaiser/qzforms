/*
 *  Find and return the array of valid callbacks for a form.
 */

function get_callbacks(){
    let cb_el = document.getElementById('__CALLBACKS__');
    if ( ! cb_el){
        console.log('get_callbacks called but __CALLBACKS__ not found');
        return false;
    }
    return JSON.parse(cb_el.innerHTML);

}
