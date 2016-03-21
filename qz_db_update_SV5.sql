INSERT INTO qz.change_history 
  (change_description,note) 
  VALUES 
  ('Update to SV5',
   'User interface cleanup'
   );

UPDATE qz.constants 
SET schema_version = '4';


SELECT SETVAL('qz.menu_set_set_id_seq', COALESCE(MAX(set_id), 1) ) FROM qz.menu_set;

-- Get rid of _Edit in the buttons
UPDATE qz.menu_item
SET menu_text = 'Table_Action'
WHERE menu_text = 'Table_Action_Edit';

UPDATE qz.menu_item
SET menu_text = 'Menu_Item'
WHERE menu_text = 'Menu_Item_Edit';

UPDATE qz.menu_item
SET menu_text = 'Menu_Set'
WHERE menu_text = 'Menu_Set_Edit';


-- Add handler_name_ro to context params for form dev
UPDATE qz.menu_item
SET context_parameters = '{form_name,handler_name_ro}'
WHERE menu_name = 'form_submenu';

-- Make handler_name_ro a fake pkey and include it in selects
UPDATE qz.table_action
SET 
  sql=$TAC$SELECT ta.form_name, ta.action, fm.handler_name handler_name_ro,
    ''::text helpful_text, ta.sql, ta.fieldnames, ta.pkey 
     FROM qz.create_table_action($1,$2) ta
    JOIN qz.form fm USING (form_name)$TAC$,
  pkey = '{form_name, handler_name_ro, action}'
WHERE form_name = 'table_action_edit'
AND action = 'create';

UPDATE qz.table_action
SET 
  sql=$TAG$SELECT ta.action, ta.helpful_text,
    fm.handler_name 
    FROM qz.table_action ta
    JOIN qz.form fm USING (form_name)
    WHERE form_name = $1
    ORDER BY form_name, action$TAG$,
  pkey = '{form_name, handler_name_ro, action}'  
WHERE form_name = 'table_action_edit'
AND action = 'getall';


UPDATE qz.table_action 
SET pkey = '{form_name, handler_name_ro}'
WHERE form_name = 'form'
AND action IN ('edit', 'update', 'delete');

UPDATE qz.table_action
SET sql=$TAFG$SELECT form_name, handler_name handler_name_ro
     FROM qz.form
     ORDER BY form_name, handler_name$TAFG$,
     pkey='{form_name, handler_name_ro}'
WHERE form_name = 'form'
AND action = 'getall';


-- Creating a new form should preset the whole record
-- instead of inserting just the key and forcing an edit.
UPDATE qz.table_action
SET sql=$TAFC$SELECT  $1::text form_name, 
    ''::text handler_name, 
    ''::text schema_name, ''::text table_name, 
    'base.xml'::text xml_template, 
    'qz'::text target_div,
     ''::text add_description, 
     ''::text prompt_container$TAFC$
WHERE form_name = 'form'
AND action = 'create';

UPDATE qz.table_action
SET sql=$TAFI$INSERT INTO qz.form
    (form_name, handler_name, schema_name, table_name,
    xml_template, target_div, 
    add_description, prompt_container)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8)$TAFI$,
    fieldnames = '{form_name,handler_name,schema_name,table_name,xml_template,target_div,add_description,prompt_container}'
WHERE form_name = 'form'
AND action = 'insert';



UPDATE qz.table_action
SET fieldnames = '{form_name, action_ro}',
    pkey = '{form_name, action_ro}'
WHERE form_name = 'table_action_edit'
AND action = 'delete';

UPDATE qz.table_action
SET
  sql=$tae$SELECT
      ta.form_name, ta.action action_ro, 
      fm.handler_name handler_name_ro,
      ta.helpful_text, ta.sql,
      ta.fieldnames, ta.pkey 
    FROM
      qz.table_action ta
    JOIN
      qz.form fm USING (form_name)
    WHERE
      ta.form_name = $1 AND ta.action = $2 $tae$
WHERE 
  form_name = 'table_action_edit' AND action = 'edit';

UPDATE qz.prompt_rule
SET onfocus = 'set_action_options()'
WHERE form_name = 'table_action_edit'
AND fieldname = 'action';

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, expand_percent_n, onchange)
VALUES
('menu_set_edit', 'set_id', 'input_hidden', 't', 'change_status(%n, "U")');

UPDATE qz.table_action
SET sql=$PRCTA$SELECT
    $1::text "form_name", 
    $2::text "fieldname",
    'input_text'::qz.prompt_types "prompt_type",
    ''::text "tabindex",
    ''::text "el_class", 
    'f'::text "readonly", 
    ''::text "regex_pattern", 
    ''::text "rows", 
    ''::text "cols", 
    ''::text "size", 
    ''::text "maxlength", 
    ''::text "options", 
    ''::text "publish_pgtype", 
    ''::text "expand_percent_n",
    ''::text "onfocus", 
    ''::text "onblur", 
    ''::text "onchange", 
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
    ''::text "onkeyup"$PRCTA$
WHERE form_name = 'prompt_rule_edit'
AND action = 'create';

UPDATE qz.table_action
SET sql=$PRITA$INSERT INTO qz.prompt_rule
  ("form_name", "fieldname", "prompt_type", "tabindex",
  "el_class", "readonly", "regex_pattern",
  "rows", "cols", "size",
  "maxlength", "options", "publish_pgtype", 
 "expand_percent_n", onfocus, onblur, onchange, onselect,
 onclick, ondblclick, onmousedown, onmouseup, 
 onmouseover, onmousemove, onmouseout,
 onkeypress, onkeydown, onkeyup )
  VALUES
  ($1,$2,$3,$4,$5,$6,$7,$8,$9,
  $10,$11,$12,$13,$14,$15,$16,$17,$18,
  $19,$20,$21,$22,$23,$24,$25,$26,$27,$28)$PRITA$,
fieldnames='{form_name,fieldname,prompt_type,tabindex,el_class,readonly,regex_pattern,rows,cols,size,maxlength,options,publish_pgtype,expand_percent_n,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup}'
WHERE form_name = 'prompt_rule_edit'
AND action = 'insert';

