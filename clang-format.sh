#!/bin/bash

DIR=`dirname $0`
ROOT=$DIR

function do_format {
    pushd $1 > /dev/null

    for CURFILE in ./*.$2; do
        clang-format -style=file -i $CURFILE
    done

    popd > /dev/null
}

# application
do_format $ROOT/include/pngpp hpp
do_format $ROOT/src           cpp
