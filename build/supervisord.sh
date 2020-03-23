#!/bin/bash

supervisord_script='/etc/init.d/supervisord'

test_supervisord=`which supervisord 2>/dev/null`
if [ -z ${test_supervisord} ]
then 
    echo 'test supervisor failed.please install supervisor first.' 
	exit 1
fi

test_supervisorctrl=`which supervisorctl 2>/dev/null`
if [ -z ${test_supervisorctrl} ]
then 
    echo 'test supervisorctl failed.please install supervisor first.' 
	exit 1
fi

if chkconfig --list|grep supervisord
then 
    echo "supervisord have registered in system service."
	exit 0
fi

echo "start register supervisord into system service."
echo "copy supervisord script"
cp -f ./supervisord /etc/rc.d/init.d/
if [ $? -ne 0 ]
then 
    "copy supervisord failed"
	exit 1
fi

echo "add supervisord into service list."
chkconfig --add supervisord
if [ $? -ne 0 ]
then 
    "add supervisord into service failed"
	exit 1
fi

chkconfig --level 345 supervisord on

if chkconfig --list|grep supervisord
then 
   echo "register supervisord into system done."
   exit 0
else
   echo "register supervisord into system failed."
   exit 1
fi

