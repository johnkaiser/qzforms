INSERT INTO qz.change_history
   (change_description,note)
   VALUES
   ('Update to SV7',
    'Finishing Form Sets'
    );

 UPDATE qz.constants
 SET schema_version = '7';

-- Add a foreign key so select_fkey works.
ALTER TABLE qz.form ADD FOREIGN KEY (form_set_name) REFERENCES qz.form_set(set_name);

---
--- table action clear context parameter flag
---

ALTER TABLE qz.table_action DROP COLUMN set_context_parameters;
ALTER TABLE qz.table_action ADD COLUMN  clear_context_parameters boolean 
NOT NULL DEFAULT false;

UPDATE qz.table_action
SET clear_context_parameters = 't'
WHERE form_name = 'form' 
AND action = 'getall';

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, options)
VALUES
('table_action_edit', 'clear_context_parameters', 'input_radio', '{yes,no}');

-- table_action create
UPDATE qz.table_action
SET 
  sql=$TAC$SELECT ta.form_name, ta.action new_action, fm.handler_name,
    ''::text helpful_text, ta.sql, ta.fieldnames, ta.pkey, 
    'f'::boolean clear_context_parameters 
     FROM qz.create_table_action($1,$2) ta
    JOIN qz.form fm USING (form_name)$TAC$,
  pkey = '{form_name, action}'
WHERE form_name = 'table_action_edit'
AND action = 'create';

-- table_action insert
UPDATE qz.table_action
SET 
  sql=$TAI$INSERT INTO qz.table_action
      (form_name, action, helpful_text, sql, fieldnames, pkey, clear_context_parameters)
      VALUES
      ($1,$2,$3,$4,$5,$6,$7)$TAI$,
  fieldnames='{form_name,new_action,helpful_text,sql,fieldnames,pkey,clear_context_parameters}'    
WHERE form_name = 'table_action_edit'
AND action = 'insert';


-- table_action edit 
UPDATE qz.table_action
set sql=$TAE$SELECT
      form_name, action, helpful_text, sql, fieldnames, pkey, clear_context_parameters
    FROM
      qz.table_action
    WHERE
    form_name = $1 AND action = $2 $TAE$
WHERE form_name = 'table_action_edit'
AND action = 'edit';

-- table_action update
UPDATE qz.table_action
set sql=$TAU$UPDATE qz.table_action
    SET helpful_text=$3,
    sql=$4, fieldnames=$5, pkey=$6, clear_context_parameters=$7
    WHERE form_name = $1 AND action = $2 $TAU$,
fieldnames = '{form_name,action,helpful_text,sql,fieldnames,pkey,clear_context_parameters}'
WHERE form_name = 'table_action_edit'
AND action = 'update';

-- table_action getall
UPDATE qz.table_action
SET pkey = '{form_name, handler_name, action}'
WHERE form_name = 'table_action_edit'
AND action = 'getall';

-- table_action delete
UPDATE qz.table_action
SET pkey = '{form_name,action}'
WHERE form_name = 'table_action_edit'
AND action = 'delete';

-- form create
UPDATE qz.table_action
SET sql=$FC$SELECT  
    $1::text form_name, 
    $2::text handler_name, 
    ''::text schema_name, ''::text table_name, 
    'base.xml'::text xml_template, 
    'qz'::text target_div,
    ''::text add_description, 
    ''::text prompt_container$FC$,
    pkey='{form_name,new_handler_name}',
    fieldnames='{form_name,new_handler_name}'
WHERE form_name = 'form'
AND action = 'create';

-- form insert
UPDATE qz.table_action
SET sql=$FI$INSERT INTO qz.form
     (form_name, handler_name, schema_name, table_name,
     xml_template, target_div, 
     add_description, prompt_container)
     VALUES ($1, $2, $3, $4, $5, $6, $7, $8)$FI$,
     fieldnames='{form_name,handler_name,schema_name,table_name,xml_template,target_div,add_description,prompt_container}' 
