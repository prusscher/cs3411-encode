OBJECTS = util.c util.h strings.h

all: encode decode

encode: encode.c ${OBJECTS}
	gcc -std=gnu99 -Wall encode.c ${OBJECTS} -o encode

decode: decode.c ${OBJECTS}
	gcc -std=gnu99 -Wall decode.c ${OBJECTS} -o decode

clean:
	-rm -f encode.o
	-rm -f encode
	-rm -f decode.o
	-rm -f decode