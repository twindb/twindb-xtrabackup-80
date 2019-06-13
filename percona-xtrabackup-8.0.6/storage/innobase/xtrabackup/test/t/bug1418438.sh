###############################################################################
# Bug #1418438: innobackupex --compress only compress innodb tables
###############################################################################

start_server

mysql -e "CREATE TABLE test (A INT PRIMARY KEY) ENGINE=MyISAM" test

xtrabackup --compress --backup --include=test.test --target-dir=$topdir/backup

diff -u <(LANG=C ls $topdir/backup/test | sed 's/[0-9]/x/g') - <<EOF
test.MYD.qp
test.MYI.qp
test_xxx.sdi.qp
EOF
