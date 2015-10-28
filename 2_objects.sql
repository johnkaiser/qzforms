CREATE TABLE qz.form(
     form_name qz.variable_name PRIMARY KEY,
     handler_name qz.variable_name REFERENCES qz.handler(handler_name),
     schema_name qz.variable_name,
     table_name qz.variable_name,
     xml_template qz.file_name,
     target_div qz.variable_name,
     hidden boolean default false, 
     add_description boolean,
     prompt_container qz.prompt_container_type
);



INSERT INTO qz.form(form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('table_action_edit', 'onetable', 'qz', 'table_action', 'base.xml', 'qz', true, 'fieldset');
INSERT INTO qz.form(form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('form', 'onetable', 'qz', 'form', 'base.xml', 'qz', true, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('menu_item_edit', 'onetable', 'qz', 'menu_item', 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('fixed_parameters', 'grid', 'qz', 'menu_item_parameter', 'base.xml', 'qz', false, 'no_container');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('prompt_rule_edit', 'onetable', 'qz', 'prompt_rule', 'base.xml', 'qz', true, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('menu_host_edit', 'grid', 'qz', 'menu_set', 'base.xml', 'qz', false, 'no_container');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('menu_edit', 'onetable', 'qz', 'menu', 'base.xml', 'qz', true, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('form_dev', 'menupage', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('pg_stat_activity', 'status', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('status', 'status', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('menu_set_edit', 'grid', 'qz', 'menu_set', 'base.xml', 'qz', false, 'no_container');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('menu', 'menupage', 'qz', 'menu', 'base.xml', 'qz', true, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('form_menu_edit', 'onetable', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('js', 'fs', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('js_edit', 'onetable', NULL, NULL, 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('page_js', 'grid', 'qz', 'page_js', 'base.xml', 'qz', false, 'no_container');

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('css', 'fs', 'qz', 'css', 'base.xml', 'qz', true, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('css_edit', 'onetable', 'qz', 'css', 'base.xml', 'qz', false, 'fieldset');
INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container) VALUES ('page_css', 'grid', 'qz', 'page_css', 'base.xml', 'qz', true, 'no_container');



