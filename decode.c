#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "strings.h"

int main(int argc, const char ** argv) {

    // First we are going to just try counting the characters in the file
    if(argc != 2) {
        printf("incorrect input dummy\n");
        return 0;
    }

    // Get the file name of the input file
    char filename[strlen(argv[1])];
    strcpy(filename, argv[1]);

    // Open the file supplied
    int file = open(filename, O_RDWR|O_CREAT, 0644);
    if(file == -1) {
        fileError(fileReadError, filename);
        return 0;
    }

    // Create the dictonary and read the 15 dictionary bytes from stdin
    unsigned char dictionary[16];
    dictionary[0] = 0;
    for(int i = 1; i < 16; i++) {
        read(0, &dictionary[i], sizeof(unsigned char));
    }

    // Create are queued read byte
    qbyte readBuffer = {0,0};
    unsigned char outputByte = 0;

    // Repeat through the entire file, decoding the input
    //while(1) { 
    for(int i = 0; i < 64; i++) {
        printf("Um\n");
        // Read the first bit from the file
        outputByte = qread(1, &readBuffer, 1);
        //if(i % 8 == 0 && i != 0)
        //    printf("\n");
        //printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(outputByte));     
    }
    printf("\n");

    close(file);
}