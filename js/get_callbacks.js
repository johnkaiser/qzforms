/*
 *  Find a named callback.
 *  Starting from any form element look for the callback in the current form.
 *
 */

function get_callback(a_form_element, callback_name){

    let this_form = false;

    if ((a_form_element) && (a_form_element.tagName == 'form')){
        this_form = a_form_element;
    }else{
        this_form = a_form_element.form;
    }

    if ( ! this_form ){
        console.log('get_callback failed on the starting form element');
        return false;
    }

    let cb_class = this_form.getElementsByClassName('__CALLBACKS__')
    let cb_el = false;

    for (let el_cnt = 0; cb_class.length < el_cnt; el_cnt++){
        if (cb_class.item(el_cnt).value.trim() == callback_name){
            cb_el = cb_class.item(el_cnt);
            break;
        }
    }

    if ( ! cb_el){
        console.log('get_callback called but ', callback_name, ' not found');
        return false;
    }
    return cb_el;
}
