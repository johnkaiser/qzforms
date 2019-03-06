
--
--  Register Templates
--

CREATE TABLE qz.template (
    template_name qz.file_name PRIMARY KEY,
    notation varchar(50)
);

CREATE TABLE qz.div_id (
    template_name  qz.file_name REFERENCES qz.template(template_name),
    div_id qz.variable_name,
    notation varchar(50),
    PRIMARY KEY (template_name, div_id)
);

INSERT INTO qz.form_set
(set_name, context_parameters)
VALUES
('templates', '{template_name}');

--
-- Create a templates form 
--

INSERT INTO qz.form 
(form_name, handler_name, schema_name, table_name, xml_template, target_div,
 add_description, prompt_container, form_set_name, pkey)
 VALUES
 ('templates', 'onetable', 'qz', 'template', 'base.xml', 'qz',
  't', 'fieldset', 'templates', '{template_name}' );

--
-- Table Actions for templates
--

INSERT INTO qz.table_action
(form_name, action, fieldnames, clear_context_parameters, sql)
VALUES
('templates', 'list', NULL, '{template_name}',
$TTAL$
SELECT template_name, notation
FROM qz.template
ORDER BY template_name
$TTAL$),

('templates', 'edit', '{template_name}', NULL,
$TTAE$
SELECT template_name, notation
FROM qz.template
WHERE template_name = $1
$TTAE$),

('templates', 'update', '{template_name, notation}', '{template_name}',
$TTAU$
UPDATE qz.template
SET notation = $2
WHERE template_name = $1
$TTAU$),

('templates', 'create', NULL, NULL,
$TTAC$
SELECT ''::text template_name, ''::text notation
$TTAC$),

('templates', 'insert', '{template_name, notation}', NULL,
$TTAI$
INSERT INTO qz.template
(template_name, notation)
VALUES
($1, $2)
$TTAI$),

('templates', 'delete', '{template_name}','{template_name}',
$TTAD$
DELETE FROM qz.template 
WHERE template_name = $1
$TTAD$)
;

--
--  Create a grid subform for the div ids
--

INSERT INTO qz.form 
(form_name, handler_name, schema_name, table_name, xml_template, target_div,
 add_description, prompt_container, form_set_name, pkey)
 VALUES
 ('div_ids', 'grid', 'qz', 'div_id', 'base.xml', 'qz',
  't', 'no_container', 'templates', '{template_name, div_id}' );

INSERT INTO qz.table_action
(form_name, action, fieldnames, sql)
VALUES
('div_ids', 'edit', '{template_name}',
$IDAE$
SELECT template_name, div_id, notation
FROM qz.div_id
WHERE template_name = $1
ORDER BY template_name, div_id
$IDAE$),

('div_ids', 'save', NULL,
$IDAE$
SELECT 1
$IDAE$),

('div_ids', 'update_row', '{template_name, div_id, notation}',
$IDAU$
UPDATE qz.div_id
SET notation = $3
WHERE template_name = $1
AND div_id = $2
$IDAU$),

('div_ids', 'insert_row', '{template_name, div_id, notation}',
$IDAI$
INSERT INTO qz.div_id
(template_name, div_id, notation)
VALUES
($1, $2, $3)
$IDAI$),

('div_ids', 'delete_row', '{template_name, div_id, notation}',
$IDAD$
DELETE FROM qz.div_id
WHERE template_name = $1
AND div_id = $2
$IDAD$)
;

--
-- Prompt Rules
--

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, size, maxlength, regex_pattern)
VALUES
('templates', 'template_name', 'input_text', 63, 63,
 '^[^\x01-\x2c\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x2f\x60]{1,63}$'),
 ('templates', 'notation', 'input_text', 50, 50, NULL);

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, size, maxlength, regex_pattern,
    expand_percent_n, onchange)

VALUES
('div_ids', 'template_name', 'input_text', 63, 63,
 '^[^\x01-\x2c\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x2f\x60]{1,63}$',
 't', 'change_status(%n, "U")'),

 ('div_ids', 'div_id', 'input_text', 63, 63,
 '^[^\x01-\x2c\x3a-\x40\x5b-\x5e\x7b-\x7f\s\x2f\x60]{1,63}$',
 't', 'change_status(%n, "U")'),

 ('div_ids', 'notation', 'input_text', 50, 50, NULL,
 't', 'change_status(%n, "U")')
 ;

--
-- Page js,css
--

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('templates', '1', 'qzforms.css'),
('div_ids', '1', 'qzforms.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('templates', '1', 'qzforms.js'),
('div_ids', '1', 'qzforms.js');

--
-- templates menu
--

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'templates', 'any'),
('form_dev', 'templates', 'any'),
('main', 'div_ids', 'any'),
('form_dev', 'div_ids', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters)
VALUES
('form_dev', '60', 'templates', 'list', 'templates', '{template_name}');

--
--  templates submenu
--

INSERT INTO qz.menu
(menu_name, target_div, description, form_set_name)
VALUES
('templates_submenu', 'pagemenu', 'Templates and div ids', 'templates');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('templates_submenu', 'templates', 'any'),
('templates_submenu', 'div_ids', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters)
VALUES
('templates_submenu', '10', 'templates', 'list', 'templates', '{template_name}'),
('templates_submenu', '20', 'div_ids', 'edit', 'div_ids', '{template_name}');

--
-- And some data for the standard templates
--

INSERT INTO qz.template
(template_name, notation)
VALUES
('base.xml', 'Generic'),
('login.xml', 'login and logout only'),
('tinymce.xml', 'tinymce editor fron thier cdn');

INSERT INTO qz.div_id
(template_name, div_id, notation)
VALUES
('base.xml', 'qzmenu', 'main menu'),
('base.xml', 'qzsubmenu', 'under the main menu'),
('base.xml', 'helpful_text', 'Under the menu'),
('base.xml', 'pagemenu', 'form specific options'),
('base.xml', 'qz', 'main form'),

('login.xml', 'qz', 'login form'),

('tinymce.xml', 'qzmenu', 'main menu'),
('tinymce.xml', 'qzsubmenu', 'under the main menu'),
('tinymce.xml', 'helpful_text', 'Under the menu'),
('tinymce.xml', 'pagemenu', 'form specific options'),
('tinymce.xml', 'qz', 'main form')
;


UPDATE qz.table_action
SET helpful_text =
$TALC$An up and coming feature will be object placement into ids from a drop list$TALC$
WHERE form_name = 'templates'
AND action = 'list';


