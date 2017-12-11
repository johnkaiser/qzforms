QZ Forms is a fastcgi program written in C.  It turns SQL SELECT statements
and other data entered into HTML forms into an application.
Each user login is a database login with a dedicated database process. 

There is additional information at http://qzforms.com

Building

There is no configure script.  
You have to edit the Makefile to select the randomness source and compiler.

The prerequisites are:

   PostgreSQL Client library for libpq-fe 
   BSD make, default on OpenBSD, bmake on Debian jessie, gnu make >= 4
   libxml2, libxml2-dev on Debian
   PCRE for javascript style regex validation
   GCC or Clang
   FastCGI Developement Kit, fcgi on OpenBSD, libfcgi-dev on Debian
   On Debian, libasprintf-dev for asprintf support
   LibreSSL or OpenSSL
   spawn-fcgi
   A randomness source, arcrandom, getrandom, or a device file
   For Apache, libapache2-mod-fcgid, or use Nginx, any fastcgi 
   webserver should work, though no buffering should be set.
   "no buffering" means every response is complete, with no partial
   data or range requests.

make will produce the executable, qzforms.fcgi and will assemble the
database install script.

Installation

The install directory must exist before running the install script
qzforms_install.sh. At the top of qzforms_install.sh are user and group
assignments, edit these as necessary.

The install script will:
   - Create a config directory with a default configuration in qzforms.conf 
   - Create a libexec directory with the fastcgi application qzforms.fcgi,
     and a simple init script qzforms.init. 
   - Create a log directory for log files.
   - Create a run directory for a pid file and ipc socket.
   - Create a templates directory with some default xml templates.
   - Create an sql directory with database setup and update scripts.

Configuration

config/qzforms.conf controls how qzforms.fcgi talks to the database and
how it runs. How the webserver talks to qzforms.fcgi is set in qzforms.init
or in your particular startup scripts as command line options to spawn-fcgi.

qzforms.conf will need the database, and perhaps other 
information to connect to the database.  The username and password
come from the login page, set anything else needed. Currently, only
username/password logins are supported.

QZForms uses threads (pthreads). Set the number of threads as
required to scale to your needs. QZ Forms uses one thread from the time
of each request through serving up the current page. 

Startup Script

qzforms.fcgi does not know how to start itself.  It needs spawn-fcgi.

There is a minimalist init script qzforms.init.  Put it where it needs 
to go on your system or use it as a starting point for what you need. 
Consult the spawn-fcgi documentation for further details. qzforms.init
contains an Options section which will contain some of the same data
as using in the install script, set the values to match.

Web Server

QZForms does not care what the hostname is, nor the first segment.
In a URL such as https://example.com/qz/edit/stuff,
the /qz/ can be anything (you can make it something random to keep 
scanners from finding it).  The first segment is used to direct the 
request from the web server to the qzforms.fcgi process.  

For nginx:
location /qz/ {
    include fastcgi_params;
    fastcgi_pass 127.0.0.1:9074;
    fastcgi_buffering off;
}

Use a port number that suits you, or use a unix domain socket.

For nginx, turn off buffering or large files or datasets will be served chopped. 

For Apache2, inside a VirtualHost add:
FastCgiExternalServer  "/var/www/html/qz" -host 127.0.0.1:8888

The path is notable in that it does not exist.
In this example, /var/www/html is the document root.
Use the concatenation of the document root and the first segment.

QZForms will work with TCP or UNIX domain sockets.

Database

You must run the install script on the database. The database install
script will have the Schema Version as part of the name, such as _SV10.
From psql "\i qz_db_install_SV10.sql" will create the schema qz
and the necessary tables and types in the current database.

qz_examples.sql will install the sample applications
stuff and gridle with the tables in the public schema.

User Permissions

QZ Forms users must have a login to PostgreSQL valid from the 
host running QZ Forms configured in the PostgreSQL pg_hba.conf file.

The simpliest case is for the form developer to be the database owner.
A more structured approach to managing permissions is at 
https://qzforms.com/users.html

The simplest case for user quasimodo, who is not a form developer, 

    GRANT USAGE ON SCHEMA qz TO USER quasimodo;
    GRANT SELECT ON ALL TABLES IN SCHEMA qz TO quasimodo;
    GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA qz TO quasimodo;
    GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA qz TO quasimodo;

Updates

The version of the executable, qzforms.fcgi, and the data in the PostgreSQL
qz schema must be kept in sync. The Schema Version is a monotonically 
increasing integer stored in the table qz.constants as attribute 
schema_version. The table qz.change_history contains a row for the 
installation and each subsequent upgrade. Do not skip a Schema Version.

For example, to get from Schema Version 8 to 9, from psql run
    \i /some/path/qz_db_update_SV9.sql

The current version of QZ Forms and the Schema Version expected vs installed
is on the Form Development menu on the Status page.

Making A Form

This is the short version. More detailed documention is at 
https://qzforms.com/form_development.html

Create a form from the Form Development menu.

The handler name determines what methods, or as they are called here,
table actions, the form may support.

  onetable - From a list of rows, select one to edit.
  grid     - The data is in an HTML table, edited all at once.
  menupage - A page without a form, but with menu buttons.
  fs       - js  or css files

Once the form is created, add Table Actions. The most important thing
about a Table Action is the SQL statement that is prepared when the table
is first used and is executed each time the action is called.

The create action may not be obvious at first. QZ Forms does not itself
know how to create an attribute. It does not even know how to make an
empty string. PostgreSQL has to do that in the create action.

    SELECT ''::text "data"

This will create a form field named data that is empty. 
''::text is appropriate for any data type and does not specify the
actual type, it is just an empty string.

To use the form, it must be on a menu, to be useful it must also contain a
menu. To add a menu to the form, Forms, page_menus to add perhaps 
the menu main. To add the form to a menu so that it may be selected,
Menu Menu, All Menus, select a menu perhaps main, then menu items. 


Creating Forms
John Kaiser 2015-10-5
Revised 2017-07-30



