
INSERT INTO qz.change_history 
  (change_description,note) 
  VALUES 
  ('Update to SV3',
   'Add regex_pattern to prompt_rule, 
    add err_msg class to qzforms.css' 
   );

ALTER TABLE qz.prompt_rule 
    ADD COLUMN regex_pattern text;

UPDATE qz.constants 
SET schema_version = '3';


UPDATE qz.css SET data = 
'.bold {font-weight:bold;}
  .light { color: #999; }
  legend {font-size: 50%;}
  table.qztablez { border: 2pt solid black; border-collapse: collapse; }
  table.qztablez tr td { border: 1pt solid #aaa; padding: 2pt; }
  table.qztablez tr th { border: 1pt solid black; padding: 2pt; font-weight:bold; }
  label { padding-right: 1em; }
/* tables */
table.tablesorter {
    font-family:arial;
    background-color: #CDCDCD;
    margin:10px 0pt 15px;
    font-size: 8pt;
    width: 100%;
    text-align: left;
}
table.tablesorter thead tr th, table.tablesorter tfoot tr th {
    background-color: #e6EEEE;
    border: 1px solid #FFF;
    font-size: 8pt;
    padding: 4px 20px 4px 4px;
}
table.tablesorter thead tr .header {
    background-image: url(/jk/bg.gif);
    background-repeat: no-repeat;
    background-position: center right;
    cursor: pointer;
}
table.tablesorter tbody td {
    color: #3D3D3D;
    padding: 4px;
    background-color: #FFF;
    vertical-align: top;
}
table.tablesorter tbody tr.odd td {
    background-color:#F0F0F6;
}
table.tablesorter thead tr .headerSortUp {
    background-image: url(/jk/asc.gif);
}
table.tablesorter thead tr .headerSortDown {
    background-image: url(/jk/desc.gif);
}
table.tablesorter thead tr .headerSortDown, table.tablesorter thead tr .headerSortUp {
background-color: #8dbdd8;
}
span.description {
    font-size: 75%;
	color: #888;
	padding-left: 1em;
}
td.yesno label { 
    background-color: #ddf;
	color: #a00;
}
td.input_hidden {
    display: none;
}
th.input_hidden {
    display: none;
}
div.menu form {
    display: inline;
}    
input.menu_button {
    background: white;
    font-weight: bold;
}
#qzmenu {
    padding-bottom: 1ex;
    border-bottom: 2pt dotted grey;
}    
.err_msg {
    background: yellow;
    border: 2pt solid red;
    padding: 3pt;
    display: table;
    font-weight: bold;
}
'
WHERE filename = 'qzforms.css';


UPDATE qz.table_action 
SET sql = 
'SELECT 
$1::text "form_name", 
$2::text "fieldname",
''input_text''::qz.prompt_types "prompt_type",
''''::text "el_class",
''''::text "options", 
''f''::boolean "readonly",
''''::text "regex_pattern",
''''::text "rows",
''''::text "cols",
''''::text "size"
'
WHERE form_name =  'prompt_rule_edit'
AND action = 'create';

UPDATE qz.table_action
SET sql = 
'SELECT "form_name", "fieldname", "prompt_type", "tabindex",
"el_class", "readonly","regex_pattern", "rows", "cols", "size", 
"maxlength", "options", "publish_pgtype", "expand_percent_n",
 "onfocus", "onblur", "onchange", "onselect",
"onclick", "ondblclick", "onmousedown", "onmouseup",
"onmouseover", "onmousemove", "onmouseout",
"onkeypress", "onkeydown", "onkeyup"
FROM qz.prompt_rule
WHERE form_name = $1 AND fieldname = $2'
WHERE form_name = 'prompt_rule_edit'
AND action = 'edit';

UPDATE qz.table_action
SET sql =
'INSERT INTO qz.prompt_rule
("form_name", "fieldname", "prompt_type", "options",
"el_class", "readonly", "regex_pattern", "rows", "cols", "size")
VALUES
($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)',
fieldnames = 
'{form_name,fieldname,prompt_type,options,el_class,readonly,regex_pattern,
 rows,cols,size}'
WHERE form_name = 'prompt_rule_edit'
AND action = 'insert';

UPDATE qz.table_action
SET sql = 
'UPDATE qz.prompt_rule
SET
"prompt_type" = $1,
"el_class" = $2,
"readonly" = $3,
"regex_pattern" = $4,
"rows" = $5,
"cols" = $6,
"size" = $7,
"tabindex" = $8,
"options" = $9, 
"maxlength" = $10,
"onfocus" = $11, 
"onblur" = $12, 
"onchange" = $13, 
"onselect" = $14, 
"onclick" = $15, 
"ondblclick" = $16,
"onmousedown" = $17, 
"onmouseup" = $18, 
"onmouseover" = $19, 
"onmousemove" = $20, 
"onmouseout" = $21,
"onkeypress" = $22, 
"onkeydown" = $23, 
"onkeyup" = $24,
"publish_pgtype" = $25,
"expand_percent_n" = $26
WHERE form_name = $27
AND fieldname = $28
',
fieldnames = 
'{prompt_type,el_class,readonly,regex_pattern,rows,cols,size,tabindex,options,maxlength,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup,publish_pgtype,expand_percent_n,form_name,fieldname}'
WHERE form_name = 'prompt_rule_edit'
AND action = 'update';

--
-- regex patterns - filename
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, etag, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) VALUES ('css_edit', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, 63, 2174, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);


INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, etag, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest) VALUES ('js_edit', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, 63, 2174, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);



UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'page_css' AND fieldname = 'filename';

UPDATE qz.prompt_rule 
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'page_js' AND fieldname = 'filename';


--
-- regex patterns - variable_name
--

-- menu_edit

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_edit', 'target_div', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

-- menu_host_edit

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_host_edit' 
AND fieldname = 'menu_name';

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_host_edit' 
AND fieldname = 'host_form_name';

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_host_edit' 
AND fieldname = 'host_form_name';


-- menu_item_edit: menu_name target_form_name action

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'target_form_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'action', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

-- menu_set_edit: insert menu_name update host_form_name update action

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_set_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_set_edit' 
AND fieldname = 'host_form_name';

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_set_edit' 
AND fieldname = 'action';

-- prompt_rule: form_name, fieldname

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'prompt_rule_edit' 
AND fieldname = 'form_name';

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('prompt_rule_edit', 'fieldname', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

-- table_action: form_name, action

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'table_action_edit' 
AND fieldname = 'form_name';

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('table_action_edit', 'action', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'table_action_edit'
AND fieldname = 'fieldnames';



