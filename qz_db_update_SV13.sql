INSERT INTO qz.change_history
(change_description,note)
VALUES
('Update to SV13',
'HTML docs'
);

 UPDATE qz.constants
 SET schema_version = '13';

---
--- Convert template to drop down list
---
ALTER TABLE qz.form 
ADD FOREIGN KEY (xml_template)
REFERENCES qz.template(template_name);

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type)
VALUES
('form', 'xml_template', 'select_fkey');

UPDATE qz.form
SET hidden = 't'
WHERE (form_name) IN ('div_ids','templates');

---
--- Inline Documents
---
ALTER TABLE qz.doc 
ADD COLUMN form_name qz.variable_name REFERENCES qz.form(form_name);

ALTER TABLE qz.doc ADD COLUMN action qz.variable_name;
ALTER TABLE qz.doc ADD COLUMN div_id qz.variable_name;
ALTER TABLE qz.doc ADD COLUMN el_class text;

ALTER TABLE qz.doc DROP CONSTRAINT doc_pkey;
ALTER TABLE qz.doc DROP COLUMN mimetype;
ALTER TABLE qz.doc DROP COLUMN filename;

ALTER TABLE qz.doc ADD PRIMARY KEY (form_name, action, div_id);


--- DROP TABLE qz.doc;
--- 
--- CREATE TABLE qz.doc (
--- form_name qz.variable_name REFERENCES qz.form(form_name),
--- action qz.variable_name,
--- div_id qz.variable_name,
--- mimetype text,
--- "data" text,
--- PRIMARY KEY (form_name, action, div_id));

GRANT SELECT ON TABLE qz.doc TO qzuser;
GRANT ALL ON TABLE qz.doc TO qzdev;

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
target_div, hidden, prompt_container, form_set_name, pkey)
VALUES
('inline_doc', 'onetable', 'qz', 'doc', 'tinymce.xml',
'qz', 't', 'fieldset', 'form_mgt', '{form_name,action,div_id}');

--- Table Actions for inline docs

INSERT INTO qz.table_action
(form_name, action, fieldnames, helpful_text, sql)
VALUES

('inline_doc', 'list', '{form_name}', 
'Use this to attach a bit of html to your form',
$DTALI$
SELECT action, div_id
FROM qz.doc
WHERE form_name = $1
ORDER BY action, div_id
$DTALI$),

('inline_doc', 'edit', '{form_name,action,div_id}', NULL,
$DTAED$
SELECT action, div_id, el_class, "data"
FROM qz.doc
WHERE form_name = $1 
AND action = $2 
AND div_id = $3
$DTAED$),

('inline_doc', 'update', '{form_name,action,div_id,el_class,data}', NULL,
$DTAUP$
UPDATE qz.doc
SET
el_class = $4,
"data" = $5
WHERE form_name = $1
AND action = $2
AND div_id = $3
$DTAUP$),

('inline_doc', 'create', '{form_name}', NULL,
$DTACR$
SELECT $1::qz.variable_name "form_name",
''::text "action",
''::text "div_id",
''::text "el_class",
''::text "data"
$DTACR$),

('inline_doc', 'insert', 
'{form_name, action, div_id, el_class, data}',
NULL,
$DTAIN$
INSERT INTO qz.doc
(form_name, action, div_id, el_class, data)
VALUES
($1,$2,$3,$4,$5)
$DTAIN$),

('inline_doc', 'delete', '{form_name,action,div_id}', NULL,
$DTADL$
DELETE FROM qz.doc
WHERE 
form_name = $1
AND action = $2
AND div_id = $3
$DTADL$);

--- Menu stuff for inline_docs
 
INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'inline_doc', 'any'),
('form_dev', 'inline_doc', 'any'),
('form_submenu', 'inline_doc', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('form_submenu', '90', 'inline_doc', 'list', 'inline doc');

--- page js/css for inline docs

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('inline_doc', '1', 'qzforms.css'),
('inline_doc', '2', 'form_edit.css');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('inline_doc', '1', 'qzforms.js');

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type, el_class, rows, cols) 
VALUES
('inline_doc', 'data', 'textarea', 'tinymce', '40', '40');


