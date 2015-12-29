CREATE TABLE qz.table_action (
    form_name qz.variable_name references qz.form(form_name),
    action qz.variable_name,
    sql text,
    fieldnames text[],
    pkey text[],
    etag bigint not null default nextval('qz.etag_seq'::regclass),
    helpful_text text,
    PRIMARY KEY (form_name, action)
);    

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form', 'insert', 
   'INSERT INTO qz.form
    (form_name, handler_name)
    VALUES ($1,$2)', 
'{form_name,handler_name}', '{form_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form', 'create', 
   'SELECT $1::text form_name,
    ''''::text handler_name', 
'{form_name}', '{form_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form', 'getall', 
    'SELECT form_name, handler_name
     FROM qz.form
     ORDER BY form_name, handler_name', 
NULL, '{form_name}', 
'A form on this list will match incoming data to a particular set of table 
 actions.  The form_name is the 2nd segment of the URL.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form', 'edit', 
    'SELECT form_name, handler_name, 
     schema_name, table_name, xml_template, target_div,
     add_description, prompt_container
     FROM qz.form
     WHERE form_name = $1', 
'{form_name}', '{form_name}', 
'The handler_name will provide a specific set of actions, such as edit, update, 
 delete.  The xml_template will be used as the starting document.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form', 'update', 
   'UPDATE qz.form SET 
    schema_name = $2,
    table_name = $3,
    xml_template = $4,
    target_div = $5,
    add_description = $6,
    prompt_container = $7,
    handler_name = $8
    WHERE form_name = $1', 
'{form_name,schema_name,table_name,xml_template,target_div,add_description,prompt_container,handler_name}', 
'{form_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form', 'delete', 
   'DELETE FROM qz.form
    WHERE form_name = $1', 
'{form_name}', '{form_name}', NULL);

--
-- prompt_rule_edit
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('prompt_rule_edit', 'create', 
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
    ''''::text "size"', 
'{form_name,fieldname}', '{form_name,fieldname}', NULL);


INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('prompt_rule_edit', 'insert',
  'INSERT INTO qz.prompt_rule
  ("form_name", "fieldname", "prompt_type", "options",
  "el_class", "readonly", "rows", "cols", "size")
  VALUES
  ($1,$2,$3,$4,$5,$6,$7,$8,$9)', 
'{form_name,fieldname,prompt_type,options,el_class,readonly,rows,cols,size}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('prompt_rule_edit', 'getall', 
   'SELECT fieldname, prompt_type, el_class, 
    readonly, rows, cols, size, options, etag 
    FROM qz.prompt_rule 
    WHERE form_name = $1
    ORDER BY form_name, fieldname', 
'{form_name}', '{form_name,fieldname}', 
'These rules determine how an attribute
from PostgreSQL is converted into a
data entry prompt.  A lookup name and
a fieldname identify a prompt rule.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('prompt_rule_edit', 'edit', 
    'SELECT "form_name", "fieldname", "prompt_type", "tabindex",
    "el_class", "readonly", "regex_pattern", "rows", "cols", "size", 
    "maxlength", "options", "publish_pgtype", "expand_percent_n",
     "onfocus", "onblur", "onchange", "onselect",
    "onclick", "ondblclick", "onmousedown", "onmouseup",
    "onmouseover", "onmousemove", "onmouseout",
    "onkeypress", "onkeydown", "onkeyup"
    FROM qz.prompt_rule
    WHERE form_name = $1 AND fieldname = $2', 
'{form_name,fieldname}', '{form_name,fieldname}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('prompt_rule_edit', 'update', 
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
    AND fieldname = $28', 
'{prompt_type,el_class,readonly,regex_pattern,rows,cols,size,tabindex,options,maxlength,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup,publish_pgtype,expand_percent_n,form_name,fieldname}',
NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('prompt_rule_edit', 'delete', 
    'DELETE FROM qz.prompt_rule
     WHERE form_name = $1 AND fieldname = $2', 
'{form_name,fieldname}', '{form_name,fieldname}', NULL);

--
-- table_action_edit
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('table_action_edit', 'edit', 
    'SELECT
      form_name, action, 
      helpful_text, sql,
      fieldnames, pkey 
    FROM
      qz.table_action
    WHERE
      form_name = $1 AND action = $2 ', 
'{form_name,action}', '{form_name,action}', 
  'A table action binds a URL and HTTP post data to an SQL statement.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('table_action_edit', 'insert', 
   'INSERT INTO qz.table_action
   (form_name, action, 
   helpful_text, sql, fieldnames, pkey)
   VALUES
   ($1,$2,$3,$4,$5,$6)', 
'{form_name,action,helpful_text,sql,fieldnames,pkey}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('table_action_edit', 'update', 
    'UPDATE qz.table_action
    SET helpful_text=$3,
    sql=$4, fieldnames=$5, pkey=$6 
    WHERE form_name = $1 AND action = $2', 
'{form_name,action,helpful_text,sql,fieldnames,pkey}', 
'{form_name,action}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('table_action_edit', 'delete', 
'DELETE FROM qz.table_action
WHERE form_name = $1 AND action = $2', 
'{form_name,action}', '{form_name,action}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('table_action_edit', 'create', 
    'SELECT form_name, action, ''''::text helpful_text, sql, fieldnames, pkey FROM create_table_action($1,$2)', '{form_name,action}', 
'{form_name,action}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('table_action_edit', 'getall', 
    'SELECT action, helpful_text 
    FROM qz.table_action
    WHERE form_name = $1
    ORDER BY form_name, action', 
'{form_name}', '{form_name,action}', 
'Edit the table actions for a given form_name.');


--
-- menu_set_edit
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_set_edit', 'insert_row', 
    'INSERT INTO qz.menu_set
    ("menu_name", "host_form_name", "action" )
    VALUES ($1,$2,$3)', 
'{menu_name,host_form_name,action}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_set_edit', 'save', 
'SELECT 1', 
NULL, NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_set_edit', 'delete_row', 
    'DELETE FROM 
     qz.menu_set
    WHERE  set_id = $1', 
'{set_id}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_set_edit', 'update_row', 
    'UPDATE qz.menu_set
    SET 
    menu_name = $1,
    "host_form_name" = $2,
    "action" = $3
    WHERE set_id = $4', 
'{menu_name,host_form_name,action,set_id}', '{menu_name}', NULL);


INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_set_edit', 'edit', 
    'SELECT set_id, host_form_name, action
    FROM qz.menu_set
    WHERE menu_name = $1
    ORDER BY host_form_name, action', 
'{menu_name}', '{menu_name}', 
'This is a table interface that ties a particular menu_name  to a particular table action.
"host_form_name" is the page hosting the given menu name.  A action of any implies all actions
for the given host_form_name.');

--
-- menu_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_edit', 'getall', 
    'SELECT menu_name, target_div, description
    FROM qz.menu
    ORDER BY menu_name, target_div',
NULL, '{menu_name}', 
'This is a list of menus that have been created.  Press Insert to add a new menu.  Press Edit on a menu for more details.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_edit', 'edit', 
    'SELECT menu_name, target_div, description
    FROM qz.menu
    WHERE menu_name = $1
    ORDER BY menu_name', 
'{menu_name}', '{menu_name}',  
'Press Menu_Item_Edit to add, change, or delete the items on a menu.
Press Menu_Set_Edit to manage which page gets a particular menu.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_edit', 'update', 
    'UPDATE qz.menu
    SET
    target_div = $2,
    description = $3
    WHERE menu_name = $1', 
'{menu_name,target_div,description}', '{menu_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_edit', 'insert', 
    'INSERT INTO qz.menu
    (menu_name, target_div, description)
    VALUES 
    ($1,$2,$3)', 
'{menu_name,target_div,description}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_edit', 'create',  
    'SELECT ''''::text menu_name, ''''::text target_div, ''''::text description', 
NULL, NULL, 
'Create a new menu  name.  The target_div is the html id of a <div> tag in the xml template that the menu will be placed into.  ');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_edit', 'delete', 
    'DELETE FROM qz.menu
    WHERE menu_name = $1', 
'{menu_name}', '{menu_name}', NULL);




--
-- menu_host_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_host_edit', 'edit',  
    'SELECT set_id, host_form_name, menu_name, action
    FROM qz.menu_set
    WHERE host_form_name = $1
    ORDER BY host_form_name, menu_name, action', 
'{form_name}', '{form_name}', 
'This is a table interface that ties a 
particular menu_name to a particular
table action.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_host_edit', 'update_row', 
    'UPDATE qz.menu_set
    SET
    menu_name = $1,
    "action" = $2
    WHERE
    host_form_name = $3
    AND
    set_id = $4', 
'{menu_name,action,host_form_name,set_id}', '{set_id}', NULL);

INSERT INTO qz.table_action (form_name, action,  sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_host_edit', 'insert_row', 
    'INSERT INTO qz.menu_set
    ("menu_name", "host_form_name", "action" )
    VALUES ($1,$2,$3)', 
'{menu_name,form_name,action}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_host_edit', 'delete_row',  
    'DELETE FROM 
      qz.menu_set
    WHERE 
      set_id = $1
    AND
      host_form_name = $2', 
'{set_id,host_form_name}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_host_edit', 'save', 
    'SELECT 1', 
NULL, NULL, NULL);



