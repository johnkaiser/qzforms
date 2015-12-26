
INSERT INTO qz.change_history 
  (change_description,note) 
  VALUES 
  ('Update to SV3',
   'Add regex_pattern to prompt_rule, 
    add err_msg class to qzforms.css' 
   );

ALTER TABLE qz.prompt_rule 
    ADD COLUMN regex_pattern text;

UPDATE qz.constants 
SET schema_version = '3';


UPDATE qz.css SET data = 
'.bold {font-weight:bold;}
  .light { color: #999; }
  legend {font-size: 50%;}
  table.qztablez { border: 2pt solid black; border-collapse: collapse; }
  table.qztablez tr td { border: 1pt solid #aaa; padding: 2pt; }
  table.qztablez tr th { border: 1pt solid black; padding: 2pt; font-weight:bold; }
  label { padding-right: 1em; }
/* tables */
table.tablesorter {
    font-family:arial;
    background-color: #CDCDCD;
    margin:10px 0pt 15px;
    font-size: 8pt;
    width: 100%;
    text-align: left;
}
table.tablesorter thead tr th, table.tablesorter tfoot tr th {
    background-color: #e6EEEE;
    border: 1px solid #FFF;
    font-size: 8pt;
    padding: 4px 20px 4px 4px;
}
table.tablesorter thead tr .header {
    background-image: url(/jk/bg.gif);
    background-repeat: no-repeat;
    background-position: center right;
    cursor: pointer;
}
table.tablesorter tbody td {
    color: #3D3D3D;
    padding: 4px;
    background-color: #FFF;
    vertical-align: top;
}
table.tablesorter tbody tr.odd td {
    background-color:#F0F0F6;
}
table.tablesorter thead tr .headerSortUp {
    background-image: url(/jk/asc.gif);
}
table.tablesorter thead tr .headerSortDown {
    background-image: url(/jk/desc.gif);
}
table.tablesorter thead tr .headerSortDown, table.tablesorter thead tr .headerSortUp {
background-color: #8dbdd8;
}
span.description {
    font-size: 75%;
	color: #888;
	padding-left: 1em;
}
td.yesno label { 
    background-color: #ddf;
	color: #a00;
}
td.input_hidden {
    display: none;
}
th.input_hidden {
    display: none;
}
div.menu form {
    display: inline;
}    
input.menu_button {
    background: white;
    font-weight: bold;
}
#qzmenu {
    padding-bottom: 1ex;
    border-bottom: 2pt dotted grey;
}    
.err_msg {
    background: yellow;
    border: 2pt solid red;
    padding: 3pt;
    display: table;
    font-weight: bold;
}
'
WHERE filename = 'qzforms.css';


UPDATE qz.table_action 
SET sql = 
'SELECT 
$1::text "form_name", 
$2::text "fieldname",
''input_text''::qz.prompt_types "prompt_type",
''''::text "el_class",
''''::text "options", 
''f''::boolean "readonly",
''''::text "regex_pattern",
''''::text "rows",
''''::text "cols",
''''::text "size"
'
WHERE form_name =  'prompt_rule_edit'
AND action = 'create';

UPDATE qz.table_action
SET sql = 
'SELECT "form_name", "fieldname", "prompt_type", "tabindex",
"el_class", "readonly","regex_pattern", "rows", "cols", "size", 
"maxlength", "options", "publish_pgtype", "expand_percent_n",
 "onfocus", "onblur", "onchange", "onselect",
"onclick", "ondblclick", "onmousedown", "onmouseup",
"onmouseover", "onmousemove", "onmouseout",
"onkeypress", "onkeydown", "onkeyup"
FROM qz.prompt_rule
WHERE form_name = $1 AND fieldname = $2'
WHERE form_name = 'prompt_rule_edit'
AND action = 'edit';

UPDATE qz.table_action
SET sql =
'INSERT INTO qz.prompt_rule
("form_name", "fieldname", "prompt_type", "options",
"el_class", "readonly", "regex_pattern", "rows", "cols", "size")
VALUES
($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)',
fieldnames = 
'{form_name,fieldname,prompt_type,options,el_class,readonly,regex_pattern,
 rows,cols,size}'
