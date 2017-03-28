COMMENT ON COLUMN qz.menu_item.action IS 'The table_action action, or the 3rd URI segment in /qz/form/action.';
COMMENT ON COLUMN qz.menu_item.menu_item_sequence IS 'Order of this item in its set.';
COMMENT ON COLUMN qz.prompt_rule.cols IS 'The number of columns or width of a text area.';
COMMENT ON COLUMN qz.prompt_rule.expand_percent_n IS 'In any event, the string %n will be replaced with an integer representing a row index. This is used in arrays and grids';
COMMENT ON COLUMN qz.prompt_rule.fieldname IS 'The name to identify a prompt.  If the attribute name on the table is different from the name used on the web form then this is the web form name';
COMMENT ON COLUMN qz.prompt_rule.form_name IS 'The name to identify a set of actions';
COMMENT ON COLUMN qz.prompt_rule.maxlength IS 'The longest allowed string for a text input.';
COMMENT ON COLUMN qz.prompt_rule.options IS 'An array of possible choices for an option drop down or a radio button. Values after any empty cell will be truncated and lost.';
COMMENT ON COLUMN qz.prompt_rule.prompt_type IS 'The prompt type determines which input element is created. text_array must not be used with the grid pattern.';

COMMENT ON COLUMN qz.prompt_rule.publish_pgtype IS 'If set a json representation of the postgresql data type is added to the element and is displayed on a double click';

COMMENT ON COLUMN qz.prompt_rule.readonly IS 'If set, the field will not be editable in the browser.';
COMMENT ON COLUMN qz.prompt_rule.rows IS 'The number of rows or height of a textarea.';
COMMENT ON COLUMN qz.prompt_rule.size IS 'The display width of a text input.';
COMMENT ON COLUMN qz.prompt_rule.tabindex IS 'The sequence that entering a tab will follow from field to field.';
COMMENT ON COLUMN qz.form.add_description IS 'Add the PostgreSQL attribute comment after an input field.  COMMENT ON COLUMN sometable.somecolumn is ...';
COMMENT ON COLUMN qz.form.handler_name IS 'The handler is a function name that will process the HTTP request.  Different handlers support different actions and present data differently.';
COMMENT ON COLUMN qz.form.form_name IS 'The form_name is the second segment of the URL and ties a particular handler to a set of actions';
COMMENT ON COLUMN qz.form.prompt_container IS 'Input prompts will be placed in this kind of HTML tag.  Use no_container for grid items.';
COMMENT ON COLUMN qz.form.schema_name IS 'The schema the table is in.';
COMMENT ON COLUMN qz.form.table_name IS 'The name of the table being edited';
COMMENT ON COLUMN qz.form.target_div IS 'An HTML div with the given id that exists in the xml_template that will contain the results.';
COMMENT ON COLUMN qz.form.xml_template IS 'An xml file that exists in the QZ_TEMPLATE_PATH directory that will be the starting document.';
COMMENT ON COLUMN qz.table_action.action IS 'Different objects will support different sets of actions. The form_name, action tuple will identify a particular SQL statement and name mapping.';
COMMENT ON COLUMN qz.table_action.fieldnames IS 'The names of the fields to update.  The names come from the incoming form post data and may not match the names used in the table. The order must match the positional parameters in the prepared SQL statement.  An empty cell truncates the data and values after an empty field are lost.';
COMMENT ON COLUMN qz.table_action.helpful_text IS 'A short paragraph of a hopefully helpful nature to be placed in a div with an id of "helpful_text", typically at the top of a form.';
COMMENT ON COLUMN qz.table_action.form_name IS 'A common identity that binds together a set of actions.';
COMMENT ON COLUMN qz.table_action.sql IS 'The actual command to be performed.  This is made into a prepared statement.  Use $n positional parameters, $1, $2, etc.  Map the positional parameters to names using fieldnames.';
COMMENT ON COLUMN qz.form.form_set_name IS 'Typically left blank.  Used when data must be passed from one form to another.';

COMMENT ON COLUMN qz.form.pkey IS 'The names of the fields that comprise the primary key.  Fields in the primary key are used to retrieve particular records and may not be edited. An empty cell truncates the data and values after an empty cell are lost.';
