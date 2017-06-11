INSERT INTO qz.change_history
   (change_description,note)
   VALUES
   ('Update to SV9',
    ''
    );

 UPDATE qz.constants
 SET schema_version = '9';

-- Cleanup from ripping out jquery.
DELETE FROM qz.page_js WHERE filename = 'document_ready.js';
DELETE FROM qz.js WHERE filename = 'document_ready.js';

-- Adding action to form_mgt context parameters caused the inline_(js|css)
-- edit functions to barf an error making them unusable for the 2nd click.
UPDATE qz.form_set
SET context_parameters = '{form_name,handler_name}'
WHERE set_name = 'form_mgt';


-- From ripping out jquery, this must change
UPDATE qz.table_action
SET inline_js = $TAINLJS$ function menu_click(on_item){

      var table_list = document.getElementsByClassName("qztable");
      var i;
      for (i = 0; i < table_list.length; i++){
          if (table_list[i].id == on_item){
              table_list[i].style.display = "block";
          }else{
              table_list[i].style.display = "none";
          }
      }
       
}

function hide_sections(){
    // $(".qztable").hide();

    var table_list = document.getElementsByClassName("qztable");
    var i;
    for (i = 0; i < table_list.length; i++){
        table_list[i].style.display = "none";
    }

}

document.addEventListener("DOMContentLoaded", hide_sections, false);
$TAINLJS$ 
WHERE form_name = 'status'
AND action = 'view';

--
-- waiting is not present in PG 9.6
--
UPDATE qz.table_action
SET sql = $TAPGSA$ SELECT datname,pid,usename,application_name,client_addr,backend_start, query_start,query FROM pg_stat_activity $TAPGSA$
WHERE form_name = 'status'
AND action = 'pg_stat_activity';

--
-- Since changing the Postgresql version can break things,
-- show in on the status page.
--
INSERT INTO qz.table_action
(form_name, action, sql)
VALUES
('status', 'pg_version', 'SELECT version()');
