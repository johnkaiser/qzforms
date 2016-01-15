
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
VALUES ('login_process.css', 'text/css', '2014-08-10 12:34:54',
$LP$
table {
    border-collapse: collapse;
    border: 2pt solid black;
}

th {
    border: 1pt solid grey;
    font-weight: bold;
    padding: 2pt;
}
td {
    border: 1pt dotted grey;
    font-size: 85%;
    text-align: center;
    padding: 2pt;
}
i {
    color: blue;
}

dd {
   margin-bottom: 1.0ex;
}
$LP$);

INSERT INTO qz.css (filename, mimetype, modtime, data) 
VALUES ('qzforms.css', 'text/css', '2015-05-26 20:43:16',
$QZF$
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

input.menu_button {
    background: white;
    font-weight: bold;
}

#qzmenu {
    padding-bottom: 1ex;
    border-bottom: 2pt dotted grey;
}    

.err_msg {
    background: yellow;
    border: 2pt solid red;
    padding: 3pt;
    display: table;
    font-weight: bold;
}

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
$QZF$);

--
-- js
--

-- data is set by an update script from qzforms.js.sql
INSERT INTO qz.js (filename, mimetype) 
VALUES ('qzforms.js', 'text/javascript; charset=utf-8');

INSERT INTO qz.js (filename, mimetype, modtime, data) 
VALUES ('document_ready.js', 'text/javascript', '2015-06-01 20:48:14',
$DR$
  $(document).ready(function() 
    { 
        $("#pg_stat_activity").tablesorter(); 
        $("#table_action").tablesorter();
        $("#counters").tablesorter();
        $("#open_tables").tablesorter();
        $("#getall").tablesorter();
        $("#listmgr").tablesorter();
        $("#form_tags").tablesorter();
        $(".qztable").hide();
        console.log('document ready executed');
    } 

  );

  function menu_click(on_item){
      var this_one = "#"+on_item;
      $(".qztable").hide();
	  $(this_one).show();
  }
  function show_pgtype(id){
      var el, attrib;
	  el = document.getElementById(id);
	  attrib = el.getAttribute("pgtype");
	  alert("pgtype_datum="+decodeURIComponent(attrib));
  }
$DR$);






