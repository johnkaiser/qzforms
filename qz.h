

// This is needed to get asprintf on Linux
#define _GNU_SOURCE
#define LIBXML2_NEW_BUFFER

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcgi_config.h>
#include <fcgiapp.h>
#include <fcntl.h>
#include <float.h>
#include <inttypes.h>
#include <libgen.h>
#include <libpq-fe.h>
#include <libxml/hash.h>
#include <libxml/hash.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/uri.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>
#include <limits.h>
#include <netdb.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <pcre.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
// for FreeBSD:
#include <arpa/inet.h>

#include "http_codes.h"
#include "qzrandom.h"
#include "tagger.h"
#include "hex_to_uchar.h"

#include "json-parser/json.h"
#include "json-builder/json-builder.h"

#ifndef QZVER 
#define QZVER 0
#endif

#ifndef SCHEMA_VER
#define SCHEMA_VER 0
#endif

#define MAX_SEGMENT_LENGTH (32)
#define MAX_LOGIN_URI (2*(MAX_SEGMENT_LENGTH+1)+2)
#define MAX_PRIMARY_KEYS (15)
#define MAX_USER_NAME_LENGTH (62)
#define MAX_NBR_SEGMENTS (32)
#define PATH_SEPARATOR "/"
#define BUFLEN 1024

// This length includes the ending null.
// This is the same as tagger.c TAG_MAX_LENGTH
#define SESSION_KEY_LENGTH 98

#define PG_NAMEDATALEN 63
#define NAME_RANDOMNESS 16

#define NO_ROW_INDEX (-1)
#define NO_OPTIONS NULL

// these are for the URI array uri_parts
#define QZ_URI_BASE_SEGMENT 0
#define QZ_URI_FORM_NAME 1
#define QZ_URI_ACTION 2
#define QZ_URI_REQUEST_DATA 3

#define QZ_MAX_CONTENT_TYPE_LENGTH 64
#define QZ_MAX_ID_LENGTH 64

#define IN_ONE_YEAR (time(NULL)+(86400*365))

//  Note in session.c state_text() turns this to text.
enum session_state {no_session, bad_session, session_no_login, 
    logged_in, logged_out };

// Explicit initializers for value to text conversion.
/* is this deletable?

*/

enum prompt_types { none=0, button=1, input_button=2, input_checkbox=3,
    input_file=4, input_hidden=5, input_image=6, input_password=7,
    input_radio=8, input_reset=9, input_submit=10, input_text=11,
    select_enum=12, select_options=13, select_fkey=14, textarea=15,
    text_array=16, input_datalist=17};

// XXXXXXXXXXX add td, div, 
enum prompt_container_type {no_container, fieldset};
enum precheck_status {notchecked,failed,passed};
enum callback_response_type {plain_text,qzforms_json,postgresql_json,html_table};

extern pthread_mutex_t log_mutex;

struct handler_args {
    char session_key[16];
    struct session* session;
    char* handler_name;
    FCGX_Request *request;
    FCGX_Stream *in;
    FCGX_Stream *out;
    FCGX_Stream *err;
    FCGX_ParamArray envpfcgi;
    FCGX_ParamArray envpmain;
    int request_id;
    struct qz_config* conf;
    FILE* log;
    double starttime;
    char** uri_parts;
    int nbr_uri_parts;
    char* cookie_buf;
    char* postbuf;
    xmlHashTablePtr cookiesin;
    xmlHashTablePtr postdata;
    struct table_action* page_ta; 
    struct form_record* posted_form;
    struct form_set* current_form_set;
    xmlBufferPtr headers;
    xmlParserCtxtPtr ctx;
    xmlDocPtr doc;
    xmlHashTablePtr id_index;
    xmlBufferPtr data;
    enum precheck_status regex_check;
    enum precheck_status pkey_check;
    unsigned int counter;
    bool error_exists;
    uint64_t integrity_token;
};

struct handler{
    int count;
    char* name;
    void (*handler)( struct handler_args* );
};

