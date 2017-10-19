# CS3411 Encode/Decode Tests

using the specified tests on the assignment page using a test text file resulted in no difference between files:

./encode test.txt | ./decode decoded.txt
diff test.txt decoded.txt

Using a bash script (located in tests/largetest.sh), I created a file of random bytes, encoded it, decoded it and used diff to check if the files are the same