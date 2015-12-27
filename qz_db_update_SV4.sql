--
--  Unified javascript file qzforms.js, lose grid.js and prompt_rule.js
--
DELETE FROM qz.page_js
  WHERE filename = 'grid.js' 
  OR filename = 'prompt_rule.js';

DELETE FROM qz.js
  WHERE filename = 'grid.js' 
  OR filename = 'prompt_rule.js';


