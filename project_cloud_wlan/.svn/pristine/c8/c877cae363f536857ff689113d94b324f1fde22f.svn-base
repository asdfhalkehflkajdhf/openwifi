mkdir -p ./config/ipipe_xml_interface/result/
mkdir -p ./config/ipipe_xml_interface/issue/

compar_ctime()
{

	src_t=`stat $1  | grep -i Modify | awk -F. '{print $1}' | awk '{print $2$3}'| awk -F- '{print $1$2$3}' | awk -F: '{print $1$2$3}'`
	des_t=`stat $2  | grep -i Modify | awk -F. '{print $1}' | awk '{print $2$3}'| awk -F- '{print $1$2$3}' | awk -F: '{print $1$2$3}'`

	if [ "$src_t" -gt "$des_t" ]
	then
		cp $1 $2 -rf
	fi

	#return $(($1+$2));
}

USR_CONFIG_DIR="./usr/config"
CONFIG_DIR="./config"

for filename in `ls $USR_CONFIG_DIR`
do
	usr_config_file=$USR_CONFIG_DIR/$filename
	config_file=$CONFIG_DIR/$filename
		
	if [ "$filename" = "db" ]
	then
		if [ ! -d $config_file ] 
		then
			cp -rf $usr_config_file $CONFIG_DIR/
		else
			for sqlname in `ls $usr_config_file`
			do
				usr_sqlfile=$usr_config_file/$sqlname
				cfg_sqlfile=$config_file/$sqlname
				if [ ! -f "$usr_sqlfile" ] 
				then
					cp -rf $usr_sqlfile $cfg_sqlfile
					compar_ctime $usr_sqlfile $cfg_sqlfile
				fi
			done
		fi
	else
		if [ ! -f $config_file ]
		then
			cp $usr_config_file $config_file -rf
		else
			compar_ctime $usr_config_file $config_file
		fi
	fi
done

