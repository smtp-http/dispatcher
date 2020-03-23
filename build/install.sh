#!/bin/bash
. util.functions.sh

export DISPSERVER_ROOT_DIR=/usr/local/DispatchServer
if [ -z "$1" ]
then
   echo "you dont input install dir for dispserver.we will use ${DISPSERVER_ROOT_DIR} as default." 
else 
   DISPSERVER_ROOT_DIR=$1
   echo "you input install dir ${DISPSERVER_ROOT_DIR} for dispserver."
fi


if [ -f "${DISPSERVER_ROOT_DIR}/dispserver_d" ]
then
   echo "install dir is ${DISPSERVER_ROOT_DIR},but exists a dispserver_d in this dir.check it and uninstall first."
   exit 1
fi 

if [ -d "/usr/local/logs" ]
then
    echo "/usr/local/logs exists now"
else
    mkdir /usr/local/logs
fi

echo "===============start check preinstall now===================="
./pre_install_dispserver.sh
if [ $? -ne 0 ]
then
   exit 1 
fi
echo "===============check preinstall ok===================="

echo "===============start check supervisord now===================="
./supervisord.sh
if [ $? -ne 0 ]
then 
   exit 1
fi
echo "===============check supervisord ok===================="


echo "===============stop all service before install===================="
supervisorctl stop all

echo "===============start install now===================="

if [ -d "${DISPSERVER_ROOT_DIR}" ]
then
   echo "${DISPSERVER_ROOT_DIR} is exist now,rename it first"
   CUR_TIME=`date "+%Y-%m-%d-%H-%M-%S"`
   mv ${DISPSERVER_ROOT_DIR} "${DISPSERVER_ROOT_DIR}_${CUR_TIME}"
else
   echo "${DISPSERVER_ROOT_DIR} is not exist now,first mkdir."
fi

rm -rf ${DISPSERVER_ROOT_DIR}
mkdir -p ${DISPSERVER_ROOT_DIR}
if [ -d ${DISPSERVER_ROOT_DIR} ]
then
   echo "${DISPSERVER_ROOT_DIR} create success." 
else
   echo "create ${DISPSERVER_ROOT_DIR} failed.please check path."             
fi


echo "unzip install.tar.gz now"
tar -zxf install.tar.gz 
echo "unzip install.tar.gz done"


service_ip="127.0.0.1"
echo 'please input dispatch service ip now:'

ip_regex="^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])$"    
while read input_ip
do
   echo "input_ip:${input_ip}"
   check_res=`echo ${input_ip} | grep -E ${ip_regex} | wc -l`
   if [ ${check_res} -eq 1 ]
   then
      echo "service_ip=${input_ip}"
      service_ip=${input_ip}
      break
   else
      echo "input_ip:${input_ip} is a invalid ip.please input a valid ip."
   fi
done


sed -i "s/\(IP=\).*/\1${service_ip}/g" ./install/config.ini
echo "dispatch_server_ip set ok"
echo "start copy file now"
cp -rf ./install/* ${DISPSERVER_ROOT_DIR}/
echo "copy file done"

chmod -R +x ${DISPSERVER_ROOT_DIR}

#modify supervisord conf 
SUPERVISOR_CONF=/etc/supervisord.conf
if [ -f "${SUPERVISOR_CONF}" ]
then 
    echo "supervisord_conf ${SUPERVISOR_CONF} exists"
else
    echo "create supervisord_conf ${SUPERVISOR_CONF} now"
    echo_supervisord_conf > ${SUPERVISOR_CONF}
fi

python mod_supervisord.py ${SUPERVISOR_CONF} ${DISPSERVER_ROOT_DIR} reg ${service_ip}

echo "===============install dispserver done in ${DISPSERVER_ROOT_DIR}===================="

echo "===============restart supervisord now===================="
/etc/init.d/supervisord restart

