CREATE TABLE qz.menu(
    menu_name qz.variable_name PRIMARY KEY,
    target_div qz.variable_name,
    description text,
    form_set_name qz.variable_name
);
ALTER TABLE qz.menu ADD FOREIGN KEY (form_set_name) REFERENCES qz.form_set(set_name);


CREATE TABLE qz.menu_item (
    menu_name qz.variable_name REFERENCES qz.menu(menu_name),
    menu_item_sequence integer,
    target_form_name qz.variable_name,
    action qz.variable_name,
    menu_text text,
    context_parameters text[],
    PRIMARY KEY (menu_name, menu_item_sequence)
);

CREATE TABLE qz.menu_set (
    menu_name qz.variable_name REFERENCES qz.menu(menu_name),
    host_form_name qz.variable_name references qz.form(form_name),
	action qz.variable_name DEFAULT 'default',
    set_id SERIAL UNIQUE NOT NULL,
	PRIMARY KEY (menu_name, host_form_name, action)
);


CREATE TABLE qz.fixed_parameter(
    menu_name qz.variable_name,
	menu_item_sequence integer,
	parameter_key qz.variable_name,
	parameter_value text,
	PRIMARY KEY (menu_name, menu_item_sequence, parameter_key),
	FOREIGN KEY (menu_name, menu_item_sequence) 
        REFERENCES qz.menu_item(menu_name, menu_item_sequence)
);

CREATE TABLE qz.user(
  user_name qz.variable_name PRIMARY KEY,
  main_menu qz.variable_name REFERENCES qz.menu(menu_name)
);


--
-- Data for Name: menu; Type: TABLE DATA; 
--

INSERT INTO qz.menu (menu_name, target_div, description) 
VALUES ('main', 'qzmenu', 'main menu');

INSERT INTO qz.menu (menu_name, target_div, description) 
VALUES ('form_dev', 'qzsubmenu', 'Form Development');

INSERT INTO qz.menu (menu_name, target_div, description) 
VALUES ('table_action_edit', 'pagemenu', 'Table actions for this form_name');

INSERT INTO qz.menu (menu_name, target_div, description) 
VALUES ('fixed_parameters', 'pagemenu', 'Set specific values');

INSERT INTO qz.menu (menu_name, target_div, description, form_set_name) 
VALUES ('menu_submenu', 'pagemenu', 'Build menu details', 'menu_mgt');

INSERT INTO qz.menu (menu_name, target_div, description, form_set_name) 
VALUES ('form_submenu', 'pagemenu', 'Edit form particulars', 'form_mgt');


--
-- Data for Name: menu_item; Type: TABLE DATA; 
--

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('fixed_parameters', 10, 'fixed_parameters', 'edit', 'Fixed_Parameters', '{menu_name,menu_item_sequence}');

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_dev', 10, 'menu_menu_page', 'view', 'Menu Menu', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_dev', 20, 'form', 'getall', 'Forms', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_dev', 30, 'js_edit', 'getall', 'js files', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_dev', 40, 'css_edit', 'getall', 'css files', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_dev', 700, 'status', NULL, 'Status', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('main', 0, 'menu', NULL, 'Main_Menu', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('main', 888, 'form_dev', NULL, 'Form Development', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('main', 999, 'logout', NULL, 'Logout', NULL);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_submenu', 1, 'form', 'edit', 'form', '{form_name, handler_name_ro}');

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_submenu', 10, 'table_action_edit', 'getall', 'Table_Action', '{form_name, handler_name_ro}');

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_submenu', 20, 'prompt_rule_edit', 'getall', 'Prompt_Rules', '{form_name, handler_name_ro}');

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_submenu', 40, 'page_js', 'edit', 'page_js', '{form_name, handler_name_ro}');

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('form_submenu', 50, 'page_css', 'edit', 'page_css', '{form_name, handler_name_ro}');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name,
  action, menu_text, context_parameters)
VALUES
('form_submenu', '60', 'page_menus',
  'edit', 'page_menus', '{form_name, handler_name_ro}');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action,
 menu_text, context_parameters)
VALUES
('menu_submenu', '10', 'user_menus', 'getall',
 'User Menus', NULL),
('menu_submenu', '20', 'menu_edit', 'getall',
 'All Menus', NULL);


--
-- Data for Name: menu_set; Type: TABLE DATA; Schema: qz; Owner: qz
--

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'menu', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'form', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'js_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'status', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'menu_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'menu_item_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'menu_set_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'form_dev', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'table_action_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'css_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'fixed_parameters', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'prompt_rule_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'menu_host_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'page_js', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'page_css', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'form', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'fixed_parameters', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'menu_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'menu_host_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'menu_item_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'menu_set_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'table_action_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'form_dev', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'css_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'prompt_rule_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'js_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'status', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'page_js', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_dev', 'page_css', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('fixed_parameters', 'menu_item_edit', 'edit');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('menu_submenu', 'menu_set_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('menu_submenu', 'menu_item_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('menu_submenu', 'fixed_parameters', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_submenu', 'form', 'edit');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_submenu', 'prompt_rule_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_submenu', 'menu_host_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_submenu', 'table_action_edit', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_submenu', 'page_js', 'any');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('form_submenu', 'page_css', 'any');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'page_menus', 'any'),
('form_submenu', 'page_menus', 'any'),
('form_dev', 'page_menus', 'any');

--
-- form_set
--
INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('form_dev', '50', 'form_set', 'getall', 'form_set');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'form_set', 'any'),
('form_dev', 'form_set', 'any');

-- Add a foreign key so select_fkey works.
ALTER TABLE qz.form ADD FOREIGN KEY (form_set_name) REFERENCES qz.form_set(set_name);

---
--- inline js and css
---

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

---
---  user menus
---

--- INSERT INTO qz.menu
--- (menu_name, target_div, description)
--- VALUES
--- ('user_menus', 'pagemenu', 'Assign menus to users');

--- INSERT INTO qz.menu_item
--- (menu_name, menu_item_sequence, target_form_name, action, menu_text)
--- VALUES
--- ('user_menus', '1', 'user_menus', 'getall', 'User Menus');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'user_menus', 'any'),
('form_dev', 'user_menus', 'any');


INSERT INTO qz.menu_set
(host_form_name, menu_name, action)
VALUES
('menu_menu_page', 'main', 'any'),
('menu_menu_page', 'form_dev', 'any'),
('menu_menu_page', 'menu_submenu', 'any');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('menu_submenu', 'user_menus', 'any'),
('menu_submenu', 'menu_edit', 'any');

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




