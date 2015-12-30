INSERT INTO qz.change_history 
  (change_description,note) 
  VALUES 
  ('Update to SV4',
   'Add regex_pattern to prompt_rule, 
    add err_msg class to qzforms.css' 
   );

UPDATE qz.constants 
SET schema_version = '4';

--
--  Unified javascript file qzforms.js, lose grid.js and prompt_rule.js
--
DELETE FROM qz.page_js
  WHERE filename = 'grid.js' 
  OR filename = 'prompt_rule.js';

DELETE FROM qz.js
  WHERE filename = 'grid.js'
  OR filename = 'prompt_rule.js';

-- WTF was I thinking????
--DELETE FROM qz.menu_set_edit
--  WHERE filename = 'grid.js' 
--  OR filename = 'prompt_rule.js';


--
-- Correct mistakes in SV3
-- css_edit filename fields in wrong place,
-- others change \]\] to \[\]
--
UPDATE qz.prompt_rule
SET
    regex_pattern = '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
    readonly = 'f'
WHERE     
    form_name = 'css_edit' AND fieldname = 'filename';

UPDATE qz.prompt_rule
SET 
    regex_pattern = '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE
    form_name = 'js_edit' AND fieldname = 'filename';

-- for filenames
UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
;

--  for variable names
UPDATE qz.prompt_rule 
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
;

--
-- Add spiffiness to form creation
--
UPDATE qz.prompt_rule
SET 
  readonly = 'f',
  prompt_type = 'select_options',
  options = '{fs,grid,menu,menupage,onetable}'
WHERE 
  form_name = 'form'
  AND fieldname = 'handler_name';

ALTER TABLE qz.form
ALTER COLUMN handler_name
SET NOT NULL;

UPDATE qz.table_action
SET sql = 
$FE$
    SELECT form_name, handler_name handler_name_ro, 
    schema_name, table_name, xml_template, target_div,
    add_description, prompt_container
    FROM qz.form
    WHERE form_name = $1
$FE$
WHERE
  form_name = 'form'
  AND action = 'edit';

UPDATE qz.table_action
SET sql = $FU$
 UPDATE qz.form SET 
 schema_name = $2,
 table_name = $3,
 xml_template = $4,
 target_div = $5,
 add_description = $6,
 prompt_container = $7
 WHERE form_name = $1
$FU$,
 fieldnames = '{form_name,schema_name,table_name,xml_template,target_div,
 add_description,prompt_container}'
WHERE form_name = 'form'
AND action = 'update';


UPDATE qz.prompt_rule
SET
  options = '{create,delete,delete_row,edit,etag_value,get,getall,
  insert,insert_row,save,update,update_row,view}',
  prompt_type = 'select_options'
WHERE
  form_name = 'table_action_edit'
  AND fieldname = 'action';



INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_item_edit', 'menu_item_sequence', NULL, false, '\d*', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

 
