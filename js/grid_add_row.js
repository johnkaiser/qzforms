/*
 *  grid_add_row
 *
 *  Add a new row to the grid table.
 */ 

function grid_add_row(){

    var next_index = get_next_row_index();

    var grid_table = document.getElementById('grid_edit_tbody');
    if (!grid_table){
        console.log('id grid_edit_tbody not found');
        return;
    }
 
    var tr = document.createElement("tr");
    var td = document.createElement("td");
    tr.appendChild(td);
    
    var input = document.createElement("input");
    var el_name = "change_status[" + next_index.toString() + "]";
    input.setAttribute("type", "text");
    input.setAttribute("name", el_name);
    input.setAttribute("id", el_name);
    input.setAttribute("size", "1");
    input.setAttribute("value", "I");
    input.setAttribute("readonly", "readonly");
    input.setAttribute("class", "change_status");
    td.appendChild(input);

    var delete_btn = document.createElement("button");
    delete_btn.appendChild( document.createTextNode("Delete") );
    delete_btn.setAttribute("type", "button");
    delete_btn.setAttribute("onclick", 
        "grid_delete_row("+next_index.toString()+",'X')" );

    td.appendChild(delete_btn);

    var add_row_form = document.getElementById('add_row_form');
    
    var new_inputs = add_row_form.getElementsByTagName('input');
    var k, some_input, p_rule, attr;
    
    for (k=0; k< new_inputs.length; k++){
        td = document.createElement("td");
        tr.appendChild(td);
        
        some_input = new_inputs.item(k);
        if ((some_input != null) && (some_input.hasAttribute('prompt_rule'))){
           attr = some_input.getAttribute('prompt_rule');

           attr = decodeURIComponent(attr);
           console.log('decoded input ' + attr);
           p_rule = JSON.parse(attr); 
           td.setAttribute("class", p_rule["fieldname"]);
           if (p_rule.prompt_type == "input_hidden"){
               td.className += " " + "input_hidden";
           }         
        }else{
            p_rule = null;
        }
        if (! p_rule){
            p_rule = new Object();
            p_rule.fieldname = some_input.id;
            p_rule.prompt_type = "input_text";
            console.log("default prompt_rule created for "+ some_input.id);
        }
        add_prompt(td, some_input.id, p_rule, next_index);
    }
   
    grid_table.appendChild(tr);   
}