struct session{
    unsigned char session_id[16];
    int zero;
    char user[MAX_USER_NAME_LENGTH+2];
    pthread_mutex_t session_lock;
    bool is_logged_in;
    time_t logged_in_time;
    time_t logged_out_time;
    time_t last_activity_time;
    PGconn* conn;
    xmlHashTablePtr opentables;
    xmlHashTablePtr pgtype_datum;
    xmlHashTablePtr form_tags;
    xmlHashTablePtr form_sets;
    uint64_t form_tag_token;
    uint64_t etag_token;
    uint64_t integrity_token;
};
 
struct table_action{
    char* form_name;
    char* action;
    char* handler_name;
    char* schema_name;
    char* table_name;
    char* prepare_name;
    uint64_t etag;
    bool etag_cache;
    int nbr_params;
    char** fieldnames;
    int nbr_pkeys;
    char** pkeys;
    char* target_div;
    char* xml_template;
    bool add_description;
    char* prompt_container;
    char* helpful_text;
    char** callbacks;
    char** js_filenames;
    char** css_filenames;
    char*  inline_js;
    char*  inline_css;
    char form_set_name[PG_NAMEDATALEN+2];
    char** context_parameters;
    char** clear_context_parameters;
    bool is_callback;
    char callback_attached_action[PG_NAMEDATALEN+2];
    enum callback_response_type callback_response;
    uint64_t integrity_token;
};

// This must match the version set in create_pgtype_datum.sql
#define PGTYPE_DATUM_VERSION 3

struct pgtype_datum{

    int version;

    // some names from and values equal to information schema columns
    char* table_schema;
    char* table_name;
    char* column_name;
    int ordinal_position;
    char* column_default;
    bool is_nullable; 

    // some from system catalog pg_type
    char* typname;
    char* typtype;
    char* typdelim;

    // a few checks for the base type in typtype
    bool is_base_type;
    bool is_boolean;
    bool is_composite;
    bool is_domain;
    bool is_enum;
    bool is_pseudo_type;

    // quantify a number
    int character_maximum_length;
    int character_octet_length;
    int numeric_precision;
    int numeric_precision_radix;
    int numeric_scale;
    int datetime_precision;

    // context
    char* typcategory_name;
    char* domain_schema;
    char* domain_name;
    char* domain_check_clause;
    char* udt_schema;
    char* udt_name;
    char* description;
    bool is_updatable;

    // enums and attributes come in as a pg array
    char* enum_labels;
    char* composite_attributes;

    // foreign key
    bool has_fkey;
    char* fkey_schema;
    char* fkey_table;
    char* fkey_attribute;
    
    // This must be PQclear'd  
    PGresult* pgresult;

};

struct prompt_rule{
    char* form_name;
    char* fieldname;
    char* prompt_type;
    char* el_class;
    bool readonly;
    char* regex_pattern;
    bool publish_pgtype;
    int rows;
    int cols;
    int size;
    int maxlength;
    int tabindex;
    bool expand_percent_n;
    pcre* comp_regex;
    uint64_t etag; // XXXXX can this be removed?
    char* options;
    char* free_options; // Internal bookkeeping.
    char* src;
    char* onblur;
    char* onchange;
    char* onclick;
    char* ondblclick;
    char* onfocus;
    char* onkeypress;
    char* onkeyup;
    char* onkeydown;
    char* onmousedown;
    char* onmouseup;
    char* onmouseout;
    char* onmouseover;
    char* onselect;
    PGresult* result;

    char strdata[]; // XXXXX don't think this is being used.
};

struct form_record{
    xmlChar form_id[16];
    bool is_valid;
    time_t created;
    time_t expires;
    time_t duration;
    bool submit_only_once;
    char form_name[PG_NAMEDATALEN+2];
    struct form_set* form_set;
    xmlHashTablePtr pkey_values;
    uint64_t session_integrity_token;
    char form_action[];
};


struct form_set{
    char set_id[16];
    int64_t ref_count;
    int64_t audit_count;
    bool is_valid;
    // A context parameter is in the form "value\0key\0".
    xmlHashTablePtr context_parameters;
    char name[64];
    uint64_t integrity_token;
};

struct id_node {
    xmlNodePtr node;
    char id[QZ_MAX_ID_LENGTH+2];
};

