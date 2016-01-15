CREATE TABLE qz.prompt_rule (
    form_name qz.variable_name NOT NULL,
    fieldname qz.variable_name NOT NULL,
    el_class character varying,
    readonly boolean DEFAULT 'f',
    rows integer,
    cols integer,
    size integer,
    regex_pattern text,
    etag bigint DEFAULT nextval('qz.etag_seq'::regclass),
    options text[],
    maxlength integer,
    onfocus text,
    onblur text,
    onchange text,
    src text,
    onselect text,
    onclick text,
    ondblclick text,
    onmousedown text,
    onmouseup text,
    onmouseover text,
    onmousemove text,
    onmouseout text,
    onkeypress text,
    onkeydown text,
    onkeyup text,
    tabindex integer,
    prompt_type qz.prompt_types NOT NULL,
    publish_pgtype boolean,
    expand_percent_n boolean DEFAULT false,
    opttest text[],
    PRIMARY KEY (form_name, fieldname)
);





-- 
--  Prompt Rules
-- 
--
-- css_edit
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('css_edit', 'data', NULL, false, 40, 80, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('css_edit', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, 63, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);

--
-- js_edit
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('js_edit', 'modtime', NULL, true, NULL, NULL, 25, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('js_edit', 'etag', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', true, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('js_edit', 'data', NULL, false, 40, 80, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('js_edit', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, 63, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);

--
-- page_css
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('page_css', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_fkey', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('page_css', 'form_name', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('page_css', 'sequence', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

--
-- page_js
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('page_js', 'filename', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_fkey', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('page_js', 'form_name', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('page_js', 'sequence', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

--
-- menu
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu', 'something', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'alert(''hey'');', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'button', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_item_edit', 'context_parameters', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_host_edit', 'host_form_name', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_host_edit', 'action', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_set_edit', 'action', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_set_edit', 'host_form_name', NULL, false, 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_host_edit', 'set_id', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_hidden', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_host_edit', 'menu_name', NULL, false, 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'change_status(%n, ''U'')', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_fkey', false, true, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_edit', 'target_div', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'action', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'menu_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('menu_item_edit', 'target_form_name', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('menu_item_edit', 'menu_item_sequence', NULL, false, '\d*', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE form_name = 'menu_set_edit' 
AND fieldname = 'host_form_name';

--
-- prompt_rule_edit
--
INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('prompt_rule_edit', 'fieldname', 'input_text',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'options', NULL, false, NULL, NULL, 60, NULL, 4096, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'form_name', NULL, false,
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'prompt_type', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'publish_pgtype', NULL, false, NULL, NULL, NULL, '{yes,no}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', false, false, '{yes,no}');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'tabindex', NULL, false, NULL, NULL, 5, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'size', NULL, false, NULL, NULL, 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'readonly', NULL, false, NULL, NULL, NULL, '{yes,no}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, '{yes,no}');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'rows', NULL, false, NULL, NULL, 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'cols', NULL, false, NULL, NULL, 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'add_description', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'prompt_container', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'expand_percent_n', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, NULL);

--
-- form 
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('form', 'handler_name_ro', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('form', 'handler_name', NULL, false,
 '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, '{fs,grid,menu,menupage,onetable}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('form', 'add_description', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('form', 'prompt_container', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('form', 'form_name', 'input_text', 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('form', 'schema_name', 'input_text', 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern)
VALUES ('form', 'table_name', 'input_text', 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$');

--
-- table_action
--

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action', 'form_name', NULL, false, 
 '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action', 'sql', NULL, false, 12, 60, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action', 'pkey', NULL, false, NULL, NULL, 50, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action', 'fieldnames', NULL, false, 3, 60, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action', 'helpful_text', NULL, false, 10, 40, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'form_name', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'sql', NULL, false, 12, 60, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'pkey', NULL, false, NULL, NULL, 50, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'fieldnames', NULL, false, 
 '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
3, 60, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'helpful_text', NULL, false, 10, 40, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'textarea', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, prompt_type, regex_pattern, options)
VALUES ('table_action_edit', 'action', 'select_options',
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
'{create,delete,delete_row,edit,etag_value,get,getall,
  insert,insert_row,save,update,update_row,view}');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'handler_name', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, regex_pattern, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('table_action_edit', 'handler_name_ro', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule
(form_name, fieldname, readonly, regex_pattern, prompt_type)
VALUES 
('table_action_edit', 'action_ro', 't', 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
'input_text');

--
-- onetable_edit
--
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('onetable_edit', 'schema_name', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('onetable_edit', 'table_name', NULL, true, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('onetable_edit', 'xml_template', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);



--
--  prompt_rule
--

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'form_name', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'prompt_type', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'publish_pgtype', NULL, false, NULL, NULL, NULL, '{yes,no}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', false, false, '{yes,no}');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'tabindex', NULL, false, NULL, NULL, 5, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'size', NULL, false, NULL, NULL, 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'readonly', NULL, false, NULL, NULL, NULL, '{yes,no}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, '{yes,no}');

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'rows', NULL, false, NULL, NULL, 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'cols', NULL, false, NULL, NULL, 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_text', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'add_description', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'prompt_container', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'select_options', false, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'expand_percent_n', NULL, false, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, NULL);

INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule', 'options', NULL, false, NULL, NULL, 60, NULL, 4096, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'text_array', NULL, false, NULL);



