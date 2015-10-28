
/* 
 * Copyright (c) John Kaiser, http://qzforms.com
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright 
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

/*
 *  This does not rely on indexes or hashes.
 *  Just walk the tree once.  
 *  Only efficient because it needs to run just once.
 *  Also, may not account for the madness when id is not id.
 */

#include "qz.h"

xmlNodePtr qzGetElementByID(struct handler_args* h, 
               xmlNodePtr cur, xmlChar* id){ 
 
    xmlNodePtr isit;
    xmlNodePtr child;
   
    if( cur == NULL) return NULL;

    // found it here.
    if ( xmlStrcmp( xmlGetProp(cur,"id"), id ) == 0){ 

        return cur; 
    }

    child = cur->xmlChildrenNode;
    while (child != NULL){
        if ((xmlIsBlankNode(child)) == 0){ // not a blank text node

             isit = qzGetElementByID(h, child, id);
             // found it in a child
             if (isit != NULL) return isit;
         }
         child = child->next; 
    }
     
    return NULL;
}