// Used by housekeeper for xmlHashScan data passing
struct form_tag_housekeeping_data {
    struct session* this_session;  // the session being cleaned
    struct handler_args* hargs;    // the housekeeper's not the session's
};

struct login_tracker {
   char remote_host[INET6_ADDRSTRLEN + 2];
   double last_attempt;
   unsigned int failed;
   unsigned int success;
};

static const char QZERR_EXPECTED_EQ[] = "Expected '='";
static const char QZERR_BAD_VALUE[] = "Bad Value";
static const char QZERR_EXPECTED_AMP[] = "Expected ampersand";

extern void params(struct handler_args*);
extern void timestamp(struct handler_args*);
extern void formtest(struct handler_args*);
extern void qz_status(struct handler_args*);
extern void qzfs(struct handler_args*);
extern void onetable(struct handler_args*);
extern void testdatum(struct handler_args*);
extern void menupage(struct handler_args*);
extern void callback(struct handler_args*);
extern void simpleform(struct handler_args* h);

/*  serve_output
 *  output.c
 *
 *  Send the document to out.
 *  If doc is set, output an xml
 *  otherwise see if data is not null and maybe send that.
 *  Unless ->error_exists is true, then do nothing.
 */
extern void serve_output( struct handler_args* );

/*
 *  do_page
 *  output.c
 *
 *  Look for a handler that matches the request,
 *  execute it if found.
 */
extern void do_page( struct handler_args* hargs );

/*
 *  doc_from_file
 *  input.c
 *
 *  Turn a file on disk into an xml document tree.
 */ 
extern void doc_from_file( struct handler_args*, char*);

/*
 *  regex_patters_are_valid
 *  input.c
 *
 *  Search the postdata for prompt_rules with a compiled regex pattern,
 *  and validate data values for any patterns found.
 */
extern bool regex_patterns_are_valid(struct handler_args* h);

/*
 *  req_login
 *  login.c
 *
 *  Serve a page requesting the user login.
 */
extern void req_login( struct handler_args* ); 


/*
 *  error_page
 *  output.c
 *
 * Turn a string into an error page and serve it
 * instead of any useful data.
 */
extern void error_page( struct handler_args*,int status_code, const char* msg );

/*
 *  make_cookie
 *  cookie.c
 *
 *  Add a Set-Cookie header
 */
extern void  make_cookie(struct handler_args*h, 
     char* name, 
     char* value, 
     char* path, 
     char* domain,
     int lifetime_seconds, 
     bool secure, 
     bool http_only);

/*
 *  str_to_array
 *  str_to_array.c
 *
 *  Copy the string to create an array of strings.
 *  The result must be freed.
 */

extern char** str_to_array(char* str, char split);

/*
 *  qzGetElementByID
 *  qzGetElementByID.c
 *
 *  Like it says on the tin.
 *
 */
extern xmlNodePtr qzGetElementByID(struct handler_args*, xmlChar*); 

/*
 *  init_session_token
 *  session.c
 *
 *  Set a random uint64_t for all session tags.
 */
extern void init_session_token(void);

/*
 *  get_session_state
 *  session.c
 *
 *  Return an enumerated type of the session state.
 */
extern enum session_state 
    get_session_state (struct handler_args*);

/*
 *  load_session_key
 *
 *  Decrypts the session cookie.
 *  Sets session_key in handler args.
 *  Returns true/false if the session key is possibly valid.
 *  Does not actually validate the session, just determines
 *  that the token could be valid.
 */
extern bool load_session_key(struct handler_args* hargs, char* session_cookie);

/*
 *  session_from_hargs
 *  session.c
 *
 *  Return a session from a handler struct session key
 */
extern struct session* 
session_from_hargs(struct handler_args* hargs, 
    xmlHashTablePtr sessions,
    struct qz_config* conf);


/*
 *  validate_login
 *  login.c
 *
 *  Check username and password, establish session.
 */
extern void validate_login( struct handler_args* );

/*
 *  setup_session
 *  session.c
 *
 *  Create a not logged in session
 */
extern void setup_session(struct handler_args*, 
    xmlHashTablePtr sessions,
    struct qz_config* conf);

