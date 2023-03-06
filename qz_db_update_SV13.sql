BEGIN;
INSERT INTO qz.change_history
(change_description,note)
VALUES
('Update to SV13',
'HTML docs and callbacks'
);

 UPDATE qz.constants
 SET schema_version = '13';

---
--- Convert template to drop down list
---

--- But first, add any missing templates
INSERT INTO qz.template
SELECT xml_template, 'upgrade' FROM qz.form f
WHERE
xml_template IS NOT NULL
AND NOT EXISTS
(SELECT template_name FROM qz.template t
 WHERE f.xml_template = t.template_name);

ALTER TABLE qz.form
ADD FOREIGN KEY (xml_template)
REFERENCES qz.template(template_name);

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type)
VALUES
('form', 'xml_template', 'select_fkey');

UPDATE qz.form
SET hidden = 't'
WHERE (form_name) IN ('div_ids','templates');

---
--- Inline Documents
---
ALTER TABLE qz.doc
ADD COLUMN form_name qz.variable_name REFERENCES qz.form(form_name);

ALTER TABLE qz.doc ADD COLUMN action qz.variable_name;
ALTER TABLE qz.doc ADD COLUMN div_id qz.variable_name;
ALTER TABLE qz.doc ADD COLUMN el_class text;

ALTER TABLE qz.doc DROP CONSTRAINT doc_pkey;
ALTER TABLE qz.doc DROP COLUMN mimetype;
ALTER TABLE qz.doc DROP COLUMN filename;

ALTER TABLE qz.doc ADD PRIMARY KEY (form_name, action, div_id);

GRANT SELECT ON TABLE qz.doc TO qzuser;
GRANT ALL ON TABLE qz.doc TO qzdev;

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
target_div, hidden, prompt_container, form_set_name, pkey)
VALUES
('inline_doc', 'onetable', 'qz', 'doc', 'tinymce.xml',
'qz', 't', 'fieldset', 'form_mgt', '{form_name,action,div_id}');

--- Table Actions for inline docs

INSERT INTO qz.table_action
(form_name, action, fieldnames, helpful_text, sql)
VALUES

('inline_doc', 'list', '{form_name}',
'Use this to attach a bit of html to your form',
$DTALI$
SELECT action, div_id
FROM qz.doc
WHERE form_name = $1
ORDER BY action, div_id
$DTALI$),

('inline_doc', 'edit', '{form_name,action,div_id}', NULL,
$DTAED$
SELECT action, div_id, el_class, "data"
FROM qz.doc
WHERE form_name = $1
AND action = $2
AND div_id = $3
$DTAED$),

('inline_doc', 'update', '{form_name,action,div_id,el_class,data}', NULL,
$DTAUP$
UPDATE qz.doc
SET
el_class = $4,
"data" = $5
WHERE form_name = $1
AND action = $2
AND div_id = $3
$DTAUP$),

('inline_doc', 'create', '{form_name}', NULL,
$DTACR$
SELECT $1::qz.variable_name "form_name",
''::text "action",
''::text "div_id",
''::text "el_class",
''::text "data"
$DTACR$),

('inline_doc', 'insert',
'{form_name, action, div_id, el_class, data}',
NULL,
$DTAIN$
INSERT INTO qz.doc
(form_name, action, div_id, el_class, data)
VALUES
($1,$2,$3,$4,$5)
$DTAIN$),

('inline_doc', 'delete', '{form_name,action,div_id}', NULL,
$DTADL$
DELETE FROM qz.doc
WHERE
form_name = $1
AND action = $2
AND div_id = $3
$DTADL$);

UPDATE qz.table_action
SET inline_js =
$TAIJSD$
window.addEventListener("DOMContentLoaded",
    () => callback_options('get_divids', 'div_id'));

window.addEventListener("DOMContentLoaded",
    () => callback_options('get_actions', 'action'));
$TAIJSD$
WHERE form_name = 'inline_doc'
AND (action) IN ('create', 'edit');


