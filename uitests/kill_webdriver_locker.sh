#!/bin/bash

RETPID=`lsof -t -i TCP:7054`
if [ "$RETPID" != "" ] ; then
	echo "Killing WebDriver locker with PID $RETPID"
	kill -9 $RETPID 
fi
