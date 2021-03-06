#!/bin/sh
## To run the install as a non-root user,
## replace RUNUSER,RUNGROUP,FORMDEVGROUP,ADMINUSER,ADMINGROUP with
## the user and a group of the account running the script.
##
##  The user and group that will be running the qzforms application. 
##  These must be the same in qzforms.init.
##
RUNUSER=qzforms
RUNGROUP=qzforms

##
##  The form developer group will have access to log files and templates.
##
FORMDEVGROUP=formdev

##
##  The admintrative account is commonly root, the group is wheel or root,
##  but these can be any accounts with the suggestion that they are not
##  the run user and group.
##
ADMINUSER=root
ADMINGROUP=$(if (id root | grep -q wheel); then echo wheel; else echo root; fi)

###########################################################################

##
##  The INSTALLDIR must be specified, /var/qzforms is suggested.
##  The necessary subdirectories are created. 

if [ -z "$1" ]; then
    echo "You must specify the target directory."
    echo "If /var/qzforms exists then try:"
    echo "$0 /var/qzforms"
    exit 1
fi

if [ -d "$1" ]; then
    echo "installing to $1"
    INSTALLDIR=$1
else
    echo "Install directory $1 does not exist"
    exit 2
fi


echo "user and group check"

idcheck=""

if id ${RUNUSER}; then 
    echo ${RUNUSER} "OK"
else
    echo "RUNUSER ${RUNUSER} not found"
    idcheck=1
fi

if [ "${RUNGROUP}" ]; then 
    echo ${RUNGROUP} "OK"
else
    echo "RUNGROUP ${RUNGROUP} not found"
    idcheck=1
fi

if [ "${FORMDEVGROUP}" ]; then 
    echo ${FORMDEVGROUP} "OK"
else
    echo "FORMDEVGROUP ${FORMDEVGROUP} not found"
    idcheck=1
fi

if id ${ADMINUSER}; then 
    echo ${ADMINUSER} "OK"
else
    echo "ADMINUSER ${ADMINUSER} not found"
    idcheck=1
fi

if [ "${ADMINGROUP}" ]; then 
    echo ${ADMINGROUP} "OK"
else
    echo "ADMINGROUP ${ADMINGROUP} not found"
    idcheck=1
fi

if [ "$idcheck" ]; then 
    echo "Installation failed on user and group checks."
    echo "You will need to setup a user and group to run the application."
    echo "It may be convenient to add the form developer to the qzforms group."
    echo "Something like:"
    echo "groupadd qzforms"
    echo "groupadd formdev"
    echo "For OpenBSD try:"
    echo "    useradd -L daemon -s /usr/bin/false -g qzforms -d ${INSTALLDIR} qzforms"
    echo "For Debian try:"
    echo "    useradd -G daemon -s /usr/bin/false -g qzforms -d ${INSTALLDIR} qzforms"
    echo "The user and groups used can also be set at the top of qzforms_install.sh"
    echo "Rerun qzforms_install.sh after making corrections."

    exit 1
fi

INSTALL_FROM=$(dirname $0)
###########################################################################
##
##  Create the installation directory
##
install -d -m 755 -o ${ADMINUSER} -g ${RUNGROUP} ${INSTALLDIR}

##
##  Copy the qzforms executable to libexec.
##  Some people may want to put the executable into /usr/local/libexec.
##  It must be read-only for the account running the fast-cgi process.
##
install -d -m 750 -o ${ADMINUSER} -g ${RUNGROUP}  ${INSTALLDIR}/libexec
install -m 550 -o ${ADMINUSER} -g ${RUNGROUP} ${INSTALL_FROM}/qzforms.fcgi \
    ${INSTALLDIR}/libexec/ 
if [ ! -f ${INSTALLDIR}/libexec/qzforms.init ]; then
install -m 744 -o ${ADMINUSER} -g ${RUNGROUP} ${INSTALL_FROM}/qzforms.init \
    ${INSTALLDIR}/libexec/ 
fi
##
##  Create and populate the templates directory
##
install -d -m 775 -o ${ADMINUSER} -g ${FORMDEVGROUP} ${INSTALLDIR}/templates
if [ ! -f ${INSTALLDIR}/templates/base.xml ]; then
    install  -m 664 -o ${ADMINUSER} -g ${FORMDEVGROUP} \
    ${INSTALL_FROM}/templates/base.xml ${INSTALLDIR}/templates
fi
if [ ! -f ${INSTALLDIR}/templates/login.xml ]; then
    install  -m 664 -o ${ADMINUSER} -g ${FORMDEVGROUP} \
    ${INSTALL_FROM}/templates/login.xml ${INSTALLDIR}/templates
fi
if [ ! -f ${INSTALLDIR}/templates/tinymce.xml ]; then
    install  -m 664 -o ${ADMINUSER} -g ${FORMDEVGROUP} \
    ${INSTALL_FROM}/templates/tinymce.xml ${INSTALLDIR}/templates
fi

##
##  The config file should be well protected, especially if it contains
##  server tokens and keys. 
##  A config should not be overwritten if it exists.
##
install -d -m 750 -o ${ADMINUSER} -g ${RUNGROUP} ${INSTALLDIR}/config
if [ ! -f ${INSTALLDIR}/config/qzforms.conf ]; then
    install -m 640 -o ${ADMINUSER} -g ${RUNGROUP} ${INSTALL_FROM}/qzforms.conf \
        ${INSTALLDIR}/config
fi

##
##  A place for the log files.
##
install -d -m 750 -o ${RUNUSER} -g ${FORMDEVGROUP} ${INSTALLDIR}/logs

##
##  The run directory will have an IPC socket and PID files.
##  The socket in particular must be protected from 
##  outside access.
##
install -d -m 770 -o ${RUNUSER} -g ${ADMINGROUP} ${INSTALLDIR}/run

##
##  Create a place for the various sql scripts
install -d -m 775 -o ${ADMINUSER} -g  ${ADMINGROUP} ${INSTALLDIR}/sql

install  -m 664 -o ${ADMINUSER} -g ${ADMINGROUP} \
    ${INSTALL_FROM}/qz_db_install_SV*.sql  \
    ${INSTALL_FROM}/qz_db_update_SV*.sql \
    ${INSTALL_FROM}/qzforms_examples.sql \
    ${INSTALLDIR}/sql

##
##  Final Instructions
##
echo 
echo "You will need to construct a spawn-fcgi command line that matches"
echo "the options above by editing ${INSTALLDIR}/libexec/qzforms.init or"
echo "setting the options for your init process."
echo ""
echo "${INSTALLDIR}/config/qzforms.conf will control how qzforms talks"
echo "to PostgreSQL and sets how qzforms itself runs."
echo ""
echo "The (SOCKET_ADDR and SOCKET_PORT) or (SOCKET_PATH) in qzforms.init"
echo "must match the configuration in your web server."

