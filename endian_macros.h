#ifndef ENDIAN_MACROS_H
#define ENDIAN_MACROS_H 1

/* little endian macro */
#define ifget8(f) ((unsigned long)fgetc(f))
#define ifget16(f) ( ifget8(f) | (ifget8(f)<<8UL) ) 
#define ifget32(f) ( ifget16(f) | (ifget16(f)<<16UL) ) 

#define iget8(p) ((unsigned char)*(p))
#define iget16(p) ( iget8(p) | (iget8(p+1)<<8UL) ) 
#define iget32(p) ( iget16(p) | (iget16(p+2)<<16UL) ) 

#endif // ENDIAN_MACROS_H