WHERE form_name = 'prompt_rule_edit'
AND action = 'insert';

UPDATE qz.table_action
SET sql = 
'UPDATE qz.prompt_rule
SET
"prompt_type" = $1,
"el_class" = $2,
"readonly" = $3,
"regex_pattern" = $4,
"rows" = $5,
"cols" = $6,
"size" = $7,
"tabindex" = $8,
"options" = $9, 
"maxlength" = $10,
"onfocus" = $11, 
"onblur" = $12, 
"onchange" = $13, 
"onselect" = $14, 
"onclick" = $15, 
"ondblclick" = $16,
"onmousedown" = $17, 
"onmouseup" = $18, 
"onmouseover" = $19, 
"onmousemove" = $20, 
"onmouseout" = $21,
"onkeypress" = $22, 
"onkeydown" = $23, 
"onkeyup" = $24,
"publish_pgtype" = $25,
"expand_percent_n" = $26
WHERE form_name = $27
AND fieldname = $28
',
fieldnames = 
'{prompt_type,el_class,readonly,regex_pattern,rows,cols,size,tabindex,options,maxlength,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup,publish_pgtype,expand_percent_n,form_name,fieldname}'
WHERE form_name = 'prompt_rule_edit'
AND action = 'update';

--
-- regex patterns - filename
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, etag, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) VALUES ('css_edit', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, 63, 2174, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);


INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, etag, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) VALUES ('js_edit', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, 63, 2174, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);



UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'page_css' AND fieldname = 'filename';

UPDATE qz.prompt_rule 
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'page_js' AND fieldname = 'filename';


--
-- regex patterns - variable_name
--

-- menu_edit

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_edit', 'target_div', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

-- menu_host_edit

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_host_edit' 
AND fieldname = 'menu_name';

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_host_edit' 
AND fieldname = 'host_form_name';

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_host_edit' 
AND fieldname = 'host_form_name';


-- menu_item_edit: menu_name target_form_name action

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'target_form_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'action', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

-- menu_set_edit: insert menu_name update host_form_name update action

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_set_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_set_edit' 
AND fieldname = 'host_form_name';

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_set_edit' 
AND fieldname = 'action';

-- prompt_rule: form_name, fieldname

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'prompt_rule_edit' 
AND fieldname = 'form_name';

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('prompt_rule_edit', 'fieldname', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

-- table_action: form_name, action

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'table_action_edit' 
AND fieldname = 'form_name';

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('table_action_edit', 'action', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'table_action_edit'
AND fieldname = 'fieldnames';

--
-- qz.js qzforms.js gets base64 handler of patterns and onactions
--
UPDATE qz.js
SET 
    modtime = '2015-03-23 21:49:31',
    data = 
$QZJS$
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
 *  form_refresh
 *
 *  Search the forms in the curren document, look for a name "form_tag".
 *  Call a post request with the form_tag as post data to /qz/refresh
 */

function form_refresh(){
    console.log("form_refresh");
    var nf;
    var nel;
    var f;
    var postdata;
    var refresh = "/" + window.location.pathname.split('/')[1] + "/refresh";
    for (nf=0; nf < document.forms.length; nf++ ){
        f = document.forms[nf];
        
        for(nel=0; nel < f.elements.length; nel++){
            if ((f.elements[nel].name == "form_tag") && 
                (f.elements[nel].getAttribute('refresh') == 1) ){

                console.log("f.elements["+nel+"].getAttribute('refresh')=" + 
                     f.elements[nel].getAttribute('refresh') );

                form_tag = f.elements[nel].value;
                postdata = "form_tag=" + encodeURIComponent(form_tag);

                httpRequest = new XMLHttpRequest();
    
                httpRequest.onreadystatechange = refresh_result;
                httpRequest.open("POST", refresh);
                httpRequest.send(postdata);
                console.log(postdata);
            }
        }
    }

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
$QZJS$
WHERE filename = 'qzforms.js';


UPDATE qz.js
SET 
    modtime = '2015-12-15 21:16:12',
    data = 
$PRJS$
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
$PRJS$
WHERE filename = 'prompt_rule.js';


