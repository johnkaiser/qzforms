
## One source of randomness must be selected.

## Use arc4random for BSD systems that support arc4random()
## QZRANDOM=-DQZ_ARC4RANDOM

## Use getrandom system call for Linux systems with kernel > 4.17
## QZRANDOM=-DQZ_GETRANDOM

## Use a device file (or perhaps a unix domain socket). 
## QZRANDOM=-DQZ_RAND_DEV=\"/dev/urandom\"

## Optionally choose either gcc or clang for the compiler
## CC=clang
## CC=gcc

## It has happened that "-L/usr/local/lib" is required here.
#LFLAGS=-L/usr/local/lib -lfcgi -lpthread -lcrypto -lxml2 
LFLAGS=-lfcgi -lpthread -lcrypto -lxml2 

## End of common changes.

PGINCLUDEDIR!=pg_config --includedir
PGLIBDIR!=pg_config --libdir

XMLCFLAGS!=xml2-config --cflags
XMLLIBDIR!=xml2-config --libs

PCRECFLAGS!=pcre-config --cflags
PCRELIBS!=pcre-config --libs

CFLAGS=-Wall \
	-I/usr/local/include \
	-Wno-pointer-sign \
	-ggdb  \
	-fPIC \
	$(XMLCFLAGS) \
	-I$(PGINCLUDEDIR)

VERSION!=cat Version
SCHEMA_VERSION=12

OBJ=qzhandlers.o onetable.o \
	str_to_array.o session.o login.o  cookie.o\
	input.o output.o menu.o utility.o \
	parse_key_eq_val.o status.o opentable.o parse_pg_array.o qzfs.o \
	pgtools.o qzrandom.o crypto_etag.o tagger.o \
	hex_to_uchar.o qzconfig.o gettime.o form_tag.o prompt_rule.o \
	grid.o form_set.o docs.o callback.o json-builder.o

FILES=Makefile qz.h qzforms.conf Version qzforms_install.sh \
	templates/base.xml templates/login.xml templates/tinymce.xml \
	qzforms.init README.txt \
	http_codes.h qzrandom.h crypto_etag.h \
	qzmain.c qzhandlers.c onetable.c callback.c \
	str_to_array.c session.c login.c cookie.c \
	input.c output.c menu.c utility.c \
	parse_key_eq_val.c status.c opentable.c parse_pg_array.c qzfs.c \
	pgtools.c qzrandom.c crypto_etag.c tagger.h tagger.c \
	hex_to_uchar.h hex_to_uchar.c qzconfig.c qzconfig.h gettime.c \
	form_tag.c prompt_rule.c grid.c form_set.c docs.c logview.py \
    json-parser/json.c json-parser/json.h \
    json-parser/LICENSE json-parser/AUTHORS \
    json-builder/json-builder.c json-builder/json-builder.h \
    json-builder/LICENSE json-builder/AUTHORS \

SQL=0_init.sql 1_handler.sql 2_objects.sql 3_table_action.sql \
	4_prompt_rule.sql 5_jscss.sql 6_jscss_data.sql 7_menu.sql \
	8_templates.sql 9_functions.sql  pgtype_datum.sql comment.sql

SQLUTIL=qz_db_update_SV3.sql qz_db_update_SV4.sql \
	qz_db_update_SV5.sql qz_db_update_SV6.sql qz_db_update_SV7.sql \
	qz_db_update_SV8.sql qz_db_update_SV9.sql qz_db_update_SV10.sql \
	qz_db_update_SV11.sql qz_db_update_SV12.sql qz_db_update_SV13.sql \

EXAMPLES=examples/release_checklist.sql \
	examples/release_checklist_data.sql \
	examples/stuff_and_gridle.sql \
	examples/todo.sql \
	examples/todo_data.sql

JS=js/add_array_input.js js/add_button.js js/add_input_hidden.js \
	js/add_input_radio.js js/add_input_text.js js/add_prompt.js \
	js/add_select_options.js js/add_text_area.js js/base64_attribs.js\
	js/change_status.js js/form_refresh.js js/form_refresh_init.js \
	js/get_next_row_index.js js/grid_add_row.js js/grid_delete_row.js \
	js/httpRequest.js js/refresh_result.js js/set_common_attributes.js \
	js/set_action_options.js js/dollarquote js/tinymce.init.js \
	get_callbacks.js qz_callback.jk

DOCS=COPYRIGHT.txt opentable.txt design_principles.html login_process.html

TESTS=tests/delete_simplelist.py tests/create_simplegrid.py \
	tests/qztest.py tests/create_simplelist.py tests/delete_simplegrid.py \
	tests/testdata.sql tests/vg