/*
 *  state_text
 *  session.c
 * 
 *  Turn a session state into a char*
 */
extern char* state_text(enum session_state);

/*
 *  close_session
 *  session.c
 * 
 *  Cleanly close a possibly corrupted session struct
 */
extern void close_session(struct handler_args* housekeeper, struct session* this_session);

/*
 *  content_type
 *  output.c
 *
 *  Add a content type header
 */

extern void content_type(struct handler_args* h, char* mime_type);

/*
 *  expires 
 *  output.c
 *
 *  Add an Expires: header
 *  The time is given in seconds since the epoch,
 *  time(NULL) + x is the recommended usage
 *  or IN_ONE_YEAR is defined for approximately never.
 */
extern void expires(struct handler_args*, time_t);
 
/* 
 *  parse_cookie
 *  cookie.c
 *
 *  Parse incoming cookies into a libxml hash table
 */
extern void parse_cookie(struct handler_args* hargs, char* ck);

/*
 *  logout
 *  login.c
 *
 *  Close the session
 */
extern void logout(struct handler_args*);

/* 
 *  location
 *  utility.c
 *
 *  Turn the response into a 302 redirect.
 *  The calling handler should return immeadiately after calling this. 
 */
extern void location(struct handler_args* h, char* new_url);

/*
 *  add_header
 *  utility.c
 *
 *  Add a new header to the list of headers
 *  The colon after the key is added by the function and should not be
 *  included in the key.
 */
extern void add_header(struct handler_args* h, char* key, char* value);

/*
 *  get_uri_part
 *  utility.c
 * 
 *  Return the array element in position or quietly return NULL
 *  for out of bounds requests.
 */
extern char* get_uri_part(struct handler_args* h, u_int position );

/*
 *  uri_part_n_is
 *  utility.c
 *
 *  Return true if the nth segment matches.
 */
extern bool uri_part_n_is(struct handler_args*, uint, char*);

/*
 *  parse_key_eq_val
 *  parse_key_eq_val.c
 *
 *  Chop up kvstr nul chars and index in a libxml hash table.
 *  fieldsep will be something like & ; \n 
 *  unescape if true will cause URI percent encoded data to be unescaped.
 */
extern xmlHashTablePtr parse_key_eq_val(struct handler_args* h, char* kvstr, 
    char fieldsep, bool unescape);

/*
 *  percent_unescape
 *  parse_key_eq_val.c
 *
 *  Turn a string with %hexhex encoding into its unencoded form.
 *  Also turn + into space.
 *  Returns false on error or true on success.
 *  On error, the string may be partially converted, partially not.
 */
extern bool percent_unescape(char*);

/*
 *  percent_unescape_vals
 *  parse_key_eq_val.c
 *
 *  For use by xmlHashScan to percent_unescape() all the values
 */
void percent_unescape_vals(void* val, void* ignore, const xmlChar* key);

/*
 *  gettime
 *  gettime.c
 * 
 *  Get the time right now as a float. 
 */
extern double gettime(void);

/*
 *  open_table
 *  opentable.c
 *
 *  Return an action_table struct from the session hash of open tables.
 *
 *  If form_name, action is not in the hash table, then fetch the
 *  definition from postgres and add it to the hash table
 *  creating a prepared statement.
 */
extern struct table_action*  open_table(struct handler_args* hargs, 
               char* form_name, char* action);

/*
 *  free_table_action
 *  opentable.c
 *  
 *  Free a table action structure
 */
extern void free_table_action(struct table_action* ta);

/*
 *  init_open_table
 *  opentable.c
 *
 *  Called once per session during login.
 *  Prepare a statement to access the table_action table,
 *  i.e., open the open table table. 
 */
extern void init_open_table(struct handler_args* h);

/*
 *  close_all_tables
 *  opentable.c
 *
 *  Deallocate all the prepared statements and free all the table entries.
 *  Handler struct is there for logging mostly and will typically belong
 *  to a user housekeeper.
 */
extern void close_all_tables(struct handler_args* h,struct session* this_session);

/*
 *  close_table
 *  opentable.c
 * 
 *  Reclaim resources from an open table
 */
