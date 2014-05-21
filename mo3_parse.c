/*
 * parses MO3 files
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include"endian_macros.h"

#include"mo3_unpack.h"
#include"mo3_parse.h" 

#define NB_NOTES 12

char *notes[NB_NOTES]={ "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };

char *compName[] = { "LosslessDelta8", "LosslessDeltaPred8", "LosslessDelta16","LosslessDeltaPred16",
  "NotCompressed", "mp3", "ogg", "removed", "oggSharedHeader" };

char *typeName[] = { "MOD", "XM", "MTM", "IT", "S3M" };

char *resoName[] = { "8bits", "16bits" };

extern int debug;


unsigned char * parseSamples(unsigned char *p, int parseLevel, struct mo3Data *mo3Hdr )
{
  int j, i;

  mo3Hdr->samples=(struct mo3Sample*)malloc(mo3Hdr->sampleNb*sizeof(struct mo3Sample));
  if (!mo3Hdr->samples) {
    perror("malloc:");
    return (unsigned char *)-1;
  }

  if (parseLevel>1)
    printf("Samples table = \n");

  for(i=0; i<mo3Hdr->sampleNb; i++) {
//  printf("offset=0x%x\n", p-src);
    if (parseLevel>1)
      printf(" Sample#%d name = %s\n", i+1, p);
    while(*(p++) != 0);

	if (mo3Hdr->version>=5) {
      if (parseLevel>1)
        printf(" Sample#%d filename = %s\n", i+1, p);
      while(*(p++) != 0);
    } 

    if (debug>2) {
      for(j=0; j<0x29; j++)
        printf("%02x ", *(p+j));
        putchar('\n');      
    }
    
    mo3Hdr->samples[i].len = iget32(p+8);

    if (parseLevel>2) {
      printf("  ftune = %d, ", *p-128); 
      printf(" trpose = %0xd, ", *(p+4)); 
      printf(" vol = %d, ", *(p+5)); 
      printf(" size = %ld, ", mo3Hdr->samples[i].len); 
    }

    mo3Hdr->samples[i].lenComp = iget32(p+35);
    mo3Hdr->samples[i].flags = iget16(p+20);

    if (parseLevel>2) {
      printf(" start = %d, ", iget32(p+12)); 
      printf(" end = %d, ", iget32(p+16)); 
      printf(" comp. size = %ld, ", mo3Hdr->samples[i].lenComp);
      printf(" flags = %04x " , mo3Hdr->samples[i].flags );
    }

    if (mo3Hdr->samples[i].flags&0x0001) {
      mo3Hdr->samples[i].reso = MO3RESO_16BITS;
      mo3Hdr->samples[i].len = mo3Hdr->samples[i].len*2;
    }
    else {
      mo3Hdr->samples[i].reso = MO3RESO_8BITS;
    }
    mo3Hdr->samples[i].method = 0;

    if ( (mo3Hdr->samples[i].flags&0x3000) == 0x2000 ) {
      if (mo3Hdr->samples[i].reso == MO3RESO_16BITS) {
        mo3Hdr->samples[i].method = unpackSamp16Delta;
        mo3Hdr->samples[i].compression = MO3COMPR_LOSSLESS16D;
      }
      else {
        mo3Hdr->samples[i].method = unpackSamp8Delta;
        mo3Hdr->samples[i].compression = MO3COMPR_LOSSLESS8D;
      }
    } // & 0x2000
    else {
      if ((mo3Hdr->samples[i].flags&0x5000) == 0x4000) {
        if (mo3Hdr->samples[i].reso == MO3RESO_16BITS) {
          mo3Hdr->samples[i].method = unpackSamp16DeltaPrediction;
          mo3Hdr->samples[i].compression = MO3COMPR_LOSSLESS16DP;
        }
        else {
          mo3Hdr->samples[i].method = unpackSamp8DeltaPrediction;
          mo3Hdr->samples[i].compression = MO3COMPR_LOSSLESS8DP;
        }
      } // & 0x4000
      else {
        if (mo3Hdr->samples[i].flags&0x1000) {
          if (mo3Hdr->samples[i].flags&0x2000)
            mo3Hdr->samples[i].compression = (mo3Hdr->samples[i].flags&0x4000) ? MO3COMPR_OGGSHARED : MO3COMPR_OGG;
          else
            mo3Hdr->samples[i].compression = MO3COMPR_MP3;
				} // lossy compression
        else { // not compressed or removed
          if (parseLevel>2) {
	          if (mo3Hdr->samples[i].len>0) {
              mo3Hdr->samples[i].compression = MO3COMPR_NONE;
            }
  				  else
              mo3Hdr->samples[i].compression = MO3COMPR_REMOVED;
          } 
        }
      }
    }

    p+=0x29;

    mo3Hdr->samples[i].oggHeaderSource = i;
    if ( (mo3Hdr->samples[i].flags&0x5000) == 0x5000 && mo3Hdr->version>=5 ) {
      // OGG header sharing
      mo3Hdr->samples[i].oggHeaderSource = iget16(p);
      p+=2;
    }

    if (parseLevel>2) {
	    printf("(%s)", compName[mo3Hdr->samples[i].compression-1]);
      if (mo3Hdr->samples[i].method==0)
        printf(" %s",resoName[mo3Hdr->samples[i].reso-1]);
      putchar('\n');
    }
  } // for

  return p;
}

int parseVoice(unsigned char *src, long offset, struct mo3Data *mo3Hdr, int parseLevel)
{
  unsigned char *p = src+offset;
  unsigned char byte;
	int c,d, i;

  p+=4;
  byte = *p++;
  while(byte!=0) { // each voice is null terminated
    c = byte&0x0f;
    d = (byte&0xf0)>>4;
    if ( c==0 ) {
      if (parseLevel>3)
        printf("0*%d ", d);
    }
    else {
      if (parseLevel>3) {
        if (d>1)
					printf("[%d*%d, ",d,c); 
        else
					printf("[%d, ",c); 
      }
      for(i=0; i<c; i++) {
        if (parseLevel>3)
					printf("%x %x ", *p, *(p+1));
				p+=2;
			}
      if (parseLevel>3)
				printf("\b] ");
    }
    byte = *p++;
  }
  if (parseLevel>3)
		putchar('\n');

  return 0;
}

int parseTrack(unsigned char *src, long offset, struct mo3Data *mo3Hdr, int write)
{
  unsigned char *p = src+offset;
  unsigned char byte;
	int c,d, i;
  char note[3+1];
  char empty[]="... ..     ...";
  char instr[2+1];
  char eff1[3+1];
  char eff2[3+1];
  int oct, notev;
  int row=0;

  if (mo3Hdr->type!=MO3TYPE_MOD) {
    printf("Not supported\n");
    return -1;
	}

  p+=4;
  byte = *p++;
  while(byte!=0) { // each voice is null terminated
    c = byte&0x0f;
    d = (byte&0xf0)>>4;
    if ( c==0 ) {
      if (!write)
        for (i=0; i<d; i++) {
          printf("[%3d] ",row++);
          puts(empty);
        }
    }
    else {
      if (!write) {
        strcpy(note,"...");
        strcpy(instr,"..");
        strcpy(eff1,"   ");
        strcpy(eff2,"...");

        for(i=0; i<c; i++) {
          switch(*p) {
          case 1:
            oct = (*(p+1))/NB_NOTES;
            notev = ( (*(p+1))%NB_NOTES );
            sprintf(note,"%s%d", notes[notev], oct+1);
            break;
          case 2:
            sprintf(instr,"%2d", (*(p+1))+1);
            break;
          default:
            sprintf(eff2,"%X%02X", (*p)-3, *(p+1));
	        }
			    p+=2;
		    }

        for(i=0; i<d; i++) {
          printf("[%3d] ",row++);
          printf("%s %s %s %s\n", note, instr, eff1, eff2);
        }
      }

    }
    byte = *p++;
  }

  return 0;
}

short findVoiceNumber(short pattern, char channel, unsigned char *header, struct mo3Data *mo3Hdr)
{
  unsigned char *p=header+mo3Hdr->patternTable;
  if (pattern >= mo3Hdr->patternNb) {
	  fprintf(stderr, "error: pattern number requested is > number of pattern in module\n"); 
    return -1;
  }
  if (channel > mo3Hdr->channelNb) {
	  fprintf(stderr, "error: channel number requested is > number of channel in module\n"); 
    return -1;
  }
  p = p + ((mo3Hdr->channelNb*pattern) + channel-1)*2;
  return iget16(p);
}

unsigned char * parseInstr(unsigned char *p, int parseLevel, struct mo3Data *mo3Hdr)
{
  int i, j;
//  unsigned char *p = header;

  for(i=0; i<mo3Hdr->instrNb; i++) {
    if (parseLevel>1)
      printf(" Instr#%d name = %s\n", i+1, p);
    while(*(p++) != 0);

	if (mo3Hdr->version>=5) {
      if (parseLevel>1)
        printf(" Instr#%d filename = %s\n", i+1, p);
      while(*(p++) != 0);
    } 

    if (debug>2) {
      for(j=0; j<4; j++)
        printf("%02x ", *(p+j));
      putchar('\n');      
    }
    if (parseLevel>1) {
      printf("  sample map=\n");
      printf("  volume envelope : nb nodes=%d\n",*(p+4+480+1));
/*      for(j=0x24e; j<0x254; j++)
        printf("%02x ", *(p+j));
      putchar('\n');   */   
      printf("  panning envelope: nb nodes=%d\n",*(p+0x24f));
      printf("  pitch envelope  : nb nodes=%d\n",*(p+0x2b9));
    }
    if (debug>2) {
      for(j=0x322; j<0x33a; j++)
        printf("%02x ", *(p+j));
        putchar('\n');      

    }
    p+=0x33a;
  }

  return p;
}

