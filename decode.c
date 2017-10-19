#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "strings.h"

#ifndef debug
    #define debug = 0
#endif

int main(int argc, const char ** argv) {

    // First we are going to just try counting the characters in the file
    if(argc != 2) {
        write(2, "Incorrect arguments\nUse: encode <file name>\n", strlen("Incorrect arguments\nUse: encode <file name>\n"));
        return 0;
    }

    // Get the file name of the input file
    char filename[strlen(argv[1])];
    strcpy(filename, argv[1]);

    // Check if the file to output to already exists
    if(fileExist(filename)) {
        fileError(fileExistsError, filename);
        return 0;
    }

    // Total Bytes that we have written
    int totalWrite = 0;

    // Open the file supplied
    int file = open(filename, O_RDWR|O_CREAT, 0644);
    if(file == -1) {
        fileError(fileReadError, filename);
        return 0;
    }

    // Create the dictonary and read the 15 dictionary bytes from stdin
    unsigned char dictionary[16];
    dictionary[0] = 0;
    for(int i = 1; i < 16; i++) read(0, &dictionary[i], sizeof(unsigned char));

    // Create are queued read byte
    qbyte readBuffer = {0,0};
    unsigned char readByte = 0;

    // File input buffer and size
    // int bufferSize = 4096, bufferMax = 0;
    // int bufferIndex = 0;
    // unsigned char buffer[bufferSize];
    
    int bytesW; // Temporary value to check how many bytes were written to the file 

    // Repeat through the entire file, decoding the input
    while(1) {

        // Read the first bit to check and see what our next move should be
        readByte = qread(0, &readBuffer, 1);
        
        // Frequent Symbol
        if(readByte == 1) { // Bit Pattern: [1...
            // Grab the next bit to see if its 0 or not
            readByte = qread(0, &readBuffer, 1);

            if(readByte == 1) { // Bit Pattern: [10...
                // Grab the next 6 bytes to check and see what the dict element and repeat count are
                readByte = qread(0, &readBuffer, 6);                
                int repeat = readByte >> 4;     // Get the repeat count
                int dictIndex = readByte & 15;  // grab the last 4 bits for the dictionary index
                
                // Check that the repeat and dictionary index are in range
                if(repeat > 3 || repeat < 0) 
                    printf("REPEAT ERROR\n");
                if(dictIndex < 0 || dictIndex > 15)
                    printf("DICTIONARY ERROR\n");
                
                // Check for end of file.
                if(repeat == 0 && dictIndex == 0) { // Bit Pattern: [11 00 0000]
                    // End of file found, break out of the while loop
                    break;
                } else { // Bit Pattern: [11 ** ****];
                    // Write the frequent byte to the file as well as each time it repeats
                    bytesW = write(file, &dictionary[dictIndex], sizeof(unsigned char));
                    totalWrite += bytesW + repeat;
                    for(int r = 0; r < repeat; r++)
                        bytesW = write(file, &dictionary[dictIndex], sizeof(unsigned char));        
                }
            } else {    // Bit Pattern: [10]
                // Found a 0 byte, writing a full 0 byte
                bytesW = write(file, &dictionary[0], sizeof(unsigned char));
                totalWrite++;
            }

        } else if(readByte == 0) {    // Bit Pattern [0********] - Infrequent byte, grabbing a full byte and writing
            readByte = qread(0, &readBuffer, 8);
            bytesW = write(file, &readByte, sizeof(unsigned char));
            totalWrite++;
        } else {
            printf("COMPLETE F A I L U R E\n");
        }

        // Progress printing
        // Print the progress every 4Kb
        if(totalWrite % 4096 == 0) {
            printf("Written: %d\r", totalWrite/4096);
            fflush(stdout);
        }

        if(bytesW != 1) {
            // printf("should be only writing one byte at a time, something is wrong\n");
            write(2, "Write to file error, we should be writing only 1 byte at a time.\n", strlen("Write to file error, we should be writing only 1 byte at a time.\n"));
            
            close(file);
            return 0;
        }
    }    
    
    close(file);
}