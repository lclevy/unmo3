/*
 * unmo3.c (unpacks and parses .MO3 files)
 * v0.5 : 24Jan2006, lclevy@free.fr, released under GPL license
 *
 * 19Jul2009 : v0.6 = updates for v2.4 encoder (version==5, compressed data at 0x0c, 2x 0 after sample names).
 *   
 * Special thanks to 
 *  Laurent Laubin (PEtite compression removal)
 *  Matthew T. Russotto (http://www.speakeasy.org/~russotto/) for the header compression explanation
 *  Stuart Caie (Kyzer@4u.net) 
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include"endian_macros.h"

#include"mo3_unpack.h"
#include"mo3_parse.h"
#include"mo3_mp3.h" 

#define VERSION "0.6"

extern char *compName[];
extern char *resoName[];

void usage()
{
  printf("unmo3 [-d debuglevel] [-a parselevel] [-h outheaderfile] [-s<all|sample_number>] [-v pattern channel] [-o] filename.mo3\n");
  exit(1);
}


int debug=1;

int saveFile(char *filename, unsigned char *buf, long len)
{
  FILE *out;
  out=(FILE*)fopen(filename,"wb");
  if (!out) {
    perror("fopen");
    return -1;
  }
  fwrite(buf, len, 1, out);
  fclose(out);

  return 0;
}

#define SAVE_NOTHING -2
#define SAVE_ALL     -1

int main(int argc, char *argv[])
{
  unsigned long uncomp_size;
  FILE *file;
  unsigned char *uncomp_header, *ptr;
  unsigned char *mo3;
  char  *saveName=0, *mo3Name=0;
  unsigned char *sample1=0;
  int sampleNr = SAVE_NOTHING;
  char track=0;
  unsigned char headerOffset = 8;

  long mo3Len;
  int i;
  int parseLevel=0;
  struct mo3Data mo3Hdr; 
  char sampleName[50]; 
  int pattern=-1, channel=0, voice;

  if (argc<2)
    usage();

  i=0;
  while (i<argc) {
    if (argv[i][0]=='-') {
      switch (tolower(argv[i][1])) {
      case 'd': // debug level
        i++;
        debug = atoi(argv[i]);
      break;
      case 'a': // parse level
        i++;
        parseLevel = atoi(argv[i]);
      break;
      case 'v': // display voice
        i++;
        pattern = atoi(argv[i++]);
        channel = atoi(argv[i]);
      break;
      case 'o': // output track
        track = 1;
      break;
      case 'h': // header
        i++;
        saveName = argv[i];
      break;
      case 's': // sample
        i++;
        if (strncmp(argv[i],"all",3)==0) 
          sampleNr = SAVE_ALL; // save all
				else
					sampleNr = atoi(argv[i]);
      break;
      default:
        printf("unknown option: %d\n",argv[i][1]);
      } 
    }
    else 
      if ( i==(argc-1) )
        mo3Name = argv[i];
    i++;
  }

  printf("unmo3 v%s (opensource version)\n\n",VERSION);

  /* open MO3 file */
  file=(FILE*)fopen(mo3Name,"rb");
  if (file==NULL) {
    fprintf(stderr, "Can not open %s\n", mo3Name);
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  mo3Len = ftell(file);
  fseek(file, 0, SEEK_SET);

  mo3=(unsigned char*)malloc(mo3Len);
  if (mo3==NULL) {
    perror("malloc:");
    exit(1);
  }
  fread(mo3, mo3Len, 1, file);
  fclose(file);

  /* 3rd byte value:
   *  for v1.8 it is 0, and 1 with OGG samples 
   *  for v2.1 it is 4, 
   *  for v2.2 it is 3,
   *  for v2.4 it is 5 
   */ 
  mo3Hdr.version = *(mo3+3);
  if ('M'==*mo3 && 'O'==*(mo3+1) && '3'==*(mo3+2) 
	  && (0==*(mo3+3) || 4==*(mo3+3) || 1==*(mo3+3) || 3==*(mo3+3) || 5==*(mo3+3)) )
    ;
  else {
    fprintf(stderr,"not an MO3 file (0x%02d%02d%02d%02d)\n",mo3[0],mo3[1],mo3[2],mo3[3]);
    fclose(file);
    exit(1);
  }
  if (mo3Hdr.version == 5)
    headerOffset = 12;
  else
   headerOffset = 8;
  uncomp_size = iget32(mo3+4);
  if (debug)  
    fprintf(stdout, "Uncompressed size of header = %ld (0x%lx)\n", uncomp_size, uncomp_size);

  uncomp_header=(unsigned char*)malloc(uncomp_size);
  if (uncomp_header==NULL) {
    perror("malloc:");
    exit(1);
  }

  ptr = unpack(mo3+headerOffset, uncomp_header, uncomp_size);
  if (debug)
    printf("Offset in compressed data after decompression = 0x%x (%d)\n",ptr-(mo3+headerOffset), ptr-(mo3+headerOffset));

//  getMp3Length(ptr);

  if (saveName!=0) 
    saveFile(saveName, uncomp_header, uncomp_size);

  parseHeader(uncomp_header, uncomp_size, parseLevel, &mo3Hdr);

 if (pattern!=-1) {
   if ( (pattern<0) || (pattern>mo3Hdr.patternNb))
     pattern = 0;
   if ( (channel < 1) || (channel > mo3Hdr.channelNb) )
     channel = 1;
   voice = findVoiceNumber(pattern, channel, uncomp_header, &mo3Hdr);
   if (voice!=-1) {
     printf("\nDisplaying encoded channel #%d of pattern #%d (length %d): unique voice #%d \n", 
		   channel, pattern, mo3Hdr.patternLen[i], voice);
     parseVoice(uncomp_header, mo3Hdr.voicePtr[voice], &mo3Hdr, 4);
     if (track) {
       printf("\nDisplaying the channel like in editors:\n");
		   parseTrack(uncomp_header, mo3Hdr.voicePtr[voice], &mo3Hdr, 0);
     }
   }
 } 

  if (sampleNr!=SAVE_NOTHING) {
    putchar('\n');
    for(i=0; i<mo3Hdr.sampleNb; i++) {
      if ( mo3Hdr.samples[i].len>0 ) {
        if (mo3Hdr.samples[i].method!=0) {
          sample1 = malloc(mo3Hdr.samples[i].len); 
          if (!sample1) {
            perror("malloc");
            break;
          }
        }
        else
          sample1 = 0;

        if (debug>1)
          printf("sample#%03d is at offset 0x%x in compressed data\n", i+1, ptr-(mo3+8));         

        if (mo3Hdr.samples[i].method!=0) {
          ptr = mo3Hdr.samples[i].method(ptr, sample1, mo3Hdr.samples[i].len);
          if ( sampleNr==SAVE_ALL || (sampleNr-1)==i ) {
            sprintf(sampleName,"sample%03d.dat",i+1);
            saveFile(sampleName, sample1, mo3Hdr.samples[i].len);
            printf("saving %s (offset 0x%x, length %ld/%ld bytes, compression %s, resolution %s)...\n",  
						  sampleName, ptr-(mo3+8), mo3Hdr.samples[i].len, 
							mo3Hdr.samples[i].lenComp, compName[mo3Hdr.samples[i].compression-1], 
							resoName[mo3Hdr.samples[i].reso-1]);
          }

        } else {

          if ( sampleNr==SAVE_ALL || (sampleNr-1)==i ) {
            if (mo3Hdr.samples[i].compression==MO3COMPR_MP3) 
              sprintf(sampleName,"sample%03d.mp3",i+1);
            if (mo3Hdr.samples[i].compression==MO3COMPR_OGG) 
              sprintf(sampleName,"sample%03d.ogg",i+1);
            if (mo3Hdr.samples[i].compression==MO3COMPR_NONE) 
              sprintf(sampleName,"sample%03d.dat",i+1);
            saveFile(sampleName, ptr, mo3Hdr.samples[i].lenComp);
            printf("saving %s (offset 0x%x, length %ld/%ld bytes, compression %s, resolution %s)...\n",  
						  sampleName, ptr-(mo3+8), mo3Hdr.samples[i].len, 
							mo3Hdr.samples[i].lenComp, compName[mo3Hdr.samples[i].compression-1], 
							resoName[mo3Hdr.samples[i].reso-1]);
          }
          ptr+=mo3Hdr.samples[i].lenComp;				

	   	  }
        if (sample1)
          free(sample1);
			} // if len
	  } // for
  } // if


  free(mo3Hdr.samples);
  free(mo3Hdr.patternLen);
  free(mo3Hdr.voicePtr);
  free(mo3);
  free(uncomp_header);
    
  return 0;
}
