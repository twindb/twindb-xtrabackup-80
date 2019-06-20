#!/usr/bin/env bash

set -ex

cd /twindb-xtrabackup-80/omnibus-twindb-xtrabackup-80

bundle install --binstubs
bin/omnibus build twindb-xtrabackup-80 --log-level debug
