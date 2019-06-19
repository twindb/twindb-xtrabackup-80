name 'twindb-xtrabackup-80'
default_version '8.0.6'

skip_transitive_dependency_licensing true
dependency 'libffi'
dependency 'libboost'

whitelist_file 'bin/xtrabackup'
whitelist_file 'bin/xbstream'

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
      '-DWITH_VERSION_CHECK=OFF ' \
      "-DINSTALL_BINDIR=#{install_dir}/embedded/bin", env: env

  make env: env
  make 'install', env: env
  delete "#{install_dir}/libboost"
  delete "#{install_dir}/embedded/bin/xbcloud"
  delete "#{install_dir}/embedded/bin/xbcloud_osenv"
  delete "#{install_dir}/embedded/bin/xbcrypt"
  delete "#{install_dir}/lib/plugin/keyring_vault.so"
  delete "#{install_dir}/lib/plugin/keyring_file.so"
  delete "#{install_dir}/xtrabackup-test"
  strip "#{install_dir}/embedded/bin"
end
