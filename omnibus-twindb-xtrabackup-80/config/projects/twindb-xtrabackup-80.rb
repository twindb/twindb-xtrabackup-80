#
# Copyright 2019 TwinDB LLC
#
# All Rights Reserved.
#

name 'twindb-xtrabackup-80'
maintainer 'TwinDB Packager (TwinDB packager key) <packager@twindb.com>'
homepage 'https://twindb.com'

install_dir "#{default_root}/#{name}"

build_version '8.0.6'
build_iteration 1

# Creates required build directories
dependency 'preparation'
dependency 'twindb-xtrabackup-80'
dependency 'version-manifest'

exclude '**/.git'
exclude '**/bundler/git'
