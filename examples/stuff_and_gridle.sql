
--
-- Name: mood; Type: TYPE; Schema: public; Owner: qz
--

CREATE TYPE mood AS ENUM (
    'sad',
    'ok',
    'happy',
    'chocolate'
);


CREATE TABLE location (
    loc_name text PRIMARY KEY
);

INSERT INTO qz.css (filename, mimetype, modtime, data) 
VALUES ('blue.css', 'text/css', '2015-05-30 17:23:51',
$B$
body {
    background: lightblue;
    color: darkblue;
}
$B$);

INSERT INTO location (loc_name) VALUES ('some place');
INSERT INTO location (loc_name) VALUES ('a place');
INSERT INTO location (loc_name) VALUES ('that place');
INSERT INTO location (loc_name) VALUES ('this place');
INSERT INTO location (loc_name) VALUES ('any place');
INSERT INTO location (loc_name) VALUES ('no place');
INSERT INTO location (loc_name) VALUES ('other place');
INSERT INTO location (loc_name) VALUES ('fireplace');
INSERT INTO location (loc_name) VALUES ('my place');
INSERT INTO location (loc_name) VALUES ('your place');
INSERT INTO location (loc_name) VALUES ('the place');
INSERT INTO location (loc_name) VALUES ('bad place');
INSERT INTO location (loc_name) VALUES ('a new place');
INSERT INTO location (loc_name) VALUES ('displace');
INSERT INTO location (loc_name) VALUES ('anyplace');
INSERT INTO location (loc_name) VALUES ('birthplace');
INSERT INTO location (loc_name) VALUES ('commonplace');
INSERT INTO location (loc_name) VALUES ('emplace');
INSERT INTO location (loc_name) VALUES ('farmplace');
INSERT INTO location (loc_name) VALUES ('foreplace');
INSERT INTO location (loc_name) VALUES ('misplace');
INSERT INTO location (loc_name) VALUES ('outplace');
INSERT INTO location (loc_name) VALUES ('overplace');
INSERT INTO location (loc_name) VALUES ('place');
INSERT INTO location (loc_name) VALUES ('postplace');
INSERT INTO location (loc_name) VALUES ('predisplace');
INSERT INTO location (loc_name) VALUES ('preplace');
INSERT INTO location (loc_name) VALUES ('replace');
INSERT INTO location (loc_name) VALUES ('someplace');
INSERT INTO location (loc_name) VALUES ('supplace');
INSERT INTO location (loc_name) VALUES ('transplace');
INSERT INTO location (loc_name) VALUES ('uncommonplace');
INSERT INTO location (loc_name) VALUES ('unplace');
INSERT INTO location (loc_name) VALUES ('workplace');

CREATE TABLE gridle (
    n integer PRIMARY KEY,
    t character varying(20),
    txt text,
    location text REFERENCES location(loc_name),
    current_mood mood,
    yesno boolean DEFAULT false
);

COMMENT ON COLUMN gridle.n IS 'An Integer';
COMMENT ON COLUMN gridle.t IS 'A varchar(20)';
COMMENT ON COLUMN gridle.txt IS 'Free form text';
COMMENT ON COLUMN gridle.location IS 'A foreign key reference';
COMMENT ON COLUMN gridle.current_mood IS 'An enumerated type';
COMMENT ON COLUMN gridle.yesno IS 'A boolean';

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, 
xml_template, target_div, add_description, prompt_container, pkey) 
VALUES ('gridle', 'grid', 'public', 'gridle', 'base.xml', 'qz', false, 
'no_container', '{n}');

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (3, 'three', 
'&lt;/textarea&gt; &lt;h1&gt;hey&lt;/&gt;', 
'the place', 'chocolate', true);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (1, 'one', 
'</textarea>
<h1>Hey</h1>', 
'commonplace', 'sad', false);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (2, 'two', 
'</textarea>
<script>alert(''hacked'');</script>', 
'predisplace', 'happy', false);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (6, 'six', 
'nope
not at all
', 
'outplace', 'sad', false);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (4, 'four', '', 'bad place', 'sad', true);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (8, 'The eighth thing.', '', 'emplace', 'chocolate', false);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (5, 'five', '5', 'fireplace', 'sad', false);

INSERT INTO gridle (n, t, txt, location, current_mood, yesno) 
VALUES (7, 'sev en', '', 'my place', 'chocolate', true);

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, 
action, menu_text, context_parameters) 
VALUES ('main', 10, 'gridle', 'edit', 'Gridle', NULL);

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'gridle', 'any');

INSERT INTO qz.page_css (form_name, sequence, filename) 
VALUES ('gridle', 10, 'qzforms.css');

INSERT INTO qz.page_css (form_name, sequence, filename) 
VALUES ('gridle', 20, 'blue.css');

