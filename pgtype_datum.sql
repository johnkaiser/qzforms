---
--- Given a table name and a column name, return a record
--- describing the attibute. Data is extracted from the
--- PostgreSQL pg_catalog and the SQL information_schema
---
--- The function
--- get_pgtype_datum(table_name, column_name)
--- will return a record of type pg_type_datum
---
--- Copyright (c) John Kaiser, http://qzforms.com
--- October 28, 2015
---

CREATE TYPE qz.pg_type_datum
AS (
    version int,

    -- some names from and values equal to information schema columns
    table_schema name,
    table_name name,
    column_name name,
    ordinal_position int,
    column_default text,
    is_nullable boolean,

    -- some from system catalog pg_type
    typname name,
    typtype char,
    typdelim char,

    -- a few checks for the base type in typtype
    is_base_type boolean,
    is_boolean boolean,
    is_composite boolean,
    is_domain boolean,
    is_enum boolean,
    is_pseudo_type boolean,

    enum_labels name[],

    character_maximum_length integer,
    character_octet_length integer,
    numeric_precision integer,
    numeric_precision_radix integer,
    numeric_scale integer,
    datetime_precision integer,

    typcategory_name name,

	domain_catalog name,
    domain_schema name,
    domain_name name,
	
	constraint_catalog name,
	constraint_schema name,
	constraint_name name,
	domain_check_clause text,

    udt_schema name,
    udt_name name,
    is_updatable boolean,

    description text,
	composite_attributes name[],

    has_fkey boolean,
    fkey_schema text,
    fkey_table text,
    fkey_attribute text
);


-- Return pg_type_datum record for the given column

CREATE OR REPLACE FUNCTION 
    qz.get_pgtype_datum(table_name name, column_name name)

RETURNS qz.pg_type_datum

AS $$
DECLARE 

   new_datum qz.pg_type_datum;
   pg_type_oid oid;
   pg_class_oid oid;
   pga_attnotnull boolean;
   pgt_typnotnull boolean;
   pgt_typcategory name;
   noop boolean;

