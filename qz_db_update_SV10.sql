INSERT INTO qz.change_history
(change_description,note)
VALUES
('Update to SV10',
'Change the goofy name in onetable "getall" to "list"'
);

 UPDATE qz.constants
 SET schema_version = '10';

INSERT INTO qz.handler_action
(handler_name, action_name)
VALUES
('onetable', 'list');

UPDATE qz.table_action
SET action = 'list'
WHERE action = 'getall';

UPDATE qz.prompt_rule
SET
   options = '{create,delete,delete_row,edit,etag_value,get,insert,insert_row,list,save,update,update_row,view}'
WHERE form_name = 'table_action_edit'
AND fieldname = 'action';

UPDATE qz.menu_item
SET action = 'list'
WHERE action = 'getall';

DELETE FROM qz.handler_action
WHERE action_name = 'getall';

UPDATE qz.js SET data = $QZ$
var httpRequest;
/*
 *  base64_attribs
 *
 *  These are base64 encoded before being encapsulated in a JSON object.
 *  This avoids a lot of ugliness from slashes and quotes, but the
 *  attribute must be decoded prior to being added to an element.
 *
 *  Used by add_array_input and set_common_attributes via  grid_add_row.
 */
var base64_attribs = {'pattern':true, 'onfocus':true, 'onblur':true, 
    'onchange':true, 'onselect':true, 'onclick':true, 
    'ondblclick':true, 'onmousedown':true, 'onmouseup':true, 
    'onmouseover':true, 'onmousemove':true, 'onkeypress':true, 
    'onkeydown':true, 'onkeyup':true};

/*
 *  refresh_result
 *
 *  Passed to httpReqest by form_refresh.
 *  
 *  Catch the results of a refresh request.
 *  If the refresh failed with a logout reply, 
 *  then go to the logout page.
 */
function refresh_result(){
    var login_form = /login_form/;
    var logout = "/" + window.location.pathname.split("/")[1] + "/logout";

    if (login_form.test(httpRequest.responseText)){
        window.location = logout;
    }
    if (httpRequest.readyState > 0){
        console.log("refresh_result readyState="+ httpRequest.readyState + 
            " status=" + httpRequest.status);
    }
}

/*
 *  form_refresh
 *
 *  Search the forms in the curren document, look for a name "form_tag".
 *  Call a post request with the form_tags as post data to /qz/refresh
 */

function form_refresh(){
    console.log("form_refresh");

    var f;
    var form_id;
    var form_tag;
    var nel;
    var nf;
    var postdata;
    var refresh = "/" + window.location.pathname.split('/')[1] + "/refresh";
    var request = new Array();
    var rcnt = 0;

    for (nf=0; nf < document.forms.length; nf++){
        f = document.forms[nf];

        for(nel=0; nel < f.elements.length; nel++){
            if ((f.elements[nel].name == "form_tag") &&
                (f.elements[nel].getAttribute('refresh') == 1) ){

                form_id = 'form_id%5B'+String(rcnt) + '%5D=' +
                    encodeURIComponent(f.id);

                form_tag = 'form_tag%5B'+String(rcnt) + '%5D=' +
                    encodeURIComponent(f.elements[nel].value);

                request[rcnt] = ( form_id+"&"+form_tag );
                console.log("refresh " + request[rcnt]);

                rcnt++;
            }
        }
    }
    httpRequest = new XMLHttpRequest();
    httpRequest.onreadystatechange = refresh_result;
    httpRequest.open("POST", refresh);
    httpRequest.send( request.join('&') );
}
/*
 *  form_refresh_init
 *
 *  Setup the call to the server to refresh forms on the current page.
 */
function form_refresh_init(interval){
    setInterval(form_refresh, interval);
    console.log("form_refresh_init");
}

/*
 *  set_common_attributes
 *
 *  Add attributes that are common to all prompts.
 */
function set_common_attributes(new_input_el, fieldname, prompt_rule, row_index){

    var attribs = ['pattern', 'onfocus', 'onblur', 'onchange', 'onselect',
        'onclick', 'ondblclick', 'onmousedown', 'onmouseup', 'onmouseover',
        'onmousemove', 'onkeypress', 'onkeydown', 'onkeyup',
        'readonly', 'tabindex', 'etag' ];

    if (!new_input_el){
        console.log('set_common_attributes called on a null input element.');
        if (prompt_rule){
            console.log(' on prompt_rule.fieldname');
        }
        return;
    }
            
    var attr, k;

    for (k=0; k < attribs.length; k++){
        if (attribs[k] in prompt_rule){
            attr = prompt_rule[attribs[k]];
            if (attr == "") continue;
            if (attribs[k] in base64_attribs){
                attr = window.atob(attr);
            }
            if (prompt_rule.expand_percent_n){
                attr = attr.replace("%n", row_index.toString());
            }
            new_input_el.setAttribute(attribs[k], attr);
       }
    }
    new_input_el.setAttribute("class", fieldname.split("[")[0]);
}

/*
 *  add_array_input
 *
 *  Add a new input field for an array input.
 *  rule is a json representation of struct prompt_rule in c
 *  which in turn comes from postgres table prompt_rule.
 */
