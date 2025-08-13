CREATE TABLE qz.table_action (
    form_name qz.variable_name references qz.form(form_name),
    action qz.variable_name,
    sql text,
    fieldnames text[],
    etag bigint not null default nextval('qz.etag_seq'::regclass),
    helpful_text text,
    inline_js text,
    inline_css text,
    clear_context_parameters varchar(63)[],
    callback_attached_action qz.variable_name,
    is_callback boolean DEFAULT 'f',
    callback_response qz.callback_response_type,
    PRIMARY KEY (form_name, action)
);

--
-- Form
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('form', 'insert',
   $TAFI$INSERT INTO qz.form
     (form_name, handler_name, schema_name, table_name, pkey,
     xml_template, target_div,
     add_description, prompt_container)
     VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)$TAFI$,
'{form_name,handler_name,schema_name,table_name,pkey,xml_template,target_div,add_description,prompt_container}',
NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('form', 'create',
   $TAFC$SELECT
    $1::text form_name,
    $2::text handler_name,
    ''::text schema_name, ''::text table_name,
    ''::text pkey,
    'base.xml'::text xml_template,
    'qz'::text target_div,
    ''::text add_description,
    ''::text prompt_container$TAFC$,
'{form_name,new_handler_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames,
    helpful_text, clear_context_parameters)