--- Menu stuff for inline_docs

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'inline_doc', 'any'),
('form_dev', 'inline_doc', 'any'),
('form_submenu', 'inline_doc', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('form_submenu', '90', 'inline_doc', 'list', 'inline doc');

---
--- page js/css for inline docs
---

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('inline_doc', '1', 'qzforms.css'),
('inline_doc', '2', 'form_edit.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('inline_doc', '1', 'qzforms.js');

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, el_class, rows, cols)
VALUES
('inline_doc', 'data', 'textarea', 'tinymce', '40', '40');

---
--- Callbacks
---

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
target_div, hidden, prompt_container, form_set_name, pkey)
VALUES
('callback', 'onetable', 'qz', 'table_action', 'base.xml',
'qz', 't', 'fieldset', 'form_mgt', '{form_name,callback_name}');

INSERT INTO qz.table_action
(form_name, action, fieldnames, helpful_text, sql)
VALUES

('callback', 'list', '{form_name}',
'Use this to enable javascript to send inquiries to Postgresql',
$CBL$
SELECT form_name, action "callback_name"
FROM qz.table_action
WHERE form_name = $1
AND is_callback
ORDER BY action
$CBL$),

('callback', 'edit', '{form_name, callback_name}', NULL,
$CBE$
SELECT form_name, action "callback_name", sql, fieldnames,
callback_attached_action, callback_response
FROM qz.table_action
WHERE form_name = $1
AND action = $2
AND is_callback
$CBE$),

('callback', 'update', '{form_name, callback_name, sql, fieldnames,
callback_attached_action, callback_response}', NULL,
$CBU$
UPDATE qz.table_action
SET
sql = $3,
fieldnames = $4,
callback_attached_action = $5,
callback_response = $6
WHERE form_name = $1
AND action = $2
AND is_callback
$CBU$),

('callback', 'create', '{form_name}', NULL,
$CBC$
SELECT $1::qz.variable_name "form_name",
''::text "callback_name",
''::text "sql",
''::text "fieldnames",
'any'::text "callback_attached_action",
''::text "callback_response"
$CBC$),

('callback', 'insert',
'{form_name, callback_name, sql, fieldnames, callback_attached_action, callback_response}',
NULL,
$CBI$
INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, is_callback,
callback_attached_action, callback_response)
VALUES
($1,$2,$3,$4,'t',$5,$6)
$CBI$),

('callback', 'delete', '{form_name, callback_name}', NULL,
$CBD$
DELETE FROM qz.table_action
WHERE
form_name = $1
AND action = $2
AND is_callback
$CBD$);

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'callback', 'any'),
('form_dev', 'callback', 'any'),
('form_submenu', 'callback', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('form_submenu', '100', 'callback', 'list', 'Callbacks');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('callback', '1', 'qzforms.css'),
('callback', '2', 'form_edit.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('callback', '1', 'qzforms.js');

INSERT INTO qz.prompt_rule
(form_name, fieldname, rows, cols, prompt_type)
VALUES
('callback', 'sql', 12, 60, 'textarea');

INSERT INTO qz.prompt_rule
(form_name, fieldname, size, prompt_type, regex_pattern)
VALUES
('callback', 'fieldnames', 63, 'text_array',
'^[^\x01-\x2f\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x60]{1,63}$');


-----
----- Callbacks - Try 2
-----

ALTER TABLE qz.table_action
ADD COLUMN is_callback boolean DEFAULT 'f';

ALTER TABLE qz.table_action
ADD COLUMN callback_attached_action qz.variable_name;
COMMENT ON COLUMN qz.table_action.callback_attached_action IS 'Not yet implemented';

INSERT INTO qz.handler
(handler_name)
VALUES
('callback');

CREATE TYPE qz.callback_response_type AS ENUM
  ('qzforms_json', 'postgresql_json', 'plain_text', 'html_table');

ALTER TABLE qz.table_action
ADD COLUMN callback_response qz.callback_response_type;

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type)
VALUES
('callback', 'callback_response', 'select_options');

---
--- No callbacks should be listed in table_action
---
UPDATE qz.table_action
SET sql = $TAL$ SELECT ta.action, ta.helpful_text,
     fm.handler_name
     FROM qz.table_action ta
     JOIN qz.form fm USING (form_name)
     WHERE form_name = $1
     AND NOT is_callback
     ORDER BY form_name, action $TAL$
WHERE form_name = 'table_action_edit'
AND action = 'list';

---
---  callbacks for inline_doc
---
INSERT INTO qz.table_action
(form_name, action, fieldnames, callback_attached_action, is_callback,
callback_response, sql)
VALUES
('inline_doc', 'get_divids', '{form_name}', 'any', 't', 'qzforms_json',
$TACBDIVID$
SELECT d.id "value", d.id || ' - ' || d.notation "text"
FROM qz.div_id d
WHERE d.template_name =
   (SELECT f.xml_template
    FROM qz.form f
    WHERE f.form_name = $1
    )
ORDER By d.id;
$TACBDIVID$),

('inline_doc', 'get_actions', '{form_name}', 'any', 't', 'qzforms_json',
$TACBACT$
SELECT action "value"
FROM qz.table_action
WHERE form_name = $1
AND NOT is_callback
$TACBACT$);

---
---  Update inline_doc prompt rule to select options
---
INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type)
VALUES
('inline_doc', 'action', 'select_options'),
('inline_doc', 'div_id', 'select_options');

