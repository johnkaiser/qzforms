CREATE TABLE public.release_checklist (
"version" numeric(6,3) PRIMARY KEY,
seems_to_work          boolean DEFAULT 'f',
compiles_clean         boolean DEFAULT 'f',
installs_clean         boolean DEFAULT 'f',
simple_test_on_install boolean DEFAULT 'f',
upgrades_clean         boolean DEFAULT 'f',
simple_test_on_upgrade boolean DEFAULT 'f',
git_up_to_date         boolean DEFAULT 'f',
qzforms_com_up_to_date boolean DEFAULT 'f'
);

INSERT INTO qz.form
(form_name, handler_name, schema_name, table_name, xml_template,
 target_div, add_description, prompt_container, pkey
)
VALUES
('release_checklist', 'onetable', 'public', 'release_checklist', 'base.xml',
  'qz', 't', 'fieldset', '{version}'
);

INSERT INTO qz.table_action
(form_name, action, fieldnames, sql)
VALUES
('release_checklist', 'create', '{version}', 
$RCCR$
SELECT $1::text "version",
    'f'::bool seems_to_work,
    'f'::bool compiles_clean,
    'f'::bool installs_clean,
    'f'::bool simple_test_on_install,
    'f'::bool upgrades_clean,
    'f'::bool simple_test_on_upgrade,
    'f'::bool git_up_to_date,
    'f'::bool qzforms_com_up_to_date
$RCCR$
),
('release_checklist', 'insert', 
    '{version, seems_to_work, compiles_clean, installs_clean,
      simple_test_on_install, upgrades_clean, simple_test_on_upgrade,
    git_up_to_date, qzforms_com_up_to_date }', 
$RCINS$
INSERT INTO public.release_checklist
    ("version", 
     seems_to_work,
    compiles_clean,
    installs_clean,
    simple_test_on_install,
    upgrades_clean,
    simple_test_on_upgrade,
    git_up_to_date,
    qzforms_com_up_to_date)
    VALUES
    ($1,$2,$3,$4,$5,$6,$7,$8,$9)
$RCINS$
),
('release_checklist', 'edit', '{version}', 
$RCED$
    SELECT 
    "version",
    seems_to_work,
    compiles_clean,
    installs_clean,
    simple_test_on_install,
    upgrades_clean,
    simple_test_on_upgrade,
    git_up_to_date,
    qzforms_com_up_to_date
    FROM release_checklist
    WHERE "version" = $1
$RCED$
),
('release_checklist', 'list', NULL, 
$RCLI$
    SELECT "version",
    (seems_to_work AND compiles_clean AND installs_clean AND 
    simple_test_on_install AND upgrades_clean AND simple_test_on_upgrade AND 
    git_up_to_date AND qzforms_com_up_to_date) "Complete"
    FROM release_checklist
$RCLI$
),
('release_checklist', 'update', 
'{version, seems_to_work, compiles_clean, installs_clean, 
  simple_test_on_install, upgrades_clean, simple_test_on_upgrade,
  git_up_to_date, qzforms_com_up_to_date}', 
$RCUPD$
    UPDATE release_checklist
    SET
    seems_to_work = $2,
    compiles_clean = $3,
    installs_clean = $4,
    simple_test_on_install = $5,
    upgrades_clean = $6,
    simple_test_on_upgrade = $7,
    git_up_to_date = $8,
    qzforms_com_up_to_date = $9
    WHERE "version" = $1
$RCUPD$
);

INSERT INTO qz.prompt_rule
(form_name, fieldname, prompt_type)
VALUES
('release_checklist','seems_to_work', 'input_radio'),
('release_checklist','compiles_clean', 'input_radio'),
('release_checklist','installs_clean', 'input_radio'),
('release_checklist','simple_test_on_install', 'input_radio'),
('release_checklist','upgrades_clean', 'input_radio'),
('release_checklist','simple_test_on_upgrade', 'input_radio'),
('release_checklist','git_up_to_date', 'input_radio'),
('release_checklist','qzforms_com_up_to_date', 'input_radio')
;

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('release_checklist', '1', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('release_checklist', '1', 'qzforms.css');

INSERT INTO qz.menu_set
(menu_name, host_form_name, action)
VALUES
('main', 'release_checklist', 'any');

INSERT INTO qz.menu_item
(menu_name, menu_item_sequence, target_form_name, action, menu_text)
VALUES
('main', '101', 'release_checklist', 'list', 'Release Checklist');

COMMENT ON COLUMN release_checklist.seems_to_work IS 'QZForms is stable and running well';

COMMENT ON COLUMN release_checklist.compiles_clean IS 'Compiles without warnings on gcc and clang';
COMMENT ON COLUMN release_checklist.installs_clean IS 'Installs without errors';
COMMENT ON COLUMN release_checklist.simple_test_on_install IS 'Test the install by creating a new onetable and grid form';
COMMENT ON COLUMN release_checklist.upgrades_clean   IS 'Test the schema version upgrade script';
COMMENT ON COLUMN release_checklist.simple_test_on_upgrade IS 'Test the upgrade by creating a new onetable and grid form';
COMMENT ON COLUMN release_checklist.git_up_to_date IS 'Github current changes should include Version and a tag for the release';
COMMENT ON COLUMN release_checklist.qzforms_com_up_to_date  IS 'Read the documentaion and update for any recent changes';


