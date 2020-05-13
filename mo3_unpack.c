/*
 * unpack functions for MO3 module
 * lclevy@free.fr
 *
 * the code is under GPL license
 * Note: This code does not guard against out-of-range reads in the input stream
 *       but buffer overflows in the output streeam are mitigated.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include"mo3_unpack.h"

extern int debug;

/* unpack macros */

/* shift control bits until it is empty:
 * a 0 bit means litteral : the next data byte (*s) is copied
 * a 1 means compressed data
 *   then the next 2 bits determines what is the LZ ptr
 *           ('00' same as previous, else stored in stream (*src))
 */
#define READ_CTRL_BIT(n)  \
    data<<=1; \
    carry = (data >= (1<<(n))); \
    data&=((1<<(n))-1); \
    if (data==0) { \
      data = *src++; \
      data = (data<<1) + 1; \
      carry = (data >= (1<<(n))); \
      data&=((1<<(n))-1); \
    }

/* length coded within control stream :
 * most significant bit is 1
 * than the first bit of each bits pair (noted n1),
 * until second bit is 0 (noted n0)
 */
#define DECODE_CTRL_BITS { \
   strLen++; \
   do { \
     READ_CTRL_BIT(8); \
     strLen = (strLen<<1) + carry; \
     READ_CTRL_BIT(8); \
   }while(carry); \
 }

/*
 * this smart algorithm has been designed by Ian Luck
 */
unsigned char * unpack(unsigned char *src, unsigned char *dst, long size)
{
  unsigned short data;
  char carry;  // x86 carry (used to propagate the most significant bit from one byte to another)
  long strLen; // length of previous string
  long strOffset; // string offset
  unsigned char *string; // pointer to previous string
  unsigned char *initDst;
  unsigned long ebp, previousPtr;
  unsigned long initSize;

  data = 0L;
  carry = 0;
  strLen = 0;
  previousPtr = 0;

  initDst = dst;
  initSize = size;

  *dst++ = *src++; // the first byte is not compressed
  size--; // bytes left to decompress


  while (size>0) {
    READ_CTRL_BIT(8);
    if (!carry) { // a 0 ctrl bit means 'copy', not compressed byte
      *dst++ = *src++;
      size--;
    }
    else { // a 1 ctrl bit means compressed bytes are following
      ebp = 0; // length adjustment
      DECODE_CTRL_BITS; // read length, anf if strLen>3 (coded using more than 1 bits pair) also part of the offset value
      strLen -=3;
      if ((long)(strLen)<0) { // means LZ ptr with same previous relative LZ ptr (saved one)
        strOffset = previousPtr;  // restore previous Ptr
        strLen++;
      }
      else { // LZ ptr in ctrl stream
         strOffset = (strLen<<8) | *src++; // read less significant offset byte from stream
         strLen = 0;
         strOffset = ~strOffset;
         if (strOffset<-1280)
           ebp++;
         ebp++; // length is always at least 1
         if (strOffset<-32000)
           ebp++;
         previousPtr = strOffset; // save current Ptr
      }

      // read the next 2 bits as part of strLen
      READ_CTRL_BIT(8);
      strLen = (strLen<<1) + carry;
      READ_CTRL_BIT(8);
      strLen = (strLen<<1) + carry;
      if (strLen==0) { // length do not fit in 2 bits
        DECODE_CTRL_BITS; // decode length : 1 is the most significant bit,
        strLen+=2;  // then first bit of each bits pairs (noted n1), until n0.
      }
      strLen = strLen + ebp; // length adjustment
      if (size>=strLen && strLen > 0) {
        string = dst + strOffset; // pointer to previous string
        if(strOffset >= 0 || (ptrdiff_t)(dst - initDst) + strOffset < 0)
          break;
        size-=strLen;
        for(; strLen>0; strLen--)
           *dst++ = *string++; // copies it
      }
      else {
        // malformed stream
        break;
      }
    } // compressed bytes

  }
  if ( (dst-initDst)!=initSize ) {
    fprintf(stderr,"Error: uncompressed data length is different (%ld) than expected (%ld)\n",
      (dst-initDst), initSize);
  }

  return src;

}


/* decode value, encoded as bit pairs (n1), until (n0) */
#define DECODE_VAL(n) { \
   do { \
     READ_CTRL_BIT((n)); \
     val = (val<<1) + carry; \
     READ_CTRL_BIT((n)); \
   }while(carry); \
 }

