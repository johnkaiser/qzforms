/*
 *  form_refresh
 *
 *  Search the forms in the curren document, look for a name "form_tag".
 *  Call a post request with the form_tags as post data to /qz/refresh
 */

function form_refresh(){
    console.log("form_refresh");

    var f;
    var form_id;
    var form_tag;
    var nel;
    var nf;
    var postdata;
    var refresh = "/" + window.location.pathname.split('/')[1] + "/refresh";
    var request = new Array();
    var rcnt = 0;

    for (nf=0; nf < document.forms.length; nf++){
        f = document.forms[nf];

        for(nel=0; nel < f.elements.length; nel++){
            if ((f.elements[nel].name == "form_tag") &&
                (f.elements[nel].getAttribute('refresh') == 1) ){

                form_id = 'form_id%5B'+String(rcnt) + '%5D=' +
                    encodeURIComponent(f.id);

                form_tag = 'form_tag%5B'+String(rcnt) + '%5D=' +
                    encodeURIComponent(f.elements[nel].value);

                request[rcnt] = ( form_id+"&"+form_tag );
                console.log("refresh " + request[rcnt]);

                rcnt++;
            }
        }
    }
    httpRequest = new XMLHttpRequest();
    httpRequest.onreadystatechange = refresh_result;
    httpRequest.open("POST", refresh);
    httpRequest.send( request.join('&') );
}
