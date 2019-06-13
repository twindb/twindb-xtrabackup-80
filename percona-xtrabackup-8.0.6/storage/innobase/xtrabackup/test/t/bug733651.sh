########################################################################
# Bug #733651: innobackupex not stores some critical
# innodb options in backup-my.cnf
########################################################################

. inc/common.sh

options="innodb_log_files_in_group innodb_log_file_size"

# innodb_page_size is supported in XtraDB 5.1+ and InnoDB 5.6+
if is_xtradb || is_server_version_higher_than 5.6.0
then
    options="$options innodb_page_size"
fi

start_server

xtrabackup --backup --target-dir=$topdir/backup

# test presence of options
for option in $options ; do

        if ! cat $topdir/backup/backup-my.cnf | grep $option
        then
                vlog "Option $option is absent"
                exit -1
        else
                vlog "Option $option is present"
        fi

done
