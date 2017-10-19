#!/usr/bin/env bash
# 64Mb file DIFF Test
{
    # Make Encode/Decode
    pushd ../
        make all
    popd

    # Create a moderatly sized test file
    dd if=/dev/urandom of=./test bs=4M count=16 iflag=fullblock

    # Encode the random input and decode it to the file: testOut
    ../encode test | ../decode testOut
} #> /dev/null # Don't hide the output, I have progress printing

# Compute the DIFF and store the result if there is one
result = $(diff -y -W 64 test testOut)
if [  $? -eq 0  ]; then
    echo "Test passed\n"
    rm ./test > /dev/null
    rm ./testOut > /dev/null
else
    echo "Test failed!\n"
    echo "$result"
    rm ./test > /dev/null
    rm ./testOut > /dev/null
    exit 1
fi