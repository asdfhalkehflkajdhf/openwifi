#gcc collect.c -o collect `mysql_config --cflags --libs`

配置文件说明
项目名　＝　参数

程序说明
程序名　配置文件路径

表结构
CREATE TABLE IF NOT EXISTS recordcollect
(
	consume_id int unsigned not null  auto_increment,
	stop_time DATETIME,
	Framed_IP_Address varchar(15),
	Acct_Session_Id	 varchar(30),
	NAS_Port int unsigned,
	NAS_Port_Type varchar(64),
	Calling_Station_Id varchar(64),
	Acct_Delay_Time int unsigned,
	Acct_Session_Time int unsigned,
	Acct_Input_Octets int unsigned,
	Acct_Input_Gigawords int unsigned,
	Acct_Output_Octets int unsigned,
	Acct_Output_Gigawords int unsigned,
	Acct_Input_Packets int unsigned,
	Acct_Output_Packets int unsigned,
	Proxy_State int unsigned,
	User_Name varchar(64),
	NAS_IP_Address varchar(15),
	Acct_Unique_Session_Id varchar(64),
	Timestamp int unsigned,

	primary key(consume_id)
);
