#ifndef SHA1_H
#define SHA1_H

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

/* BYTE defines a unsigned character */
typedef unsigned char BYTE;

#ifndef TRUE
  #define FALSE	0
  #define TRUE	( !FALSE )
#endif /* TRUE */

/* The structure for storing SHS info */

typedef struct
{
    UINT4 digest[ 5 ];            /* Message digest */
    UINT4 countLo, countHi;       /* 64-bit bit count */
    UINT4 data[ 16 ];             /* SHS data buffer */
    int Endianness;
} SHA_CTX;

/* Initialize the SHS values */
void SHAInit(SHA_CTX *);

/* Update SHS for a block of data */
void SHAUpdate(SHA_CTX *, BYTE *buffer, int count);

/* Final wrapup - pad to SHS_DATASIZE-byte boundary with the bit pattern
   1 0* (64-bit count of bits processed, MSB-first) */
void SHAFinal(BYTE *output, SHA_CTX *);

#ifndef _ENDIAN_H_
#define _ENDIAN_H_ 1

void endianTest(int *endianness);

#endif /* end _ENDIAN_H_ */

#endif // SHA1_H
