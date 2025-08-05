
/* 
 *  Given a callback button element and a js function to catch the result,
 *
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
function setup_callback(callback_el, func, xhr, args={}){

   console.log('setup_callback for ' + callback_el.value);

   if ( ! callback_el){
      console.log('setup_callback: callback_el is false');
      return;
   }
   // Build a www-form-urlencoded string for post data.
   // The form_tag is a token authorizing the action.

   let form_tag = "form_tag=" + 
       encodeURIComponent(callback_el.getAttribute('x-form_tag'));

   let formaction = callback_el.getAttribute('formaction');
   console.log('setup_callback: callback_el.id = ' + callback_el.id);
   console.log('setup_callback: formaction = ' + formaction);

   let form_fields = form_tag;
 
   //  The callback object will have a list of fields for the
   //  query input data. 

   let fn_len = 0;
   let fieldnames = false;
   try{
        let xfieldnames = callback_el.attributes.getNamedItem('x-fieldnames');
        console.log("setup_callback: xfieldnames = " + xfieldnames.value);

        fieldnames = JSON.parse(xfieldnames.value.replace(/\'/g, '"'));

        if (fieldnames) fn_len = fieldnames.length;
    }catch(e){
        console.log(e);
    }

    var form_present = (callback_el.form !== undefined) &&
           (callback_el.form !== null) &&
           (callback_el.form.name !== undefined) &&
           (callback_el.form.name !== null) ;

   if ( form_present ){
       var form_name = callback_el.form.name;

       console.log("form " + form_name + " is present");

       for (let n=0; n < fn_len; n++){
           let fn = fieldnames[n];
           console.log("fieldname n="+n + " fn="+fn);

           // args take precendence over form data
           if ( ! (fn in args)){
               // find the fieldname in the form
               let el = callback_el.form[fn];
               if (el){
                   args[fn] = el.value;
                   console.log(fn + " = " + args[fn] + " tagname = " + el.tagName );
               }else{
                   console.log("element " + fn + " not found");
               }
           }
       }
   } else {
        console.log("form not present");
   }
   // Turn the args object into form urlencoded data for posting

   let arg_keys = Object.getOwnPropertyNames(args);
   for (let k=0; k < arg_keys.length; k++){
       form_fields += "&" + encodeURIComponent(arg_keys[k]) + "=" + 
           encodeURIComponent(args[arg_keys[k]]);
   }

    // create and post the request
    xhr.onload = func;
    // xhr.callback_name = callback_name;
    // xhr.form_name = form_name;
    xhr.args = args;
    xhr.open("POST", formaction);
    xhr.setRequestHeader('Content-Type',"application/x-www-form-urlencoded");
    xhr.send(form_fields);

    console.log('sent '+form_fields);
}


