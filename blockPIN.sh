#!/bin/bash
if [ $# != 2 ]
then 
	echo Usage: `basename $0` readerID pinNumber
	exit 1
fi
pkcs15-tool --reader $1 --change-pin --auth-id $2 --pin 00000 --new-pin 00900 2> /dev/null
pkcs15-tool --reader $1 --change-pin --auth-id $2 --pin 00000 --new-pin 00900 2> /dev/null
pkcs15-tool --reader $1 --change-pin --auth-id $2 --pin 00000 --new-pin 00900 2> /dev/null
