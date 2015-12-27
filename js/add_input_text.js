/*
 *  add_input_text
 *
 *  The simplest case, <input type=text...
 *  except you don't have to type it.
 */

function add_input_text(parent_el, fieldname, prompt_rule, row_index){

    var new_input = document.createElement("input");
    
    var id;
    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
    }
    new_input.setAttribute("name", id);
    new_input.setAttribute("id", id);
    new_input.setAttribute("type", "text");
    
    if ('size' in prompt_rule){
        new_input.setAttribute('size', prompt_rule['size']);
    }

    if ('maxlength' in prompt_rule){
        new_input.setAttribute('maxlength', prompt_rule['maxlength']);
    } 

    parent_el.appendChild(new_input);
    
    set_common_attributes(new_input, fieldname, prompt_rule, row_index);

    return new_input;
}