VALUES ('form', 'list',
    'SELECT form_name, handler_name
     FROM qz.form
     WHERE NOT hidden
     ORDER BY form_name',
NULL,
'A form on this list will match incoming data to a particular set of table
 actions.  The form_name is the 2nd segment of the URL.',
 '{form_name,handler_name}');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('form', 'edit',
  $FTAE$ SELECT form_name, handler_name,
     schema_name, table_name, pkey, xml_template, target_div,
     add_description, prompt_container, form_set_name
  FROM qz.form
  WHERE form_name = $1 $FTAE$,
'{form_name}',
'The handler_name will provide a specific set of actions, such as edit, update,
 delete.  The xml_template will be used as the starting document.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('form', 'update',
   $FTAU$ UPDATE qz.form SET
     schema_name = $2,
     table_name = $3,
     pkey = $4,
     xml_template = $5,
     target_div = $6,
     add_description = $7,
     prompt_container = $8,
     form_set_name = $9
   WHERE form_name = $1 $FTAU$,
'{form_name,schema_name,table_name,pkey,xml_template,target_div,add_description,prompt_container, form_set_name}',
NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('form', 'delete',
   'DELETE FROM qz.form
    WHERE form_name = $1',
'{form_name}', NULL);

--
-- prompt_rule_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('prompt_rule_edit', 'create',
   $PRCTA$ SELECT
    $1::text "form_name",
    $2::text "fieldname",
    'input_text'::qz.prompt_types "prompt_type",
    ''::text "tabindex",
    ''::text "el_class",
    'f'::text "readonly",
    'f'::text "required",
    ''::text "regex_pattern",
    ''::text "rows",
    ''::text "cols",
    ''::text "size",
    ''::text "maxlength",
    ''::text "options",
    ''::text "publish_pgtype",
    CASE WHEN 'grid' = $3 then
        't'::text
    ELSE
        'f'::text
    END "expand_percent_n",
    ''::text "onfocus",
    ''::text "onblur",
    CASE WHEN 'grid' = $3 then
        $CS$ change_status(%n, 'U') $CS$::text
    ELSE
        ''::text
    END "onchange",
    ''::text "onselect",
    ''::text "onclick",
    ''::text "ondblclick",
    ''::text "onmousedown",
    ''::text "onmouseup",
    ''::text "onmouseover",
    ''::text "onmousemove",
    ''::text "onmouseout",
    ''::text "onkeypress",
    ''::text "onkeydown",
    ''::text "onkeyup" $PRCTA$,
'{form_name,fieldname,handler_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('prompt_rule_edit', 'insert',
  $PRITA$INSERT INTO qz.prompt_rule
  ("form_name", "fieldname", "prompt_type", "tabindex",
  "el_class", "readonly", "required", "regex_pattern",
  "rows", "cols", "size",
  "maxlength", "options", "publish_pgtype",
 "expand_percent_n", onfocus, onblur, onchange, onselect,
 onclick, ondblclick, onmousedown, onmouseup,
 onmouseover, onmousemove, onmouseout,
 onkeypress, onkeydown, onkeyup )
  VALUES
  ($1,$2,$3,$4,$5,$6,$7,$8,$9,
  $10,$11,$12,$13,$14,$15,$16,$17,$18,
  $19,$20,$21,$22,$23,$24,$25,$26,$27,$28,$29)$PRITA$,
'{form_name,fieldname,prompt_type,tabindex,el_class,readonly,required,regex_pattern,rows,cols,size,maxlength,options,publish_pgtype,expand_percent_n,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup}',
NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('prompt_rule_edit', 'list',
   'SELECT fieldname, prompt_type, el_class,
    readonly, rows, cols, size, options, etag
    FROM qz.prompt_rule
    WHERE form_name = $1
    ORDER BY form_name, fieldname',
'{form_name}',
'These rules determine how an attribute
from PostgreSQL is converted into a
data entry prompt.  A lookup name and
a fieldname identify a prompt rule.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
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
'{form_name,fieldname}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('prompt_rule_edit', 'update',
    'UPDATE qz.prompt_rule
    SET
    "prompt_type" = $1,
    "el_class" = $2,
    "readonly" = $3,
    "required" = $4,
    "regex_pattern" = $5,
    "rows" = $6,
    "cols" = $7,
    "size" = $8,
    "tabindex" = $9,
    "options" = $10,
    "maxlength" = $11,
    "onfocus" = $12,
    "onblur" = $13,
    "onchange" = $14,
    "onselect" = $15,
    "onclick" = $16,
    "ondblclick" = $17,
    "onmousedown" = $18,
    "onmouseup" = $19,
    "onmouseover" = $20,
    "onmousemove" = $21,
    "onmouseout" = $22,
    "onkeypress" = $23,
    "onkeydown" = $24,
    "onkeyup" = $25,
    "publish_pgtype" = $26,
    "expand_percent_n" = $27
    WHERE form_name = $28
    AND fieldname = $29',
'{prompt_type,el_class,readonly,required,regex_pattern,rows,cols,size,tabindex,options,maxlength,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup,publish_pgtype,expand_percent_n,form_name,fieldname}',
NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('prompt_rule_edit', 'delete',
    'DELETE FROM qz.prompt_rule
     WHERE form_name = $1 AND fieldname = $2',
'{form_name,fieldname}', NULL);

--
-- table_action_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('table_action_edit', 'edit',
    $TAE$SELECT
      form_name, action, helpful_text, sql, fieldnames, clear_context_parameters
    FROM
      qz.table_action
    WHERE
    form_name = $1 AND action = $2 $TAE$,
'{form_name,action}',
  'A table action binds a URL and HTTP post data to an SQL statement.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text, inline_js)
VALUES ('table_action_edit', 'insert',
   $TAI$INSERT INTO qz.table_action
      (form_name, action, helpful_text, sql, fieldnames,
      clear_context_parameters)
      VALUES
      ($1,$2,$3,$4,$5,$6)$TAI$,
'{form_name,new_action,helpful_text,sql,fieldnames,clear_context_parameters}',
NULL,
$TAIJS$ window.addEventListener("DOMContentLoaded",set_action_options, true); $TAIJS$ );

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text, inline_js)
VALUES ('table_action_edit', 'update',
    $TAU$UPDATE qz.table_action
    SET helpful_text=$3,
    sql=$4, fieldnames=$5, clear_context_parameters=$6
    WHERE form_name = $1 AND action = $2 $TAU$,
'{form_name,action,helpful_text,sql,fieldnames,clear_context_parameters}',
NULL,
$TAUJS$ window.addEventListener("DOMContentLoaded",set_action_options, true); $TAUJS$ );

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('table_action_edit', 'delete',
'DELETE FROM qz.table_action
WHERE form_name = $1 AND action = $2',
'{form_name,action}', 'NULL');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('table_action_edit', 'create',
    $TAC$SELECT ta.form_name, ta.action new_action, fm.handler_name,
    ''::text helpful_text, ta.sql, ta.fieldnames,
    ''::text clear_context_parameters
     FROM qz.create_table_action($1,$2) ta
    JOIN qz.form fm USING (form_name)$TAC$,
'{form_name, action}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text, inline_js)
VALUES ('table_action_edit', 'list',
    $TAG$SELECT ta.action, ta.helpful_text,
    fm.handler_name
    FROM qz.table_action ta
    JOIN qz.form fm USING (form_name)
    WHERE form_name = $1
    AND NOT is_callback
    ORDER BY form_name, action$TAG$,
'{form_name}', 'Edit the table actions for a given form_name.',
$TALJS$ window.addEventListener("DOMContentLoaded",set_action_options, true); $TALJS$ );

--
-- menu_set_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_set_edit', 'insert_row',
    'INSERT INTO qz.menu_set
    ("menu_name", "host_form_name", "action" )
    VALUES ($1,$2,$3)',
'{menu_name,host_form_name,action}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_set_edit', 'save',
'SELECT 1',
NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_set_edit', 'delete_row',
    'DELETE FROM
     qz.menu_set
    WHERE  set_id = $1',
'{set_id}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_set_edit', 'update_row',
    'UPDATE qz.menu_set
    SET
    menu_name = $1,
    "host_form_name" = $2,
    "action" = $3
    WHERE set_id = $4',
'{menu_name,host_form_name,action,set_id}', NULL);


INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_set_edit', 'edit',
    'SELECT set_id, host_form_name, action
    FROM qz.menu_set
    WHERE menu_name = $1
    ORDER BY host_form_name, action',
'{menu_name}',
'This is a table interface that ties a particular menu_name  to a particular table action.
"host_form_name" is the page hosting the given menu name.  A action of any implies all actions
for the given host_form_name.');

--
-- menu_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames,
    helpful_text, clear_context_parameters)
VALUES ('menu_edit', 'list',
    'SELECT menu_name, target_div, description
    FROM qz.menu
    ORDER BY menu_name, target_div',
NULL,
'This is a list of menus that have been created.  Press Insert to add a new menu.  Press Edit on a menu for more details.', '{menu_name,menu_item_sequence}');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_edit', 'edit',
    $MED$SELECT menu_name, target_div, description, form_set_name
    FROM qz.menu
    WHERE menu_name = $1 $MED$,
'{menu_name}',
'Press Menu_Item to add, change, or delete the items on a menu.
Press Menu_Set to manage which page gets a particular menu.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_edit', 'update',
    $MUPD$UPDATE qz.menu
    SET
    target_div = $2,
    description = $3,
    form_set_name = $4
    WHERE menu_name = $1 $MUPD$,
'{menu_name,target_div,description,form_set_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_edit', 'insert',
    $MINS$INSERT INTO qz.menu
    (menu_name, target_div, description, form_set_name)
    VALUES
    ($1,$2,$3,$4)$MINS$,
'{menu_name,target_div,description,form_set_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_edit', 'create',
    $MCR$SELECT ''::text menu_name,
    ''::text target_div,
    ''::text description,
    ''::text form_set_name$MCR$,
NULL,
'Create a new menu  name.  The target_div is the html id of a <div> tag in the xml template that the menu will be placed into.  ');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_edit', 'delete',
    'DELETE FROM qz.menu
    WHERE menu_name = $1',
'{menu_name}', NULL);

--
-- menu_host_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_host_edit', 'edit',
    'SELECT set_id, host_form_name, menu_name, action
    FROM qz.menu_set
    WHERE host_form_name = $1
    ORDER BY host_form_name, menu_name, action',
'{form_name}',
'This is a table interface that ties a
particular menu_name to a particular
table action.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_host_edit', 'update_row',
    'UPDATE qz.menu_set
    SET
    menu_name = $1,
    "action" = $2
    WHERE
    host_form_name = $3
    AND
    set_id = $4',
'{menu_name,action,host_form_name,set_id}', NULL);

