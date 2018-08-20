#drop database if exists cwlan;

#create database cwlan;

#use cwlan;

create table IF NOT EXISTS T_ap_info
(
	Id int unsigned auto_increment,
	Mac varchar(20) not null,
	Online_state int,
	Ap_group_id int,
	PRIMARY KEY (`Id`)
);

create table IF NOT EXISTS T_admin
(
	Id int unsigned auto_increment,
	User varchar(32) not null,
	Password varchar(32) not null,
	Admin_level int,
	PRIMARY KEY (`Id`)
);

create table IF NOT EXISTS T_ap_base_config
(
	Id int unsigned auto_increment,
	Logon_user varchar(32) not null,
	Logon_Password varchar(32) not null,
	Ssid varchar(32) not null,
	channel int,
	power_rate int,
	PRIMARY KEY (`Id`)
);

create table IF NOT EXISTS T_user_white_list
(
	Mac varchar(20),
	Online_time int,
	PRIMARY KEY (`Mac`)
);

create table IF NOT EXISTS T_ap_url_white_list
(
	Id int unsigned auto_increment,
	url varchar(128) not null,
	PRIMARY KEY (`Id`)
);

create table IF NOT EXISTS T_ap_portal_list
(
	Id int unsigned auto_increment,
	url varchar(128) not null,
	PRIMARY KEY (`Id`)
);

create table IF NOT EXISTS T_ap_group_list
(
	Id int unsigned auto_increment,
	Portal_id int,
	#用户白名单全局下
	#url白名单全局下
	#多选 
	Ap_info_id_list int,
	Ap_base_config_id int,
	PRIMARY KEY (`Id`)
);

create table IF NOT EXISTS T_communicat
(
	#apmac地址
	Ap_Id int,
	Ap_Mac varchar(20) not null,
	#需要下发配置的数据库表名
	Table_name varchar(64) not null,
	#表中数据的id号
	Id int,
	#	//需要下发配置的字段
	Field_name varchar(64) not null
);


create table IF NOT EXISTS T_user_online_log
(
	User_Mac  varchar(20) not null,
	Ap_mac  varchar(20) not null,
	Logon_time time,
	Auth_time time,
	Logout_time time
);

create table IF NOT EXISTS T_user_url_log
(
	User_admin  varchar(32) not null,
	User_Mac  varchar(20) not null,
	Ap_mac  varchar(20) not null,
	Saddr  varchar(20) not null,
	Daddr  varchar(20) not null,
	Visit_time time,
	url  varchar(128) not null,
	ua  varchar(20) not null,
	host  varchar(20) not null
);

