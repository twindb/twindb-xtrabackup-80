.DEFAULT_GOAL := help
define BROWSER_PYSCRIPT
import os, webbrowser, sys
try:
	from urllib import pathname2url
except:
	from urllib.request import pathname2url

webbrowser.open("file://" + pathname2url(os.path.abspath(sys.argv[1])))
endef
export BROWSER_PYSCRIPT

define PRINT_HELP_PYSCRIPT
import re, sys

for line in sys.stdin:
	match = re.match(r'^([a-zA-Z_-]+):.*?## (.*)$$', line)
	if match:
		target, help = match.groups()
		print("%-40s %s" % (target, help))
endef
export PRINT_HELP_PYSCRIPT
BROWSER := python -c "$$BROWSER_PYSCRIPT"

PYTHON := python
PYTHON_LIB := $(shell $(PYTHON) -c "from distutils.sysconfig import get_python_lib; import sys; sys.stdout.write(get_python_lib())" )


PLATFORM ?= centos
OS_VERSION ?= 7

pwd := $(shell pwd)
DOCKER_IMAGE := $(shell if test -z "${DOCKER_IMAGE}"; then echo "centos:centos7"; else echo ${DOCKER_IMAGE}; fi)

.PHONY: help
help:
	@python -c "$$PRINT_HELP_PYSCRIPT" < $(MAKEFILE_LIST)

.PHONY: clean
clean: clean-build ## remove all artifacts

.PHONY: clean-build
clean-build: ## remove build artifacts
	rm -rf omnibus/pkg/

.PHONY: docker-start
docker-start:
	@docker run \
		-v ${pwd}:/twindb-xtrabackup-80 \
		-it \
		--name builder_twindb-xtrabackup-80 \
		--rm \
		--dns 8.8.8.8 \
		--dns 208.67.222.222 \
		--env PLATFORM=${PLATFORM} \
		--env OS_VERSION=${OS_VERSION} \
		"twindb/omnibus-${PLATFORM}:backup-${OS_VERSION}" \
		bash -l


.PHONY: package
package: ## Build package - PLATFORM must be one of "centos", "debian", "ubuntu". OS_VERSION must be: 6, 7, jessie, stretch, xenial, bionic, cosmic.
	@docker run \
		-v ${pwd}:/twindb-xtrabackup-80 \
		--name builder_twindb-xtrabackup-80 \
		--rm \
		--dns 8.8.8.8 \
		--dns 208.67.222.222 \
		--env PLATFORM=${PLATFORM} \
		--env OS_VERSION=${OS_VERSION} \
		"twindb/omnibus-${PLATFORM}:backup-${OS_VERSION}" \
		bash -l /twindb-xtrabackup-80/omnibus-twindb-xtrabackup-80/omnibus_build.sh
