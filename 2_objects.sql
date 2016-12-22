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
     form_set_name qz.variable_name
);

CREATE TABLE qz.form_set (
  set_name qz.variable_name PRIMARY KEY,
  context_parameters varchar(63)[]
);

INSERT INTO qz.form(form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('table_action_edit', 'onetable', 'qz', 'table_action', 'base.xml', 'qz', true, 'fieldset');

INSERT INTO qz.form(form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('form', 'onetable', 'qz', 'form', 'base.xml', 'qz', true, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container, form_set_name) 
VALUES ('menu_item_edit', 'onetable', 'qz', 'menu_item', 'base.xml', 'qz', false, 'fieldset', 'menu_mgt');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('fixed_parameters', 'grid', 'qz', 'menu_item_parameter', 'base.xml', 'qz', false, 'no_container');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container, form_set_name) 
VALUES ('prompt_rule_edit', 'onetable', 'qz', 'prompt_rule', 'base.xml', 'qz', true, 'fieldset', 'form_mgt');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('menu_host_edit', 'grid', 'qz', 'menu_set', 'base.xml', 'qz', false, 'no_container');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container, form_set_name) 
VALUES ('menu_edit', 'onetable', 'qz', 'menu', 'base.xml', 'qz', true, 'fieldset', 'menu_mgt');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('form_dev', 'menupage', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('pg_stat_activity', 'status', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('status', 'status', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container, form_set_name) 
VALUES ('menu_set_edit', 'grid', 'qz', 'menu_set', 'base.xml', 'qz', false, 'no_container', 'menu_mgt');


INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('menu', 'menupage', 'qz', 'menu', 'base.xml', 'qz', true, 'fieldset');


INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('form_menu_edit', 'onetable', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');


INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('js', 'fs', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('js_edit', 'onetable', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('page_js', 'grid', 'qz', 'page_js', 'base.xml', 'qz', false, 'no_container');


INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('css', 'fs', 'qz', 'css', 'base.xml', 'qz', true, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('css_edit', 'onetable', 'qz', 'css', 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) 
VALUES ('page_css', 'grid', 'qz', 'page_css', 'base.xml', 'qz', true, 'no_container');


INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name,
xml_template, target_div, add_description, 
prompt_container, form_set_name)
VALUES
('form_set', 'onetable', 'qz', 'form_set',
'base.xml', 'qz', 't', 
'fieldset', NULL);

INSERT INTO qz.form_set
(set_name, context_parameters)
VALUES
('form_mgt', '{form_name, handler_name}'),
('menu_mgt', '{menu_name, menu_item_sequence}'),
('fixed_parameters', '{menu_name,menu_item_sequence}');

UPDATE qz.form
SET
  form_set_name = 'form_mgt'
WHERE form_name IN ( 'form', 'table_action_edit', 'prompt_rule', 
  'page_js', 'page_css');

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
   target_div, add_description, prompt_container, form_set_name)
VALUES
('page_menus', 'grid', 'qz', 'menu_set', 'base.xml',
   'qz', 't', 'no_container', 'form_mgt');