# The random test is first so make fails early without randomness source
all: test_qzrandom qzforms.fcgi \
	qz_db_install_SV$(SCHEMA_VERSION).sql \
	qzforms_examples.sql

qzforms.fcgi: $(OBJ) qzmain.o
	$(CC)   -o qzforms.fcgi $(OBJ) qzmain.o \
		$(CFLAGS) $(LFLAGS) \
		-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
		$(XMLLIBDIR) $(PCRELIBS) \
		-lpq

qzmain.o: qzmain.c qz.h
	$(CC) $(CFLAGS)  -DQZVER="$(VERSION)" -DSCHEMA_VER=$(SCHEMA_VERSION) \
	-c qzmain.c

qzhandlers.o: qzhandlers.c qz.h
	$(CC) $(CFLAGS) -c qzhandlers.c

str_to_array.o: str_to_array.c 
	$(CC) $(CFLAGS) -c str_to_array.c

test_str_to_array: str_to_array.c
	$(CC) $(CFLAGS) -D STR_TO_ARRAY_MAIN  str_to_array.c -o test_str_to_array

session.o: session.c
	$(CC) $(CFLAGS) -c session.c

session_test: session.c gettime.o crypto_etag.o tagger.o qzrandom.o \
	hex_to_uchar.o cookie.o parse_key_eq_val.o utility.o qzconfig.o  \
	opentable.o parse_pg_array.o
	$(CC) $(CFLAGS) $(XMLLIBDIR) -L$(PGLIBDIR) $(LFLAGS) -DSESSION_MAIN \
	-L$(PCRELIBS) \
	session.c gettime.o crypto_etag.o tagger.o qzrandom.o \
	hex_to_uchar.o cookie.o parse_key_eq_val.o utility.o qzconfig.o \
	opentable.o parse_pg_array.o  form_set.o pgtools.o form_tag.o \
	str_to_array.o login.o prompt_rule.o output.o \
	input.o menu.o qzhandlers.o status.o qzfs.o onetable.o grid.o \
	-lpq -o session_test

onetable.o: onetable.c qz.h
	$(CC) $(CFLAGS) -c onetable.c

login.o: login.c qz.h
	$(CC) $(CFLAGS) -c login.c

menu.o: menu.c qz.h
	$(CC) $(CFLAGS) -c menu.c

output.o: output.c qz.h
	$(CC) $(CFLAGS) -c output.c

utility.o: utility.c qz.h
	$(CC) $(CFLAGS) -c utility.c

array_base_test: utility.c qz.h
	$(CC) $(CFLAGS) \
	qzhandlers.o onetable.o \
	str_to_array.o session.o login.o  cookie.o\
	input.o output.o  menu.o \
	parse_key_eq_val.o status.o opentable.o parse_pg_array.o qzfs.o \
	pgtools.o qzrandom.o crypto_etag.o tagger.o \
	hex_to_uchar.o qzconfig.o gettime.o form_tag.o prompt_rule.o \
	grid.o form_set.o \
	-DARRAY_BASE_TEST utility.c \
	$(XMLLIBDIR) $(PCRELIBS) -lcrypto -lpq -lfcgi \
	-o array_base_test

get_random_key.o: get_random_key.c qz.h
	$(CC) $(CFLAGS) -c get_random_key.c

cookie.o: cookie.c qz.h
	$(CC) $(CFLAGS) -c cookie.c

parse_key_eq_val.o:  parse_key_eq_val.c qz.h
	$(CC) $(CFLAGS) -c parse_key_eq_val.c

test_parse_key_eq_val: parse_key_eq_val.c qz.h gettime.o
	$(CC) $(CFLAGS) $(LFLAGS) $(XMLLIBDIR) -DPARSE_KEY_EQ_VAL_MAIN \
	parse_key_eq_val.c gettime.o \
	-o test_parse_key_eq_val

status.o: status.c qz.h
	$(CC) $(CFLAGS) -DQZVER="$(VERSION)" \
	-DSCHEMA_VER=\"$(SCHEMA_VERSION)\" -c status.c

opentable.o: opentable.c qz.h
	$(CC) $(CFLAGS) -DSCHEMA_VER=\"$(SCHEMA_VERSION)\" -c opentable.c

opentable_test: opentable.c \
	parse_pg_array.o qz.h gettime.o qzrandom.o pgtools.o
	$(CC) $(CFLAGS) $(LFLAGS) -DSCHEMA_VER=\"$(SCHEMA_VERSION)\" \
	-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
	$(XMLLIBDIR) $(PCRELIBS) \
	-lpq \
	-DOPENTABLE_TEST -g \
	gettime.o pgtools.o parse_pg_array.o output.o qzrandom.o \
	prompt_rule.o tagger.o crypto_etag.o utility.o str_to_array.o grid.o \
	hex_to_uchar.o form_tag.o qzhandlers.o  status.o qzfs.o form_set.o \
	onetable.o menu.o login.o input.o cookie.o session.o parse_key_eq_val.o \
	qzconfig.o \
	opentable.c \
	-o opentable_test

