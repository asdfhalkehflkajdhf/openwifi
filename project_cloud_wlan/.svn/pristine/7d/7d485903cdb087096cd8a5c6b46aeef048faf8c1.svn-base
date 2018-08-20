#!/bin/bash

HOSTNAME="localhost"
PORT="13306"
USERNAME="cloudac"
PASSWORD="cloudac"
DBNAME="cloudac_release"
SQLFILE="./collect.sql"

if [ -f ${SQLFILE} ]; then 
	mysql -h${HOSTNAME}  -P${PORT}  -u${USERNAME} -p${PASSWORD} ${DBNAME} <${SQLFILE}
	rm ${SQLFILE} -rf
fi
