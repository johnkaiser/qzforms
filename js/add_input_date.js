/*
 *  add_input_date
 *
 *  A calender date picker
 */

function add_input_date(parent_el, fieldname, prompt_rule, row_index){

    var new_input = document.createElement("input");
    
    var id;
    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
    }
    new_input.setAttribute("name", id);
    new_input.setAttribute("id", id);
    new_input.setAttribute("type", "date");
    
    parent_el.appendChild(new_input);
    
    set_common_attributes(new_input, fieldname, prompt_rule, row_index);

    return new_input;
}