pgtools.o: pgtools.c qz.h
	$(CC) $(CFLAGS) -c pgtools.c

parse_pg_array.o: parse_pg_array.c
	$(CC) $(CFLAGS) -c parse_pg_array.c 

test_parse_pg_array: parse_pg_array.c
	$(CC) $(CFLAGS) $(LFLAGS) \
	-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
	-lpq \
	-DPARSE_PG_ARRAY_MAIN -ggdb \
	parse_pg_array.c \
	-o test_parse_pg_array

qzfs.o: qzfs.c qz.h
	$(CC) $(CFLAGS) -c qzfs.c

qzrandom.o: qzrandom.c qzrandom.h
	$(CC) $(CFLAGS) $(QZRANDOM) -c qzrandom.c

test_qzrandom: qzrandom.c qzrandom.h hex_to_uchar.o
	echo "\n\tIf this fails then edit Makefile and set QZRANDOM\n"
	$(CC) $(CFLAGS) $(QZRANDOM) -lcrypto -DTEST_QZRANDOM  qzrandom.c \
	hex_to_uchar.o \
	-o test_qzrandom

hex_to_uchar.o: hex_to_uchar.c
	$(CC) $(CFLAGS) -c hex_to_uchar.c

hex_to_uchar_test: hex_to_uchar.c gettime.o
	$(CC) $(CFLAGS) -Wstack-protector -DHEX_TO_UCHAR_MAIN hex_to_uchar.c \
	gettime.o -lcrypto -o hex_to_uchar_test

crypto_etag.o: crypto_etag.c crypto_etag.h
	$(CC) $(CFLAGS) -c crypto_etag.c

crypto_etag_test: crypto_etag.c crypto_etag.h qzrandom.o hex_to_uchar.o gettime.o
	$(CC) $(CFLAGS) -DCRYPTO_ETAG_MAIN  crypto_etag.c qzrandom.o \
	hex_to_uchar.o  gettime.o -lcrypto -o crypto_etag_test

tagger.o:tagger.c
	$(CC) $(CFLAGS) -c tagger.c

# launch tagger process and test against it.
tagger_test: tagger.c qzrandom.o crypto_etag.o hex_to_uchar.o qzconfig.o gettime.o
	$(CC) -Wall -g -lcrypto  -DTAGGER_MAIN tagger.c \
	qzrandom.o crypto_etag.o qzconfig.o hex_to_uchar.o gettime.o \
	$(XMLCFLAGS) $(XMLLIBDIR)\
	-o tagger_test

# send tests to a running tagger
test_tagger: tagger.c
	$(CC) -Wall -g -lcrypto  -DTEST_TAGGER tagger.c qzrandom.o crypto_etag.o \
	gettime.o hex_to_uchar.o -o test_tagger

qzconfig.o:qzconfig.c qzconfig.h 
	$(CC) $(CFLAGS)  -Wall -c qzconfig.c 

qzconfig_test:qzconfig.c qzconfig.h qzrandom.o gettime.o
	$(CC) $(CFLAGS) -lcrypto -DQZCONFIG_TEST  qzconfig.c \
	$(XMLLIBDIR) -lxml2 \
	qzrandom.o gettime.o \
	-o qzconfig_test

prompt_rule.o: prompt_rule.c qz.h 
	$(CC) $(CFLAGS) $(PCRECFLAGS)  -Wall -c prompt_rule.c 

test_prompt_rule: prompt_rule.c qz.h 
	$(CC) $(CFLAGS) $(LFLAGS) -lcrypto -DPROMPT_RULE_MAIN  prompt_rule.c \
	qzhandlers.o onetable.o \
	str_to_array.o session.o login.o  cookie.o \
	 input.o output.o menu.o utility.o \
	parse_key_eq_val.o status.o opentable.o parse_pg_array.o qzfs.o \
	pgtools.o qzrandom.o crypto_etag.o tagger.o \
	hex_to_uchar.o qzconfig.o gettime.o form_tag.o grid.o form_set.o \
	-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
	$(XMLLIBDIR) $(PCRELIBS) \
	-lpq \
	-o test_prompt_rule

gettime.o:gettime.c
	$(CC) $(CFLAGS)  -Wall -c gettime.c 
    
