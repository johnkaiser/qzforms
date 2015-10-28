
--
-- Functions and Triggers
--

CREATE OR REPLACE FUNCTION
create_table_action(form_name varchar, action varchar)
RETURNS qz.table_action
AS $$
DECLARE
    new_table_action qz.table_action;
BEGIN
    new_table_action.form_name := form_name;
	new_table_action.action := action;

	RETURN new_table_action;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION qz.table_action_etag_set()
RETURNS TRIGGER AS $BODY$
BEGIN
    CASE TG_OP
        WHEN 'INSERT' THEN
            UPDATE qz.table_action ta
	        SET etag = nextval('qz.etag_seq')
	        WHERE ta.form_name = NEW.form_name;
	        RETURN NEW;

        WHEN 'UPDATE' THEN
            UPDATE qz.table_action ta
	        SET etag = nextval('qz.etag_seq')
	        WHERE ta.form_name = NEW.form_name
            OR ta.form_name = OLD.form_name;
	        RETURN NEW;

        WHEN 'DELETE' THEN
            UPDATE qz.table_action ta
	        SET etag = nextval('qz.etag_seq')
	        WHERE ta.form_name = OLD.form_name;
	       RETURN OLD;
    END CASE;    
END;
$BODY$
LANGUAGE plpgsql;

CREATE TRIGGER table_action_etag_set BEFORE UPDATE
ON qz.form
FOR EACH ROW
EXECUTE PROCEDURE qz.table_action_etag_set();

CREATE TRIGGER page_css_etag_set 
BEFORE UPDATE OR INSERT OR DELETE
ON qz.page_css 
FOR EACH ROW
EXECUTE PROCEDURE qz.table_action_etag_set();

CREATE TRIGGER page_css_etag_set BEFORE UPDATE
ON qz.page_js 
FOR EACH ROW
EXECUTE PROCEDURE qz.table_action_etag_set();

-- use as (for example on table js):
-- CREATE TRIGGER js_etag BEFORE UPDATE ON js 
-- FOR EACH ROW EXECUTE PROCEDURE etag_update();

CREATE OR REPLACE FUNCTION qz.etag_update() 
RETURNS TRIGGER AS $BODY$
BEGIN
    NEW.etag =  nextval('qz.etag_seq'::regclass);
    RETURN NEW;
END;
$BODY$
LANGUAGE plpgsql;

--
-- use as (for example on table js):
-- CREATE TRIGGER js_modtime BEFORE UPDATE ON js 
--   FOR EACH ROW EXECUTE PROCEDURE modtime_update();
--

CREATE OR REPLACE FUNCTION qz.modtime_update()
RETURNS TRIGGER AS $BODY$
BEGIN
    NEW.modtime = now();
    RETURN NEW;
END;
$BODY$
LANGUAGE plpgsql;



CREATE TRIGGER css_etag_update
    BEFORE UPDATE 
        ON qz.css
        FOR EACH ROW EXECUTE PROCEDURE qz.etag_update();


CREATE TRIGGER js_etag_update
    BEFORE UPDATE 
        ON qz.js
        FOR EACH ROW EXECUTE PROCEDURE qz.etag_update();


CREATE TRIGGER doc_etag_update
    BEFORE UPDATE 
        ON qz.doc 
        FOR EACH ROW EXECUTE PROCEDURE qz.etag_update();



CREATE TRIGGER qz_table_action_etag_update
BEFORE UPDATE
ON qz.table_action
FOR EACH ROW
EXECUTE PROCEDURE qz.etag_update();

CREATE TRIGGER qz_prompt_rule_etag_update
BEFORE UPDATE
ON qz.prompt_rule
FOR EACH ROW
EXECUTE PROCEDURE qz.etag_update();

CREATE TRIGGER qz_page_css_etag_update
BEFORE INSERT
ON qz.page_css
FOR EACH ROW
EXECUTE PROCEDURE qz.table_action_etag_set();

CREATE TRIGGER qz_page_js_etag_update
BEFORE INSERT
ON qz.page_js
FOR EACH ROW
EXECUTE PROCEDURE qz.table_action_etag_set();