extern void close_table(struct handler_args* h, char* form_name, char* action);

/*
 *  parse_pg_array
 *  parse_pg_array.c
 *
 *  Turn a Postgresql array of values into a C array of strings.
 *  Return a block of memory which begins with a char*[] pointing
 *  to strings in the top of the same memory block.
 *
 *  The returned value must be freed.
 */
extern char** parse_pg_array(char* pg_ar);

/*
 *  str_ar_to_json
 *  parse_pg_array.c
 *
 *  Turn an array of strings into a single string representation of a 
 *  JSON array.  The array of string from parse_pg_array is the 
 *  intended input.
 * 
 *  The returned value must be freed.
 *
 */
char* str_ar_to_json(char* ar[]);

/*
 *  pg_ar_to_json
 *  parse_pg_array.c
 *
 *  Turn a pg array into a JSON array.
 *  Combines parse_pg_array and str_ar_to_json into a single function.
 *
 *  Replaces curly braces for square brackets
 *  and encloses each comma separated value in double quotes.
 *
 *  The returned value must be freed.
 */

char* pg_ar_to_json(char* pg_ar);

/*
 *  perform_post_action
 *  opentable.c
 *
 *  Look through the postdata hash table for the fields in ta->fieldnames
 *  The handler args postdata hash may be null when table_action struct 
 *  nbr_fields is 0.
 *  Construct a char* array and run ta->prepare_name.
 *  Return the result pointer, or null on error.
 */
extern PGresult* perform_post_action(struct handler_args* h, 
    struct table_action* ta);

/*
 *  perform_action
 *  opentable.c
 *
 *  Run ta->prepare_name on data[]
 *  Return the result pointer, or null on error.
 */
 
extern PGresult* perform_action(struct handler_args* h, 
    struct table_action* ta, char** data);

/*
 *  perform_post_row_action
 *  opentable.c
 *
 *  Like perform_post_action, but on only one row
 *  in the result set.
 */
extern PGresult* perform_post_row_action(struct handler_args* h,
    struct table_action* ta, int row);

/*
 *  table_action_exists
 *  opentable.c
 *
 *  Quickly check for the existance of a table action
 */

bool table_action_exists(struct handler_args* h,
    char* form_name, char* action);

/*
 *  rs_to_table
 *  output.c
 *
 *  Turn the given result set into an html table with tuples in rows.
 *  Give the created table attribute the id of id.
 */

extern void rs_to_table(xmlNodePtr add_here, PGresult* rs, char* id);

/*
 *  rs_to_sideways_table
 *  output.c
 *
 *  Turn the given result set into an html table with tuples in columns.
 *  Give the created table attribute the id of id.
 */

extern void rs_to_sideways_table(xmlNodePtr add_here, PGresult* rs, char* id);

/*
 *  build_path
 *  utility.c
 *
 *  Given an array of strings, pack them into a buffer separated by /
 *  Kinda like python "/".join([]) but with a leading /
 *  Returned buffer must be freed by the calling code. 
 *
 *  XXXXX Use asprintf instead.
 */
extern char* build_path(char* str_ar[]);

/*
 *  get_pgtype_datum
 *  pgtools.c
 *
 *  Return a pgtype_datum struct, either from a hash table or from Postgresql.
 *  In general, the result should not be freed.  
 *  It is in the hash table and will be retrieved from there.
 *  If it is freed, it also must be removed from the hash table.
 */  
extern struct pgtype_datum* get_pgtype_datum( struct handler_args* h,
    char* table_name, char* column_name);
 
/*
 *  pgtype_datum_to_json
 *  pgtools.c
 *
 *  Turn a pgtype_datum struct into a json object.
 *  The result must be freed.
 */
 
extern char* pgtype_datum_to_json(struct pgtype_datum* datum);

/*
 *  register_form
 *  form_tag.c
 *
 *  Add an entry to the form_tag hash and add an
 *  html hidden input of the tag.
 */
#define SUBMIT_ONLY_ONCE true
#define SUBMIT_MULTIPLE false

extern struct form_record* register_form(struct handler_args* h, 
    xmlNodePtr form_node,
    bool submit_only_once,
    char* form_action
);