WHERE form_name = 'form'
AND action = 'insert';

-- form update
UPDATE qz.table_action
SET sql=$FTAU$ UPDATE qz.form SET 
     schema_name = $2,
     table_name = $3,
     xml_template = $4,
     target_div = $5,
     add_description = $6,
     prompt_container = $7,
     form_set_name = $8
   WHERE form_name = $1 $FTAU$, 
   fieldnames = '{form_name,schema_name,table_name,xml_template,target_div,add_description,prompt_container, form_set_name}'
WHERE form_name = 'form'
AND action = 'update';

-- prompt_rule form new_handler_name
INSERT INTO qz.prompt_rule
(form_name, fieldname, readonly, regex_pattern, options, prompt_type)
VALUES 
('form', 'new_handler_name', 'f', 
$PRNH$^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$$PRNH$,
'{fs,grid,menu,menupage,onetable}',
'select_options');

-- menu form set 
UPDATE qz.form
SET form_set_name = 'menu_mgt'
WHERE form_name IN ( 'menu_edit', 'menu_item_edit', 'menu_set_edit' );



UPDATE qz.prompt_rule
SET 
prompt_type = 'input_text',
readonly = 't'
WHERE form_name = 'form'
AND fieldname = 'handler_name';

UPDATE qz.form
SET form_set_name = 'form_mgt'
WHERE form_name = 'prompt_rule_edit';

--- 
--- Add menus to a form from the form form
---
INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
   target_div, add_description, prompt_container, form_set_name)
VALUES
('page_menus', 'grid', 'qz', 'menu_set', 'base.xml',
   'qz', 't', 'no_container', 'form_mgt');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'page_menus', 'any'),
('form_submenu', 'page_menus', 'any'),
('form_dev', 'page_menus', 'any');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('page_menus', '1', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('page_menus', '1', 'qzforms.css'),
('page_menus', '10', 'form_edit.css');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name,
  action, menu_text, context_parameters)
VALUES
('form_submenu', '60', 'page_menus',
  'edit', 'page_menus', '{form_name, handler_name_ro}');


INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, pkey, helpful_text)
VALUES
('page_menus', 'edit', 
$PMED$SELECT set_id, host_form_name, menu_name, action 
FROM qz.menu_set
WHERE host_form_name = $1
ORDER BY menu_name, action$PMED$,
'{form_name}', '{set_id}', 'Change the menus this form contains');

INSERT INTO qz.table_action
(form_name, action, sql)
VALUES
('page_menus', 'save', 'SELECT 1');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames)
VALUES
('page_menus', 'insert_row', 
    $PMIR$INSERT INTO qz.menu_set
    (menu_name, host_form_name, action)
    VALUES
    ($1,$2,$3)$PMIR$, 
 '{menu_name, form_name, action}');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, pkey)
VALUES
('page_menus', 'update_row', 
    $PMUR$UPDATE qz.menu_set
    SET "menu_name" = $2,
        "host_form_name" = $3,
        "action" = $4
    WHERE set_id = $1$PMUR$,
'{set_id, menu_name, form_name, action}', '{set_id}');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, pkey)
VALUES
('page_menus', 'delete_row',
   $PMDR$DELETE FROM qz.menu_set
   WHERE set_id = $1$PMDR$,
 '{set_id}', '{set_id}');


INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, expand_percent_n, onchange)
VALUES
 ('page_menus', 'set_id', 'input_text', 't', $C1$change_status(%n, 'U')$C1$),
 ('page_menus', 'host_form_name', 'input_text', 't', $C2$change_status(%n, 'U')$C2$),
 ('page_menus', 'menu_name', 'input_text', 't', $C3$change_status(%n, 'U')$C3$),
 ('page_menus', 'action', 'input_text', 't', $C4$change_status(%n, 'U')$C4$);

 
