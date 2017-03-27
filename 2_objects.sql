CREATE TABLE qz.form(
     form_name qz.variable_name PRIMARY KEY,
     handler_name qz.variable_name REFERENCES qz.handler(handler_name) NOT NULL,
     schema_name qz.variable_name,
     table_name qz.variable_name,
     xml_template qz.file_name,
     target_div qz.variable_name,
     hidden boolean default false, 
     add_description boolean,
     prompt_container qz.prompt_container_type,
     form_set_name qz.variable_name,
     pkey varchar(63)[]
);

CREATE TABLE qz.form_set (
  set_name qz.variable_name PRIMARY KEY,
  context_parameters varchar(63)[]
);

INSERT INTO qz.form_set
(set_name, context_parameters)
VALUES
('form_mgt', '{form_name, handler_name}'),
('menu_mgt', '{menu_name, menu_item_sequence}');

INSERT INTO qz.form(
   form_name, handler_name, schema_name, table_name, xml_template, target_div,
   add_description, prompt_container, pkey) 
VALUES 
('table_action_edit', 'onetable', 'qz', 'table_action', 'base.xml', 'qz', 
true, 'fieldset', '{form_name,action}'),

('form', 'onetable', 'qz', 'form', 'base.xml', 'qz', 
true, 'fieldset', '{form_name}'),

('menu_item_edit', 'onetable', 'qz', 'menu_item', 'base.xml', 'qz', 
false, 'fieldset', '{menu_name, menu_item_sequence}'),

('fixed_parameters', 'grid', 'qz', 'fixed_parameter', 'base.xml', 'qz', 
false, 'no_container', '{menu_name, menu_item_sequence}'),

('prompt_rule_edit', 'onetable', 'qz', 'prompt_rule', 'base.xml', 'qz',
true, 'fieldset', '{form_name, fieldname}'),

('menu_host_edit', 'grid', 'qz', 'menu_set', 'base.xml', 'qz', 
false, 'no_container', '{menu_name, host_form_name, action}'),

('menu_edit', 'onetable', 'qz', 'menu', 'base.xml', 'qz', 
true, 'fieldset', '{menu_name}'),

('form_dev', 'menupage', NULL, NULL, 'base.xml', 'qz', 
false, 'fieldset', '{none}'),

('pg_stat_activity', 'status', NULL, NULL, 'base.xml', 'qz', 
false, 'fieldset', '{none}'),

('status', 'status', NULL, NULL, 'base.xml', 'qz',
false, 'fieldset','{none}'),

('menu_set_edit', 'grid', 'qz', 'menu_set', 'base.xml', 'qz',
false, 'no_container', '{menu_name, host_form_name, action}'),

('menu', 'menupage', 'qz', 'menu', 'base.xml', 'qz',
true, 'fieldset', '{none}'),

('js', 'fs', 'qz', 'js', 'base.xml', 'qz',
false, 'fieldset', '{filename}'),

('js_edit', 'onetable', NULL, NULL, 'base.xml', 'qz',
false, 'fieldset', '{filename}'),

('page_js', 'grid', 'qz', 'page_js', 'base.xml', 'qz',
false, 'no_container', '{form_name, sequence}'),

('css', 'fs', 'qz', 'css', 'base.xml', 'qz',
true, 'fieldset', '{filename}'),

('css_edit', 'onetable', 'qz', 'css', 'base.xml', 'qz',
false, 'fieldset', '{filename}'),

('page_css', 'grid', 'qz', 'page_css', 'base.xml', 'qz',
true, 'no_container', '{form_name, sequence}'),

('form_set', 'onetable', 'qz', 'form_set', 'base.xml', 'qz',
true, 'fieldset', '{set_name}'),

('page_menus', 'grid', 'qz', 'menu_set', 'base.xml', 'qz',
true, 'no_container', '{menu_name, host_form_name, action}' ),

('inline_js', 'onetable', 'qz', 'form', 'base.xml', 'qz', 
true, 'fieldset', '{form_name}'),

('inline_css', 'onetable', 'qz', 'form', 'base.xml', 'qz',
true, 'fieldset', '{form_name}');

--- 

UPDATE qz.form
SET form_set_name = 'form_mgt'
WHERE (form_name) IN ( 'form', 'table_action_edit', 'prompt_rule_edit', 
  'page_js', 'page_css', 'page_menus', 'inline_css', 'inline_js');

UPDATE qz.form
SET form_set_name = 'menu_mgt'
WHERE (form_name) IN ('menu_edit', 'menu_item_edit', 'menu_set_edit', 'fixed_parameters' );

---
---  user menus
---
INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
target_div, add_description, prompt_container, pkey)
VALUES
('user_menus', 'onetable', 'qz', 'user', 'base.xml',
'qz', 't', 'fieldset', '{user_name}');

INSERT INTO qz.form
(form_name, handler_name, xml_template, target_div, pkey)
VALUES
('menu_menu_page', 'menupage', 'base.xml', 'qz', '{none}');