INSERT INTO qz.table_action (form_name, action,  sql, fieldnames, helpful_text)
VALUES ('menu_host_edit', 'insert_row',
    'INSERT INTO qz.menu_set
    ("menu_name", "host_form_name", "action" )
    VALUES ($1,$2,$3)',
'{menu_name,form_name,action}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_host_edit', 'delete_row',
    'DELETE FROM
      qz.menu_set
    WHERE
      set_id = $1
    AND
      host_form_name = $2',
'{set_id,host_form_name}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_host_edit', 'save',
    'SELECT 1',
NULL, NULL);

--
-- menu_item_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_item_edit', 'list',
    'SELECT  menu_item_sequence, target_form_name, action,
    menu_text, context_parameters
    FROM qz.menu_item
    WHERE menu_name = $1
    ORDER BY menu_item_sequence',
'{menu_name}',
'Maintain the list of choices on a menu.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text,
clear_context_parameters)
VALUES ('menu_item_edit', 'delete',
    'DELETE FROM qz.menu_item
    WHERE menu_name = $1
    AND menu_item_sequence = $2',
'{menu_name,menu_item_sequence}', NULL, '{menu_item_sequence}');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_item_edit', 'create',
   'SELECT $1::text menu_name,
    $2::text menu_item_sequence,
    ''''::text target_form_name,
    ''''::text "action",
    ''''::text menu_text,
    ''''::text context_parameters',