--
-- menu_item_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_item_edit', 'getall',  
    'SELECT  menu_item_sequence, target_form_name, action,
    menu_text, context_parameters
    FROM qz.menu_item
    WHERE menu_name = $1
    ORDER BY menu_item_sequence', 
'{menu_name}', '{menu_name,menu_item_sequence}', 
'Maintain the list of choices on a menu.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_item_edit', 'delete', 
    'DELETE FROM qz.menu_item
    WHERE menu_name = $1
    AND menu_item_sequence = $2', 
'{menu_name,menu_item_sequence}', '{menu_name,menu_item_sequence}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_item_edit', 'create', 
   'SELECT $1::text menu_name, 
    $2::text menu_item_sequence, 
    ''''::text target_form_name,
    ''''::text "action", 
    ''''::text menu_text,
    ''''::text context_parameters', 
'{menu_name,menu_item_sequence}', '{menu_name,menu_item_sequence}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_item_edit', 'edit', 
    'SELECT menu_name, menu_item_sequence, target_form_name, action, 
    menu_text, context_parameters
    FROM qz.menu_item
    WHERE menu_name = $1
    AND menu_item_sequence = $2', 
'{menu_name,menu_item_sequence}', '{menu_name,menu_item_sequence}',
'For one menu choice, identify the form_name and optionally the action (not all 
handlers have an action, but most do) that is offered for selection.
The context parameters are fields in the current page to be passed to the named 
target lookup.  Fixed parameters are set keys and values allowing for example a 
button to edit a particular row.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_item_edit', 'insert', 
    'INSERT INTO qz.menu_item
    (menu_name, menu_item_sequence, target_form_name, action, menu_text)
    VALUES
    ($1,$2,$3,$4,$5)', 
'{menu_name,menu_item_sequence,target_form_name,action,menu_text}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu_item_edit', 'update', 
    'UPDATE qz.menu_item
    SET
      target_form_name = $3,
      action = $4,
      menu_text = $5,
      context_parameters = $6
    WHERE
      menu_name = $1
    AND 
      menu_item_sequence = $2', 
'{menu_name,menu_item_sequence,target_form_name,action,menu_text,context_parameters}', 
'{menu_name,menu_item_sequence}', NULL);

--
-- fixed_parameters
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('fixed_parameters', 'save',  'SELECT 1', NULL, NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('fixed_parameters', 'delete_row', 
    'DELETE FROM qz.menu_item_parameter
    WHERE menu_name = $1 
    AND menu_item_sequence = $2
    AND parameter_key = $3', 
'{menu_name,menu_item_sequence,parameter_key}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text)
VALUES ('fixed_parameters', 'insert_row', 
    'INSERT INTO qz.menu_item_parameter
    (menu_name, menu_item_sequence, parameter_key, parameter_value)
    VALUES
    ($1,$2,$3,$4)', 
'{menu_name,menu_item_sequence,parameter_key,parameter_value}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('fixed_parameters', 'update_row', 
    'UPDATE qz.menu_item_parameter
    SET parameter_value = $4
    WHERE menu_name = $1
    AND menu_item_sequence = $2
    AND parameter_key = $3', 
'{menu_name,menu_item_sequence,parameter_key,parameter_value}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('fixed_parameters', 'edit', 
    'SELECT
    parameter_key,
    parameter_value, menu_item_sequence
    FROM
    qz.menu_item_parameter
    WHERE
    menu_name = $1
    AND 
    menu_item_sequence = $2', 
'{menu_name,menu_item_sequence}', '{menu_name,menu_item_sequence}', 
'Fixed parameters are key value pairs
that are set to a specific key and value.
This allows a menu option to edit a 
specific row or set of rows.');

--
-- form_menu_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form_menu_edit', 'edit', 
    'SELECT menu_name, action 
    FROM qz.menu_set
    WHERE host_form_name = $1', 
'{form_name}', NULL, 'Add menus to this form here.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form_menu_edit', 'update_row', 
    'UPDATE qz.menu_set
    SET
    "menu_name" = $1.
    "action" = $2
    WHERE host_form_name = $3', 
'{menu_name,action,host_form_name}', '{host_form_name}', NULL);

--
-- css
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css', 'etag_value', 
    'SELECT etag
    FROM qz.css 
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css', 'get', 
    'SELECT filename,mimetype,modtime,etag,data 
    FROM qz.css
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);

--
-- js
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js', 'etag_value', 
    'SELECT etag
    FROM qz.js 
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js', 'get', 
    'SELECT filename,mimetype,modtime,etag,data 
    FROM qz.js
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);