BEGIN
     
    new_datum.version := 3;

    -- For the row type
    SELECT pg_type.typname, pg_namespace.nspname 
        FROM pg_type, pg_namespace
        WHERE pg_type.typrelid = table_name::regclass
            AND pg_type.typnamespace = pg_namespace.oid
        INTO new_datum.table_name, new_datum.table_schema; 

    -- column name
    new_datum.column_name := column_name;

    -- For the column
    -- saving the type oid for use further down
    SELECT attnum, atttypid, attnotnull, attrelid
        FROM pg_attribute
        WHERE attrelid = table_name::regclass
            AND attname = column_name
        INTO new_datum.ordinal_position, pg_type_oid, pga_attnotnull, pg_class_oid;

    -- Attribute default
    SELECT  pg_get_expr(pg_attrdef.adbin, pg_attrdef.adrelid) 
        FROM pg_attribute JOIN pg_attrdef 
        ON pg_attribute.attrelid = pg_attrdef.adrelid
            AND pg_attribute.attnum = pg_attrdef.adnum
        WHERE pg_attribute.attrelid = table_name::regclass
            AND pg_attribute.attname = column_name
        INTO new_datum.column_default;

    -- data type
    -- saving type attributes for is nullable
    SELECT typname, typtype, typnotnull, typcategory
        FROM pg_type
        WHERE pg_type.oid = pg_type_oid
        INTO new_datum.typname, new_datum.typtype, pgt_typnotnull, pgt_typcategory;

    -- give names to the categories
    CASE pgt_typcategory
        WHEN 'A' THEN new_datum.typcategory_name := 'array';
        WHEN 'B' THEN new_datum.typcategory_name := 'boolean';
        WHEN 'C' THEN new_datum.typcategory_name := 'composite';
        WHEN 'D' THEN new_datum.typcategory_name := 'date_time';
        WHEN 'E' THEN new_datum.typcategory_name := 'enum';
        WHEN 'G' THEN new_datum.typcategory_name := 'geometric';
        WHEN 'I' THEN new_datum.typcategory_name := 'network_address';
        WHEN 'N' THEN new_datum.typcategory_name := 'numeric';
        WHEN 'P' THEN new_datum.typcategory_name := 'pseudo';
        WHEN 'S' THEN new_datum.typcategory_name := 'string';
        WHEN 'T' THEN new_datum.typcategory_name := 'timespan';
        WHEN 'U' THEN new_datum.typcategory_name := 'user_defined';
        WHEN 'V' THEN new_datum.typcategory_name := 'bit_string';
        WHEN 'X' THEN new_datum.typcategory_name := 'unknown';
        ELSE new_datum.typcategory_name := 'undefined';
    END CASE;        

    -- is nullable, from information_schema columns
    -- typtype d is a domain 
    IF (pga_attnotnull OR ((new_datum.typtype = 'd'::"char") AND pgt_typnotnull)) THEN 
        new_datum.is_nullable := false;
    ELSE
        new_datum.is_nullable := true;
    END IF;

    -- From the information schema columns view
    -- describe numeric parameters.
    -- type schema and name
    SELECT character_maximum_length, character_octet_length, 
        numeric_precision, numeric_precision_radix, 
        numeric_scale, datetime_precision,
        domain_catalog, domain_schema, domain_name,
        udt_schema, udt_name,
        (is_updatable = 'YES')::boolean 
        FROM information_schema.columns
        WHERE table_schema = new_datum.table_schema
            AND information_schema.columns.table_name = new_datum.table_name
            AND information_schema.columns.column_name = new_datum.column_name
        INTO new_datum.character_maximum_length, 
		    new_datum.character_octet_length,
            new_datum.numeric_precision, 
			new_datum.numeric_precision_radix, 
            new_datum.numeric_scale, 
			new_datum.datetime_precision,
            new_datum.domain_catalog, 
			new_datum.domain_schema, 
			new_datum.domain_name,
            new_datum.udt_schema, 
			new_datum.udt_name,
            new_datum.is_updatable;

    -- From the information schema check constraints
	-- get the check clause,
	-- but first, get the constraint name
    SELECT constraint_catalog, constraint_schema, constraint_name 
	    FROM information_schema.domain_constraints
		WHERE   domain_catalog = new_datum.domain_catalog
		    AND domain_schema  = new_datum.domain_schema
		    AND domain_name    = new_datum.domain_name
        INTO new_datum.constraint_catalog, 
		     new_datum.constraint_schema,
			 new_datum.constraint_name;

    SELECT check_clause
	    FROM information_schema.check_constraints
		WHERE   constraint_catalog = new_datum.constraint_catalog
		    AND constraint_schema  = new_datum.domain_schema
		    AND constraint_name    = new_datum.constraint_name
        INTO new_datum.domain_check_clause;

    -- set the base type tests
    new_datum.is_base_type := false;
    new_datum.is_boolean := false;
    new_datum.is_composite := false;
    new_datum.is_domain := false;
    new_datum.is_enum := false;
    new_datum.is_pseudo_type := false;

    CASE new_datum.typtype
        WHEN 'b' THEN
            new_datum.is_base_type := true;
        WHEN 'c' THEN 
            new_datum.is_composite := true;
        WHEN 'd'  THEN
            new_datum.is_domain := true;
        WHEN 'e' THEN
            new_datum.is_enum := true;
            SELECT array(
                SELECT enumlabel
                    FROM pg_attribute, pg_enum
                    WHERE pg_attribute.attrelid = table_name::regclass
                        AND   pg_attribute.attname = column_name
                        AND   pg_attribute.atttypid = pg_enum.enumtypid
                    ORDER BY pg_enum.enumsortorder
            ) INTO new_datum.enum_labels;
            
        WHEN 'p' THEN
            new_datum.is_pseudo_type := true;
        ELSE noop := true;
    END CASE;

    -- check for boolean
    CASE new_datum.udt_name
        WHEN 'bool' THEN
            new_datum.is_boolean := true;
        ELSE noop := true;    
    END CASE;

    -- The description
    SELECT description 
        FROM pg_description
        WHERE objoid = table_name::regclass
            AND classoid = 'pg_class'::regclass
            AND objsubid = new_datum.ordinal_position
        INTO new_datum.description;    

    IF new_datum.is_composite THEN
	    SELECT array(
		    SELECT attname 
			    FROM pg_attribute
			    WHERE attrelid = new_datum.typname::regclass
				ORDER BY attnum
         ) INTO new_datum.composite_attributes;
    END IF;

    -- Foreign Key
    new_datum.has_fkey = false;

    IF new_datum.ordinal_position > 0 THEN
        SELECT ns.nspname, cl.relname, at.attname, at.attname is not null 
        FROM
            pg_namespace ns, pg_class cl, pg_attribute at,
            (SELECT
                z.cfk[s] attrib, fk_table tbl
             FROM
                 (SELECT c.conkey ck, c.confkey cfk, 
                     generate_subscripts(c.conkey,1) s, c.confrelid fk_table
                  FROM pg_constraint c
                  WHERE c.conrelid = pg_class_oid and c.contype = 'f'
                 ) z
             WHERE z.ck[s] = new_datum.ordinal_position
            ) fk
        WHERE cl.oid = tbl
        AND ns.oid = cl.relnamespace
        AND at.attrelid = tbl and attnum = attrib
        INTO new_datum.fkey_schema, new_datum.fkey_table, 
            new_datum.fkey_attribute, new_datum.has_fkey;
      

    END IF;
    RETURN new_datum;


END; $$ LANGUAGE plpgsql;
