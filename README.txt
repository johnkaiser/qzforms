QZ Forms is a fastcgi program written in C.  It turns SQL SELECT statements
and other data entered into HTML forms into an application.
Each user login is a database login with a dedicated database process. 

There is additional information at http://qzforms.com

There is no configure script.  
You have to edit the Makefile to select the randomness source and compiler.

The prerequisites are:

   PostgreSQL Client library for libpq-fe 
   BSD make, default on OpenBSD, bmake on Debian
   libxml2, libxml2-dev on Debian
   GCC or Clang
   FastCGI Developement Kit, fcgi on OpenBSD, libfcgi-dev on Debian
   On Debian, libasprintf-dev for asprintf support
   LibreSSL or OpenSSL
   spawn-fcgi
   A randomness source, arcrandom or a device file
   For Apache, libapache2-mod-fcgid, or use Nginx  

Make will produce the executable, qzforms.fcgi and will assemble the
database install script.

The script qzforms_install.sh will create /var/qzforms and the directory
structures and files under that to run.

The file config/qzforms.conf will need the database, and perhaps other 
information to connect to the database.  The username and password
come from the login page, set anything else needed.

QZForms uses threads (pthreads). Set the number of threads as
required to scale to your needs.  

qzforms.fcgi does not know how to start itself.  It needs spawn-fcgi.

There is a somewhat minimalist init script qzforms.init. 
Put it where it needs to go on your system or use it as a starting point
for what you need.

Web Server Setup:
QZForms does not care what the hostname is, nor the first segment.
In a URL such as https://example.com/qz/edit/stuff,
the /qz/ can be anything (you can make it something random to keep 
scanners from finding it).  The first segment is used to direct the 
request from the web server to the qzforms.fcgi process.  

For nginx:
location /qz/ {
    include fastcgi_params;
    fastcgi_pass 192.168.1.1:9991;
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

You must run the install script for the database.
From psql "\i qz_db_install_SV2.sql" will create the schema qz
and the necessary tables and types.

qz_examples.sql will install the sample applications
stuff and gridle with the tables in the public schema.

For a user, esmeralda, who is not a form developer set Postgres permissions as 
> grant usage on schema qz to esmeralda;
> grant select,references,trigger on all tables in schema qz to esmeralda;
Plus of course, permission for the tables to be edited.


John Kaiser
2015-10-5