form_tag.o: form_tag.c
	$(CC) $(CFLAGS)  -Wall -c form_tag.c 

grid.o: grid.c
	$(CC) $(CFLAGS) -Wall -c grid.c

form_set.o: form_set.c
	$(CC) $(CFLAGS) -Wall -c form_set.c

docs.o: docs.c
	$(CC) $(CFLAGS) -Wall -c docs.c

callback.o: callback.c json-builder.o
	$(CC) $(CFLAGS) -Wall -c callback.c

json-builder.o: json-builder/json-builder.c
	$(CC) $(CFLAGS) -Wall -c json-builder/json-builder.c

id_index_test: input.c
	$(CC) $(CFLAGS) $(LFLAGS) -lcrypto -DID_INDEX_TEST input.c \
	qzhandlers.o onetable.o \
	str_to_array.o session.o login.o  cookie.o \
	 output.o menu.o utility.o form_set.o \
	parse_key_eq_val.o status.o opentable.o parse_pg_array.o qzfs.o \
	pgtools.o qzrandom.o crypto_etag.o tagger.o \
	hex_to_uchar.o qzconfig.o gettime.o form_tag.o grid.o prompt_rule.o \
	-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
	$(XMLLIBDIR) $(PCRELIBS) \
	-lpq \
	-o id_index_test
inc:
	echo $(VERSION)"+0.001"|bc > Version.new
	printf "%.3f\n" `cat Version.new` > Version
	cat Version

qzforms_examples.sql: $(EXAMPLES)
	cat examples/release_checklist.sql > qzforms_examples.sql
	cat examples/release_checklist_data.sql >> qzforms_examples.sql
	cat examples/stuff_and_gridle.sql >> qzforms_examples.sql
	cat examples/todo.sql >> qzforms_examples.sql
	cat examples/todo_data.sql >> qzforms_examples.sql

qzforms.js.sql: $(JS) 
	echo "UPDATE qz.js SET data = "  > qzforms.js.sql
	cat js/dollarquote              >> qzforms.js.sql
	cat js/httpRequest.js           >> qzforms.js.sql
	cat js/base64_attribs.js        >> qzforms.js.sql
	cat js/refresh_result.js        >> qzforms.js.sql
	cat js/form_refresh.js          >> qzforms.js.sql
	cat js/form_refresh_init.js     >> qzforms.js.sql
	cat js/set_common_attributes.js >> qzforms.js.sql
	cat js/add_array_input.js       >> qzforms.js.sql
	cat js/add_button.js            >> qzforms.js.sql
	cat js/add_input_hidden.js      >> qzforms.js.sql
	cat js/add_input_radio.js       >> qzforms.js.sql
	cat js/add_input_text.js        >> qzforms.js.sql
	cat js/add_select_options.js    >> qzforms.js.sql
	cat js/add_text_area.js         >> qzforms.js.sql
	cat js/add_prompt.js            >> qzforms.js.sql
	cat js/change_status.js         >> qzforms.js.sql
	cat js/get_next_row_index.js    >> qzforms.js.sql
	cat js/grid_add_row.js          >> qzforms.js.sql
	cat js/grid_delete_row.js       >> qzforms.js.sql
	cat js/set_action_options.js    >> qzforms.js.sql
	cat js/tinymce.init.js          >> qzforms.js.sql
	cat js/get_callbacks.js         >> qzforms.js.sql
	cat js/qz_callback.js           >> qzforms.js.sql
	cat js/dollarquote              >> qzforms.js.sql
	echo " WHERE filename = 'qzforms.js';" >> qzforms.js.sql

qz_db_install_SV$(SCHEMA_VERSION).sql : $(SQL) qzforms.js.sql
	echo "BEGIN;" > qz_db_install_SV$(SCHEMA_VERSION).sql
	cat $(SQL) >> qz_db_install_SV$(SCHEMA_VERSION).sql
	cat qzforms.js.sql >> qz_db_install_SV$(SCHEMA_VERSION).sql
	echo "COMMIT;" >> qz_db_install_SV$(SCHEMA_VERSION).sql

tar:
	tar -cz -s '|^|qzforms-$(VERSION)/|' -f qzforms-$(VERSION).tgz \
    $(FILES) $(SQL) $(SQLUTIL) $(DOCS) $(JS) $(EXAMPLES) $(TESTS)

clean:
	rm -f $(OBJ) qzmain.o qzforms.fcgi qzforms.core test_parse_pg_array \
	testopentable test_qzrandom crypto_etag_test test_prompt_rule \
	hex_to_uchar_test qzforms.js.sql qz_db_install_SV$(SCHEMA_VERSION).sql \
	qzforms_examples.sql
