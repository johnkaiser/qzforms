
CREATE TABLE qz.css (
    filename text PRIMARY KEY,
    mimetype text,
    modtime timestamp,
    etag bigint not null default nextval('qz.etag_seq'::regclass),
    data text );

CREATE TABLE qz.js (
    filename text PRIMARY KEY,
        mimetype text,
        modtime timestamp,
        etag bigint not null default nextval('qz.etag_seq'::regclass),
        data text );

CREATE TABLE qz.doc (
    filename text PRIMARY KEY,
        mimetype text,
        modtime timestamp,
        etag bigint not null default nextval('qz.etag_seq'::regclass),
        data text );

CREATE TABLE qz.page_css (
    form_name qz.variable_name NOT NULL,
    sequence integer NOT NULL,
    filename text REFERENCES qz.css(filename),
    PRIMARY KEY (form_name, sequence)
);


CREATE TABLE qz.page_js (
    form_name qz.variable_name NOT NULL,
    sequence integer NOT NULL,
    filename text REFERENCES qz.js(filename),
    PRIMARY KEY (form_name, sequence)
);


--
-- css
--

INSERT INTO qz.css (filename, mimetype, modtime, etag, data) VALUES ('login_process.css', 'text/css', '2014-08-10 12:34:54.600325', 109, 'table {
    border-collapse: collapse;
    border: 2pt solid black;
}

th {
    border: 1pt solid grey;
    font-weight: bold;
    padding: 2pt;
}
td {
    border: 1pt dotted grey;
    font-size: 85%;
    text-align: center;
    padding: 2pt;
}
i {
    color: blue;
}

dd {
   margin-bottom: 1.0ex;
}');
INSERT INTO qz.css (filename, mimetype, modtime, etag, data) VALUES ('qzforms.css', 'text/css', '2015-05-26 20:43:16.217704', 2152, '.bold {font-weight:bold;}
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
');
INSERT INTO qz.css (filename, mimetype, modtime, etag, data) VALUES ('blue.css', 'text/css', '2015-05-30 17:23:51.594153', 2247, 'body {
    background: lightblue;
    color: darkblue;
}');

INSERT INTO qz.css (filename, mimetype, modtime, etag, data) VALUES ('form_edit.css', 'text/css', 
 '2015-06-22 08:47:51.535663', 467, '#pagemenu {
    float:left;
    width: 11em;
}
#pagemenu form{
    display: block;
}

#qz {
   margin-left: 12em;
}');