---
--- Change menu_item context_parameters
--- to menu form_set_name
---

ALTER TABLE qz.menu ADD COLUMN form_set_name qz.variable_name;
ALTER TABLE qz.menu ADD FOREIGN KEY (form_set_name) REFERENCES qz.form_set(set_name);

UPDATE qz.menu
SET form_set_name = 'form_mgt'
WHERE menu_name = 'form_submenu';

UPDATE qz.menu
SET form_set_name = 'menu_mgt'
WHERE menu_name = 'menu_submenu';

UPDATE qz.table_action
SET sql = $MCR$SELECT ''::text menu_name,
    ''::text target_div,
    ''::text description,
    ''::text form_set_name$MCR$
WHERE form_name = 'menu_edit'
AND action = 'create';

UPDATE qz.table_action
SET sql = $MINS$INSERT INTO qz.menu
    (menu_name, target_div, description, form_set_name)
    VALUES
    ($1,$2,$3,$4)$MINS$,
    fieldnames = '{menu_name,target_div,description,form_set_name}'
WHERE form_name = 'menu_edit'
AND action = 'insert';

UPDATE qz.table_action
SET sql = $MED$SELECT menu_name, target_div, description, form_set_name
    FROM qz.menu
    WHERE menu_name = $1 $MED$
WHERE form_name = 'menu_edit'
AND action = 'edit';

UPDATE qz.table_action
SET sql = $MUPD$UPDATE qz.menu
    SET
    target_div = $2,
    description = $3,
    form_set_name = $4
    WHERE menu_name = $1 $MUPD$,
    fieldnames = '{menu_name,target_div,description,form_set_name}'
WHERE form_name = 'menu_edit'
AND action = 'update';

UPDATE qz.table_action
SET clear_context_parameters = 't'
WHERE form_name = 'menu_edit'
AND action = 'getall';

DELETE FROM qz.menu_item
WHERE menu_name = 'menu_submenu'
AND menu_item_sequence = '2'
AND menu_text = 'Menu_Set';

UPDATE qz.form_set
SET context_parameters = '{menu_name, menu_item_sequence}'
WHERE set_name = 'menu_mgt';

---
--- rename
---

ALTER TABLE qz.menu_item_parameter
RENAME TO fixed_parameter;

UPDATE qz.table_action
SET sql = $FPTADR$DELETE FROM qz.fixed_parameter
    WHERE menu_name = $1 
    AND menu_item_sequence = $2
    AND parameter_key = $3 $FPTADR$
WHERE form_name = 'fixed_parameters'
AND action = 'delete_row';

UPDATE qz.table_action
SET sql = $FPTAUR$UPDATE qz.fixed_parameter
    SET parameter_value = $4
    WHERE menu_name = $1
    AND menu_item_sequence = $2
    AND parameter_key = $3$FPTAUR$
WHERE form_name = 'fixed_parameters'
AND action = 'update_row';

UPDATE qz.table_action
SET sql = $FPTAIR$INSERT INTO qz.fixed_parameter
    (menu_name, menu_item_sequence, parameter_key, parameter_value)
    VALUES
    ($1,$2,$3,$4)$FPTAIR$
WHERE form_name = 'fixed_parameters'
AND action = 'insert_row';

UPDATE qz.table_action
SET sql = $FPTAE$SELECT
    parameter_key,
    parameter_value, menu_item_sequence
    FROM
    qz.fixed_parameter
    WHERE
    menu_name = $1
    AND 
    menu_item_sequence = $2 $FPTAE$
WHERE form_name = 'fixed_parameters'
AND action = 'edit';

---
--- Clean up
---
DELETE FROM qz.menu_set
WHERE menu_name = 'table_action_edit';




UPDATE qz.table_action
SET sql = $FMTAUP$SELECT form_name, handler_name
    FROM qz.form
    ORDER BY form_name$FMTAUP$
