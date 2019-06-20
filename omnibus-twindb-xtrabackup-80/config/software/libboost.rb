name 'libboost'
default_version '1.68.0'

skip_transitive_dependency_licensing true

source path: '/twindb-xtrabackup-80/boost_1_68_0'

build do
  env = with_standard_compiler_flags(with_embedded_path)
end
