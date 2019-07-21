#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "Usage: get_starting_addr_after.sh elf-binary"
	echo " Print the starting address of next os component after elf-binary in IX"
	exit
fi
FIRST_EMPTY_SECTION=`readelf -S $1 | grep 00000000 | head -n 2 | tail -n 1`
FIRST_EMPTY_SECTION_NUM=`echo ${FIRST_EMPTY_SECTION} | grep -oE "\[ [0-9]\]" | grep -oE [0-9]`
let LAST_VALID_SECTION_NUM=${FIRST_EMPTY_SECTION_NUM}-1
LAST_VALID_SECTION=`readelf -S $1 | grep "\[ ${LAST_VALID_SECTION_NUM}\]"`
#echo ${FIRST_EMPTY_SECTION_NUM}
#echo ${LAST_VALID_SECTION_NUM}
#echo ${LAST_VALID_SECTION}
LAST_VALID_SECTION_ADDR=`echo ${LAST_VALID_SECTION} | awk '{print $5}'`
LAST_VALID_SECTION_SIZE=`echo ${LAST_VALID_SECTION} | awk '{print $7}'`
LAST_VALID_SECTION_ADDR="0x${LAST_VALID_SECTION_ADDR}"
LAST_VALID_SECTION_SIZE="0x${LAST_VALID_SECTION_SIZE}"
printf "%x\n" $((${LAST_VALID_SECTION_ADDR} + ${LAST_VALID_SECTION_SIZE}))

