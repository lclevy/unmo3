#ifndef MO3_PARSE_H 
#define MO3_PARSE_H 1

typedef unsigned char *(*samplePackMethod)(unsigned char *, unsigned char *, long);

#define MO3TYPE_MOD 1
#define MO3TYPE_XM 2
#define MO3TYPE_MTM 3
#define MO3TYPE_IT 4
#define MO3TYPE_S3M 5

#define MO3COMPR_LOSSLESS8D 1
#define MO3COMPR_LOSSLESS8DP 2
#define MO3COMPR_LOSSLESS16D 3
#define MO3COMPR_LOSSLESS16DP 4
#define MO3COMPR_NONE 5
#define MO3COMPR_MP3 6
#define MO3COMPR_OGG 7
#define MO3COMPR_REMOVED 8

#define MO3RESO_8BITS 1
#define MO3RESO_16BITS 2


struct mo3Sample {
  samplePackMethod method;
  long len;
  long lenComp;
  unsigned short flags;
  char compression;
  char reso;
};

struct mo3Data {
  char channelNb;
  short songLen;
  short uniqueVoice;
  short patternNb;
  short instrNb;
  short sampleNb;

  struct mo3Sample *samples;  

  short *patternLen;
  long *voicePtr;
  long patternTable;
  long instr;
  unsigned long flags;
  int type;
  int version;
};

int parseHeader(unsigned char *src, unsigned long size, int parseLevel, struct mo3Data *mo3Hdr);
int parseVoice(unsigned char *src, long offset, struct mo3Data *mo3Hdr, int parseLevel);
short findVoiceNumber(short pattern, char channel, unsigned char *mo3, struct mo3Data *mo3Hdr);
int parseTrack(unsigned char *src, long offset, struct mo3Data *mo3Hdr, int write);

#endif // MO3_PARSE_H
