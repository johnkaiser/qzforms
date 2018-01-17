
CREATE TABLE qz.css (
    filename text PRIMARY KEY,
    mimetype text,
    modtime timestamp,
    etag bigint not null default nextval('qz.etag_seq'::regclass),
    data text );

CREATE TABLE qz.js (
    filename text PRIMARY KEY,
        mimetype text,
        modtime timestamp,
        etag bigint not null default nextval('qz.etag_seq'::regclass),
        data text );

CREATE TABLE qz.doc (
    filename text PRIMARY KEY,
        mimetype text,
        modtime timestamp,
        etag bigint not null default nextval('qz.etag_seq'::regclass),
        data text );

CREATE TABLE qz.page_css (
    form_name qz.variable_name NOT NULL,
    sequence integer NOT NULL,
    filename text REFERENCES qz.css(filename),
    PRIMARY KEY (form_name, sequence)
);


CREATE TABLE qz.page_js (
    form_name qz.variable_name NOT NULL,
    sequence integer NOT NULL,
    filename text REFERENCES qz.js(filename),
    PRIMARY KEY (form_name, sequence)
);
--
-- css
--

INSERT INTO qz.css (filename, mimetype, modtime, data) 
VALUES ('qzforms.css', 'text/css', '2018-01-06 13:49:00',
$QZF$
div.menu {
    display: block;
    width: 100%;
    clear: left;
}
ul.menu {
    list-style-type: none;
    margin: 0;
    padding: 0;
    display: block;
}
li.menu {
   float: left;
   display: inline;
}

#helpful_text {
    display: block;
    clear: both;
    width: 40em;
}

.bold {font-weight:bold;}
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
.err_msg {
    background: yellow;
    border: 2pt solid red;
    padding: 3pt;
    display: table;
    font-weight: bold;
}
$QZF$);

INSERT INTO qz.css (filename, mimetype, modtime, data) 
VALUES ('form_edit.css', 'text/css', '2015-06-22 08:47:51',
$FE$
#pagemenu {
    float:left;
    width: 11em;
}
#pagemenu form{
    display: block;
}

#qz {
   margin-left: 12em;
}

ol#fieldnames {
    counter-reset: item;
    list-style-type: none;
}

ol#fieldnames li:before {
    content: '$' counter(item) '=';
    counter-increment: item;
}

$FE$);


--
-- js
--

-- data is set by an update script from qzforms.js.sql
INSERT INTO qz.js (filename, mimetype) 
VALUES ('qzforms.js', 'text/javascript; charset=utf-8');

INSERT INTO qz.page_js
(form_name, sequence, filename)
VALUES
('page_menus', '1', 'qzforms.js');

INSERT INTO qz.page_css
(form_name, sequence, filename)
VALUES
('page_menus', '1', 'qzforms.css'),
('page_menus', '10', 'form_edit.css');