'{menu_name,menu_item_sequence}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu_item_edit', 'edit',
    'SELECT menu_name, menu_item_sequence, target_form_name, action,
    menu_text, context_parameters
    FROM qz.menu_item
    WHERE menu_name = $1
    AND menu_item_sequence = $2',
'{menu_name,menu_item_sequence}',
'For one menu choice, identify the form_name and optionally the action (not all
handlers have an action, but most do) that is offered for selection.
The context parameters are fields in the current page to be passed to the named
target lookup.  Fixed parameters are set keys and values allowing for example a
button to edit a particular row.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text,
clear_context_parameters)
VALUES ('menu_item_edit', 'insert',
    'INSERT INTO qz.menu_item
    (menu_name, menu_item_sequence, target_form_name, action, menu_text)
    VALUES
    ($1,$2,$3,$4,$5)',
'{menu_name,menu_item_sequence,target_form_name,action,menu_text}', NULL,
'{menu_item_sequence}');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text,
clear_context_parameters)
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
NULL, '{menu_item_sequence}');

--
-- fixed_parameters
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('fixed_parameters', 'save',  'SELECT 1', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('fixed_parameters', 'delete_row',
    'DELETE FROM qz.fixed_parameter
    WHERE menu_name = $1
    AND menu_item_sequence = $2
    AND parameter_key = $3',
'{menu_name,menu_item_sequence,parameter_key}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('fixed_parameters', 'insert_row',
    'INSERT INTO qz.fixed_parameter
    (menu_name, menu_item_sequence, parameter_key, parameter_value)
    VALUES
    ($1,$2,$3,$4)',
'{menu_name,menu_item_sequence,parameter_key,parameter_value}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('fixed_parameters', 'update_row',
    'UPDATE qz.fixed_parameter
    SET parameter_value = $4
    WHERE menu_name = $1
    AND menu_item_sequence = $2
    AND parameter_key = $3',
'{menu_name,menu_item_sequence,parameter_key,parameter_value}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('fixed_parameters', 'edit',
    $TAFPED$SELECT
    parameter_key, parameter_value
    FROM
    qz.fixed_parameter
    WHERE
    menu_name = $1
    AND
    menu_item_sequence = $2 $TAFPED$,
'{menu_name,menu_item_sequence}',
'Fixed parameters are key value pairs
that are set to a specific key and value.
This allows a menu option to edit a
specific row or set of rows.');

--
-- css
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css', 'etag_value',
    'SELECT to_hex(etag) "etag"
    FROM qz.css
    WHERE filename = $1',
'{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css', 'get',
    'SELECT filename,mimetype,modtime,to_hex(etag) "etag",data
    FROM qz.css
    WHERE filename = $1',
'{filename}', NULL);

--
-- js
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js', 'etag_value',
    'SELECT to_hex(etag) "etag"
    FROM qz.js
    WHERE filename = $1',
'{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js', 'get',
    'SELECT filename,mimetype,modtime,to_hex(etag) "etag",data
    FROM qz.js
    WHERE filename = $1',
'{filename}', NULL);