/*
 *  set_form_name
 *
 *  Copies the given form name to the form record.
 *  This is to be used with callbacks, so the callback
 *  can be told which form it is for.
 */
extern void set_form_name(struct form_record* form, char* form_name);

/*
 *  post_contains_valid_form_tag
 *  form_tag.c
 *
 *  Check that the form_tag is present and valid.
 *  If the form is marked use only once then set
 *  the form as invalid but treat it as valid.
 */
extern bool post_contains_valid_form_tag(struct handler_args* h);

/*
 *  get_posted_form_record
 *  form_tag.c
 *
 *  Return the form record that invokes the current request.
 *  The returned form may or may not be flagged as valid.
 */
extern struct form_record* get_posted_form_record(struct handler_args* h);

/*
 *  do_housekeeping
 *  session.c
 */
extern void do_housekeeping(struct handler_args* h, xmlHashTablePtr sessions, 
    struct qz_config* conf, uint64_t thread_state[]);

/*
 *  form_tag_housekeeping
 *  form_tag.c
 */
extern void form_tag_housekeeping(struct handler_args* hargs, 
    struct session* this_session);

/*
 *  close_all_form_tags
 *  form_tag.c
 */
extern void close_all_form_tags(struct handler_args*, struct session*);

/*
 *  fetch_prompt_rule
 *  prompt_rule.c
 */
extern struct prompt_rule* fetch_prompt_rule(struct handler_args* h, 
    char* form_name, char* fieldname);

/*
 *  free_prompt_rule
 *  prompt_rule.c
 */
extern void free_prompt_rule(struct handler_args* h, 
    struct prompt_rule* rule);

/*
 *  close_all_pgtype_datums
 *  pgtools.c
 */
extern void close_all_pgtype_datums(struct session*);

/*
 *  refresh_form_tag
 *  form_tag.c
 */
extern void refresh_form_tag(struct handler_args* h);

/*
 *  init_prompt_type_hash
 *  prompt_rule.c
 */
extern void init_prompt_type_hash(void);

/*
 *  init_handler_hash
 *  qzhandlers.c
 */
extern void init_handler_hash(void);
/*
 *  handler_hash
 *  qzhandlers.c
 */
extern xmlHashTablePtr handler_hash;

/*
 *  add_prompt
 *  prompt_rule.c
 */
extern void add_prompt(struct handler_args*, struct table_action*, 
     struct prompt_rule*, struct pgtype_datum*, char* options[],
     int row_index, xmlNodePtr, xmlChar*, xmlChar*);

/*
 *  append_class
 *  utility.c
 */
extern void append_class(xmlNodePtr, xmlChar*);

/*
 *  get_value
 *  get_bool
 *  has_data
 *  pgtools.c
 */
extern char* get_value(const PGresult*, int, char*);
extern bool get_bool(const PGresult*, int, char*);
extern bool has_data(char*);

/*
 *  nlfree_error_msg
 *  pgtools.c
 */
extern char* nlfree_error_msg(PGresult* some_rs);

/*   XXXXXX Remove header detail 
 */ 
extern void hdrdet(struct handler_args* h);
extern void init_hdrdet_table_action(struct handler_args*);

/*
 *  grid
 *  grid.c
 */
extern void grid(struct handler_args* h);

/*
 *  add_jscss_links
 *  utility.c
 */
extern void add_jscss_links(struct handler_args* h);

/*
 *  json_add_element_args
 *  utility.c
 */
extern char* json_add_element_args(char* func_name, struct prompt_rule* rule, char* option_ar[]);

/*
 * foreign_key_list
 *  pgtools.c
 */
extern char** foreign_key_list(struct handler_args*, struct pgtype_datum*);

/*
 *  fetch_options
 *  prompt_rule.c
 */
extern char**  fetch_options(
    struct handler_args* h, 
    struct pgtype_datum* pgtype, 
    struct prompt_rule* p_rule, 
    char* fname);

/*
 *  init_menu
 *  menu.c
 */
extern void init_menu(struct handler_args *);

/*
 *  add_all_menus
 *  menu.c
 */
