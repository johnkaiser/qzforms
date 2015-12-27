--
--  Unified javascript file qzforms.js, lose grid.js and prompt_rule.js
--
DELETE FROM qz.page_js
  WHERE filename = 'grid.js' 
  OR filename = 'prompt_rule.js';

DELETE FROM qz.js
  WHERE filename = 'grid.js' 
  OR filename = 'prompt_rule.js';

--
-- Correct mistakes in SV3
-- css_edit filename fields in wrong place,
-- others change \]\] to \[\]
--
UPDATE qz.prompt_rule
SET
    regex_pattern = '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
    readonly = 'f'
WHERE     
    form_name = 'css_edit' AND fieldname = 'filename';

UPDATE qz.prompt_rule
SET 
    regex_pattern = '^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$',
WHERE
    form_name = 'js_edit' AND fieldname = 'filename'

-- for filenames
UPDATE qz.prompt_rule
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
;

--  for variable names
UPDATE qz.prompt_rule 
SET regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\[\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
WHERE regex_pattern = 
'^[^\s\x01-\x1f\x5c\|\!\"\#\$\%\&\(\)\]\]\*\+\,\-\.\/\:\;\<\=\>\?\@\/\^\`\{\}\~]{1,63}$'
;


