/*
 *  Use callback_name to get the options for node field_name.
 *  field_name may refer to a SELECT tag or an INPUT tag.
 *  The callback must have a field "value", which will be the
 *  value of the created options.
 *  In the case of a SELECT tag, the callback may have a field "text",
 *  which will be the text displayed in the drop down list.
 *
 *  To use, add a line of javascript like this:
 *
 *  window.addEventListener("DOMContentLoaded", 
 *  () => callback_options('my_callback_name', 'my_input_or_select_name'));
 */

function callback_options(callback_name, field_name){

    var xhr = new XMLHttpRequest();

    function set_options(){
      
        /*
         *  Find the given form, field, and put xhr value and text in
         *  html options under the given field.
         *  The field can be a select, options or an input, datalist.
         */
    
        console.log('set_options for form ' + xhr.form_name + 
            'field ' + field_name);
    
        if (xhr.status != 200){
            console.log('set_options exiting xhr_status = ' + xhr.status);
            return;
        }
        /*
         * new_options will contain the result of the callback.
         */
        let new_options = JSON.parse(xhr.responseText);
        let this_form = false;
        let this_el = false;
        let match_found = false;
        
        let selected = [];
    
        if (new_options){
    
            this_el = document.forms[xhr.form_name][field_name];
            if (this_el){
                console.log('new_options found, tag name = ' + this_el.tagName);
            }else{
              console.log('new_options, node "' + fieldname + '" not found');
              return;
            }
            let option_el = false;
    
            /*
             *  If the tagname is input, then this is a datalist.
             */
            if (this_el.tagName.match(/INPUT/i)){
                match_found = true;
    
                // If there is a current datalist, remove the contents.
                let datalist_id = this_el.getAttribute("list");
                let datalist_el = false;
    
                if (datalist_id){
                    datalist_el = document.getElementById(datalist_id);
                    if (datalist_el){
                        while (datalist_el.childElementCount > 0){
                            datalist_el.removeChild(datalist_el.childNodes[0]);
                        }    
                    }else{
                        // id present but no datalist, odd
                        datalist_el = document.createElement("datalist");
                        datalist_el.setAttribute("id", datalist_id);
                        this_el.parentNode.appendChild(datalist_el);
                    }
                }else{
                    // no list id, no datalist
                    // Create the datalist node and id
                    datalist_id = "id" + Math.floor(Math.random() * 
                        1000000).toString();

                    this_el.setAttribute("list", datalist_id);
                    
                    datalist_el = document.createElement("datalist");
                    datalist_el.setAttribute("id", datalist_id);
                    this_el.parentNode.appendChild(datalist_el);
                }
    
                // Add the new options to the datalist
                for (let n=0; n < new_options.length; n++){
                    let newopt_el = document.createElement("option");
                    newopt_el.setAttribute("value", new_options[n]['value']);
                    datalist_el.appendChild(newopt_el);
                }
            }
    
            /*
             * If the tagname is select, then it is a select, options
             */
            if (this_el.tagName.match(/SELECT/i)){
                match_found = true;
    
                // Remove the current items 
                // keeping track of what was selected
                for (let k = (this_el.options.length - 1); k >= 0; k--){
                    if (this_el.options[k].selected){
                        selected.push(this_el.options[k].value);
                    }
                    this_el.remove(k);
                }
    
                // Add the new options under the select
                for (let n=0; n < new_options.length; n++){
                    let new_value = new_options[n]['value'];
    
                    // Use value for text if text not present 
                    let new_text = new_options[n]['text'];
                    if ( ! new_text ) new_text = new_value;
        
                    this_el[n] = new Option(new_text, new_value);
                    if (selected.includes(new_value)){
                        this_el[n].selected = true;
                    }
                }
            }
            /*
             * If it was neither input, nor select, then why are we here?
             */
            if ( ! match_found ){
                console.log('set_options failed to find an input or select node.',
                    ' looked for form "' + xhr.form_name + '" field "',
                    field_name + '"');
            }
        }else{
            // new_options is false
            console.log('set_options failed to find options from callback');
        }
    }

    setup_callback(callback_name, set_options, xhr);
}
