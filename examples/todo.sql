CREATE TYPE vtodo_class
AS ENUM ('public','private','confidential');

CREATE DOMAIN vtodo_percent AS int
CHECK (
     VALUE >= 0 AND VALUE <= 100
);

CREATE DOMAIN vtodo_priority AS int
CHECK (
    VALUE >= 0 AND VALUE <= 5
);

CREATE TYPE vtodo_status
AS ENUM ('needs_action','completed','in_process','cancelled','on_hold');

CREATE TABLE vtodo (
    uid serial primary key,
    created timestamp without time zone,
    last_mod timestamp without time zone,
    description text,
    class vtodo_class,
    location varchar(30),
    percent vtodo_percent,
    priority vtodo_priority,
    seq int,
    status vtodo_status,
    summary text,
    url varchar(255)
);

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
 target_div, add_description, prompt_container, pkey
)
VALUES
('todo', 'onetable', 'public', 'vtodo', 'base.xml',
 'qz', 't', 'fieldset', '{uid}'
);

INSERT INTO qz.table_action
(form_name, action, fieldnames, sql)
VALUES
('todo', 'list', NULL,
$TODOLI$
SELECT uid, priority, summary, status
FROM vtodo
WHERE (status) IN ('needs_action', 'in_process', 'on_hold')
ORDER BY priority, uid
$TODOLI$
),

('todo', 'create', NULL, 
$TODOCR$
    SELECT nextval('vtodo_uid_seq'::regclass) "uid",
    ''::text "summary", 
    ''::text "description",
    'needs_action'::text "status",
    ''::text "class",
    ''::text "location",
    ''::text "priority",
    ''::text "url"
$TODOCR$
),
('todo', 'insert', 
'{uid, summary, description, status, class, location, priority, url}', 
$TODOINS$
    INSERT INTO vtodo
    ("uid", "summary", "description", "status", "class",
    "location", "priority", "url", 
    "created", "last_mod", "seq")
    VALUES
    ($1, $2, $3, $4, $5, $6, $7, $8,
     now(), now(), 1)
$TODOINS$
),
('todo', 'edit', '{uid}', 
$TODOED$
SELECT "uid", "created", "last_mod", 
"summary", "description", "class", "location",
"percent", "priority", "seq", "status", "url"
FROM vtodo
WHERE uid = $1
$TODOED$
),
('todo', 'update', 
'{uid, summary, description, class, location, percent, priority, status}',
$TODOUPD$
UPDATE vtodo
SET
  "summary" = $2, 
  "description" = $3,
  "class" = $4,
  "location" = $5,
  "percent" = $6,
  "priority" = $7,
  "status" = $8,
  "last_mod" = now(),
  "seq" = "seq" + 1
WHERE uid = $1
$TODOUPD$
);

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type,readonly,size,rows,cols)
VALUES
('todo', 'class', 'select_options', 'f',NULL,NULL,NULL),
('todo', 'created', 'input_text', 't','30',NULL,NULL),
('todo', 'description', 'textarea', 'f',NULL,'8','40'),
('todo', 'last_mod', 'input_text', 't','30',NULL,NULL),
('todo', 'priority', 'input_radio', 'f', NULL,NULL,NULL),
('todo', 'seq', 'input_text', 't','30',NULL,NULL),
('todo', 'status', 'select_options', 'f',NULL,NULL,NULL),
('todo', 'summary', 'input_text', 't','40',NULL,NULL),
('todo', 'uid', 'input_hidden', 't','5',NULL,NULL),
('todo', 'url', 'input_text', 't','40',NULL,NULL);


UPDATE qz.prompt_rule
SET options = '{1,2,3,4,5}'
WHERE form_name = 'todo' AND fieldname = 'priority';

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('todo', '1', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('todo', '1', 'qzforms.css');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'todo', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('main', '102', 'todo', 'list', 'ToDo');