WHERE form_name = 'form'
AND action = 'getall';

UPDATE qz.table_action
SET sql = $FMTAED$SELECT form_name, handler_name,
    schema_name, table_name, xml_template, target_div,
    add_description, prompt_container, form_set_name
   FROM qz.form
   WHERE form_name = $1 $FMTAED$
WHERE form_NAME = 'form'
AND action = 'edit';

UPDATE qz.table_action
SET pkey = '{form_name, handler_name, action}'
WHERE form_name = 'table_action_edit'
AND action = 'create';

UPDATE qz.table_action
SET sql = $TATAED$SELECT
    parameter_key, parameter_value
    FROM
    qz.fixed_parameter
    WHERE
    menu_name = $1
    AND 
    menu_item_sequence = $2 $TATAED$
WHERE form_name = 'fixed_parameters'
AND action = 'edit';

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, expand_percent_n, onchange)
VALUES
('fixed_parameters', 'parameter_key', 'input_text', 't', 
$PRFK$change_status(%n, 'U')$PRFK$),
('fixed_parameters', 'parameter_value', 'input_text', 't', 
$PRFK$change_status(%n, 'U')$PRFK$);

UPDATE qz.form
SET form_set_name = 'menu_mgt'
WHERE form_name = 'fixed_parameters';

UPDATE qz.menu_item
SET menu_text = 'Menu Items'
WHERE menu_name = 'menu_submenu'
AND menu_item_sequence = 1;

DELETE FROM qz.form_set
WHERE set_name = 'fixed_parameters';

UPDATE qz.table_action
SET sql = $PMED$SELECT set_id, menu_name, action
           FROM qz.menu_set
           WHERE host_form_name = $1
           ORDER BY menu_name, action $PMED$
WHERE form_name = 'page_menus'           
AND action = 'edit';

UPDATE qz.prompt_rule
SET prompt_type = 'select_fkey'
WHERE form_name = 'page_menus'
AND fieldname = 'menu_name';

UPDATE qz.prompt_rule
SET prompt_type = 'input_hidden'
WHERE form_name = 'page_menus'
AND fieldname = 'set_id';

UPDATE qz.table_action
SET helpful_text = 'Change the menus this form contains. Set the action to "any" if uncertain'
WHERE form_name = 'page_menus'
AND action = 'edit';

UPDATE qz.table_action
SET helpful_text = 'A form set allows menu items to share context parameters, 
    attributes returned from a table action and named in a form set are 
    added to menu items as hidden fields.'
WHERE form_name = 'form_set'
AND (action) IN ('getall','edit','create');

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, "size")
VALUES
('prompt_rule_edit', 'regex_pattern', 'input_text', '60') ;

-- Variable Name
-- Forbiden Characters:
-- !"#$%&'()*+,-./:;<=>?@[\]^`{|}~
-- Control Characters, whitespace

UPDATE qz.prompt_rule
SET regex_pattern = $NPRRX$^[^\x01-\x2f\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x60]{1,63}$$NPRRX$
WHERE regex_pattern = $OPRRX$^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$$OPRRX$;


-- Filename 
--  Same as Variable name except . and - are allowed
UPDATE qz.prompt_rule
SET regex_pattern = $NFNRX$^[^\x01-\x2c\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x2f\x60]{1,63}$$NFNRX$
WHERE fieldname = 'filename';

COMMENT ON COLUMN qz.form.schema_name IS 'The schema the table is in.';
COMMENT ON COLUMN qz.form.table_name IS 'The name of the table being edited';
COMMENT ON COLUMN qz.prompt_rule.onchange IS $PROC$ Set to "change_status(%n, 'U')" for grid items.$PROC$;



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
        "onetable": ["getall", "create", "insert", "edit", "update", "delete"], 
        "grid": ["edit", "save", "insert_row", "update_row", "delete_row"],
        "fs": ["get", "etag_value" ]
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
