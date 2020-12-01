

--
-- Data for Name: page_css; Type: TABLE DATA; Schema: qz; Owner: qz
--

INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('form', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('fixed_parameters', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('form_dev', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu_host_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu_item_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu_set_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('prompt_rule_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('table_action_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('status', 10, 'qzforms.css');

INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('page_css', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('page_js', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('css_edit', 10, 'qzforms.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('js_edit', 10, 'qzforms.css');

INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('form', 20, 'form_edit.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('table_action_edit', 20, 'form_edit.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('prompt_rule_edit', 20, 'form_edit.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu_host_edit', 20, 'form_edit.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('page_css', 20, 'form_edit.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('page_js', 20, 'form_edit.css');
INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('menu_set_edit', 20, 'form_edit.css');

INSERT INTO qz.page_css (form_name, sequence, filename) VALUES ('form_set', 1, 'qzforms.css');



--
-- Data for Name: page_js; Type: TABLE DATA; Schema: qz; Owner: qz
--

INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('fixed_parameters', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('menu_host_edit', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('table_action_edit', 30, 'qzforms.js');

INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('js_edit', 30, 'qzforms.js');

INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('css_edit', 3, 'qzforms.js');

INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('page_js', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('page_css', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('menu_item_edit', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('menu_set_edit', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('menu_edit', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('form', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('prompt_rule_edit', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('form_dev', 3, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('menu', 30, 'qzforms.js');
INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('status', 30, 'qzforms.js');

INSERT INTO qz.page_js (form_name, sequence, filename) VALUES ('form_set', 1, 'qzforms.js');

---
---  inline js and css
---

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

---
---  user menus
---

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('user_menus', '3', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('user_menus', '1', 'qzforms.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('menu_menu_page', '1', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('menu_menu_page', '1', 'qzforms.css');

---
--- doc
---

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('inline_doc', '1', 'qzforms.css'),
('inline_doc', '2', 'form_edit.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('inline_doc', '1', 'qzforms.js');

---
--- callbacks
---

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('callback', '1', 'qzforms.css'),
('callback', '2', 'form_edit.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('callback', '1', 'qzforms.js');


