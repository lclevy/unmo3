/*
 * mo3_mp3.c
 */

/* the MP3 header is explained here :
 * http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
 * 
 * see also ISO/CEI 11172-3 and ISO/CEI 13818-3
 * http://en.wikipedia.org/wiki/MP3   
 */

#include"endian_macros.h"

#include<stdio.h>

extern int debug;

int br1[3][16]={ 
  { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
  { 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
  { 0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0}
} ;
int br2[2][16]={
  { 0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
  { 0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0}
};

int freq[3][4] = { 
  {44100, 48000, 32000, 0}, // L1
  {22050, 24000, 16000, 0}, // L2
  {11025, 12000,  8000, 0}  // L2.5
};

long getMp3Length(unsigned char *src)
{
  unsigned short s;
  unsigned char *p=src;
  int version, layer, bitrateI, frequency;
  int bitrate;
  
  s = iget16(p);

  /* 0000 0000 000vv000 */
  version = (s & (3<<3))>>3;
  if (debug>1)
	  printf("MPEG version bits = %x\n", version);
  switch(version) {
  case 0: version=3; break; // 2.5
  case 2: break;
  case 3: version=1; break;
  default: 
    printf("version=1\n");
	}

  /* 0000 0000 00000ll0 */
  layer = (s & (3<<1))>>1;
  if (debug>1)
    printf("Layer bits = %x\n", layer);
  switch(layer) {
  case 1: layer=3; break;
  case 2: break;
  case 3: layer=1; break;
  default: 
    printf("layer=0\n");
	}
    
  p+=2;
  /* bbbb0000 */
  bitrateI = (*p & 0xf0)>>4;
  if (debug>1)
    printf("Bitrate index = %x\n", bitrateI);

  if (version==1) { // v1
    bitrate = br1[layer][bitrateI];
    if (debug)
      printf("Bitrate = %d kbps\n", bitrate);
  } else if (version==2 || version==3) { // resp. v2 or v2.5
    bitrate = br2[layer][bitrateI];
    if (debug>0)
      printf("Bitrate = %d kbps\n", bitrate);
	}

  /* 0000ff00 */
  frequency = (*p & (3<<2))>>2;
  if (debug>1)
    printf("Frequency bits = %x\n", frequency);
  if (debug>0)
    printf("Frequency = %d hz\n", freq[layer][frequency]);

  return 0;
}
