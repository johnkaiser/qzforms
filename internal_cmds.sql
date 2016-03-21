-- These are presented here for documentation and reference.
-- These commands are compiled into the executable.

PREPARE fetch_table_action AS	
        SELECT fm.schema_name, fm.table_name, ta.sql, 
        ta.fieldnames, ta.pkey, ta.etag, 
        fm.target_div, fm.handler_name, fm.xml_template, 
        fm.add_description, fm.prompt_container,  
        ta.helpful_text, 
        ARRAY( 
          SELECT 'js/get/'|| f.filename filename  
          FROM qz.page_js f 
          WHERE f.form_name = $1 
          ORDER BY sequence 
        ) js_filenames, 
        ARRAY( 
          SELECT 'css/get/' || c.filename filename 
          FROM qz.page_css c 
          WHERE c.form_name = $1 
          ORDER BY sequence 
        ) css_filenames 
        FROM qz.table_action ta 
        JOIN qz.form fm USING (form_name) 
        WHERE ta.form_name = $1 
        AND ta.action = $2;


PREPARE fetch_datum AS
   SELECT version, table_schema, table_name, column_name, 
        ordinal_position, column_default, is_nullable, typname, typtype, 
        typdelim, is_base_type, is_boolean, is_composite, is_domain, is_enum, 
        is_pseudo_type, character_maximum_length, 
        character_octet_length, numeric_precision, numeric_precision_radix, 
        numeric_scale, datetime_precision, typcategory_name, 
        domain_schema, domain_name, domain_check_clause, 
        udt_schema, udt_name, 
        description, is_updatable, enum_labels, composite_attributes 
        has_fkey, fkey_schema, fkey_table, fkey_attribute 
        FROM qz.get_pgtype_datum($1, $2);


PREPARE fetch_table_action_etag AS
        SELECT etag 
        FROM qz.table_action 
        WHERE form_name = $1 AND action = $2;


PREPARE fetch_rule AS
        SELECT form_name, fieldname, prompt_type, el_class, 
        readonly, rows, cols, size, maxlength, tabindex, publish_pgtype, 
        expand_percent_n, options, src, onfocus, onblur, onchange, 
        onselect, onclick, ondblclick, onmousedown, onmouseup, 
        onmouseover, onmouseout, onkeypress, onkeydown, 
        onkeyup, tabindex 
        FROM qz.prompt_rule 
        WHERE form_name = $1 AND fieldname = $2;

PREPARE menu_items AS
        SELECT menu_name, sequence, target_form_name, 
        action, menu_text, context_parameters 
        FROM qz.menu_item 
        WHERE menu_name = $1 
        ORDER BY sequence ;


PREPARE menu_set AS
        SELECT DISTINCT 
        s.menu_name, m.target_div, m.description 
        FROM qz.menu_set s 
        JOIN qz.menu m ON ( m.menu_name = s.menu_name ) 
        WHERE s.host_form_name = $1 
        AND (s.action = '' OR s.action = $2);


PREPARE menu_item_parameters AS
        SELECT parameter_key, parameter_value 
        FROM qz.menu_item_parameter 
        WHERE menu_name = $1 
        AND menu_item_sequence = $2 ;

PREPARE menu_exists_check AS
        SELECT EXISTS (
        SELECT form_name 
        FROM qz.qzobject 
        WHERE form_name = $1 
        AND handler_name = 'menupage') ;

