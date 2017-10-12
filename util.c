#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "util.h"

/**
 * Returns 1 if a file exists and 0 if the file doesn't exist
 * char * file: The filename to check for existence
 * */
int fileExist(char * file)
{
    struct stat buf;   
    return (stat (file, &buf) == 0);
}

/**
 * Uses stat to determine the size of a file in number of bytes
 * char * file: The filename to check the size of
 * 
 * returns: The file's size in Bytes
 * */
int fileSize(char * file)
{
    struct stat buf;   
    if(stat (file, &buf) != 0) {
        printf("There was an error reading the file size?\n");
        return -1;
    }
    return buf.st_size;
}

/**
 * Returns a file error message to stderr
 * const char * errorMsg: The error message itself
 * char * file: The file name that caused an error
 * */
void fileError(const char * errorMsg, char * file) {
    char out[strlen(file) + 1];
    strcpy(out, file);
    strcat(out, "\n");

    write(2, errorMsg, strlen(errorMsg));
    write(2, out, strlen(out));
}

/**
 * 
 * */
unsigned char qread(int fileDes, qbyte * readBuffer, int size) {
    if(size <= 0 || size > 7) {
        write(2, "Invalid Size, Canceling qwrite\n", strlen("Invalid Size, Canceling qwrite\n"));
        return 0;
    }

    // Check to see if the readbuffer has enough stored data to satifsy our request
    if(readBuffer->index < size) {
        unsigned char in;
        int bytesR = read(fileDes, &in, sizeof(unsigned char));
        
        if(bytesR == -1) {
            write(2, "Invalid Read, Canceling qread\n", strlen("Invalid Read, Canceling qread\n"));    
            return 0;
        }
        
        printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(in));         
        readBuffer->data = (readBuffer->data << 8) | in;
        readBuffer->index = readBuffer->index + 8;
    }

    // We now have enough data in the buffer
    
    // Create our output and set it to the amount of data that we want from the buffer
    unsigned char outputByte;
    outputByte = (unsigned char)(readBuffer->data >> (readBuffer->index - size));
    
    // Shift the bits that we used into oblivion
    int shift = 16 - (readBuffer->index) + 1;
    readBuffer->data = (readBuffer->data << shift) >> shift;
    readBuffer->index = readBuffer->index-size;

    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(outputByte)); 

    return outputByte;
}

/**
 * Add specified amount of data to qbyte between 1 and 8 bits and
 * writes to the specfied file descriptor when a byte can be written.
 * int fileDes: File Descriptor to write to
 * qbyte * writeBuffer: qbyte to add data specified input to
 * unsigned char input: 8 usable input bits
 * int size: amount of data from input to write to writeBuffer
 * */
int qwrite(int fileDes, qbyte * writeBuffer, unsigned char input , int size) {
    // Check if size is correct
    if(size > 8 || size <= 0) {
        write(2, "Invalid Size, Canceling qwrite\n", strlen("Invalid Size, Canceling qwrite\n"));
        return -1;
    }

    // First add our stuff to the Queued Byte
    writeBuffer->data = writeBuffer->data << size;
    writeBuffer->data = writeBuffer->data | input;
    writeBuffer->index = writeBuffer->index + size;

    // Queued Byte is in a writable state
    if(writeBuffer->index > 7) {
        unsigned char outputByte;
        int bytesW;

        // Grab the 8 bits that we want to write to stdout
        uint16_t maskedQ = writeBuffer->data;
        maskedQ = maskedQ >> (writeBuffer->index-8);
    
        // Shove the shifted output into an unsigned char for output
        outputByte = (unsigned char)maskedQ;

        // Remove the 8 bits from the queue that we just wrote
        uint16_t mask = UINT16_MAX << (writeBuffer->index-8);
        mask = ~mask;
        writeBuffer->data = writeBuffer->data & mask;
        writeBuffer->index = writeBuffer->index-8;
        
        // Write to stdout and return that a byte was written
        bytesW = write(fileDes, &outputByte, sizeof(unsigned char));
        return bytesW;
    }
    
    // Return that no bytes were written, queue isn't full enough
    return 0;
}

/**
 * Flush a qbyte to the specified file descriptor 
 * int fileDes: File Descriptor to write to
 * qbyte * writeBuffer: Queue Byte that still has information to be written 
 * */
int qflush(int fileDes, qbyte * writeBuffer) {

    if(writeBuffer->index == 0)
        return -1;
    if(writeBuffer->index > 7) {
        write(2, "There is a legitimate problem with write, fix it.\n", strlen("There is a legitimate problem with write, fix it.\n"));
        return -1;
    }

    // Grab the rest of the write buffer and shift it to the beginning of the byte
    unsigned char outputByte = writeBuffer->data;
    outputByte = outputByte << (8-writeBuffer->index);
    
    // Write the byte to the specified file
    int bytesW = write(fileDes, &outputByte, sizeof(unsigned char));

    // Clear the queued byte
    writeBuffer->data = 0;
    writeBuffer->index = 0;

    // Return the number of bytes written
    return bytesW;
}

/**
    printf("Start: Looking for: %d\n", size);

    // Output whats in the current queue [..., c2, c1, ...]
    unsigned char c1 = readBuffer->data;
    unsigned char c2 = (readBuffer->data >> 8);

    printf("\nBuffer: ");
    printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(c2));
    printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(c1));
    printf("\n");

    // Check if we have enough space to read a full byte
    if(readBuffer->index < 7) { // Read the next byte
        // Shift the data to the left by the size of a byte
        readBuffer->data = readBuffer->data << 8;
        readBuffer->index = readBuffer->index + 8;

        // Read a byte from stdin
        unsigned char byte;
        read(0, &byte, sizeof(unsigned char));

        // Extend our input byte
        uint16_t extendedByte = (uint16_t)byte;

        printf("New Byte: ");
        printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(((unsigned char)(extendedByte >> 8))));
        printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY((unsigned char)((extendedByte << 8) >> 8)));
        printf("\nindex: %d\n", readBuffer->index);

        // And our newly read data into the buffer
        readBuffer->data = readBuffer->data | extendedByte;
    
        printf("Read another byte\n");
    }

    c1 = readBuffer->data;
    c2 = (readBuffer->data >> 8);
    
    printf("Buffer: ");
    printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(c2));
    printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(c1));
    printf("\nindex: %d\n", readBuffer->index);

    unsigned char outputByte;
    
    // Grab the current qbyte and shift it to grab the current byte
    uint16_t toTruncateQByte = readBuffer->data;
    toTruncateQByte = toTruncateQByte >> (readBuffer->index - size);

    printf("Shift: %d\n", (readBuffer->index - size));
    
    // Write the byte to the output
    outputByte = (unsigned char)toTruncateQByte;

    printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(outputByte));

    // Remove the byte we just wrote to output
    uint16_t mask = 0;
    mask = ~mask;
    mask = mask << (readBuffer->index - size);
    mask = ~mask;
    printf("mask %d\n", mask);
        
    // Modify the buffer data and reduce the index
    readBuffer->data = readBuffer->data & mask;
    readBuffer->index = readBuffer->index - (readBuffer->index - size);

    c1 = readBuffer->data;
    c2 = (readBuffer->data >> 8);
    
    printf("Buffer: ");
    printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(c2));
    printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(c1));
    printf("\nindex: %d\n\n", readBuffer->index);
    

    return outputByte;
*/