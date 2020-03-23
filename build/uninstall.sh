#!/bin/bash

DISPSERVER_ROOT_DIR=$PWD
SUPERVISOR_CONF=/etc/supervisord.conf

if [ -f ${PWD}/dispserver_d ]
then
    echo "start uninstall dispserver in ${DISPSERVER_ROOT_DIR} now!"
else
    echo "dispserver not installed in ${DISPSERVER_ROOT_DIR}.see you!"
    exit 0 
fi

echo "===============stop all service before uninstall===================="
supervisorctl stop all

#unreg dispserver from supervisord
python mod_supervisord.py ${SUPERVISOR_CONF} ${DISPSERVER_ROOT_DIR} unreg 127.0.0.1

echo "===============restart supervisord now===================="
/etc/init.d/supervisord restart

echo "===============remove dispserver now===================="
rm -rf ${DISPSERVER_ROOT_DIR}

echo "===============uninstall dispserver done===================="
