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

