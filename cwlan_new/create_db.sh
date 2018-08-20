HOSTNAME="localhost"
PORT="3306"
USERNAME="root"
PASSWORD="lzc123"
DBNAME="cwlan"
SQLFILE="./cwlan.sql"

mysql -h${HOSTNAME}  -P${PORT}  -u${USERNAME} -p${PASSWORD}<< EOF 2>/dev/null
create database IF NOT EXISTS $DBNAME;
quit
EOF

if [ -f ${SQLFILE} ]; then
	mysql -h${HOSTNAME}  -P${PORT}  -u${USERNAME} -p${PASSWORD} ${DBNAME} <${SQLFILE}
fi