function add_array_input(rule_esc){
    var rule = JSON.parse(decodeURIComponent(rule_esc));
    console.log("add_array_input rule.fieldname");

    var ul_el = document.getElementById(rule.fieldname);
    var brackets = /\[[0-9]+\]/;
    var number = /[0-9]+/;
    var needs_nbr = /%n/
    var next_val = 0;
    var last_li, sb_input, new_li, new_input, new_id, attr_val;
  
    last_li = ul_el.lastElementChild;
    if (last_li){
        sb_input = last_li.firstElementChild;
        next_val = parseInt(number.exec(brackets.exec(sb_input.id)))+1;        
    }
    new_li = document.createElement("li");
    new_input = document.createElement("input");
    ul_el.appendChild(new_li);
    new_li.appendChild(new_input);
    
    new_id = rule.fieldname+"["+next_val+"]";
    new_input.setAttribute("type", "text");
    new_input.setAttribute("id", new_id);
    new_input.setAttribute("name", new_id);

    for(attr in rule){
        if(attr != "fieldname"){
            attr_val = rule[attr];
            if (attr_val == "") continue;
            if (attr in base64_attribs){
                attr_val = window.atob(attr_val);
            }    
            if ( needs_nbr.test(attr_val) ){
                attr_val = attr_val.replace("%n", next_val);
            }
            new_input.setAttribute(attr, attr_val);
        }
    }
}

function add_button(parent_el, fieldname, prompt_rule, row_index){
    console.log("add_button for field " + fieldname);
    
    var btn = document.createElement("button");
    parent_el.appendChild(btn);

    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
        name = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
        name = fieldname;
    }
    btn.setAttribute("name", id);
    btn.setAttribute("id", id);
    // A button type button, not a reset or submit.
    btn.setAttribute("type", "button");

    var btn_text = document.createTextNode(fieldname.split("[")[0]);
    btn.appendChild(btn_text);

    set_common_attributes(btn, fieldname, prompt_rule, row_index);
}

function add_input_hidden(parent_el, fieldname, prompt_rule, row_index){

    var new_hidden = document.createElement("input");
    
    var id;
    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
    }
    new_hidden.setAttribute("name", id);
    new_hidden.setAttribute("id", id);
    new_hidden.setAttribute("type", "hidden");
    
    parent_el.appendChild(new_hidden);
    
    set_common_attributes(new_hidden, fieldname, prompt_rule, row_index);

    return new_hidden;

}

function add_input_radio(parent_el, fieldname, prompt_rule, row_index){
    
    console.log("add_input_radio for field " + fieldname);
    
    var radio_btn; 
    var label_for;
    var label_text;
    var opt_index;
    var id;
    var name;

    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
        name = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
        name = fieldname;
    }
    console.log("expand_percent_n="+prompt_rule.expand_percent_n);
    console.log("name="+name);

    for (opt_index in prompt_rule.options){    
        radio_btn = document.createElement("input");
        radio_btn.setAttribute("type", "radio");
        radio_btn.setAttribute("value", prompt_rule.options[opt_index]);
        radio_btn.setAttribute("name", name);
        id = fieldname + "_" + row_index.toString();
        radio_btn.setAttribute("id", id);

        set_common_attributes(radio_btn, fieldname, prompt_rule, row_index);

        label_for = document.createElement("label");
        label_for.setAttribute("for", id);
        label_text = document.createTextNode(prompt_rule.options[opt_index]);
        label_for.appendChild(label_text);
        label_for.setAttribute("class", fieldname.split("[")[0]);

        parent_el.appendChild(radio_btn);
        parent_el.appendChild(label_for);

    }

}

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

function add_select_options(parent_el, fieldname, prompt_rule, row_index){

    console.log("add_select_options for field " + fieldname);

    var select = document.createElement("select");
    var select_option;
    var option_text;

    parent_el.appendChild(select);

    var id;
    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
    }

    select.setAttribute("name", id);
    select.setAttribute("id", id);
    
    // XXXXXXXXXXXXX
    // Add a class for the pg attribute name
    // but remove the [n] from base[n]

    for (row_index in prompt_rule.options){
        
        console.log("adding option "+ prompt_rule.options[row_index]);
        select_option = document.createElement("option");
        select_option.setAttribute("value", prompt_rule.options[row_index]);
        option_text = document.createTextNode(prompt_rule.options[row_index]);
        select_option.appendChild(option_text);

        select.appendChild(select_option);
    }

    set_common_attributes(select, fieldname, prompt_rule, row_index);
}

function add_textarea(parent_el, fieldname, prompt_rule, row_index){
    console.log("add_textarea for field " + fieldname);

    var textarea = document.createElement("textarea");
    parent_el.appendChild(textarea);
  
    if (("expand_percent_n" in prompt_rule) && (prompt_rule.expand_percent_n)){
        id = fieldname.replace("%n", row_index.toString());
        name = fieldname.replace("%n", row_index.toString());
    }else{
        id = fieldname;
        name = fieldname;
    }
    textarea.setAttribute("name", id);
    textarea.setAttribute("id", id);
    textarea.setAttribute("type", "textarea");

    if ('rows' in prompt_rule){
        textarea.setAttribute('rows', prompt_rule['rows']);
    }

    if ('cols' in prompt_rule){
        textarea.setAttribute('cols', prompt_rule['cols']);
    }
    set_common_attributes(textarea, fieldname, prompt_rule, row_index);

}