--
-- js
--
INSERT INTO qz.js (filename, mimetype, modtime, etag, data) VALUES ('encodeFormData.js', 'text/javascript', '2014-07-27 14:58:06.889344', 104, '/**
 * Encode the properties of a form if they were name/value pairs from
 * an HTML form, using application/x-www-form-urlencoded format
 *
 * Lifted from:
 * https://www.inkling.com/read/javascript-definitive-guide-david-\
 * flanagan-6th/chapter-18/encoding-an-object-for-an-http
 */
function encodeFormData(data) {
    if (!data) return "";    // Always return a string
    var pairs = [];          // To hold name=value pairs
    for(var name in data) {                                    // For each name
        if (!data.hasOwnProperty(name)) continue;              // Skip inherited
        if (typeof data[name] === "function") continue;        // Skip methods
        var value = data[name].toString();                     // Value as string
        name = encodeURIComponent(name).replace("%20","+");    // Encode name
        value = encodeURIComponent(value).replace("%20", "+"); // Encode value
        pairs.push(name + "=" + value);   // Remember name=value pair
    }
    return pairs.join(''&''); // Return joined pairs separated with &
}');


INSERT INTO qz.js (filename, mimetype, modtime, etag, data) VALUES ('qzforms.js', 'text/javascript; charset=utf-8', '2015-03-23 21:49:31.300949', 1685, 'var httpRequest;

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
    var refresh = "/" + window.location.pathname.split(''/'')[1] + "/refresh";
    for (nf=0; nf < document.forms.length; nf++ ){
        f = document.forms[nf];
        
        for(nel=0; nel < f.elements.length; nel++){
            if ((f.elements[nel].name == "form_tag") && 
                (f.elements[nel].getAttribute(''refresh'') == 1) ){

                console.log("f.elements["+nel+"].getAttribute(''refresh'')=" + 
                     f.elements[nel].getAttribute(''refresh'') );

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
            if( needs_nbr.test(attr_val) ){
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
    allowed[''>I''] = true;
    allowed[''I>X''] = true;
    allowed[''E>U''] = true;
    allowed[''E>D''] = true;
          
    var chg_st_name = "change_status["+row_index+"]";
    var chg_st_el = document.getElementById(chg_st_name);
     
    if (chg_st_el){
        var transition = chg_st_el.value + ''>'' + new_status;
        if (allowed[transition]){
        
            chg_st_el.value = new_status;

        }            
    }else{
        console.log(''getElementById(''+chg_st_name+'') returned null'');
    }
}');


INSERT INTO qz.js (filename, mimetype, modtime, etag, data) VALUES ('grid.js', 'text/javascript', '2015-03-14 16:41:05.85443', 1626, '/*
 *  get_next_row_index
 *
 *  Search the grid table for the row numbers and
 *  return one more than the largest present.
 */

function get_next_row_index(){
    var el_list = document.getElementsByClassName(''change_status'');
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

    var grid_table = document.getElementById(''grid_edit_tbody'');
    if (!grid_table){
        console.log(''id grid_edit_tbody not found'');
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
        "grid_delete_row("+next_index.toString()+",''X'')" );

    td.appendChild(delete_btn);

    var add_row_form = document.getElementById(''add_row_form'');
    
    var new_inputs = add_row_form.getElementsByTagName(''input'');
    var k, some_input, p_rule, attr;
    
    for (k=0; k< new_inputs.length; k++){
        td = document.createElement("td");
        tr.appendChild(td);
        
        some_input = new_inputs.item(k);
        if ((some_input != null) && (some_input.hasAttribute(''prompt_rule''))){
           attr = some_input.getAttribute(''prompt_rule'');
           console.log( ''input ''+k.toString()+'' prompt_rule ''+attr);
           attr = decodeURIComponent(attr);
           console.log(''decoded '' + attr);
           p_rule = JSON.parse(attr); 
           td.setAttribute("class", p_rule["fieldname"]);
           if (p_rule.prompt_type == "input_hidden"){
               td.className += " " + "input_hidden";
           }         
           console.log("prompt_rule from JSON is "+ 
               some_input.getAttribute(''prompt_rule''));
        }else{
            p_rule = null;
        }
        if (! p_rule){
            p_rule = new Object();
            p_rule.fieldname = some_input.id;
            p_rule.prompt_type = "input_text";
            console.log("default prompt_rule created for "+ some_input.id);
        }
        console.log("some_input "+ next_index.toString()+" "+ some_input.id);
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
}');


INSERT INTO qz.js (filename, mimetype, modtime, etag, data) VALUES ('prompt_rule.js', 'text/javascript', '2015-03-14 16:34:53.211068', 1625, '/*
 *  add_input_text
 *
 *  The simplest case, <input type=text...
 *  except you don''t have to type it.
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
    
    if (''size'' in prompt_rule){
        new_input.setAttribute(''size'', prompt_rule[''size'']);
    }

    if (''maxlength'' in prompt_rule){
        new_input.setAttribute(''maxlength'', prompt_rule[''maxlength'']);
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

    if (''rows'' in prompt_rule){
        textarea.setAttribute(''rows'', prompt_rule[''rows'']);
    }

    if (''cols'' in prompt_rule){
        textarea.setAttribute(''cols'', prompt_rule[''cols'']);
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

    var attribs = [''onfocus'', ''onblur'', ''onchange'', ''onselect'',
        ''onclick'', ''ondblclick'', ''onmousedown'', ''onmouseup'', ''onmouseover'',
        ''onmousemove'', ''onkeypress'', ''onkeydown'', ''onkeyup'',
        ''readonly'', ''tabindex'', ''etag'' ];

    if (!new_input_el){
        console.log(''set_common_attributes called on a null input element.'');
        if (prompt_rule){
            console.log('' on prompt_rule.fieldname'');
        }
        return;
    }
            
    var attr, k;

    for (k=0; k < attribs.length; k++){
        if (attribs[k] in prompt_rule){
            attr = prompt_rule[attribs[k]];

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
    if ( ! prompt_rule.hasOwnProperty(''fieldname'') ){
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

}');


INSERT INTO qz.js (filename, mimetype, modtime, etag, data) VALUES ('document_ready.js', 'text/javascript', '2015-06-01 20:48:14.15958', 2248, '  $(document).ready(function() 
    { 
        $("#pg_stat_activity").tablesorter(); 
        $("#table_action").tablesorter();
        $("#counters").tablesorter();
        $("#open_tables").tablesorter();
        $("#getall").tablesorter();
        $("#listmgr").tablesorter();
        $("#form_tags").tablesorter();
        $(".qztable").hide();
        console.log(''document ready executed'');
    } 

  );

  function menu_click(on_item){
      var this_one = "#"+on_item;
      $(".qztable").hide();
	  $(this_one).show();
  }
  function show_pgtype(id){
      var el, attrib;
	  el = document.getElementById(id);
	  attrib = el.getAttribute("pgtype");
	  alert("pgtype_datum="+decodeURIComponent(attrib));
  }	');