/* decode value , encoded as bit triplets (nn1), until (nn0) */
#define DECODE_TRIBITS \
   if (dh<5) { \
     do { \
     READ_CTRL_BIT(8); \
     val = (val<<1) + carry; \
     READ_CTRL_BIT(8); \
     val = (val<<1) + carry; \
     READ_CTRL_BIT(8); \
     }while(carry); \
   } \
   else \
     DECODE_VAL(8);


/* for 16 bits samples (was unpack2)
 *
 * very similar to unpackSamp4, but adapted to 16 bits samples
 *  initial dh=8,
 *  if dh<5 the left most bits may are encoded using bits triplet (nn1), until 'nn0'
 */
unsigned char * unpackSamp16Delta(unsigned char *src, unsigned char *dst, long size)
{

  unsigned char dh, cl;
  char carry;
  unsigned short data;
  unsigned short *pdst, *psrc, *initDst;
  unsigned short val;
  short previous;
  unsigned long initSize;

  size/=2;
  initDst = pdst = (unsigned short *)dst;
  psrc = (unsigned short *)src;
  initSize = size;

  dh = 8;
  cl = 0;
  data = 0;      // used in READ_CTRL_BIT
  previous = 0;  // initial value is 0, sample values are coded using delta.

  while(size>0) {
    val = 0;
    DECODE_TRIBITS;    // decode bits triplets
    cl = dh;           // length in bits of  : delta second part (right most bits of delta) and sign bit
    for(; cl>0; cl--) {
      READ_CTRL_BIT(8);
      val = (val<<1) + carry;
    }
    cl = 1;
    if (val>=4) {
      // bsr ecx, val
      cl = 15;
      while( ((1<<cl) & val) == 0 && cl>1)
        cl--;
    }
    dh = dh + cl;
    dh>>=1;       // next length in bits of encoded delta second part
    carry = val&1;  // sign of delta 1=+, 0=not
    val>>=1;
    if (carry==0)
      val=~val;    // negative delta
    val+=previous; // previous value + delta
    *pdst++ = val;
    previous = val;
    size--;
  }

  if ( (pdst-initDst)!=initSize ) {
    fprintf(stderr,"Error: uncompressed data length is different (%ld) than expected (%ld)\n",
      (pdst-initDst), initSize);
  }

  return src;
}

/* for 16 bits samples (was unpack3)
 *
 * same as unpackSamp5, but for 16 bits samples
 */
unsigned char * unpackSamp16DeltaPrediction(unsigned char *src, unsigned char *dst, long size)
{
 /*
  * NOT TESTED
  */

  unsigned char dh, cl;
  char carry;
  short sval;
  unsigned short data;
  long next, initSize;
  unsigned short *pdst, *psrc, *initDst;
  unsigned short val;
  short delta, previous;

  size/=2;
  dh = 8;
  cl = 0;
  data = 0; // used in READ_CTRL_BIT
  previous = next = 0;  // initial and next value are 0, sample values are coded using delta against next (predicted).

  initDst = pdst = (unsigned short *)dst;
  psrc = (unsigned short *)src;
  initSize = size;

  while(size>0) {
    val = 0;
    DECODE_TRIBITS;    // decode bits pair : first part of delta (left most bits)
    cl = dh;       // length in bits of  : delta second part (right most bits of delta) and sign bit
    for(; cl>0; cl--) {
      READ_CTRL_BIT(8);
      val = (val<<1) + carry;
    }
    cl = 1;
    if (val>=4) {
      // bsr ecx, val
      cl = 15;
      while( ((1<<cl) & val) == 0 && cl>1)
        cl--;
    }
    dh = dh + cl;
    dh>>=1;       // next length in bits of encoded delta second part
    carry = val&1;  // sign of delta 1=+, 0=not
    val>>=1;
    if (carry==0)
      val=~val;    // negative delta

    delta = (short)val;
    val=val+next;     // predicted value + delta
    *pdst++ = val;
    sval=(short)val;
    next = (sval<<1) + (delta>>1) - previous;  // corrected next value

    if (next>32767) // check overflow of next value (signed short)
      next = 32767;
    else if (next<-32768)
      next = -32768;

    previous = sval;
    size--;
  }

  if ( (pdst-initDst)!=initSize ) {
    fprintf(stderr,"Error: uncompressed data length is different (%ld) than expected (%ld)\n",
      (pdst-initDst), initSize);
  }

  return src;
}