--
-- css_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css_edit', 'create',
    'SELECT
     $1::text filename,
    ''text/css'' mimetype,
    now() modtime,
    '''' "data"',
'{filename}', NULL);

INSERT INTO qz.table_action (form_name, action,  sql, fieldnames, helpful_text)
VALUES ('css_edit', 'insert',
    'INSERT INTO qz.css (filename, mimetype, modtime, "data")
    VALUES ($1,$2, $3, $4);',
'{filename,mimetype,modtime,data}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css_edit', 'list',
    'SELECT filename, modtime, mimetype, length(data) length
    FROM qz.css
    ORDER BY filename',
NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css_edit', 'edit',
    'SELECT filename, modtime, etag, mimetype, "data"
    FROM qz.css
    WHERE filename = $1',
'{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css_edit', 'update',
    'UPDATE qz.css
    SET mimetype = $1,
    data = $2,
    modtime = now()
    WHERE filename = $3',
'{mimetype,data,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('css_edit', 'delete',
    'DELETE FROM qz.css
    WHERE filename = $1',
'{filename}', NULL);

--
-- js_edit
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js_edit', 'create',
    'SELECT
    $1::text filename,
    ''text/javascript'' mimetype,
    now() modtime,
    '''' "data"', '{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js_edit', 'insert',
    'INSERT INTO qz.js (filename, mimetype, modtime, "data")
    VALUES ($1,$2, $3, $4);',
'{filename,mimetype,modtime,data}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js_edit', 'list',
   'SELECT filename, modtime, mimetype, length(data) length
    FROM qz.js
    ORDER BY filename',
NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js_edit', 'edit',
    'SELECT filename, modtime, etag, mimetype, "data"
    FROM qz.js
    WHERE filename = $1',
'{filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js_edit', 'update',
    'UPDATE qz.js
    SET mimetype = $1,
    data = $2,
    modtime = now()
    WHERE filename = $3',
'{mimetype,data,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('js_edit', 'delete',
    'DELETE FROM qz.js
    WHERE filename = $1',
'{filename}', NULL);

--
-- page_css
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_css', 'edit',
    'SELECT
    "sequence", filename
    FROM
    qz.page_css
    WHERE
    form_name = $1',
'{form_name}',
'Select cascading style sheet files that are to be loaded in the html header as a link attribute.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_css', 'save', 'SELECT 1', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_css', 'insert_row',
    'INSERT INTO qz.page_css
    (form_name, "sequence", filename)
    VALUES
    ($1,$2,$3)',
'{form_name,sequence,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_css', 'update_row',
    'UPDATE qz.page_css
    SET
    "sequence" = $1
    WHERE
    form_name = $2
    AND
    filename =  $3',
'{sequence,form_name,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_css', 'delete_row',
    'DELETE FROM qz.page_css
    WHERE
    form_name = $1
    AND
    filename = $2',
'{form_name,filename}', NULL);

--
-- page_js
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_js', 'edit',
    'SELECT
    "sequence", filename
    FROM
    qz.page_js
    WHERE form_name = $1
    ORDER BY "sequence"',
'{form_name}',
'Select the javascript files that are to be loaded in the html header as a script element, src attribute.');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_js', 'save', 'SELECT 1', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_js', 'insert_row',
    'INSERT INTO qz.page_js
    (form_name, "sequence", filename)
    VALUES
    ($1,$2,$3)',
'{form_name,sequence,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_js', 'update_row',
    'UPDATE qz.page_js
    SET
    "sequence" = $1
    WHERE
    form_name = $2
    AND
    filename =  $3',
'{sequence,form_name,filename}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('page_js', 'delete_row',
    'DELETE FROM qz.page_js
    WHERE
    form_name = $1
    AND
    filename = $2',
'{form_name,filename}', NULL);


