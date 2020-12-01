
CREATE TABLE qz.handler (
    handler_name qz.variable_name PRIMARY KEY
);    


CREATE TABLE qz.handler_action (
    handler_name qz.variable_name REFERENCES qz.handler(handler_name), 
    action_name qz.variable_name,
    PRIMARY KEY (handler_name, action_name)
);


--
-- Data for Name: handlers; Type: TABLE DATA; Schema: qz; Owner: qz
--

INSERT INTO qz.handler (handler_name) VALUES ('onetable');
INSERT INTO qz.handler (handler_name) VALUES ('failtest');
INSERT INTO qz.handler (handler_name) VALUES ('refresh');
INSERT INTO qz.handler (handler_name) VALUES ('status');
INSERT INTO qz.handler (handler_name) VALUES ('fs');
INSERT INTO qz.handler (handler_name) VALUES ('logout');
INSERT INTO qz.handler (handler_name) VALUES ('menu');
INSERT INTO qz.handler (handler_name) VALUES ('login');
INSERT INTO qz.handler (handler_name) VALUES ('hdrdet');
INSERT INTO qz.handler (handler_name) VALUES ('none');
INSERT INTO qz.handler (handler_name) VALUES ('grid');
INSERT INTO qz.handler (handler_name) VALUES ('menupage');
INSERT INTO qz.handler (handler_name) VALUES ('callback');


--
-- Data for Name: handler_actions; Type: TABLE DATA; Schema: qz; Owner: qz
--

INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'create');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'insert');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'list');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'edit');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'update');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'delete');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('onetable', 'view');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('fs', 'get');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('fs', 'etag_value');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('grid', 'edit');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('grid', 'save');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('grid', 'update_row');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('grid', 'insert_row');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('grid', 'delete_row');



