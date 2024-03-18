#!/usr/bin/env bash

set -ex

PACKAGE_NAME=tdx-quote-verification-sample
SCRIPT_DIR=$(dirname "$0")
ROOT_DIR="${SCRIPT_DIR}/../../.."
SGX_VERSION=$(awk '/STRFILEVER/ {print $3}' ${ROOT_DIR}/QuoteGeneration/common/inc/internal/se_version.h|sed 's/^\"\(.*\)\"$/\1/')
RPM_BUILD_FOLDER=${PACKAGE_NAME}-${SGX_VERSION}

pre_build() {
	rm -fR ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}
	mkdir -p ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
	cp -f ${SCRIPT_DIR}/${PACKAGE_NAME}.spec ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}/SPECS
}

post_build() {
	for FILE in $(find ${SCRIPT_DIR}/${RPM_BUILD_FOLDER} -name "*.rpm" 2> /dev/null); do
		cp "${FILE}" ${SCRIPT_DIR}
	done
	rm -fR ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}
}

update_spec() {
	pushd ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}
	sed -i "s/@version@/${SGX_VERSION}/" SPECS/${PACKAGE_NAME}.spec
	popd
}

create_upstream_tarball() {
	tar -czvf ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}/SOURCES/${RPM_BUILD_FOLDER}.tar.gz \
		--exclude-vcs-ignores \
		--exclude-vcs \
		--exclude ./dist \
		--exclude ${RPM_BUILD_FOLDER}.tar.gz \
		--transform "s,^./,${RPM_BUILD_FOLDER}/," \
		$(realpath --relative-to ${PWD} ./dist/..)
}

build() {
	pushd ${SCRIPT_DIR}/${RPM_BUILD_FOLDER}
	rpmbuild --define='_debugsource_template %{nil}' --define="_topdir `pwd`" -ba SPECS/${PACKAGE_NAME}.spec
	popd
}

main() {
	pre_build
	update_spec
	create_upstream_tarball
	build
	post_build
}

main $@
