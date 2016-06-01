################################
# INICPP Makefile for GNU make #
################################

# This Makefile is only provided as part of submission of Recommended
# Programming Practices class homework and is not meant for normal
# usage. Inicpp provides cmake build scipt, which is multiplatform
# and easily used in bigger projects with submodules (instead of
# nonportable GNU make).
#
# Inicpp uses Google Test unit testing framework, which is also build
# by cmake, so it's pointless to write here old fashioned make script
# (but we can do it pretty good). Use cmake instead - instructions are
# in README.md file).


# Build library (shared and static) and unit tests using Unix Makefiles
# in build subdirectory
.PHONY: build

build:
	mkdir -p build
	cd build; cmake -G "Unix Makefiles" -DBUILD_STATIC=ON -DBUILD_SHARED=ON ..
	cd build; make -j 4
	cd build; make -j 4 -f tests/Makefile


# Run unit tests
.PHONY: test

test: build
	./build/tests/run_tests


# Build and run examples
.PHONY: examples

examples: build
	cd build; make -j 4 -f examples/Makefile
	./build/examples/basic/inicpp_basic
	./build/examples/schema_validation/inicpp_schema

