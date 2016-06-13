
INSERT INTO qz.change_history 
  (change_description,note) 
  VALUES 
  ('Update to SV6',
   'Form Sets'
   );

UPDATE qz.constants 
SET schema_version = '6';

-- A new thing, a form set,
-- a list of names bound together
-- with a set name.

CREATE TABLE qz.form_set (
  set_name qz.variable_name PRIMARY KEY,
  context_parameters varchar(63)[]
);

INSERT INTO qz.form_set
(set_name, context_parameters)
VALUES
('form_mgt', '{form_name, handler_name}'),
('menu_mgt', '{menu_name}'),
('fixed_parameters', '{menu_name,menu_item_sequence}');

-- Each table action can elect to nab values from the 
-- current sql result set to be stored in a hash table by qzf. 
-- Can be used after a SELECT of course, but also 
-- INSERT...RETURNING... or UPDATE...RETURNING...

ALTER TABLE qz.table_action 
ADD COLUMN set_context_parameters boolean NOT NULL DEFAULT 'f';

-- A form can choose to be a member of a form set. 

ALTER TABLE qz.form
ADD COLUMN form_set_name qz.variable_name
REFERENCES qz.form_set(set_name);

COMMENT ON COLUMN qz.form.form_set_name IS 
'Typically left blank.  Used when data must be passed from one form to another.';

-- The form form edit needs to present the form set.

UPDATE qz.table_action
SET 
  sql = $FTAE$ 
  SELECT form_name, handler_name handler_name_ro, 
     schema_name, table_name, xml_template, target_div,
     add_description, prompt_container, form_set_name
  FROM qz.form
  WHERE form_name = $1
$FTAE$
WHERE form_name = 'form'
AND action = 'edit';

-- The form form update needs to set the form set.

UPDATE qz.table_action
SET
   sql = $FTAU$
   UPDATE qz.form SET 
     schema_name = $2,
     table_name = $3,
     xml_template = $4,
     target_div = $5,
     add_description = $6,
     prompt_container = $7,
     form_set_name = $8
   WHERE form_name = $1
$FTAU$,
fieldnames = '{form_name,schema_name,table_name,xml_template,
    target_div,add_description,prompt_container,form_set_name}'
WHERE form_name = 'form'
AND action = 'update';

-- The form sets for qzforms itself.

UPDATE qz.form
SET
  form_set_name = 'form_mgt'
WHERE form_name IN ( 'form', 'table_action_edit', 'prompt_rule', 
  'page_js', 'page_css');

-- The table action for editing and updating the table action
-- needs to set the context parameter flag 

UPDATE qz.table_action
SET
  sql = $TATAE$SELECT
    form_name, action action_ro, 
    helpful_text, sql,
    fieldnames, pkey,
    set_context_parameters
  FROM
    qz.table_action
  WHERE
    form_name = $1 AND action = $2
  $TATAE$
  WHERE form_name = 'table_action_edit'
  AND action = 'edit';

UPDATE qz.table_action
SET
  sql = $TATAU$
  UPDATE qz.table_action
  SET helpful_text=$3,
    sql=$4, fieldnames=$5, pkey=$6,
    set_context_parameters=$7
  WHERE form_name = $1 AND action = $2
$TATAU$,
fieldnames = '{form_name,action_ro,helpful_text,sql,fieldnames,pkey,set_context_parameters}'
WHERE form_name = 'table_action_edit'
AND action = 'update';

-- set context parameters gets a radio button.

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type)
VALUES ('table_action_edit', 'set_context_parameters', 'input_radio');

-- A new form for form_set

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name,
xml_template, target_div, add_description, 
prompt_container, form_set_name)
VALUES
('form_set', 'onetable', 'qz', 'form_set',
'base.xml', 'qz', 't', 
'fieldset', NULL);

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, pkey)
VALUES
('form_set', 'getall', 
'SELECT set_name FROM qz.form_set ORDER BY set_name', 
NULL, '{set_name}'),
('form_set', 'create', 
$FSC$SELECT ''::text set_name, ''::text context_parameters$FSC$,
NULL, NULL),
('form_set', 'insert',
$FSI$INSERT INTO qz.form_set (set_name,context_parameters) VALUES ($1,$2) $FSI$,
'{set_name,context_parameters}', '{set_name}'),
('form_set', 'edit',
$FSE$SELECT set_name, context_parameters 
     FROM qz.form_set
     WHERE set_name = $1$FSE$,
'{set_name}', '{set_name}'),
('form_set', 'update',
$FSU$UPDATE qz.form_set
     SET context_parameters = $2
     WHERE set_name = $1$FSU$,
'{set_name,context_parameters}', '{set_name}'),
('form_set', 'delete',
$FSD$DELETE FROM qz.form_set
     WHERE set_name = $1$FSD$,
'{set_name}', '{set_name}');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('form_dev', '50', 'form_set', 'getall', 'form_set');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'form_set', 'any'),
('form_dev', 'form_set', 'any');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('form_set', 1, 'qzforms.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('form_set', 1, 'qzforms.js');

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, regex_pattern)
VALUES
('form_set', 'context_parameters', 'text_array',
$FSPRCP$^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$$FSPRCP$),
('form_set', 'set_name', 'input_text',
$FSPRFS$^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$$FSPRFS$),
('form', 'form_set_name', 'select_fkey',
$FFSN$^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$$FFSN$) ;

-- Fix a screw up
-- remove _ro from handler_name

UPDATE qz.table_action
  SET pkey = '{form_name}'
  WHERE form_name = 'form'
  AND action = 'create';