--
-- misc
--

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('form_dev', 'view', 'SELECT 1', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('menu', 'view', 'SELECT 1', NULL, NULL);


INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text)
VALUES ('pg_stat_activity', 'fetch',
    'SELECT datname,pid,usename,application_name,client_addr,backend_start,
    query_start,query FROM pg_stat_activity',
NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text, inline_js)
VALUES ( 'status', 'view', 'SELECT 1', NULL, NULL,
$TAINLJS$ function menu_click(on_item){

      var table_list = document.getElementsByClassName("qztable");
      var i;
      for (i = 0; i < table_list.length; i++){
          if (table_list[i].id == on_item){
              table_list[i].style.display = "block";
          }else{
              table_list[i].style.display = "none";
          }
      }
}
function hide_sections(){
    // $(".qztable").hide();

    var table_list = document.getElementsByClassName("qztable");
    var i;
    for (i = 0; i < table_list.length; i++){
        table_list[i].style.display = "none";
    }

}
window.addEventListener("DOMContentLoaded", hide_sections, false);
$TAINLJS$
);

INSERT INTO qz.table_action
(form_name, action, sql)
VALUES
('status', 'pg_stat_activity',  'SELECT datname,pid,usename,application_name,client_addr,backend_start, query_start,query FROM pg_stat_activity');

INSERT INTO qz.table_action
(form_name, action, sql)
VALUES
('status', 'pg_version', 'SELECT version()');

INSERT INTO qz.table_action
(form_name, action, sql)
VALUES
('status', 'change_history',
  'SELECT change_id, changed, changed_by,
    change_description, note
  FROM qz.change_history
  ORDER BY change_id');

INSERT INTO qz.table_action
(form_name, action, sql)
VALUES
('status', 'schema_version',
    'SELECT schema_version FROM qz.constants');

--
-- form_set
--

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, helpful_text)
VALUES
('form_set', 'list',
'SELECT set_name FROM qz.form_set ORDER BY set_name',
NULL,
'A form set allows menu items to share context parameters,
    attributes returned from a table action and named in a form set are
    added to menu items as hidden fields.');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, helpful_text)
VALUES
('form_set', 'create',
$FSC$SELECT ''::text set_name, ''::text context_parameters$FSC$,
NULL,
'A form set allows menu items to share context parameters,
    attributes returned from a table action and named in a form set are
    added to menu items as hidden fields.');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames)
VALUES
('form_set', 'insert',
$FSI$INSERT INTO qz.form_set (set_name,context_parameters) VALUES ($1,$2) $FSI$,
'{set_name,context_parameters}');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, helpful_text)
VALUES
('form_set', 'edit',
$FSE$SELECT set_name, context_parameters
     FROM qz.form_set
     WHERE set_name = $1$FSE$,
'{set_name}',
'A form set allows menu items to share context parameters,
    attributes returned from a table action and named in a form set are
    added to menu items as hidden fields.');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames)
VALUES
('form_set', 'update',
$FSU$UPDATE qz.form_set
     SET context_parameters = $2
     WHERE set_name = $1$FSU$,
'{set_name,context_parameters}');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames)
VALUES
('form_set', 'delete',
$FSD$DELETE FROM qz.form_set
     WHERE set_name = $1$FSD$,
'{set_name}');

---
--- page_menus
---

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, helpful_text)
VALUES
('page_menus', 'edit',
$PMED$SELECT set_id, menu_name, action
FROM qz.menu_set
WHERE host_form_name = $1
ORDER BY menu_name, action$PMED$,
'{form_name}',
'Change the menus this form contains. Set the action to "any" if uncertain');

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
(form_name, action, sql, fieldnames)
VALUES
('page_menus', 'update_row',
    $PMUR$UPDATE qz.menu_set
    SET "menu_name" = $2,
        "host_form_name" = $3,
        "action" = $4
    WHERE set_id = $1$PMUR$,
'{set_id, menu_name, form_name, action}');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames)
VALUES
('page_menus', 'delete_row',
   $PMDR$DELETE FROM qz.menu_set
   WHERE set_id = $1$PMDR$,
 '{set_id}');

--- inline js and css
INSERT INTO qz.table_action
(form_name, action, sql, fieldnames)
VALUES
('inline_js', 'edit',
$TAJSED$ SELECT form_name, action, inline_js
    FROM qz.table_action
    WHERE form_name = $1 and action = $2 $TAJSED$,
'{form_name,action}'),