---
---  div_id should not be both a field name and a table name
---
ALTER TABLE qz.div_id RENAME COLUMN "div_id" TO "id";

UPDATE qz.table_action
SET sql=$IDTAE$
SELECT id, notation
FROM qz.div_id
WHERE template_name = $1
ORDER BY template_name, id
$IDTAE$
WHERE form_name = 'div_ids'
AND action = 'edit';

UPDATE qz.table_action
SET sql=$IDTAU$
UPDATE qz.div_id
SET notation = $3
WHERE template_name = $1
AND id = $2
$IDTAU$,
fieldnames = '{template_name,id,notation}'
WHERE form_name = 'div_ids'
AND action = 'update_row';

UPDATE qz.table_action
SET sql=$IDTAI$
INSERT INTO qz.div_id
(template_name, id, notation)
VALUES
($1, $2, $3)
$IDTAI$,
fieldnames = '{template_name,id,notation}'
WHERE form_name = 'div_ids'
AND action = 'insert_row';

UPDATE qz.table_action
SET sql=$IDTAD$
DELETE FROM qz.div_id
WHERE template_name = $1
AND id = $2
$IDTAD$,
fieldnames = '{template_name,id}'
WHERE form_name = 'div_ids'
AND action = 'delete_row';

---
--- Make menu build suck less
---

COMMENT ON COLUMN qz.menu.menu_name IS 'The menu name is used to add the menu to a page.';
COMMENT ON COLUMN qz.menu.target_div IS 'The div ID where the menu will be placed.';
COMMENT ON COLUMN qz.menu.description IS 'A handy notation for form developers, does not appear to the user.';
COMMENT ON COLUMN qz.menu.form_set_name IS 'An optional reference to a form set. A form set allows forms to pass data to sub-forms.';

---
--- Lists should be sorted
---

UPDATE qz.table_action
SET sql=$JSLTA$ SELECT form_name, action, inline_js::varchar(30) inline_js_
   FROM qz.table_action
   WHERE form_name = $1
   AND NOT is_callback
   ORDER BY action $JSLTA$
WHERE form_name = 'inline_js'
AND action = 'list';

UPDATE qz.table_action
SET sql=$CSSLTA$ SELECT form_name, action, inline_css::varchar(30) inline_css_
   FROM qz.table_action
   WHERE form_name = $1
   AND NOT is_callback
   ORDER BY action $CSSLTA$
WHERE form_name = 'inline_css'
AND action = 'list';