int parseHeader(unsigned char *src, unsigned long size, int parseLevel, struct mo3Data *mo3Hdr)
{
  unsigned char *p = src;
  int i, j, k=0;

  long voiceLen;

  if (parseLevel>0) 
    printf("Songname = %s\n", src);

  /* songname terminated by 00 */
  while(*p!=0)
    p++;
  p++;

  /* comments terminated by 00 */
  while(*p!=0)
    p++;
  p++;

//  p2=p;
  mo3Hdr->channelNb = *p;
  mo3Hdr->songLen = iget16(p+1);
  mo3Hdr->patternNb = iget16(p+5);
  mo3Hdr->uniqueVoice = iget16(p+7);
  mo3Hdr->instrNb = iget16(p+9);
  mo3Hdr->sampleNb = iget16(p+11);
  mo3Hdr->flags = iget32(p+15);

  if (parseLevel>0) {
    printf("Nb channels = %d\n", mo3Hdr->channelNb);
    printf("Song length = %d\n", mo3Hdr->songLen);
    printf("Song restart = %d\n", iget16(p+3));
    printf("Nb pattern = %d\n", mo3Hdr->patternNb);
    printf("Nb unique voice = %d\n", mo3Hdr->uniqueVoice);
    printf("Nb instr = %d\n", mo3Hdr->instrNb);
    printf("Nb sample = %d\n", mo3Hdr->sampleNb);
    printf("Ticks/row = %d\n", *(p+13));
    printf("Initial tempo = %d\n", *(p+14));
  }
  if (mo3Hdr->flags & 0x0100)
    mo3Hdr->type = MO3TYPE_IT;
  else if (mo3Hdr->flags & 0x0002)
    mo3Hdr->type = MO3TYPE_S3M;
  else if ( mo3Hdr->flags & 0x0080)
    mo3Hdr->type = MO3TYPE_MOD;
  else if (mo3Hdr->flags & 0x0008)
    mo3Hdr->type = MO3TYPE_MTM;
  else
    mo3Hdr->type = MO3TYPE_XM;

  if (parseLevel>0) {
    printf("Initial format is ");
    printf("%s (0x%08lx)\n", typeName[mo3Hdr->type-1], mo3Hdr->flags);
  }

  mo3Hdr->voicePtr=(long*)malloc(mo3Hdr->uniqueVoice*sizeof(long));
  if (!mo3Hdr->voicePtr) {
    perror("malloc:");
    return -1;
  }

  mo3Hdr->patternLen=(short*)malloc(mo3Hdr->patternNb*sizeof(short));
  if (!mo3Hdr->patternLen) {
    perror("malloc:");
    return -1;
  }

  p+=0x1a6;
  if (parseLevel>1)
    printf("Song sequence = ");
  for(i=0; i<mo3Hdr->songLen; i++) {
    if (parseLevel>1)
      printf("%d ",*p);
    p++;
  } 
  if (parseLevel>1)
    putchar('\n');

  if (parseLevel>2)
    printf("Unique voice number for each pattern = ");
  mo3Hdr->patternTable = p-src;
  for(i=0; i < mo3Hdr->patternNb * mo3Hdr->channelNb; i++) {
    if (parseLevel>2) {
      if ( ((i)%mo3Hdr->channelNb)==0 || i==0)
        printf("p[%d]: ",(i)/mo3Hdr->channelNb);
      printf("%d ",iget16(p));
    }
    p+=2;
  }
  if (parseLevel>2)
    putchar('\n');

  if (parseLevel>3)
    printf("Pattern size table = ");
  for(i=0; i < mo3Hdr->patternNb; i++) {
    mo3Hdr->patternLen[i] = iget16(p);
    if (parseLevel>3)
      printf("%d ",mo3Hdr->patternLen[i]);
    p+=2;
  }
  if (parseLevel>3)
    putchar('\n');

  if (parseLevel>3)
    printf("Voice data table = ");
    
  for(j=0; j<mo3Hdr->uniqueVoice; j++) {
    voiceLen = iget32(p);

    if (parseLevel>3)
      printf("v[%d] l=0x%lx ", j, voiceLen);
      
    mo3Hdr->voicePtr[k++] = p-src;

    parseVoice(src, p-src, mo3Hdr, parseLevel);
    p+=4;
    p+=voiceLen;
  }
  if (parseLevel>3)
    putchar('\n');

  if (parseLevel>1)
    printf("Instr table = \n");

  mo3Hdr->instr = p-src;
  p = parseInstr(p, parseLevel, mo3Hdr);

  p = parseSamples(p, parseLevel, mo3Hdr);

  if (parseLevel>1) 
    printf("end offset=0x%x\n", p-src);

  return 0;
} 

