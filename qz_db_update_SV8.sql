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
('inline_js', '3', 'qzforms.js'),
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

-- Fix
-- Set arrays of type variable name, which isn't
-- because pg won't do arrays of domains, to at
-- least check the length.

ALTER TABLE qz.table_action
ALTER COLUMN fieldnames
TYPE VARCHAR(63)[];

-- Fix
-- Don't need

DELETE FROM qz.menu
WHERE menu_name = 'not_a_menu';

DELETE FROM qz.table_action
WHERE form_name = 'form_menu_edit';

DELETE FROM qz.form
WHERE form_name = 'form_menu_edit';

DELETE FROM qz.page_css
WHERE form_name = 'form_menu_edit';

DELETE FROM qz.css
WHERE FILENAME = 'login_process.css';

-- Move pkey from table_action to form.

ALTER TABLE qz.form
ADD COLUMN pkey VARCHAR(63)[];

UPDATE qz.form fm
SET pkey =
  (SELECT pkey
   FROM qz.table_action ta
   WHERE ta.form_name = fm.form_name
   AND ACTION = 'getall')
WHERE fm.handler_name = 'onetable';

UPDATE qz.form fm
SET pkey =
  (SELECT pkey
   FROM qz.table_action ta
   WHERE ta.form_name = fm.form_name
   AND ta.action = 'edit')
WHERE fm.handler_name = 'grid';

UPDATE qz.form fm
SET pkey =
  (SELECT pkey
   FROM qz.table_action ta
   WHERE ta.form_name = fm.form_name
   AND ta.action = 'get')
WHERE fm.handler_name = 'fs';

UPDATE qz.form fm
SET pkey =  '{none}'
WHERE (handler_name) IN ('menupage', 'status', 'timestamp');

UPDATE qz.table_action
SET sql=$TAECR$ SELECT ta.form_name, ta.action new_action, fm.handler_name,
    ''::text helpful_text, ta.sql, ta.fieldnames,
    'f'::boolean clear_context_parameters
     FROM qz.create_table_action($1,$2) ta
    JOIN qz.form fm USING (form_name) $TAECR$
WHERE form_name = 'table_action_edit'
AND action = 'create';

UPDATE qz.table_action
SET sql=$TAEINS$ INSERT INTO qz.table_action
      (form_name, action, helpful_text, sql, fieldnames,
      clear_context_parameters)
      VALUES
      ($1,$2,$3,$4,$5,$6) $TAEINS$,
    fieldnames = '{form_name,new_action,helpful_text,sql,fieldnames,clear_context_parameters}'
WHERE form_name = 'table_action_edit'
AND action = 'insert';

UPDATE qz.table_action
SET sql=$TAEED$ SELECT
      form_name, action, helpful_text, sql, fieldnames, clear_context_parameters
    FROM
      qz.table_action
    WHERE
    form_name = $1 AND action = $2 $TAEED$
WHERE form_name = 'table_action_edit'
AND action = 'edit';

UPDATE qz.table_action
SET sql=$TAEUP$ UPDATE qz.table_action
    SET helpful_text=$3,
    sql=$4, fieldnames=$5, clear_context_parameters=$6
    WHERE form_name = $1 AND action = $2 $TAEUP$,
    fieldnames = '{form_name,action,helpful_text,sql,fieldnames,clear_context_parameters}'
WHERE form_name = 'table_action_edit'
AND action = 'update';


INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, size, maxlength)
VALUES
('form', 'pkey', 'text_array', 50, 63);

ALTER TABLE qz.table_action
DROP COLUMN pkey;

UPDATE qz.table_action
SET sql = $FRMCR$ SELECT
    $1::text form_name,
    $2::text handler_name,
    ''::text schema_name, ''::text table_name,
    ''::text pkey,
    'base.xml'::text xml_template,
    'qz'::text target_div,
    ''::text add_description,
    ''::text prompt_container $FRMCR$
WHERE form_name = 'form'
AND action = 'create';

UPDATE qz.table_action
SET sql = $FRMINS$ INSERT INTO qz.form
     (form_name, handler_name, schema_name, table_name, pkey,
     xml_template, target_div,
     add_description, prompt_container)
     VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) $FRMINS$,
     fieldnames = '{form_name,handler_name,schema_name,table_name,pkey,xml_template,target_div,add_description,prompt_container}'
WHERE form_name = 'form'
AND action = 'insert';