---
--- set_action_options needed for additional forms
---
UPDATE qz.table_action
SET inline_js =  'window.addEventListener("DOMContentLoaded",set_action_options, true);'
WHERE form_name = 'table_action_edit'
AND (action) IN ('list','insert','update');

---
---  qzforms.js
---

UPDATE qz.js SET data = 
$QZJS$ 
"use strict";
// httpRequest is used for form_refresh and refresh_result
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

    var id;
    var name;

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
        "onetable": ["view", "list", "create", "insert", "edit", "update", "delete"],
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

       var newopt;
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

if (window.hasOwnProperty('tinymce')){
    tinymce.init(
     {selector: 'textarea.tinymce',
      entity_encoding : "numeric",
      plugins: [ 'lists table code link image' ],
      toolbar: ['undo redo |styleselect bold italic | bullist numlist blockquote  outdent indent  | link table code | image'],
      menubar: false,
      image_uploadtab: false
     });
}
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

/* 
 *  Look through script id __CALLBACKS__ in the variable callbacks
 *  for callback_name. 
 *  Pass func to XMLHttpRequest.
 *  Optionaly include an object that will be the preferred
 *  source for the callback parameters.
 *  Named callback parameters not in args will be searched for
 *  in the form's elements.
 *
 *  Initialize xhr in the same context as func as:
 *      var xhr = new XMLHttpRequest();
 *
 */
function setup_callback(callback_name, func, xhr, args={}){

   const callbacks = get_callbacks();

   // Build a www-form-urlencoded string for post data.
   // The form_tag is a token authorizing the action.

   let form_tag = "form_tag=" + 
       encodeURIComponent(callbacks[callback_name]['form_tag']);

   let form_action = callbacks[callback_name]['form_action'];
   let form_fields = form_tag;
 
   //  The callback object will have a list of fields for the
   //  query input data. 

   let fieldnames = callbacks[callback_name]['fieldnames'];
   let fn_len = fieldnames.length;
   var form_name = callbacks[callback_name]['form_name'];
   let postdata = {};

   for (let n=0; n < fn_len; n++){
       let fn = fieldnames[n];
       console.log("n="+n + " fn="+fn);

       if (fn in args){
           // args take precendence over form data
           postdata[fn] = args[fn];
       }else{
           // find the fieldname in the form
           let el =  document.forms[form_name][fn];
           if (el){
               args[fn] = el.value;
               console.log(fn + " = " + args[fn] + " tagname = " + el.tagName );
           }else{
               console.log("element " + fn + " not found");
           }
       }
   }
   // Turn the args object into form urlencoded data for posting

   let arg_keys = Object.getOwnPropertyNames(args);
   for (let k=0; k < arg_keys.length; k++){
       form_fields += "&" + encodeURIComponent(arg_keys[k]) + "=" + 
           encodeURIComponent(args[arg_keys[k]]);
   }

    // create and post the request
    xhr.onload = func;
    xhr.callback_name = callback_name;
    xhr.form_name = form_name;
    xhr.args = args;
    xhr.open("POST", form_action);
    xhr.setRequestHeader('Content-Type',"application/x-www-form-urlencoded");
    xhr.send(form_fields);

    console.log('sent '+form_fields);
}


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
            ' field ' + field_name);
    
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
                    if (this_el.options[k].selected &&
                        this_el.options[k].value.length > 0){

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
                        // remove the new value from selected to detect
                        // selected not in current options
                        selected.splice(new_value,1);
                    }
                }
                // It can happen that the selected value is not
                // in the list of options. Add the selected value.
                if (selected.length > 0){
                    console.log('callback_options: current value not used',
                        selected, selected.length);
                    for (let op = 0; op < selected.length; op++){
                        let new_pos =  this_el.options.length;
                        this_el[new_pos] = new Option(selected[op], selected[op]);
                        this_el[new_pos].selected = true;
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
$QZJS$ 
 WHERE filename = 'qzforms.js';
COMMIT;
