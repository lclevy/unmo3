#ifndef MO3_UNPACK_H 
#define MO3_UNPACK_H 1

unsigned char * unpack(unsigned char *src, unsigned char *dst, long size);
unsigned char * unpackSamp8Delta(unsigned char *src, unsigned char *dst, long size);
unsigned char * unpackSamp8DeltaPrediction(unsigned char *src, unsigned char *dst, long size);
unsigned char * unpackSamp16Delta(unsigned char *src, unsigned char *dst, long size);
unsigned char * unpackSamp16DeltaPrediction(unsigned char *src, unsigned char *dst, long size);

unsigned char * notCompressed(unsigned char *src, unsigned char *dst, long size); 

#endif // MO3_UNPACK_H
