INSERT INTO qz.change_history
(change_description,note)
VALUES
('Update to SV11',
'Clearing context parameter is now a list intead of a boolean and other menu fixes.'
);

 UPDATE qz.constants
 SET schema_version = '11';

-- Only form menu_menu_page should have menu_submenu

DELETE FROM qz.menu_set
WHERE menu_name = 'menu_submenu'
AND host_form_name != 'menu_menu_page';

-- On menu_item_edit, target_form_name should be a select options 
-- from the foreign key

INSERT INTO qz.form
(form_name, handler_name, hidden)
VALUES
('logout', 'logout', 't');

ALTER TABLE qz.menu_item 
ADD FOREIGN KEY (target_form_name) REFERENCES qz.form (form_name);

-- On menu_item_edit, action should be a select options
-- from a list

UPDATE qz.prompt_rule
SET prompt_type = 'select_options',
options =  '{list,create,insert,edit,update,delete,save,view}'
WHERE form_name = 'menu_item_edit'
AND fieldname = 'action';

--
-- Turn clear context parameters into an array or list
--

-- Rename the old data

ALTER TABLE qz.table_action
RENAME COLUMN clear_context_parameters 
TO old_clear_context_parameters;

-- Add the new column

ALTER TABLE qz.table_action
ADD COLUMN clear_context_parameters varchar(63)[];

-- Set the new column to be all the posible values
-- if the  old boolean was true.

UPDATE qz.table_action ta
SET clear_context_parameters = 
(SELECT fs.context_parameters
 FROM qz.form_set fs, qz.form f
 WHERE ta.form_name = f.form_name
 AND f.form_set_name = fs.set_name
 )
WHERE ta.old_clear_context_parameters;

-- Remove the old column

ALTER TABLE qz.table_action
DROP COLUMN old_clear_context_parameters;

-- Update the rule

UPDATE qz.prompt_rule
SET options = NULL,
prompt_type = 'text_array'
WHERE form_name = 'table_action_edit'
AND fieldname = 'clear_context_parameters';

-- All that, for this.
-- Drop the value after using it so it does not persist
-- where it is not wanted

UPDATE qz.table_action
SET clear_context_parameters = '{menu_item_sequence}'
WHERE form_name = 'menu_item_edit'
AND (action) IN ('insert','update','delete');

-- Do this the better way
UPDATE qz.prompt_rule
SET onfocus = NULL
WHERE form_name  = 'table_action_edit'
AND fieldname = 'action';

UPDATE qz.table_action
SET inline_js = $TAIJS$
document.addEventListener("DOMContentLoaded",set_action_options, true);
$TAIJS$
WHERE form_name = 'table_action_edit'
AND action = 'list';


