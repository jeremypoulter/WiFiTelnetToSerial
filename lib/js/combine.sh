#!/bin/bash

LIB_DIR=$(dirname $(realpath $0))
SRC_DIR=$LIB_DIR/../../src/data

cat $LIB_DIR/jquery.min.js \
    $LIB_DIR/jquery.terminal.min.js \
    $LIB_DIR/jquery.mousewheel.min.js \
    $LIB_DIR/knockout.min.js \
    $LIB_DIR/knockout.mapping.min.js \
    $LIB_DIR/tether.min.js \
    $LIB_DIR/sammy.min.js \
    $LIB_DIR/bootstrap.min.js \
    > $SRC_DIR/lib.js

cat $LIB_DIR/jquery.terminal.min.css \
    $LIB_DIR/bootstrap.min.css \
    > $SRC_DIR/lib.css
