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
-- Data for Name: release_checklist; Type: TABLE DATA; Schema: public; Owner: test
--

INSERT INTO release_checklist (version, seems_to_work, compiles_clean, installs_clean, simple_test_on_install, upgrades_clean, simple_test_on_upgrade, git_up_to_date, qzforms_com_up_to_date) VALUES (0.124, true, true, true, true, true, true, true, true);
INSERT INTO release_checklist (version, seems_to_work, compiles_clean, installs_clean, simple_test_on_install, upgrades_clean, simple_test_on_upgrade, git_up_to_date, qzforms_com_up_to_date) VALUES (0.123, true, true, true, true, true, true, true, true);
INSERT INTO release_checklist (version, seems_to_work, compiles_clean, installs_clean, simple_test_on_install, upgrades_clean, simple_test_on_upgrade, git_up_to_date, qzforms_com_up_to_date) VALUES (0.126, true, true, true, true, true, true, false, false);
INSERT INTO release_checklist (version, seems_to_work, compiles_clean, installs_clean, simple_test_on_install, upgrades_clean, simple_test_on_upgrade, git_up_to_date, qzforms_com_up_to_date) VALUES (0.127, true, true, true, true, true, true, true, true);


--
-- PostgreSQL database dump complete
--