/*
 *  add_prompt
 * 
 *
 *  Add the html form elements under parent_el as specified
 *  by the other arguments.
 *
 *  If expand_percent_n is true, then substitute row_index for every %n.
 *  This is the public interface, the add_* functions s/b considered private.
 *  
 *
 */
function add_prompt(parent_el, fieldname, prompt_rule, row_index){

    // XXXXXX Make sure there is a prompt_rule. 
    // Set row_index to 0 if it is not an integer.
    if ( ! prompt_rule.hasOwnProperty('fieldname') ){
        console.log( "prompt_rule has no fieldname");
        return;
    }

    var input_el;

    switch (prompt_rule.prompt_type){

        case "input_text":
            add_input_text(parent_el, fieldname, prompt_rule, row_index);
            break;

        case "input_hidden":
            add_input_hidden(parent_el, fieldname, prompt_rule, row_index);
            break;

        case "select_options":
        case "select_fkey":
           add_select_options(parent_el, fieldname, prompt_rule, row_index);
           break;

        case "input_radio":
           add_input_radio(parent_el, fieldname, prompt_rule, row_index);
           break;

        case "textarea":
            add_textarea(parent_el, fieldname, prompt_rule, row_index);
            break;

        case "button":
            add_button(parent_el, fieldname, prompt_rule, row_index);
            break;
    }

}

/*
 *  change_status
 *
 *  When a field in a grid is updated, inserted, or deleted there is
 *  a field change_status[n] that must be updated.
 *  change_status can have the following values:
 * 
 *      E = Presented for editing but not yet changed.
 *      U = At least one field in the row has been changed.
 *      I = The row has been inserted
 *      D = The row has been deleted
 *      X = Was inserted then deleted, ignore.
 */

function change_status(row_index, new_status){
    
    var allowed =  new Object();
    allowed['>I'] = true;
    allowed['I>X'] = true;
    allowed['E>U'] = true;
    allowed['E>D'] = true;
          
    var chg_st_name = "change_status["+row_index+"]";
    var chg_st_el = document.getElementById(chg_st_name);
     
    if (chg_st_el){
        var transition = chg_st_el.value + '>' + new_status;
        if (allowed[transition]){
        
            chg_st_el.value = new_status;

        }            
    }else{
        console.log('getElementById('+chg_st_name+') returned null');
    }
}

/*
 *  get_next_row_index
 *
 *  Search the grid table for the row numbers and
 *  return one more than the largest present.
 */

function get_next_row_index(){
    var el_list = document.getElementsByClassName('change_status');
    var max_index = -1;
    var is_valid_change_status = /change_status\[[0-9]+\]/;
    var brackets = /\[[0-9]+\]/;
    var number = /[0-9]+/;
    
    var some_el, val_str, val_nbr, k;

    for (k=0; k<el_list.length; k++){
        some_el = el_list.item(k);

        if (is_valid_change_status.test( some_el.id )){
            val_str = number.exec( brackets.exec( some_el.id ) );
            val_nbr = parseInt(val_str);
            
            if (val_nbr > max_index) max_index = val_nbr;   
        }
    }

    return max_index +1;

}

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

function grid_delete_row(row_index, delete_flag){

     change_status(row_index, delete_flag);

     // I want to change the class of the containing tr, but
     // the containing tr needs an id for that to work.
     // XXXXXXXXXX 
     // var tr = document.getElementById(_)
    return false;
}

/* 
 *  set_action_options
 *
 *  In table action edit, limit the options presented
 *  for the action to those appropriate to the handler.
 */

function set_action_options(){

    var allowed_options = { 
        "onetable": ["list", "create", "insert", "edit", "update", "delete"], 
        "grid": ["edit", "save", "insert_row", "update_row", "delete_row"],
        "fs": ["get", "etag_value" ],
        "menupage":["view"]
    };

   var handler_el = document.getElementById('handler_name');

   if (handler_el && (handler_el.value.length > 0)){

       var action_el = document.getElementById("action");
       console.log("set_action_options found "+ action_el.children.length +
           "children");
       while( action_el.children.length > 0 ){
           console.log("Removing " +  action_el.children.item(0).value);
           action_el.children.item(0).remove();
       }
       console.log("handler_name is " + handler_el.value);

       // if handler_el is null, don't go whacking the values above.XXXXXXXXXXX

    
       for( newopt in allowed_options[handler_el.value] ){
            // add children options
            
            var opt_txt = allowed_options[handler_el.value][newopt];
            console.log("adding option "+ opt_txt);
            var opt_el = document.createElement("option");
            opt_el.setAttribute("value", opt_txt);
            opt_el.appendChild( 
                document.createTextNode(opt_txt)
            );
           action_el.appendChild(opt_el);
       }
    }
}

$QZ$ WHERE filename = 'qzforms.js'
