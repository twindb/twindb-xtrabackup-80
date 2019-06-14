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
end
