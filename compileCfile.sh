#!/bin/bash
# warning=0
# error=0
# result=0

if [ $# -eq 0 ]; then
	echo "Usage: $0 <filename.c> use a c file"
	exit 1
fi

if test -f "$1"; then
	result=$(gcc -Wall -o m "$1" 2>&1 | tee /dev/tty)
	error=$(echo "$result" | grep -c "error" | wc -l)
	warning=$(echo "$result" | grep -c "warning" | wc -l)

	echo "warnings: $warning"
	echo "errors: $error"
else
	echo "use a C file"
fi
