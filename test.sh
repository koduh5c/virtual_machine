#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <folder name>"
    exit 1
fi

tests="$1"

echo -e "\n=========================================\n"
echo -e "              RUNNING TESTS              "
echo -e "\n=========================================\n"

for t in "$tests"/*; do
    if [ ! -d "$t" ]; then
        continue
    fi
    
    fn="${t##*/}"
    txt_file="$t/$fn.txt"
    out_file="$t/$fn.out"
    in_file="$t/$fn.in"
    mi_file="$t/$fn.mi"
    
    echo -e "\nTest: $fn\n"
    echo -e "Description: $(cat $txt_file)\n" 

    dos2unix "$out_file" >/dev/null 2>&1

    if [ -f "$in_file" ]; then
        output="$(./vm_riskxvii "$mi_file" < "$in_file")"
    else
        output="$(./vm_riskxvii "$mi_file")"
    fi

    if diff -q "$out_file" <(echo "$output") >/dev/null; then
        echo -e "Test PASSED"
    else
        echo -e "Test FAILED"
        echo -e "\n"
        echo "Actual:"
        echo "$output"
        echo -e "\n"
        echo "Expected:"
        cat "$out_file"
    fi
    echo -e "\n-----------------------------------------\n"
done