UPDATE qz.table_action
SET sql = $FRMED$  SELECT form_name, handler_name,
     schema_name, table_name, pkey, xml_template, target_div,
     add_description, prompt_container, form_set_name
     FROM qz.form
     WHERE form_name = $1 $FRMED$
WHERE form_name = 'form'
AND action = 'edit';

UPDATE qz.table_action
SET sql = $FRMUP$  UPDATE qz.form SET
     schema_name = $2,
     table_name = $3,
     pkey = $4,
     xml_template = $5,
     target_div = $6,
     add_description = $7,
     prompt_container = $8,
     form_set_name = $9
     WHERE form_name = $1 $FRMUP$,
     fieldnames = '{form_name,schema_name,table_name,pkey,xml_template,target_div,add_description,prompt_container,form_set_name}'
WHERE form_name = 'form'
AND action = 'update';

UPDATE qz.table_action
SET sql = $TAPRC$ SELECT
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
    ''::text "onkeyup" $TAPRC$,
    fieldnames = '{form_name, fieldname, handler_name}'
WHERE form_name = 'prompt_rule_edit'
AND action = 'create';

--- newest yet
--- Rework the menu heirchy.

INSERT INTO qz.form
(form_name, handler_name, xml_template, target_div, pkey)
VALUES
('menu_menu_page', 'menupage', 'base.xml', 'qz', '{none}');

INSERT INTO qz.table_action
(form_name, action, sql, helpful_text)
VALUES
('menu_menu_page', 'view', 'SELECT 1',
    $MMPV$You can use All Menus to create or modify a menu
    or User Menus to assign a main menu to a user.$MMPV$);

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('menu_menu_page', '1', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('menu_menu_page', '1', 'qzforms.css');

INSERT INTO qz.menu_set
(host_form_name, menu_name, action)
VALUES
('menu_menu_page', 'main', 'any'),
('menu_menu_page', 'form_dev', 'any'),
('menu_menu_page', 'menu_submenu', 'any');

UPDATE qz.prompt_rule
SET options = '{fs,grid,menupage,onetable}'
WHERE form_name = 'form'
AND (fieldname = 'handler_name' OR fieldname = 'new_handler_name');

UPDATE qz.menu_item
SET target_form_name = 'menu_menu_page',
    action = 'view',
    menu_text = 'Menu Menu'
WHERE menu_name = 'form_dev'
AND   menu_item_sequence = '10';
----
DELETE FROM qz.menu_item
WHERE menu_name = 'menu_submenu';

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action,
 menu_text, context_parameters)
VALUES
('menu_submenu', '10', 'user_menus', 'getall',
 'User Menus', NULL),
('menu_submenu', '20', 'menu_edit', 'getall',
 'All Menus', NULL);

DELETE FROM qz.page_css
WHERE filename = 'form_edit.css'
AND (form_name) IN ('user_menus', 'menu_edit', 'menu_item_edit',
'fixed_parameters');

DELETE FROM qz.menu_set
WHERE menu_name = 'user_menus'
AND   host_form_name = 'menu_edit'
AND   action = 'getall';

DELETE FROM qz.menu_set
WHERE menu_name = 'menu_submenu'
AND   host_form_name = 'menu_edit'
AND   action = 'edit';

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('menu_submenu', 'user_menus', 'any'),
('menu_submenu', 'menu_edit', 'any');

UPDATE qz.table_action
SET helpful_text =
'User menus allows you to assign a main menu to a user
 to replace the default main menu.'
 WHERE form_name = 'user_menus'
 AND   action = 'getall';


INSERT INTO qz.menu
(menu_name, target_div, description, form_set_name)
VALUES
('menu_items', 'pagemenu', 'Edit the list of items on a menu', 'menu_mgt');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text,
 context_parameters)
VALUES
('menu_items', '30', 'menu_item_edit', 'getall', 'Menu Items', '{menu_name}');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('menu_items', 'menu_edit', 'edit');

--- Rip out jquery.

DELETE FROM qz.page_js
WHERE (filename) IN ('jquery-2.1.4.js', 'jquery.tablesorter.js');

--
--  Add a small dash of spiffyness to menus
--

UPDATE qz.prompt_rule
SET size = '40', maxlength = '63'
WHERE form_name = 'menu_edit'
AND   (fieldname) IN ('menu_name', 'target_div');

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, size, maxlength)
VALUES
('menu_edit', 'description', 'input_text', '40', '63'),
('menu_edit', 'form_set_name', 'select_fkey', NULL, NULL);
