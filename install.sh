#!/bin/bash

if [ ! -d tmp ]; then
    mkdir tmp
fi

if [ ! -e tmp/pali_text.txt ]; then
    tar zxf material/pali_text.tgz -C tmp/
fi

