#!/bin/sh -ex

cd ${CONVEY_WORKSPACE}

BUILD_DIR=${TARGET:-_build}

meson ${MESON_OPTIONS} "${BUILD_DIR}"
meson compile -C "${BUILD_DIR}"
meson test -C "${BUILD_DIR}"