INSERT INTO qz.page_js (form_name, sequence, filename) 
VALUES ('gridle', 30, 'qzforms.js');

INSERT INTO qz.prompt_rule (form_name, fieldname, readonly, regex_pattern, rows, 
cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect,
onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, 
onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, 
publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 'n', false, '\d*', NULL, NULL, 5, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 'yesno', 'testing', false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 'current_mood', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 'location', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_fkey', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 't', NULL, false, NULL, NULL, 10, '{one,two,three,four,five,six,"sev en","The eighth thing.",niner}', 20, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', false, true, '{one,two,three,four,five,six,"sev en"}');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 'Button', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'alert(''ouch! '' + ''%n''.toString() )', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'button', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('gridle', 'txt', NULL, false, 2, 20, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', false, true, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('gridle', 'edit', 'SELECT n,t, location, current_mood, yesno, 
''Button''::text "Button", txt 
FROM gridle 
ORDER BY n', NULL, 'Gridle is an example of a grid handler. The location drop down is from a foreign key, current_mood is enumerated, yesno boolean.   ');


INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('gridle', 'delete_row', 'DELETE FROM gridle WHERE n = $1', '{n}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('gridle', 'insert_row', 'INSERT INTO gridle  ("n", "t", "location", "current_mood", "yesno", "txt") VALUES ($1, $2, $3, $4, $5, $6)', '{n,t,location,current_mood,yesno,txt}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('gridle', 'update_row', 'UPDATE gridle
SET "t" = $2, 
"location" = $3,
"current_mood" = $4,
"yesno" = $5,
txt = $6
WHERE n = $1', '{n,t,location,current_mood,yesno,txt}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('gridle', 'save', 'SELECT ''t''::bool "value";', NULL, NULL);


--
-- Name: stuff; Type: TABLE; Schema: public; Owner: qz; Tablespace: 
--

--
-- Name: posval; Type: DOMAIN; Schema: public; Owner: qz
--

CREATE DOMAIN posval AS integer
	CONSTRAINT posval_check CHECK ((VALUE > 0));


--
-- Name: postuple; Type: TYPE; Schema: public; Owner: qz
--

CREATE TYPE postuple AS (
	x posval,
	y posval
);

CREATE SEQUENCE stuff_n;


CREATE TABLE stuff (
    n integer PRIMARY KEY DEFAULT nextval('stuff_n'::regclass),
    words text DEFAULT 'XXXX'::text,
    trouble postuple,
    ar integer[],
    pos posval,
    current_mood mood,
    addr inet,
    mask bit(4),
    nbr numeric,
    fixed numeric(9,6),
    name character varying(15),
    yesno boolean
);


--
-- Name: TABLE stuff; Type: COMMENT; Schema: public; Owner: qz
--

COMMENT ON TABLE stuff IS 'A table of various types';


--
-- Name: COLUMN stuff.n; Type: COMMENT; Schema: public; Owner: qz
--

COMMENT ON COLUMN stuff.n IS 'Integer primary key';


--
-- Name: COLUMN stuff.words; Type: COMMENT; Schema: public; Owner: qz
--

COMMENT ON COLUMN stuff.words IS 'text with a default value';


--
-- Name: COLUMN stuff.trouble; Type: COMMENT; Schema: public; Owner: qz
--

COMMENT ON COLUMN stuff.trouble IS 'Compound item containing a domain';


--
-- Name: COLUMN stuff.ar; Type: COMMENT; Schema: public; Owner: qz
--

COMMENT ON COLUMN stuff.ar IS 'An array.  Caution: an empty cell truncates and the values after the empty cell are lost.  Empty cells at the end of valid data are harmlessly ignored.';


--
-- Name: COLUMN stuff.current_mood; Type: COMMENT; Schema: public; Owner: qz
--

COMMENT ON COLUMN stuff.current_mood IS 'an enumerated type';


--
-- Name: create_stuff(); Type: FUNCTION; Schema: qz; Owner: qz
--

CREATE FUNCTION create_stuff() RETURNS public.stuff
    LANGUAGE plpgsql
    AS $$
DECLARE
   new_record stuff;
BEGIN
select nextval('stuff_n') into new_record.n;
    RETURN new_record;
END;
$$;



--
-- Data for Name: stuff; Type: TABLE DATA; Schema: public; Owner: qz
--

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), '<script>alert("hey")</alert>', '(4,5)', '{1,2,3,4}', 1, 'happy', NULL, NULL, NULL, NULL, 'fourty five', false);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'oh no', '(5,6)', '{752,569}', NULL, 'ok', NULL, NULL, NULL, NULL, NULL, false);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'xyz', '(1,2)', '{1,2}', NULL, 'chocolate', NULL, NULL, NULL, NULL, NULL, false);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'one, two, three', '(2,8)', '{99999,88888,77777}', 11, 'sad', '192.168.1.1', B'0001', 1.01, 1.000000, NULL, true);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'seven', '(8,2)', '{9,7,5,3,1}', 17, 'chocolate', '192.168.1.7', B'0111', 7, 7.000000, NULL, false);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'jk', '(,)', '{1,2}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, false);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'three', '(1,1)', '{0,1,2,6,9,12}', 13, 'ok', '192.168.1.3', B'0011', 3.3, 3.000000, 'z', false);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'was new, now old', '(4,5)', '{1,3,6}', NULL, 'happy', NULL, NULL, NULL, NULL, NULL, true);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'whatever', NULL, '{2,3}', 7, 'ok', NULL, B'1010', 5, NULL, NULL, true);

