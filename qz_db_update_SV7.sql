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

