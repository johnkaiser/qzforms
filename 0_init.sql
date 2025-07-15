
 --  Copyright (c) John Kaiser, http://qzforms.com
 --  All rights reserved.
 --  
 --  Redistribution and use in source and binary forms, with or without
 --  modification, are permitted provided that the following conditions
 --  are met:
 --  
 --  1. Redistributions of source code must retain the above copyright 
 --  notice, this list of conditions and the following disclaimer.
 --  
 --  2. Redistributions in binary form must reproduce the above copyright 
 --  notice, this list of conditions and the following disclaimer in the 
 --  documentation and/or other materials provided with the distribution.
 --  
 --  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 --  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 --  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 --  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 --  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 --  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 --  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 --  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 --  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 --  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 --  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 --  POSSIBILITY OF SUCH DAMAGE.



CREATE SCHEMA IF NOT EXISTS qz;

--
-- Types
--
CREATE DOMAIN qz.variable_name AS VARCHAR(63) 
CHECK(
   VALUE !~  '([[:cntrl:]]| |\!|\"|#|\$|%|&|[\x027]|[\(]|[\)]|\*|\+|,|\-|\.|/|\:|\;|<|=|>|\?|@|\[|\/|\]|\^|`|\{|\||\}|\~|\x07f)'
   AND
   VALUE ~ '^[[:alpha:]]'
   AND char_length(VALUE) < 64

);

CREATE DOMAIN qz.file_name AS VARCHAR(63)
CHECK (
   VALUE !~  '([[:cntrl:]]| |\!|\"|#|\$|%|&|[\x027]|[\(]|[\)]|\*|\+|,|\-|/|\:|\;|<|=|>|\?|@|\[|\/|\]|\^|`|\{|\||\}|\~|\x07f)'
   AND 
   VALUE !~ '\.\.'
   AND char_length(VALUE) < 64
);

CREATE TYPE qz.prompt_container_type 
    AS ENUM ('no_container','fieldset');

create type qz.prompt_types as enum (
    'input_hidden', 'input_text', 'select_options', 'select_fkey',
    'input_radio', 'textarea', 'text_array', 'button'
);

-- etag values less than 2^16 are reserved for use by the distribution.
CREATE SEQUENCE qz.etag_seq START 65537;

CREATE TABLE qz.constants (
    key boolean PRIMARY KEY CONSTRAINT one_row CHECK(key),
    schema_version int
);

CREATE TYPE qz.callback_response_type AS ENUM
  ('qzforms_json', 'postgresql_json', 'plain_text', 'html_table');

--
-- Set the Schema Version
--
INSERT INTO qz.constants (key, schema_version) VALUES ('t', '14');

CREATE TABLE qz.change_history (
    change_id serial primary key,
    changed timestamp DEFAULT now(),
    changed_by text DEFAULT CURRENT_USER,
    change_description text,
    note text
);

INSERT INTO qz.change_history 
  (change_description) 
  VALUES 
  ('First install Schema Version '||
    (SELECT schema_version FROM qz.constants));


