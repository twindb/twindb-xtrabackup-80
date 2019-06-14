name 'twindb-xtrabackup-80'
default_version '8.0.6'

skip_transitive_dependency_licensing true
dependency 'libffi'
dependency 'libboost'

source path: '/twindb-xtrabackup-80/percona-xtrabackup-8.0.6/'

build do
    env = with_standard_compiler_flags(with_embedded_path)
    command 'cmake -DBUILD_CONFIG=xtrabackup_release ' \
        "-DCMAKE_INSTALL_PREFIX=#{install_dir} " \
        '-DWITH_SSL=system ' \
        '-DWITH_MAN_PAGES=OFF ' \
        '-DDOWNLOAD_BOOST=0 ' \
        '-DWITH_BOOST=../libboost ' \
        '-DFORCE_INSOURCE_BUILD=1 ' \
        "-DINSTALL_BINDIR=#{install_dir}/embedded/bin", env: env

    make "-j #{workers}", env: env
    make 'install', env: env
    delete "#{install_dir}/libboost"
    delete "#{install_dir}/xtrabackup-test"
    strip "#{install_dir}/embeddedbin"
    copy '/lib64/libaio.so.1', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libcom_err.so.2', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libcrypto.so.10', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libcurl.so.4', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libev.so.4', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libgcrypt.so.11', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libgpg-error.so.0', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libgssapi_krb5.so.2', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libidn.so.11', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libk5crypto.so.3', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libkeyutils.so.1', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libkrb5.so.3', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libkrb5support.so.0', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/liblber-2.4.so.2', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libldap-2.4.so.2', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libnspr4.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libnss3.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libnssutil3.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libpcre.so.1', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libplc4.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libplds4.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libsasl2.so.3', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libselinux.so.1', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libsmime3.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libssh2.so.1', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libssl.so.10', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libssl3.so', '/opt/twindb-xtrabackup-80/embedded/lib'
    copy '/lib64/libz.so.1', '/opt/twindb-xtrabackup-80/embedded/lib'
end