('inline_js', 'update',
$TAJSUP$ UPDATE qz.table_action
    SET inline_js = $3
    WHERE form_name = $1 and action = $2 $TAJSUP$,
'{form_name,action,inline_js}'),

('inline_js', 'list',
$TAJSGA$ SELECT form_name, action, inline_js::varchar(30) inline_js_
    FROM qz.table_action
    WHERE form_name = $1
    AND NOT is_callback
    ORDER BY action $TAJSGA$,
'{form_name}'),

('inline_css', 'edit',
$TAJSED$ SELECT form_name, action, inline_css
    FROM qz.table_action
    WHERE form_name = $1 and action = $2 $TAJSED$,
'{form_name,action}'),

('inline_css', 'update',
$TAJSUP$ UPDATE qz.table_action
    SET inline_css = $3
    WHERE form_name = $1 and action = $2 $TAJSUP$,
'{form_name,action,inline_css}'),

('inline_css', 'list',
$TAJSGA$ SELECT form_name, action, inline_css::varchar(30) inline_css_
    FROM qz.table_action
    WHERE form_name = $1
    AND NOT is_callback
    ORDER BY action $TAJSGA$,
'{form_name}');

UPDATE qz.table_action
SET helpful_text = $IJSGA$ Inline JavaScript is added to HTML head
in a <script> tag $IJSGA$
WHERE form_name = 'inline_js'
AND action = 'list';

UPDATE qz.table_action
SET helpful_text = $ICSSGA$ Inline CSS is added to HTML head
in a <style> tag $ICSSGA$
WHERE form_name = 'inline_css'
AND action = 'list';

---
--- user menus
---

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, helpful_text)
VALUES
('user_menus', 'list',
 $UMGA$ SELECT user_name, main_menu
  FROM qz.user
  ORDER BY user_name $UMGA$,
  '{}', 'User menus allows you to assign a main menu to a user
 to replace the default main menu.'),

('user_menus', 'edit',
 $UMED$ SELECT user_name, main_menu
 FROM qz.user
 WHERE user_name = $1 $UMED$,
 '{user_name}', NULL),

 ('user_menus', 'update',
  $UMUP$ UPDATE qz.user
  SET main_menu = $2
  WHERE user_name = $1 $UMUP$,
 '{user_name, main_menu}', NULL),

('user_menus', 'create',
  $UMCR$ SELECT ''::text user_name,
  ''::text main_menu $UMCR$,
  '{}', NULL),

('user_menus', 'insert',
 $UMIN$ INSERT INTO qz.user
 (user_name, main_menu)
 VALUES
 ($1, $2) $UMIN$,
 '{user_name, main_menu}', NULL),

('user_menus', 'delete',
 $UMDL$ DELETE FROM qz.user
 WHERE user_name = $1 $UMDL$,
 '{user_name}', NULL);

INSERT INTO qz.table_action
(form_name, action, sql, helpful_text)
VALUES
('menu_menu_page', 'view', 'SELECT 1',
    $MMPV$You can use All Menus to create or modify a menu
    or User Menus to assign a main menu to a user.$MMPV$);


INSERT INTO qz.table_action
(form_name, action, fieldnames, callback_attached_action, is_callback, callback_response, sql)
VALUES
('inline_doc', 'get_divids', '{form_name}', 'any', 't', 'qzforms_json',
$TACBIDS$
SELECT d.id "value", d.id || ' - ' || d.notation "text"
FROM qz.div_id d
WHERE d.template_name =
    (SELECT f.xml_template
     FROM qz.form f
     WHERE f.form_name = $1
     )
ORDER By d.id;
$TACBIDS$),


('inline_doc', 'get_actions', '{form_name}', 'any', 't', 'qzforms_json',
$TACBACT$
    SELECT action "value"
    FROM qz.table_action
    WHERE form_name = $1
    AND NOT is_callback
$TACBACT$);

---
--- docs
---

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

---
--- callbacks
---

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


---
--- Hide from the standard view
---

UPDATE qz.form
SET hidden = 't';

