
## One source of randomness must be selected.
QZRANDOM=-DQZ_ARC4RANDOM
#  QZRANDOM=-DQZ_RAND_DEV=\"/dev/arandom\"

LFLAGS=-L/usr/local/lib -lfcgi -lpthread -lcrypto -lxml2 

PGINCLUDEDIR!=pg_config --includedir
PGLIBDIR!=pg_config --libdir

XMLCFLAGS!=xml2-config --cflags
XMLLIBDIR!=xml2-config --libs

PCRECFLAGS!=pcre-config --cflags
PCRELIBS!=pcre-config --libs

CC=clang -Wno-pointer-sign 
#CC=gcc 

CFLAGS=-Wall \
	-I/usr/local/include \
	-ggdb  \
	-fPIC \
	$(XMLCFLAGS) \
	-I$(PGINCLUDEDIR)

VERSION!=cat Version
SCHEMA_VERSION=5

OBJ=qzhandlers.o timestamp.o  onetable.o \
	str_to_array.o qzGetElementByID.o session.o login.o  cookie.o\
	input.o output.o strbuf.o menu.o utility.o strbufs.o \
	parse_key_eq_val.o status.o opentable.o parse_pg_array.o qzfs.o \
	pgtools.o qzrandom64.o crypto_etag.o tagger.o \
	hex_to_uchar.o qzconfig.o gettime.o form_tag.o prompt_rule.o \
	grid.o

FILES=Makefile qz.h qzforms.conf Version qzforms_install.sh \
	templates/base.xml templates/login.xml qzforms.init README.txt \
	strbuf.c strbuf.h \
	http_codes.h qzrandom64.h crypto_etag.h \
	qzmain.c qzhandlers.c timestamp.c  onetable.c \
	str_to_array.c qzGetElementByID.c session.c login.c cookie.c \
	input.c output.c menu.c utility.c strbufs.c \
	parse_key_eq_val.c status.c opentable.c parse_pg_array.c qzfs.c \
	pgtools.c qzrandom64.c crypto_etag.c tagger.h tagger.c \
	hex_to_uchar.h hex_to_uchar.c qzconfig.c qzconfig.h gettime.c \
	form_tag.c prompt_rule.c grid.c


SQL=0_init.sql 1_handler.sql 2_objects.sql 3_table_action.sql \
	4_prompt_rule.sql 5_jscss.sql 6_jquery.sql 7_jscss_data.sql 8_menu.sql \
	9_functions.sql  pgtype_datum.sql comment.sql 

JS=js/add_array_input.js js/add_button.js js/add_input_hidden.js \
	js/add_input_radio.js js/add_input_text.js js/add_prompt.js \
	js/add_select_options.js js/add_text_area.js js/base64_attribs.js\
	js/change_status.js js/form_refresh.js js/form_refresh_init.js \
	js/get_next_row_index.js js/grid_add_row.js js/grid_delete_row.js \
	js/httpRequest.js js/refresh_result.js js/set_common_attributes.js \
	js/set_action_options.js

DOCS=COPYRIGHT.txt opentable.txt design_principles.html \
	internal_cmds.sql qz_examples.sql qz_db_update_SV3.sql \
	qz_db_update_SV4.sql qz_db_update_SV5.sql

all: qzforms.fcgi qz_db_install_SV$(SCHEMA_VERSION).sql

qzforms.fcgi: $(OBJ) qzmain.o
	$(CC)   -o qzforms.fcgi $(OBJ) qzmain.o \
		$(CFLAGS) $(LFLAGS) \
		-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
		-L$(XMLLIBDIR) $(PCRELIBS) \
		-lpq

qzmain.o: qzmain.c qz.h
	$(CC) $(CFLAGS)  -DQZVER="$(VERSION)" -c qzmain.c

qzhandlers.o: qzhandlers.c qz.h
	$(CC) $(CFLAGS) -c qzhandlers.c

str_to_array.o: str_to_array.c 
	$(CC) $(CFLAGS) -c str_to_array.c

test_str_to_array: str_to_array.c
	$(CC) $(CFLAGS) -D STR_TO_ARRAY_MAIN  str_to_array.c -o test_str_to_array

session.o: session.c
	$(CC) $(CFLAGS) -c session.c

