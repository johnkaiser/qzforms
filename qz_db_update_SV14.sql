BEGIN;

INSERT INTO qz.change_history
(change_description, note)
VALUES
('Update to SV14',
'work in progress');

COMMENT ON COLUMN qz.form.pkey IS 'The primary key of the table the form is to manage';

-- update returns to form list, discard current form name
UPDATE qz.table_action
SET clear_context_parameters = '{form_name}'
WHERE form_name  = 'table_action_edit' 
AND action = 'update';


-- add a new form type
INSERT INTO qz.handler (handler_name) VALUES ('simpleform');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('simpleform', 'view');
INSERT INTO qz.handler_action (handler_name, action_name) VALUES ('simpleform', 'action');

-- add prompt rule attribute required
ALTER TABLE  qz.prompt_rule
ADD COLUMN required boolean
DEFAULT false;

-- prompt_rule create 
UPDATE qz.table_action
SET sql = 
$PRCTA$
 SELECT
    $1::text "form_name",
    $2::text "fieldname",
    'input_text'::qz.prompt_types "prompt_type",
    ''::text "tabindex",
    ''::text "el_class",
    'f'::text "readonly",
    'f'::text "required",
    ''::text "regex_pattern",
    ''::text "rows",
    ''::text "cols",
    ''::text "size",
    ''::text "maxlength",
    ''::text "options",
    ''::text "publish_pgtype",
    CASE WHEN 'grid' = $3 then
        't'::text
    ELSE
        'f'::text
    END "expand_percent_n",
    ''::text "onfocus",
    ''::text "onblur",
    CASE WHEN 'grid' = $3 then
        $CS$ change_status(%n, 'U') $CS$::text
    ELSE
        ''::text
    END "onchange",
    ''::text "onselect",
    ''::text "onclick",
    ''::text "ondblclick",
    ''::text "onmousedown",
    ''::text "onmouseup",
    ''::text "onmouseover",
    ''::text "onmousemove",
    ''::text "onmouseout",
    ''::text "onkeypress",
    ''::text "onkeydown",
    ''::text "onkeyup" 
$PRCTA$    
WHERE 
form_name = 'prompt_rule_edit'
AND action = 'create';

-- prompt_rule edit
UPDATE qz.table_action
SET sql = 
$PRETA$
SELECT "form_name", "fieldname", "prompt_type", "tabindex",
"el_class", "readonly", "required", "regex_pattern", "rows", "cols", "size", 
"maxlength", "options", "publish_pgtype", "expand_percent_n",
 "onfocus", "onblur", "onchange", "onselect",
"onclick", "ondblclick", "onmousedown", "onmouseup",
"onmouseover", "onmousemove", "onmouseout",
"onkeypress", "onkeydown", "onkeyup"
FROM qz.prompt_rule
WHERE form_name = $1 AND fieldname = $2
$PRETA$
WHERE 
form_name = 'prompt_rule_edit'
AND action = 'edit';

-- prompt_rule insert
UPDATE qz.table_action
SET sql = 
$PRITA$INSERT INTO qz.prompt_rule
  ("form_name", "fieldname", "prompt_type", "tabindex",
  "el_class", "readonly", "required", "regex_pattern",
  "rows", "cols", "size",
  "maxlength", "options", "publish_pgtype",
 "expand_percent_n", onfocus, onblur, onchange, onselect,
 onclick, ondblclick, onmousedown, onmouseup,
 onmouseover, onmousemove, onmouseout,
 onkeypress, onkeydown, onkeyup )
  VALUES
  ($1,$2,$3,$4,$5,$6,$7,$8,$9,
  $10,$11,$12,$13,$14,$15,$16,$17,$18,
  $19,$20,$21,$22,$23,$24,$25,$26,$27,$28,$29)
$PRITA$
WHERE
form_name = 'prompt_rule_edit'
AND action = 'insert';

-- prompt_rule update
UPDATE qz.table_action 
SET sql = 
$PRUTA$
UPDATE qz.prompt_rule
    SET
    "prompt_type" = $1,
    "el_class" = $2,
    "readonly" = $3,
    "required" = $4,
    "regex_pattern" = $5,
    "rows" = $6,
    "cols" = $7,
    "size" = $8,
    "tabindex" = $9,
    "options" = $10,
    "maxlength" = $11,
    "onfocus" = $12,
    "onblur" = $13,
    "onchange" = $14,
    "onselect" = $15,
    "onclick" = $16,
    "ondblclick" = $17,
    "onmousedown" = $18,
    "onmouseup" = $19,
    "onmouseover" = $20,
    "onmousemove" = $21,
    "onmouseout" = $22,
    "onkeypress" = $23,
    "onkeydown" = $24,
    "onkeyup" = $25,
    "publish_pgtype" = $26,
    "expand_percent_n" = $27
    WHERE form_name = $28
    AND fieldname = $29
$PRUTA$,
fieldnames = 
'{prompt_type,el_class,readonly,required,regex_pattern,rows,cols,size,tabindex,options,maxlength,onfocus,onblur,onchange,onselect,onclick,ondblclick,onmousedown,onmouseup,onmouseover,onmousemove,onmouseout,onkeypress,onkeydown,onkeyup,publish_pgtype,expand_percent_n,form_name,fieldname}'
WHERE
form_name = 'prompt_rule_edit'
AND action = 'update';

-- prompt_rule for the prompt_rule
INSERT INTO qz.prompt_rule (form_name, fieldname, el_class, readonly, rows, cols, size, options, maxlength, onfocus, onblur, onchange, src, onselect, onclick, ondblclick, onmousedown, onmouseup, onmouseover, onmousemove, onmouseout, onkeypress, onkeydown, onkeyup, tabindex, prompt_type, publish_pgtype, expand_percent_n, opttest)
VALUES ('prompt_rule_edit', 'required', NULL, false, NULL, NULL, NULL, '{yes,no}', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 'input_radio', NULL, false, '{yes,no}');