extern void add_all_menus(struct handler_args *);

/*
 * form_name_is_menu
 * menu.c
 */
extern bool form_name_is_menu(struct handler_args*);

/*
 *  add_helpful_text
 *  utility.c
 */
extern void add_helpful_text(struct handler_args* h, struct table_action* ta);

/*
 *  base64_encode
 *  utility.c
 */
extern char* base64_encode(char*);

/*
 *  decrement_form_set
 *  form_set.c
 */
extern void decrement_form_set(struct form_record* form_rec);

/*
 *  form_set_housekeeping_scanner
 *  form_set.c
 */
extern void form_set_housekeeping_scanner(void* payload, void* data, const xmlChar* name);

/*
 *  close_all_form_sets
 *  form_set.c
 */
extern void close_all_form_sets(struct form_tag_housekeeping_data*);

/*
 *  save_context_parameters
 *  form_set.c
 */
extern void save_context_parameters(struct handler_args* h, 
    struct form_record* new_form_rec,
    PGresult* values_rs, int row);

/*
 *  get_form_set
 *  form_set.c
 */
extern struct form_set* get_form_set(struct handler_args*, char* form_set_id);

/*
 *  form_set_is_valid
 *  form_set.c
 */
extern bool form_set_is_valid(struct handler_args* h, struct form_set* fs); 

/*
 *  check_postdata
 *  input.c
 */
extern bool check_postdata(struct handler_args* h);

/*
 *  save_pkey_values
 *  input.c
 */
extern void save_pkey_values(struct handler_args* h,
    struct form_record* form_rec,
    struct table_action* ta,
    PGresult* rs,
    int row);

/*
 *  log_file_rotation
 *  utility.c
 */
extern void log_file_rotation(struct qz_config* conf, char* logfile_name);

/*
 *  add_listener_for_*
 *  utility.c
 */
void add_listener_for_doc(struct handler_args* h, char* event, char* js);
void add_listener_for_id(struct handler_args* h, char* id, char* event, char* js);
void add_listener_for_name(struct handler_args* h, char* form_name,
    char* field_name, char* event, char* js);

void add_dom_content_loaded(struct handler_args* h, char* id, char* js);

/*
 *  add_to_id_index
 *  intput.c
 */
extern void add_to_id_index(struct handler_args* h, xmlNodePtr the_node);

/*
 *  init_login_tracker
 *  login.c
 */
extern void init_login_tracker(void);

/*
 *  login_tracking_housekeeping(struct handler_args* hargs
 *  login.c
 */
extern void login_tracking_housekeeping(struct handler_args* hargs);

/*
 *  item_in_list
 *  utility.c
 */
extern bool item_in_list(char* item, char** list);

/*
 *  array_base
 *  utility.c
 */
extern char* array_base(const char* name);

/*
 *  doc_adder
 *  docs.c
 */
void init_doc(struct handler_args* h);
void doc_adder(struct handler_args* h);

/*
 *  callback_adder
 *  callback.c
 */
extern void callback_adder(struct handler_args* h, xmlNodePtr form_node,
    struct table_action* ta);

/*
 *  callback_name_lookup
 *  callback.c
 */
enum callback_response_type callback_name_lookup(char* type_name);

/*
 *  callback_enum_lookup
 *  callback.c
 */
char* callback_enum_lookup(enum callback_response_type cb_response);

/*
 *  add_parameter_table
 *  utility.c
 */
void add_parameter_table(struct handler_args* h,
    struct table_action* ta, xmlNodePtr addnode, char* new_id);

/*
 *  add_hidden_input
 *
 *  Add a hidden input element under the named form.
 *  If row_nbr >= 0, then the id is in the form name[0],
 *  otherwise the id is the name.
 */
xmlNodePtr add_hidden_input(struct handler_args* h,
    xmlNodePtr form_node, char* name, int row_nbr, char* value);

/*
 *  edit_form
 *
 *  Add the given form to the page.
 */
 void edit_form(struct handler_args* h, char* next_action,
    struct table_action* edit_ta, PGresult* edit_rs, char* form_name,
    xmlNodePtr divqz);