INSERT INTO stuff (n, words, trouble, ar, pos, current_mood, addr, mask, nbr, fixed, name, yesno) 
VALUES (nextval('stuff_n'), 'be moody dangit.', '(2,3)', '{57,67,47,37,27}', NULL, 'happy', NULL, NULL, 1, NULL, NULL, true);

INSERT INTO qz.form (form_name, handler_name, schema_name, table_name, xml_template, target_div, add_description, prompt_container, pkey) 
VALUES ('stuff', 'onetable', 'public', 'stuff', 'base.xml', 'qz', false, 'fieldset', '{n}');

INSERT INTO qz.menu_set (menu_name, host_form_name, action) 
VALUES ('main', 'stuff', 'any');

INSERT INTO qz.page_css (form_name, sequence, filename) 
VALUES ('stuff', 10, 'qzforms.css');

INSERT INTO qz.page_js (form_name, sequence, filename) 
VALUES ('stuff', 30, 'qzforms.js');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'ar', 'testclass', false, '\d*', NULL, NULL, 25, NULL, 64, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'trouble', NULL, false, '\(\d*,\d*\)', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'words', NULL, false, NULL, NULL, 50, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'yesno', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'addr', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size,  options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'pos', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'current_mood', 'current_mood', false, NULL, NULL, NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) 
VALUES ('stuff', 'button', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'alert(''ouch!'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'button', true, false, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('stuff', 'delete', 'DELETE FROM stuff
WHERE n = $1', '{n}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('stuff', 'insert', 'INSERT INTO stuff
("n", "words", "trouble", "ar","pos", "addr", "mask", "nbr", "fixed", "yesno", "current_mood")
VALUES
($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11)
', '{n,words,trouble,ar,pos,addr,mask,nbr,fixed,yesno,current_mood}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('stuff', 'update', 'UPDATE stuff SET words = $2, trouble = $3,ar = $4,
pos = $5,addr = $6,mask = $7,nbr = $8,fixed = $9,
current_mood = $10, name = $11, yesno = $12
WHERE n = $1', '{n,words,trouble,ar,pos,addr,mask,nbr,fixed,current_mood,stf_name,yesno}', NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('stuff', 'edit', 'SELECT  
  n, words, current_mood, trouble, ar, pos, addr, mask, nbr,
  fixed, name stf_name, yesno, ''clickit''::text button 
FROM 
  stuff
WHERE 
  n = $1', '{n}', 'Test Data');

INSERT INTO qz.table_action (form_name, action, sql, fieldnames, helpful_text) 
VALUES ('stuff', 'create', 'SELECT n, words, trouble, ar, pos, current_mood, addr, mask, nbr,fixed, ''f''::bool yesno
FROM create_stuff()', NULL, NULL);

INSERT INTO qz.table_action (form_name, action, sql, fieldnames,  helpful_text) 
VALUES ('stuff', 'list', 'SELECT "n", "words", "trouble", "ar", "nbr", "current_mood",
CASE WHEN "yesno" THEN ''yes'' WHEN not "yesno" THEN ''no'' 
END "yesno", 
"fixed"
FROM "stuff" 
ORDER BY "n"', NULL, 'Stuff is a test table with lots of 
different kinds of data.');

INSERT INTO qz.menu_item (menu_name, menu_item_sequence, target_form_name, action, menu_text, context_parameters) 
VALUES ('main', 1, 'stuff', 'list', 'Stuff', NULL);




COMMENT ON TABLE stuff IS 'A table of various types';
COMMENT ON COLUMN stuff.n IS 'Integer primary key';
COMMENT ON COLUMN stuff.words IS 'text with a default value';
COMMENT ON COLUMN stuff.trouble IS 'Compound item containing a domain';
COMMENT ON COLUMN stuff.ar IS 'An array.  Caution: an empty cell truncates and the values after the empty cell are lost.  Empty cells at the end of valid data are harmlessly ignored.';
COMMENT ON COLUMN stuff.current_mood IS 'an enumerated type';
