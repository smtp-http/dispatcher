#!/bin/bash

killall -0 supervisord 1>/dev/null 2>&1
if [ "$?" -ne "0" ];then
	echo "supervisord is stopped"
	/etc/init.d/supervisord restart
	if [ "$?" -ne "0" ];then
		echo "supervisord restart failed,check it!!!"
		exit -1
	fi
	chkconfig --level 345 supervisord on
fi	
	
supervisorctl start dispserver