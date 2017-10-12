#ifndef UTIL_H_   /* Include guard */
#define UTIL_H_

#include <stdint.h>

// From https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
// lmao
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

typedef struct {
    uint16_t data;
    int index;
}qbyte;

int fileExist(char * file);
int fileSize(char * file);
void fileError(const char * errorMsg, char * file);
unsigned char qread(int fileDes, qbyte * writeBuffer, int size);
int qwrite(int fileDes, qbyte * writeBuffer, unsigned char input, int size);
int qflush(int fileDes, qbyte * writeBuffer);

#endif // FOO_H_