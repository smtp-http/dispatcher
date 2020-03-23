#!/bin/bash

. util.functions.sh


check_python()
{
	check_program "python" 
	if [ $? -ne 0 ]
	then 
	    echo "please install python 2.6 first."
		return 1
	fi
	
	#TODO:check python version here
	
	return 0
}


check_python
if [ $? -ne 0 ]
then
   exit 1
fi
exit 0
