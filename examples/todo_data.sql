--
-- PostgreSQL database dump
--

SET search_path = public, pg_catalog;

--
-- Data for Name: vtodo; Type: TABLE DATA; Schema: public; Owner: jk
--

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (7, 'View Action', '2017-04-02 12:08:31.419145', '2017-04-02 12:08:45.523055', 'There should be a onetable view which is like getone but with no submit button.', 'public', 'onetable', 0, 5, 2, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (8, 'handler_queue', '2017-04-02 12:09:58.49689', '2017-04-02 12:09:58.49689', 'There isn''t one. Instead there are a bunch of threads and a read mutex. This could turn into a whole scheduler implementation; modest steps are called for. Currently, a malicious user can send requests for 10 times the number of threads, each doing the slowest thing possible, and DOS the server. Perhaps, do_page enqueues the request in a user queue, then there is a set of users with runnable jobs. Worker threads then round robin through the users.', 'public', 'qzmain', NULL, 5, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (9, 'Date and Time prompts', '2017-04-02 12:11:21.114175', '2017-04-02 12:11:21.114175', 'There are no date or time prompt types', 'public', 'prompt_types', NULL, 4, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (11, 'doc type', '2017-04-02 12:12:52.574879', '2017-04-02 12:12:52.574879', 'There needs to be a doc handler to display documentation.', 'public', 'handlers', NULL, 4, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (12, 'regex validation process', '2017-04-02 12:13:55.917513', '2017-04-02 12:13:55.917513', 'Fork a process for regex validation to isolate toxic data.', 'public', 'input', NULL, 5, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (13, 'Hide input_array in grid objects', '2017-04-02 12:15:56.425056', '2017-04-02 12:15:56.425056', 'Grid handler can not handle input array because both use name[n] notation. Hide the option.', 'public', 'inline_js prompt_rule', NULL, 3, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (14, 'prompt containers div and dl', '2017-04-02 12:18:17.484426', '2017-04-02 12:18:17.484426', 'Add prompt containers to put prompts in div''s and dl,dt,dd lists.', 'public', 'prompt_rule', NULL, 3, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (15, 'varchar(n) should have maxlength n', '2017-04-02 12:26:48.428817', '2017-04-02 12:26:48.428817', 'An input_text with a varchar type should have a maxlength n', 'public', 'input', NULL, 5, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (10, 'Counting Bytes', '2017-04-02 12:12:09.559047', '2017-04-05 18:46:48.209183', 'Building buffers by counting bytes needs to be replaced with a buffer library.', 'public', 'various', NULL, 4, 2, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (25, 'no delete for insert', '2017-05-07 22:49:54.995004', '2017-05-07 22:49:54.995004', 'An onetable insert page has no use for a delete button.', 'public', 'onetable.c', NULL, 3, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (27, 'menus to list', '2017-05-15 20:21:23.947922', '2017-05-15 20:21:23.947922', 'All the cool menu examples want the menu to be inside <li>''s.', 'public', 'menu.c', NULL, 2, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (28, 'js regex for variable name', '2017-05-15 20:31:58.496726', '2017-05-15 20:31:58.496726', 'The js regex for a variable name does not stop a name of "1something", while the postgresql domain_check_clause will block it.', NULL, NULL, NULL, 2, 1, 'needs_action', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (6, 'Log Rotation', '2017-04-02 12:06:25.617204', '2017-05-28 14:55:01.792779', 'Housekeeper should rename the log file when it gets to a configurable size.', 'public', 'Housekeeper', NULL, 3, 3, 'completed', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (24, 'Testing', '2017-04-11 10:22:54.601943', '2017-05-28 14:55:21.803705', 'Automated testing', 'public', 'deployed', 10, 3, 2, 'in_process', NULL);

INSERT INTO vtodo (uid, summary, created, last_mod, description, class, location, percent, priority, seq, status, url) VALUES (22, 'Valgrind', '2017-04-05 20:56:52.711936', '2017-05-28 14:55:42.152983', 'Test more under valgrind', 'public', 'Debian', NULL, 4, 2, 'in_process', NULL);



--
-- Name: vtodo_uid_seq; Type: SEQUENCE SET; Schema: public; Owner: jk
--

SELECT pg_catalog.setval('vtodo_uid_seq', 29, true);


--
-- PostgreSQL database dump complete
--

