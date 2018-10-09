--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = public, pg_catalog;

--
-- Data for Name: vtodo; Type: TABLE DATA; Schema: public; Owner: test
--

INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (7, '2017-04-02 12:08:31.419145', '2017-04-02 12:08:45.523055', 'There should be a onetable view which is like getone but with no submit button.', 'public', 'onetable', 0, 5, 2, 'needs_action', 'View Action', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (8, '2017-04-02 12:09:58.49689', '2017-04-02 12:09:58.49689', 'There isn''t one. Instead there are a bunch of threads and a read mutex. This could turn into a whole scheduler implementation; modest steps are called for. Currently, a malicious user can send requests for 10 times the number of threads, each doing the slowest thing possible, and DOS the server. Perhaps, do_page enqueues the request in a user queue, then there is a set of users with runnable jobs. Worker threads then round robin through the users.', 'public', 'qzmain', NULL, 5, 1, 'needs_action', 'handler_queue', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (9, '2017-04-02 12:11:21.114175', '2017-04-02 12:11:21.114175', 'There are no date or time prompt types', 'public', 'prompt_types', NULL, 4, 1, 'needs_action', 'Date and Time prompts', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (11, '2017-04-02 12:12:52.574879', '2017-04-02 12:12:52.574879', 'There needs to be a doc handler to display documentation.', 'public', 'handlers', NULL, 4, 1, 'needs_action', 'doc type', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (12, '2017-04-02 12:13:55.917513', '2017-04-02 12:13:55.917513', 'Fork a process for regex validation to isolate toxic data.', 'public', 'input', NULL, 5, 1, 'needs_action', 'regex validation process', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (13, '2017-04-02 12:15:56.425056', '2017-04-02 12:15:56.425056', 'Grid handler can not handle input array because both use name[n] notation. Hide the option.', 'public', 'inline_js prompt_rule', NULL, 3, 1, 'needs_action', 'Hide input_array in grid objects', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (14, '2017-04-02 12:18:17.484426', '2017-04-02 12:18:17.484426', 'Add prompt containers to put prompts in div''s and dl,dt,dd lists.', 'public', 'prompt_rule', NULL, 3, 1, 'needs_action', 'prompt containers div and dl', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (10, '2017-04-02 12:12:09.559047', '2017-04-05 18:46:48.209183', 'Building buffers by counting bytes needs to be replaced with a buffer library.', 'public', 'various', NULL, 4, 2, 'needs_action', 'Counting Bytes', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (25, '2017-05-07 22:49:54.995004', '2017-05-07 22:49:54.995004', 'An onetable insert page has no use for a delete button.', 'public', 'onetable.c', NULL, 3, 1, 'needs_action', 'no delete for insert', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (28, '2017-05-15 20:31:58.496726', '2017-05-15 20:31:58.496726', 'The js regex for a variable name does not stop a name of "1something", while the postgresql domain_check_clause will block it.', NULL, NULL, NULL, 2, 1, 'needs_action', 'js regex for variable name', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (6, '2017-04-02 12:06:25.617204', '2017-05-28 14:55:01.792779', 'Housekeeper should rename the log file when it gets to a configurable size.', 'public', 'Housekeeper', NULL, 3, 3, 'completed', 'Log Rotation', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (27, '2017-05-15 20:21:23.947922', '2018-09-03 19:44:42.39736', 'All the cool menu examples want the menu to be inside <li>''s.', 'public', 'menu.c', NULL, 2, 2, 'completed', 'menus to list', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (24, '2017-04-11 10:22:54.601943', '2018-09-03 19:45:16.980706', 'Automated testing', 'public', 'deployed', 10, 3, 3, 'completed', 'Testing', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (22, '2017-04-05 20:56:52.711936', '2018-09-03 19:45:32.995619', 'Test more under valgrind', 'public', 'Debian', NULL, 4, 3, 'completed', 'Valgrind', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (34, '2018-09-04 08:50:17.778669', '2018-09-04 08:50:17.778669', 'A bit like onetable-list, except instead of each row being a link to edit one row, each would be a link to another list or sublist.', 'public', NULL, NULL, 1, 1, 'needs_action', 'sublist', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (35, '2018-09-04 08:50:54.080157', '2018-09-04 08:50:54.080157', 'A doc handler that would insert html text onto the page', 'public', NULL, NULL, 1, 1, 'needs_action', 'doc handler', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (36, '2018-09-04 08:53:31.531057', '2018-09-04 08:53:31.531057', 'A table of templates such that each would be selected from a drop down list.
A table of div id''s that each template supports.', 'public', NULL, NULL, 2, 1, 'needs_action', 'Register Template', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (33, '2018-09-04 08:47:12.830108', '2018-10-09 08:52:03.576853', 'Token:
128 bits, 16 bytes, random iv
64 bits, 8 bytes, server token
64 bits, 8 bytes, domain token
128 bits, 8 bytes, payload

Payload:
120 bits 7 bytes, non-null bytes
8 bits, 1 byte, null byte

domain token is unique to tag application. ', 'public', 'crypto_etag.c', NULL, 1, 2, 'completed', '3rd Generation Form Tag', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (15, '2017-04-02 12:26:48.428817', '2018-10-09 08:58:49.870369', 'An input_text with a varchar type should have a maxlength n', 'public', 'input', NULL, 5, 2, 'completed', 'varchar(n) should have maxlength n', NULL);
INSERT INTO vtodo (uid, created, last_mod, description, class, location, percent, priority, seq, status, summary, url) VALUES (39, '2018-10-09 08:54:51.476628', '2018-10-09 08:59:27.587407', 'Log writes can step on each other. Wrap them in a mutex. Only 353 things to modify.', 'public', 'all .c files', NULL, 1, 4, 'needs_action', 'mutex the fprintf''s to log', NULL);


--
-- Name: vtodo_uid_seq; Type: SEQUENCE SET; Schema: public; Owner: test
--

SELECT pg_catalog.setval('vtodo_uid_seq', 39, true);


--
-- PostgreSQL database dump complete
--

