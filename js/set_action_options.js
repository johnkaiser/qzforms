/* 
 *  set_action_options
 *
 *  In table action edit, limit the options presented
 *  for the action to those appropriate to the handler.
 */

function set_action_options(){

    var allowed_options = { 
        "onetable": ["view", "list", "create", "insert", "edit", "update", "delete"],
        "grid": ["edit", "save", "insert_row", "update_row", "delete_row"],
        "fs": ["get", "etag_value" ],
        "menupage":["view"],
        "simpleform":["view", "action"]
    };

   var handler_el = document.getElementById('handler_name');

   if (handler_el && (handler_el.value.length > 0)){

       var action_el = document.getElementById("action");
       console.log("set_action_options found "+ action_el.children.length +
           "children");
       while( action_el.children.length > 0 ){
           console.log("Removing " +  action_el.children.item(0).value);
           action_el.children.item(0).remove();
       }
       console.log("handler_name is " + handler_el.value);

       // if handler_el is null, don't go whacking the values above.XXXXXXXXXXX

       var newopt;
       for( newopt in allowed_options[handler_el.value] ){
            // add children options
            
            var opt_txt = allowed_options[handler_el.value][newopt];
            console.log("adding option "+ opt_txt);
            var opt_el = document.createElement("option");
            opt_el.setAttribute("value", opt_txt);
            opt_el.appendChild( 
                document.createTextNode(opt_txt)
            );
           action_el.appendChild(opt_el);
       }
    }
}

