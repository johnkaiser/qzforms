INSERT INTO qz.change_history
   (change_description,note)
   VALUES
   ('Update to SV8',
    'Inline js,css and user menus'
    );

 UPDATE qz.constants
 SET schema_version = '8';


--- Inline js and css
ALTER TABLE qz.table_action
ADD COLUMN inline_js text;

ALTER TABLE qz.table_action
ADD COLUMN inline_css text;

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
target_div, add_description, prompt_container, form_set_name)
VALUES
('inline_js', 'onetable', 'qz', 'form', 'base.xml',
'qz', 't', 'fieldset', 'form_mgt'),
('inline_css', 'onetable', 'qz', 'form', 'base.xml',
'qz', 't', 'fieldset', 'form_mgt');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, pkey)
VALUES
('inline_js', 'edit',
$TAJSED$ SELECT form_name, action, inline_js
    FROM qz.table_action
    WHERE form_name = $1 and action = $2 $TAJSED$,
'{form_name,action}', '{form_name,action}'),

('inline_js', 'update', 
$TAJSUP$ UPDATE qz.table_action
    SET inline_js = $3
    WHERE form_name = $1 and action = $2 $TAJSUP$,
'{form_name,action,inline_js}', '{form_name,action}'),

('inline_js', 'getall',
$TAJSGA$ SELECT form_name, action, inline_js::varchar(30) inline_js_
    FROM qz.table_action
    WHERE form_name = $1 $TAJSGA$,
'{form_name}', '{form_name,action}'),

('inline_css', 'edit',
$TAJSED$ SELECT form_name, action, inline_css
    FROM qz.table_action
    WHERE form_name = $1 and action = $2 $TAJSED$,
'{form_name,action}', '{form_name,action}'),

('inline_css', 'update', 
$TAJSUP$ UPDATE qz.table_action
    SET inline_css = $3
    WHERE form_name = $1 and action = $2 $TAJSUP$,
'{form_name,action,inline_css}', '{form_name,action}'),

('inline_css', 'getall',
$TAJSGA$ SELECT form_name, action, inline_css::varchar(30) inline_css_
    FROM qz.table_action
    WHERE form_name = $1 $TAJSGA$,
'{form_name}', '{form_name,action}');

UPDATE qz.table_action
SET helpful_text = $IJSGA$ Inline JavaScript is added to HTML head
in a <script> tag $IJSGA$
WHERE form_name = 'inline_js' 
AND action = 'getall';

UPDATE qz.table_action
SET helpful_text = $ICSSGA$ Inline CSS is added to HTML head
in a <style> tag $ICSSGA$
WHERE form_name = 'inline_css'
AND action = 'getall';

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, rows, cols, readonly)
VALUES
('inline_js', 'inline_js', 'textarea', 40, 80, 'f'),
('inline_js_', 'inline_js', 'textarea', 2, 30, 't'),
('inline_css', 'inline_css', 'textarea', 40, 80, 'f'),
('inline_css_', 'inline_js', 'textarea', 2, 30, 't');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, 
menu_text, context_parameters)
VALUES
('form_submenu', '70', 'inline_js', 'getall', 'inline_js', 
'{form_name, handler_name_ro}'),
('form_submenu', '80', 'inline_css', 'getall', 'inline_css', 
'{form_name, handler_name_ro}');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'inline_js', 'any'),
('main', 'inline_css', 'any'),
('form_submenu', 'inline_js','any'),
('form_submenu', 'inline_css', 'any'),
('form_dev', 'inline_js', 'any'),
('form_dev', 'inline_css', 'any');


INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('inline_js', '1', 'jquery-2.1.4.js'),
('inline_js', '2', 'jquery.tablesorter.js'),
('inline_js', '3', 'qzforms.js'),
('inline_css', '1', 'jquery-2.1.4.js'),
('inline_css', '2', 'jquery.tablesorter.js'),
('inline_css', '3', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('inline_js', '1', 'qzforms.css'),
('inline_css','1', 'qzforms.css'),
('inline_js', '2', 'form_edit.css'),
('inline_css','2', 'form_edit.css');


--- user menus

CREATE TABLE qz.user(
  user_name qz.variable_name PRIMARY KEY,
  main_menu qz.variable_name REFERENCES qz.menu(menu_name)
);

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
target_div, add_description, prompt_container)
VALUES
('user_menus', 'onetable', 'qz', 'user', 'base.xml',
'qz', 't', 'fieldset');

INSERT INTO qz.table_action
(form_name, action, sql, fieldnames, pkey)
VALUES
('user_menus', 'getall',
 $UMGA$ SELECT user_name, main_menu
  FROM qz.user
  ORDER BY user_name $UMGA$,
  '{}', '{user_name}'),

('user_menus', 'edit',
 $UMED$ SELECT user_name, main_menu
 FROM qz.user
 WHERE user_name = $1 $UMED$,
 '{user_name}', '{user_name}'),

 ('user_menus', 'update',
  $UMUP$ UPDATE qz.user
  SET main_menu = $2
  WHERE user_name = $1 $UMUP$,
 '{user_name, main_menu}', '{user_name}'),

('user_menus', 'create',
  $UMCR$ SELECT ''::text user_name,
  ''::text main_menu $UMCR$,
  '{}', '{user_name}'),

('user_menus', 'insert',
 $UMIN$ INSERT INTO qz.user
 (user_name, main_menu)
 VALUES
 ($1, $2) $UMIN$,
 '{user_name, main_menu}', '{user_name}'),

('user_menus', 'delete',
 $UMDL$ DELETE FROM qz.user
 WHERE user_name = $1 $UMDL$,
 '{user_name}', '{user_name}');

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, size, regex_pattern)
VALUES
('user_menus', 'user_name', 'input_text', '63',
 '^[^\x01-\x2f\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x60]{1,63}$'),
('user_menus', 'main_menu', 'select_fkey', '63', NULL);

INSERT INTO qz.menu
(menu_name, target_div, description)
VALUES
('user_menus', 'pagemenu', 'Assign menus to users');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('user_menus', '1', 'user_menus', 'getall', 'User Menus');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('user_menus', 'menu_edit', 'getall'),
('main', 'user_menus', 'any'),
('form_dev', 'user_menus', 'any');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('user_menus', '1', 'jquery-2.1.4.js'),
('user_menus', '2', 'jquery.tablesorter.js'),
('user_menus', '3', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('user_menus', '1', 'qzforms.css'),
('user_menus', '2', 'form_edit.css');

-- Fix
-- Remove menu_item_sequence from form set menu_mgt.
UPDATE qz.form_set
SET context_parameters = '{menu_name}'
WHERE set_name = 'menu_mgt';


