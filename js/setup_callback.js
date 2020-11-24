
/* 
 *  Look through script id __CALLBACKS__ in the variable callbacks
 *  for callback_name. 
 *  Pass func to XMLHttpRequest.
 *  Optionaly include an object that will be the preferred
 *  source for the callback parameters.
 *  Named callback parameters not in args will be searched for
 *  in the form's elements.
 *
 *  Initialize xhr in the same context as func as:
 *      var xhr = new XMLHttpRequest();
 *
 */
function setup_callback(callback_name, func, xhr, args={}){

   const callbacks = get_callbacks();

   // Build a www-form-urlencoded string for post data.
   // The form_tag is a token authorizing the action.

   let form_tag = "form_tag=" + 
       encodeURIComponent(callbacks[callback_name]['form_tag']);

   let form_action = callbacks[callback_name]['form_action'];
   let form_fields = form_tag;
 
   //  The callback object will have a list of fields for the
   //  query input data. 

   let fieldnames = callbacks[callback_name]['fieldnames'];
   let fn_len = fieldnames.length;
   var form_name = callbacks[callback_name]['form_name'];
   let postdata = {};

   for (let n=0; n < fn_len; n++){
       let fn = fieldnames[n];
       console.log("n="+n + " fn="+fn);

       if (fn in args){
           // args take precendence over form data
           postdata[fn] = args[fn];
       }else{
           // find the fieldname in the form
           let el =  document.forms[form_name][fn];
           if (el){
               args[fn] = el.value;
               console.log(fn + " = " + args[fn] + " tagname = " + el.tagName );
           }else{
               console.log("element " + fn + " not found");
           }
       }
   }
   // Turn the args object into form urlencoded data for posting

   let arg_keys = Object.getOwnPropertyNames(args);
   for (let k=0; k < arg_keys.length; k++){
       form_fields += "&" + encodeURIComponent(arg_keys[k]) + "=" + 
           encodeURIComponent(args[arg_keys[k]]);
   }

    // create and post the request
    xhr.onload = func;
    xhr.callback_name = callback_name;
    xhr.form_name = form_name;
    xhr.args = args;
    xhr.open("POST", form_action);
    xhr.setRequestHeader('Content-Type',"application/x-www-form-urlencoded");
    xhr.send(form_fields);

    console.log('sent '+form_fields);
}


