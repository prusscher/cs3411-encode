#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "strings.h"

// Debug Logging
#ifndef debug
    #define debug 0
#endif

// USE UNSIGNED CHAR EVERYWHERE.

/*
This should return no difference:
    encode <input-file | decode new.input-file
    diff input-file new.input-file
*/

int main(int argc, const char ** argv) {

    // First we are going to just try counting the characters in the file
    if(argc != 2) {
        write(2, "Incorrect arguments\nUse: encode <file name>\n", strlen("Incorrect arguments\nUse: encode <file name>\n"));
        return 0;
    }

    // Get the file name of the input file
    char filename[strlen(argv[1])];
    strcpy(filename, argv[1]);

    // Open the file supplied
    int file = open(filename, O_RDONLY, 0644);
    if(file == -1) {
        fileError(fileReadError, filename);
        return 0;
    }

    #if debug==1
        // Create a log file for debugging
        int log = open("log.txt", O_RDWR|O_TRUNC|O_CREAT, 0644);
        const int maxLog = 0x1000;  // Maximum amount of logging to perform if we are using it
    #endif

    // Temp character and frequency of each uchar
    //unsigned char c;
    int frequency[256];
    
    // Set the frequencies to all 0
    for(int i = 0; i < 256; i++) frequency[i] = 0;

    // File input buffer and size
    int bufferSize = 4096;
    unsigned char buffer[bufferSize];

    // Total bytes read and the number of bytes read per iteration
    int bytesRead = 0;
    int n_bytes;

    // Till we hit the EOF, document each unsigned byte
    while(1) {
        if((n_bytes = read(file, &buffer, bufferSize)) == 0) {
            break;
        }
        if(n_bytes == -1) {
            fileError(fileReadError, filename);
            return 0;
        }

        bytesRead += n_bytes;

        // Count the character at each position in the buffer
        for(int i = 0; i < n_bytes; i++)
            frequency[(unsigned int)buffer[i]]++;
    }

    // Reset totalBytes read to use it for the encription portion
    bytesRead = 0;

    // Dictionary of the 15 largest characters starting at 1-Most 15-LeastMost
    unsigned char dictionary[16];
    dictionary[0] = 0;

    // Get the 15 largest characters in terms of count in the program
    for(int i = 0; i < 15; i++) {
        int nextLargest = frequency[0];
        int nextIndex = 0;

        for(int c = 0; c < 256; c++) {
            if(frequency[c] > nextLargest) {
                nextLargest = frequency[c];
                nextIndex = c;
            }
        }

        // Add the next largest character
        dictionary[i + 1] = nextIndex;
        frequency[nextIndex] *= -1; // Reverse the frequence to indicate we used it
    }

    // Re-negated the dictionary bois if they exist
    for(int i = 1; i < 16; i++) if(frequency[dictionary[i]] < 0) frequency[dictionary[i]] *= -1;

    // Debug Dictionary Printing
    #if debug==1
        write(log, "Dictionary:\n", strlen("Dictionary:\n"));
        for(int i = 1; i < 16; i++) {
            int d = dictionary[i];
            dprintf(log, "%d - %d\n", i, d);
        }    
    #endif

    // We have our dictionary now, Now were going to encode our file.
    lseek(file, 0, SEEK_SET);

    int bytesW;

    // Write the dictionary to the archive
    for(int i = 1; i < 16; i++) {
        bytesW = write(1, &dictionary[i], sizeof(unsigned char));
        if(bytesW < sizeof(unsigned char)) {
            fileError(dictWriteError, "");
        }
    }

    // Buffered byte that we are reading from for the entirity of the file
    qbyte bufferedByte = {0,0};

    // Use the trusty whlie loop to cycle through all the bytes of the file
    while(1) {
        if((n_bytes = read(file, &buffer, bufferSize)) == 0) {
            break;
        }
        if(n_bytes == -1) {
            fileError(fileReadError, filename);
            return 0;
        }

        bytesRead += n_bytes;

        // Now we have our buffer of bytes, cycle through and translate to encoded values
        for(int i = 0; i < n_bytes; i++) {
            // Check if the byte is a null byte and write accordinly if it is
            if(buffer[i] == 0) {    // file: [00000000] -> Encode: [10]
                #if debug==1
                    if(bytesRead < maxLog) dprintf(log, "%d\t0: 10\n", bytesRead);
                #endif

                unsigned char input = 2;
                qwrite(1, &bufferedByte, input, 2);
                continue;
            }

            // Check if item is in the dictionary
            int inDict = 0;
            for(int d = 0; d < 16; d++) {
                if(dictionary[d] == buffer[i]) {
                    inDict = 1;
                    break;
                }
            }

            if(inDict) {
                // Find the element in the dictionary
                int dictIndex = -1;
                for(int d = 0; d < 16; d++) {
                    if(dictionary[d] == buffer[i]) {
                        dictIndex = d;
                        break;
                    }
                }

                // If we didn't for some reason find the index of the byte in the dictionary, leave
                if(dictIndex == -1) {
                    write(2, wrongDictElement, strlen(wrongDictElement));
                    return 0;
                }
                
                // Count the number of times the byte occurs
                int count = 1;
                for(int c = 1; c < 4; c++) {
                    if(buffer[i+c] == buffer[i])
                        count++;
                    else
                        break;
                }

                // Skip the next bytes because we are expressing them as compressed
                i += count-1;

                // Using our count, write the byte
                unsigned char input = 3 << 6;   // Set the last two bytes
                input = input | ((count-1) << 4);   // Add the count of up to 4 bytes
                input = input | dictIndex;      // Add our last 4 bytes for the dictionary element
                
                qwrite(1, &bufferedByte, input, 8);

                #if debug==1
                    if(bytesRead < maxLog) dprintf(log, "%d\tF: "BYTE_TO_BINARY_PATTERN" - %d\n", bytesRead, BYTE_TO_BINARY(input), count);
                #endif
            } else {
                // Symbol is infrequent, writing a 0 and then the byte
                unsigned char input = 0;
                qwrite(1, &bufferedByte, input, 1);
                input = buffer[i];
                qwrite(1, &bufferedByte, input, 8);
                
                #if debug==1
                    if(bytesRead < maxLog) dprintf(log, "%d\tI: %d_"BYTE_TO_BINARY_PATTERN"\n", bytesRead, 0, BYTE_TO_BINARY(input));
                #endif

                continue;
            }
        }
    }

    // Write the end of file tag to the end of stdout
    // Note to self, place the EOF outside the while loop so you don't get an EOF at the end of each buffer... Thats bad.
    unsigned char input = 192; // 11 00 0000
    qwrite(1, &bufferedByte, input, 8);
    qflush(1, &bufferedByte);

    #if debug==1
        if(total < maxLog) dprintf(log, "EOF\n");
        close(log);
    #endif
    
    close(file);
}