session_test: strbuf.o session.c gettime.o crypto_etag.o tagger.o qzrandom64.o \
	hex_to_uchar.o cookie.o parse_key_eq_val.o utility.o qzconfig.o  \
	opentable.o parse_pg_array.o
	$(CC) $(CFLAGS) -L$(XMLLIBDIR) -L$(PGLIBDIR) $(LFLAGS) -DSESSION_MAIN \
		session.c strbuf.o gettime.o crypto_etag.o tagger.o qzrandom64.o \
		hex_to_uchar.o cookie.o parse_key_eq_val.o utility.o qzconfig.o \
		opentable.o parse_pg_array.o  \
		-lpq -o session_test

qzGetElementByID.o: qzGetElementByID.c
	$(CC) $(CFLAGS) -c qzGetElementByID.c

timestamp.o: timestamp.c qz.h
	$(CC) $(CFLAGS) -c timestamp.c

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

get_random_key.o: get_random_key.c qz.h
	$(CC) $(CFLAGS) -c get_random_key.c

strbuf.o: strbuf.c strbuf.h
	$(CC) $(CFLAGS) -c strbuf.c

cookie.o: cookie.c qz.h
	$(CC) $(CFLAGS) -c cookie.c

parse_key_eq_val.o:  parse_key_eq_val.c qz.h
	$(CC) $(CFLAGS) -c parse_key_eq_val.c

test_parse_key_eq_val: parse_key_eq_val.c qz.h gettime.o
	$(CC) $(CFLAGS) $(LFLAGS) $(XMLLIBDIR) -DPARSE_KEY_EQ_VAL_MAIN \
	parse_key_eq_val.c gettime.o \
	-o test_parse_key_eq_val

status.o: status.c qz.h
	$(CC) $(CFLAGS) -DQZVER="$(VERSION)" -c status.c

opentable.o: opentable.c qz.h
	$(CC) $(CFLAGS) -DSCHEMA_VER=\"$(SCHEMA_VERSION)\" -c opentable.c

opentable_test: opentable.c parse_pg_array.o qz.h gettime.o qzrandom64.o pgtools.o
	$(CC) $(CFLAGS) $(LFLAGS) \
	-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
	-L$(XMLLIBDIR) \
	-lpq \
	-DOPENTABLE_TEST -g \
	gettime.o pgtools.o strbuf.o parse_pg_array.o output.o qzrandom64.o \
	prompt_rule.o tagger.o crypto_etag.o utility.o str_to_array.o hex_to_uchar.o \
	form_tag.o qzGetElementByID.o qzhandlers.o  timestamp.o status.o qzfs.o \
	onetable.o menu.o login.o input.o cookie.o session.o parse_key_eq_val.o \
	opentable.c \
	-o opentable_test

pgtools.o: pgtools.c qz.h
	$(CC) $(CFLAGS) -L$(PGLIBDIR) -c pgtools.c 

parse_pg_array.o: parse_pg_array.c
	$(CC) $(CFLAGS) -c parse_pg_array.c 

test_parse_pg_array: parse_pg_array.c
	$(CC) $(CFLAGS) $(LFLAGS) \
	-lpq \
	-DPARSE_PG_ARRAY_MAIN -ggdb \
	parse_pg_array.c \
	-o test_parse_pg_array

qzfs.o: qzfs.c qz.h
	$(CC) $(CFLAGS) -c qzfs.c

strbufs.o: strbufs.c qz.h
	$(CC) $(CFLAGS) -c strbufs.c

qzrandom64.o: qzrandom64.c qzrandom64.h 
	$(CC) $(CFLAGS) $(QZRANDOM) -c qzrandom64.c

qzrandom64_test: qzrandom64.c qzrandom64.h
	$(CC) $(CFLAGS) $(QZRANDOM) -lcrypto -DR64_MAIN  qzrandom64.c -o qzrandom64_test

hex_to_uchar.o: hex_to_uchar.c
	$(CC) $(CFLAGS) -c hex_to_uchar.c

hex_to_uchar_test: hex_to_uchar.c gettime.o
	$(CC) $(CFLAGS) -Wstack-protector -DHEX_TO_UCHAR_MAIN hex_to_uchar.c \
	gettime.o -lcrypto -o hex_to_uchar_test