--
-- css_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css_edit', 'create', 
    'SELECT
     $1::text filename, 
    ''text/css'' mimetype, 
    now() modtime,
    '''' "data"', 
'{filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action,  sql, fieldnames, pkey, helpful_text) 
VALUES ('css_edit', 'insert', 
    'INSERT INTO qz.css (filename, mimetype, modtime, "data")
    VALUES ($1,$2, $3, $4);', 
'{filename,mimetype,modtime,data}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css_edit', 'getall',  
    'SELECT filename, modtime, mimetype, length(data) length
    FROM qz.css
    ORDER BY filename', 
NULL, '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css_edit', 'edit', 
    'SELECT filename, modtime, etag, mimetype, "data"
    FROM qz.css
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css_edit', 'update', 
    'UPDATE qz.css
    SET mimetype = $1, 
    data = $2
    WHERE filename = $3', 
'{mimetype,data,filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('css_edit', 'delete', 
    'DELETE FROM qz.css
    WHERE filename = $1', 
'{filename}', NULL, NULL);

--
-- js_edit
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js_edit', 'create', 
    'SELECT
    $1::text filename, 
    ''text/javascript'' mimetype, 
    now() modtime,
    '''' "data"', '{filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js_edit', 'insert', 
    'INSERT INTO qz.js (filename, mimetype, modtime, "data")
    VALUES ($1,$2, $3, $4);', 
'{filename,mimetype,modtime,data}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js_edit', 'getall', 
   'SELECT filename, modtime, mimetype, length(data) length
    FROM qz.js
    ORDER BY filename', 
NULL, '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js_edit', 'edit', 
    'SELECT filename, modtime, etag, mimetype, "data"
    FROM qz.js
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js_edit', 'update', 
    'UPDATE qz.js
    SET mimetype = $1, 
    data = $2
    WHERE filename = $3', 
'{mimetype,data,filename}', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('js_edit', 'delete', 
    'DELETE FROM qz.js
    WHERE filename = $1', 
'{filename}', '{filename}', NULL);

--
-- page_css
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_css', 'edit', 
    'SELECT
    "sequence", filename
    FROM 
    qz.page_css
    WHERE
    form_name = $1', 
'{form_name}', '{form_name}',
'Select cascading style sheet files that are to be loaded in the html header as a link attribute.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_css', 'save', 'SELECT 1', NULL, NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_css', 'insert_row', 
    'INSERT INTO qz.page_css
    (form_name, "sequence", filename)
    VALUES
    ($1,$2,$3)', 
'{form_name,sequence,filename}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_css', 'update_row', 
    'UPDATE qz.page_css
    SET
    "sequence" = $1
    WHERE
    form_name = $2
    AND
    filename =  $3', 
'{sequence,form_name,filename}', '{form_name,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_css', 'delete_row', 
    'DELETE FROM qz.page_css
    WHERE
    form_name = $1
    AND
    filename = $2', 
'{form_name,filename}', NULL, NULL);

--
-- page_js
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_js', 'edit', 
    'SELECT
    "sequence", filename
    FROM 
    qz.page_js
    WHERE form_name = $1 
    ORDER BY "sequence"', 
'{form_name}', '{form_name}',
'Select the javascript files that are to be loaded in the html header as a script element, src attribute.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_js', 'save', 'SELECT 1', NULL, NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_js', 'insert_row', 
    'INSERT INTO qz.page_js
    (form_name, "sequence", filename)
    VALUES
    ($1,$2,$3)', 
'{form_name,sequence,filename}', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_js', 'update_row', 
    'UPDATE qz.page_js
    SET
    "sequence" = $1
    WHERE
    form_name = $2
    AND
    filename =  $3',
'{sequence,form_name,filename}', '{form_name,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('page_js', 'delete_row', 
    'DELETE FROM qz.page_js
    WHERE
    form_name = $1
    AND
    filename = $2', 
'{form_name,filename}', NULL, NULL);


--
-- misc
--
INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('form_dev', 'view', 'SELECT 1', NULL, NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('menu', 'view', 'SELECT 1', NULL, NULL, NULL);


INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('pg_stat_activity', 'fetch', 
    'SELECT datname,pid,usename,application_name,client_addr,backend_start,
    query_start,waiting,query FROM pg_stat_activity', 
NULL, NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, pkey, helpful_text) 
VALUES ('status', 'view', 'SELECT 1', NULL, NULL, NULL);
