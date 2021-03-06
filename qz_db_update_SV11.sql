INSERT INTO qz.change_history
(change_description,note)
VALUES
('Update to SV11',
'Menus are in html lists, clearing context parameter is now by variable, and other minor fixes.'
);

 UPDATE qz.constants
 SET schema_version = '11';

-- Only form menu_menu_page should have menu_submenu

DELETE FROM qz.menu_set
WHERE menu_name = 'menu_submenu'
AND host_form_name != 'menu_menu_page';

-- On menu_item_edit, target_form_name should be a select options 
-- from the foreign key

-- But First,
-- If there are any menu items currently pointing nowhere,
-- they will break the foreign key, so add them.

INSERT INTO qz.form
(form_name, handler_name, hidden)
SELECT
mi.target_form_name "form_name",
'none' "handler_name",
't'::boolean "hidden"
FROM qz.menu_item mi
WHERE NOT EXISTS (
    SELECT 1 FROM qz.form f
    WHERE mi.target_form_name = f.form_name );

ALTER TABLE qz.menu_item 
ADD FOREIGN KEY (target_form_name) REFERENCES qz.form (form_name);

-- form name logout may already exist, dupe key error OK.
INSERT INTO qz.form
(form_name, handler_name, hidden, xml_template, target_div)
VALUES
('logout', 'logout', 't', 'login.xml', 'qz'),
('login', 'login', 't', 'login.xml', 'qz')
ON CONFLICT (form_name) DO NOTHING;

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

-- Set the new column to be all the possible values
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
AND (action) IN ('list', 'update');

-- modtime should update

UPDATE qz.table_action
SET sql =
$TACSSED$ UPDATE qz.css
    SET mimetype = $1,
    data = $2,
    modtime = now()
    WHERE filename = $3
$TACSSED$
WHERE (form_name) IN ('css_edit','js_edit')
AND action = 'update';

-- menus to lists

UPDATE qz.css
SET 
   modtime = '2018-01-06 13:49:00',
   data = $QZF$
div.menu {
    display: block;
    width: 100%;
    clear: left;
    padding: 0.5ex;
}
ul.menu {
    list-style-type: none;
    margin: 0;
    padding: 0;
    display: block;
}
li.menu {
   float: left;
   display: inline;
}

#helpful_text {
    display: block;
    clear: both;
    width: 40em;
}

.bold {font-weight:bold;}
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

.err_msg {
    background: yellow;
    border: 2pt solid red;
    padding: 3pt;
    display: table;
    font-weight: bold;
}
$QZF$
WHERE filename = 'qzforms.css';

COMMENT ON COLUMN qz.table_action.fieldnames IS
'The names of the fields passed to the SQL prepared statement.  The names come from the incoming form post data and may or may not match the names used in the table. The order must match the positional parameters in the prepared SQL statement.  An empty cell truncates the data and values after an empty field are lost.';

COMMENT ON COLUMN qz.table_action.clear_context_parameters IS
'Context parameters allow a form to pass attributes to other forms in the same form_set. Clear_context_parameters will remove the values after the form executes. For example, to clear one attribute in a multi part key.';

COMMENT ON COLUMN qz.prompt_rule.el_class IS
'An HTML Element class to be appended to any other classes assigned';

UPDATE qz.table_action
SET clear_context_parameters = '{form_name,handler_name}'
WHERE form_name = 'form'
AND action = 'list';

-- This will cause any connected clients to drop their cache of
-- Posgres schema information.
NOTIFY pg_db_change;