crypto_etag.o: crypto_etag.c crypto_etag.h
	$(CC) $(CFLAGS) -c crypto_etag.c

crypto_etag_test: crypto_etag.c qzrandom64.o hex_to_uchar.o
	$(CC) $(CFLAGS) -DCRYPTO_ETAG_MAIN  crypto_etag.c qzrandom64.o hex_to_uchar.o \
		-lcrypto -o crypto_etag_test

tagger.o:tagger.c
	$(CC) $(CFLAGS) -c tagger.c

# launch tagger process and test against it.
tagger_test: tagger.c qzrandom64.o crypto_etag.o hex_to_uchar.o qzconfig.o
	$(CC) -Wall -g -lcrypto  -DTAGGER_MAIN tagger.c \
		qzrandom64.o crypto_etag.o qzconfig.o hex_to_uchar.o \
		$(XMLCFLAGS) -L$(XMLLIBDIR)\
		-o tagger_test

# send tests to a running tagger
test_tagger: tagger.c
	$(CC) -Wall -g -lcrypto  -DTEST_TAGGER tagger.c qzrandom64.o crypto_etag.o \
		hex_to_uchar.o -o test_tagger

qzconfig.o:qzconfig.c qzconfig.h 
	$(CC) $(CFLAGS)  -Wall -c qzconfig.c 

test_qzconfig:qzconfig.c qzconfig.h qzrandom64.o gettime.o  
	$(CC) $(CFLAGS) -lcrypto -DQZCONFIG_MAIN  qzconfig.c \
		-L$(XMLLIBDIR) -lxml2 \
		qzrandom64.o gettime.o \
		-o test_qzconfig

prompt_rule.o: prompt_rule.c qz.h 
	$(CC) $(CFLAGS) $(PCRECFLAGS)  -Wall -c prompt_rule.c 

test_prompt_rule: prompt_rule.c qz.h 
	$(CC) $(CFLAGS) $(LFLAGS) -lcrypto -DPROMPT_RULE_MAIN  prompt_rule.c \
	qzhandlers.o timestamp.o  onetable.o \
	str_to_array.o qzGetElementByID.o session.o login.o  cookie.o \
	 input.o output.o strbuf.o menu.o utility.o strbufs.o \
	parse_key_eq_val.o status.o opentable.o parse_pg_array.o qzfs.o \
	pgtools.o qzrandom64.o crypto_etag.o tagger.o \
	hex_to_uchar.o qzconfig.o gettime.o form_tag.o \
		-I$(PGINCLUDEDIR) -L$(PGLIBDIR) \
		-L$(XMLLIBDIR) \
		-lpq \
		-o test_prompt_rule

gettime.o:gettime.c
	$(CC) $(CFLAGS)  -Wall -c gettime.c 
    
form_tag.o: form_tag.c
	$(CC) $(CFLAGS)  -Wall -c form_tag.c 

grid.o: grid.c
	 $(CC) $(CFLAGS) -Wall -c grid.c

inc:
	echo $(VERSION)"+0.001"|bc > Version.new
	printf "%.3f\n" `cat Version.new` > Version
	cat Version

qzforms.js.sql: $(JS) 
	echo "UPDATE qz.js SET data = \\044QZ\\044"  > qzforms.js.sql
	cat js/httpRequest.js >> qzforms.js.sql
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
	echo "\\044QZ\\044 WHERE filename = 'qzforms.js'" >> qzforms.js.sql

qz_db_install_SV$(SCHEMA_VERSION).sql : $(SQL) qzforms.js.sql
	cat $(SQL) > qz_db_install_SV$(SCHEMA_VERSION).sql
	cat qzforms.js.sql >> qz_db_install_SV$(SCHEMA_VERSION).sql

tar:
	tar -cz -s '|^|qzforms-$(VERSION)/|' -f qzforms-$(VERSION).tgz \
    $(FILES) $(SQL) $(DOCS) $(JS)

# XXXXX add all the tests
clean:
	rm -f $(OBJ) qzmain.o qzforms.fcgi qzforms.core test_parse_pg_array testopentable \
		qzrandom64_test crypto_etag_test test_prompt_rule hex_to_uchar_test \
        qzforms.js.sql qz_db_install_SV$(SCHEMA_VERSION).sql