/* for 8 bits samples (was unpack4)
 *
 * 8 bits samples values are encoded as delta. the initial value is 0.
 * The 4 left most bits of the delta is first encoded using bit pairs (n1), until n0.
 *   for example (1)1 (1)1 (1)0 means 7. for low deltas, it is 00.
 * then the next bits are encoding the 4 right most bits of the delta, and the delta sign
 *   if sign bit is 0, it means +delta, else it means not(delta)
 *   Here is also coded the length of the next 4 right most bits + sign bit.
 *   This value is initially dh=4, then is divided by 2 each step. The minimal value is 1 for the sign.
 *   dh may be augmented by cl if val >= 4.
 */
unsigned char * unpackSamp8Delta(unsigned char *src, unsigned char *dst, long size)
{
  unsigned char dh, cl, val, previous;
  char carry;
  unsigned short data;
  long initSize;
  unsigned char *initDst;

  initSize = size;
  initDst = dst;
  dh = 4;
  cl = 0;
  data = 0; // used in READ_CTRL_BIT
  previous = 0;  // initial value is 0, sample values are coded using delta.

  while(size>0) {
    val = 0;
    DECODE_VAL(8);    // decode bits pair : first part of delta (4 left most bits)
    cl = dh;       // length in bits of  : delta second part (4 right most bits of delta) and sign bit
    for(; cl>0; cl--) {
      READ_CTRL_BIT(8);
      val = (val<<1) + carry;
    }
    cl = 1;
    if (val>=4) {
      // bsr ecx, val
      cl = 7;
      while( ((1<<cl) & val) == 0 && cl>1)
        cl--;
      }
    dh = dh + cl;
    dh>>=1;       // next length in bits of encoded delta second part
    carry = val&1;  // sign of delta 1=+, 0=not
    val>>=1;
    if (carry==0)
      val=~val;    // negative delta
    val+=previous; // previous value + delta
//    *dst++ = 0;    // 8bits->16bits conversion
    *dst++ = val;
    previous = val;
    size--;
  }

  if ( (dst-initDst)!=initSize ) {
    fprintf(stderr,"Error: uncompressed data length is different (%ld) than expected (%ld)\n",
      (dst-initDst), initSize);
  }

  return src;
}

/* for 8 bits samples
 *
 * the same encoding method as unpackSamp4, but instead of what is coded
 *   is delta value against predicted value and not only previous value.
 *   for example if 0 and 3 are 2 following samples values, the next predicted one is 7
 *   and the delta encoded is -4 (real value is 3). then the predicted value is corrected
 *   using recent real value (the prediction is converging to the good value)
 * it could be called "differential delta coding"
 */
unsigned char * unpackSamp8DeltaPrediction(unsigned char *src, unsigned char *dst, long size)
{
  unsigned char dh, cl, val;
  char carry, delta, previous;
  char sval;
  unsigned short data;
  short next;

  dh = 4;
  cl = 0;
  data = 0; // used in READ_CTRL_BIT
  previous = next = 0;  // initial and next value are 0, sample values are coded using delta against next (predicted).

  while(size>0) {
    val = 0;
    DECODE_VAL(8);    // decode bits pair : first part of delta (left most bits)
    cl = dh;       // length in bits of  : delta second part (right most bits of delta) and sign bit
    for(; cl>0; cl--) {
      READ_CTRL_BIT(8);
      val = (val<<1) + carry;
    }
    cl = 1;
    if (val>=4) {
      // bsr ecx, val
      cl = 7;
      while( ((1<<cl) & val) == 0 && cl>1)
        cl--;
      }
    dh = dh + cl;
    dh>>=1;       // next length in bits of encoded delta second part
    carry = val&1;  // sign of delta 1=+, 0=not
    val>>=1;
    if (carry==0)
      val=~val;    // negative delta

    delta = (char)val;
    val=val+next;     // predicted value + delta
//    *dst++ = 0;    // 8bits->16bits conversion
    *dst++ = val;
    sval=(char)val;
    next = (sval<<1) + (delta>>1) - previous;  // corrected next value

    if (next>127) // check overflow of next value (signed byte)
      next = 127;
    else if (next<-128)
      next = -128;

    previous = sval;
    size--;
  }
  return src;
}

unsigned char * notCompressed(unsigned char *src, unsigned char *dst, long size)
{

  memcpy(dst, src, size);
  dst+=size;

  return src+size;
}
