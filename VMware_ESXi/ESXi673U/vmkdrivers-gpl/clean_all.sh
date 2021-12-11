#!/bin/sh

find -name "*.o"|xargs rm -rf

rm -rf \
BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/moxa_dio \
BLD/build/vmkdriver-moxa_pled-CUR/release/vmkernel64/moxa_pled \
DA820PLED.vib
