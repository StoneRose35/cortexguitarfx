#include <stdint.h>
#include <stdio.h>
//#include <io.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "globalConfig.h"
#include "memoryRegions.h"

/***************************************************************************
\
dfu.c
This is sample software to demonstrate a simple method of manipulating
the DFU suffix as specified in the DFU specification version 1.0.
The following authors have contributed to this sample code:
Robert Nathan
Greg Kroah-Hartman
Trenton Henry
Stephen Satchell
Chuck Foresburg
Gary S. Brown
The CRC algorithm derives from the works of the last three authors listed.
The authors hereby grant developers the right to incorporate any portion
of this source into their own works, provided that proper credit is given
to Gary S. Brown, Stephen Satchell, and Chuck Forsberg. Reference the
following source for the proper format.
Every attempt has been made to ensure that this source is portable.
To that end, it uses only ANSI C libraries. Any identifiers that are not
part of ANSI C have names starting with leading underscores. The purpose
is to differentiate what has been "invented" and what was "pre-existing".
This example cannot modify an existing suffix. To modify a suffix,
delete the current one and then append a new suffix.
*/

/***************************************************************************
\
CRC polynomial 0xedb88320 – Contributed unknowingly by Gary S. Brown.
"Copyright (C) 1986 Gary S. Brown. You may use this program, or code or
tables extracted from it, as desired without restriction."
Paraphrased comments from the original:
The 32 BIT ANSI X3.66 CRC checksum algorithm is used to compute the 32-bit
frame check sequence in ADCCP. (ANSI X3.66, also known as FIPS PUB 71 and
FED-STD-1003, the U.S. versions of CCITT's X.25 link-level protocol.)
The polynomial is:
X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
Put the highest-order term in the lowest-order bit. The X^32 term is
implied, the LSB is the X^31 term, etc. The X^0 term usually shown as +1)
results in the MSB being 1. Put the highest-order term in the lowest-order
bit. The X^32 term is implied, the LSB is the X^31 term, etc. The X^0
term (usually shown as +1) results in the MSB being 1.
The feedback terms table consists of 256 32-bit entries. The feedback
terms
simply represent the results of eight shift/xor operations for all
combinations of data and CRC register values. The values must be right-
shifted by eight bits by the UPDCRC logic so the shift must be unsigned.
*/
unsigned long _crctbl[] = {
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};
/***************************************************************************
**/
unsigned char _suffix[] = {
0x00,
/* bcdDevice lo */
0x00,
/* bcdDevice hi */
0x00,
/* idProduct lo */
0x00,
/* idProduct hi */
0x00,
/* idVendor lo */
0x00,
/* idVendor hi */
0x00,
/* bcdDFU lo */
0x01,
/* bcdDFU hi */
'U',
/* ucDfuSignature lsb */
'F',
/* ucDfuSignature --- */
'D',
/* ucDfuSignature msb */
16,
/* bLength for this version */
0x00,
/* dwCRC lsb */
0x00,
/* dwCRC --- */
0x00,
/* dwCRC --- */
0x00
/* dwCRC msb */
};
/***************************************************************************
\
\*/
void _fatal(const char *);
void _fatal(const char *_str)
{
perror(_str);
fcloseall();
abort();
}
/***************************************************************************
\
The updcrc macro (referred to here as _crc) is derived from an article
Copyright © 1986 by Stephen Satchell.
“Programmers may incorporate any or all code into their programs, giving
proper credit within the source. Publication of the source routines is
permitted so long as proper credit is given to Steven Satchell, Satchell
Evaluations, and Chuck Forsberg, Omen technology."
\**/
#define _crc(accum,delta) (accum)=_crctbl[((accum)^(delta))&0xff]^((accum)>>8)
#define _usage                                                      \
"\nusage: dfu fname [options]\n\n"                                  \
" to check for a suffix use: dfu fname\n\n"                         \
" to remove a suffix use: dfu fname -del\n\n"                       \
" to add a suffix use: dfu fname -did val -pid val -vid val\n\n"    \
" e.g., dfu myfile -did 0x0102 -pid 2345 -vid 017\n"                \
" sets idDevice 0x0102 idProduct 0x0929 idVendor 0x000F\n\n"
#define _getarg(ident,index);                                \
if (!strcmp(argv[_i], (ident)))                              \
{                                                            \
    _write_suffix = 1;                                       \
    if (argc-1 == _i) _fatal(_usage);                        \
    _tmpl = strtol(argv[_i+1], &_charp, 0);                  \
    _suffix[(index)] = (unsigned char)(_tmpl & 0x000000FF);  \
    _tmpl /= 256;                                            \
    _suffix[(index)+1] = (unsigned char)(_tmpl & 0x000000FF);\
}
/***************************************************************************
\
\**/
void writeDfuSuffix(int argc, char **argv)
{
    FILE *_fp;
    FILE *_tmpfp;
    int _remove_suffix = 0;
    int _write_suffix = 0;
    unsigned long _filecrc;
    unsigned long _fullcrc;
    long _i;
    long _tmpl;
    char *_charp;
    /* make sure there is at least one argument */
    errno = EINVAL; 
    if (argc < 2)
        _fatal(_usage);
    /* make sure the file is there */
    _fp = fopen(argv[1], "r+b");
    if (!_fp)
    _fatal(argv[1]);
    /* compute the CRC up to the last 4 bytes */
    fseek(_fp, -4L, SEEK_END);
    _i = ftell(_fp);
    rewind(_fp);
    _filecrc = 0xffffffff;  
    for (; _i; _i--)
        _crc(_filecrc, (unsigned char) fgetc(_fp));
    /* printf("file crc: 0x%08lX\n", _filecrc); */
    /* compute the CRC of everything including the last 4 bytes */
    _fullcrc = _filecrc;
    for (_i = 0; _i < 4; _i++)
        _crc(_fullcrc, (unsigned char) fgetc(_fp));
    /* printf("full crc: 0x%08lX\n", _fullcrc); */
    /* store the file crc away for comparison */
    for (_i = 12; _i < 16; _i++) {
        _suffix[_i] = (unsigned char) (_filecrc & 0x000000ff);
        _filecrc /= 256;
    }
    /* pretend that a suffix exists and try to validate it */
    fseek(_fp, -16L, SEEK_END);
    /* read in the existing suffix */
    for (_i = 0; _i < 6; _i++)
        _suffix[_i] = (unsigned char) fgetc(_fp);
    /* print out whats in there already */
    printf(" idDevice: 0x%02X%02X\n",
        (unsigned char) _suffix[1], (unsigned char) _suffix[0]);
    printf("idProduct: 0x%02X%02X\n",
        (unsigned char) _suffix[3], (unsigned char) _suffix[2]);
    printf(" idVendor: 0x%02X%02X\n",
        (unsigned char) _suffix[5], (unsigned char) _suffix[4]);
    /* now parse the command arguments to overwrite the suffix w/ new values
    */
    for (_i = 1; _i < argc; _i++) {
        errno = EINVAL;
    if (!strcmp(argv[_i], "-del"))
        _remove_suffix = 1;
        _getarg("-did", 0);
        _getarg("-pid", 2);
        _getarg("-vid", 4);
    }
    /* compare the ‘presumed’ file suffix to the suffix in memory */
    for (_i = 6; _i < sizeof(_suffix); _i++)
        if ((unsigned char) fgetc(_fp) != _suffix[_i])
        break;
    if (_i < 8)
        printf("bad bcdDFU\n");
    else if (_i < 11)
        printf("bad ucDfuSignature\n");
    else if (_i < 12)
        printf("bad bLength\n");
    else if (_i < 16)
        printf("bad dwCRC\n");
    if (_i < 16) {
    /* can't remove a suffix if there isn't one there */
        if (_remove_suffix)
            printf("invalid or missing suffix\n");
        _remove_suffix = 0;
    } else {
        printf("valid dfu suffix found\n");
        errno = EINVAL;
        if (_write_suffix)
            _fatal("delete suffix before making changes\n");
    }
    /* now it is known if a suffix exists, and the important
    information has been printed out. so, either the user wants
    to delete the suffix, or to add a new one */
    /* remove an existing suffix? */
    if (_remove_suffix) {    /* pretend that a suffix exists and try to validate it */
        fseek(_fp, -16L, SEEK_END);
        /* read in the existing suffix */
        for (_i = 0; _i < 6; _i++)
            _suffix[_i] = (unsigned char) fgetc(_fp);
        /* print out whats in there already */
            printf(" idDevice: 0x%02X%02X\n",
                (unsigned char) _suffix[1], (unsigned char) _suffix[0]);
            printf("idProduct: 0x%02X%02X\n",
                (unsigned char) _suffix[3], (unsigned char) _suffix[2]);
            printf(" idVendor: 0x%02X%02X\n",
                (unsigned char) _suffix[5], (unsigned char) _suffix[4]);
        _tmpfp = fopen("dfu.tmp", "w+b");
        if (!_tmpfp)
            _fatal("dfu.tmp");
        /* this is not an exercise in how to do buffered file io ;-) */
        fseek(_fp, -_suffix[11], SEEK_END);
        _i = ftell(_fp);
        if (_i > 0) {
            rewind(_fp);
        for (; _i; _i--)
            fputc(fgetc(_fp), _tmpfp);
        fclose(_tmpfp);
        fclose(_fp);
        chmod(argv[1], S_IWRITE);
        remove(argv[1]);
        rename("dfu.tmp", argv[1]); 
        /* warm fuzzies */
        printf("dfu suffix removed from %s\n", argv[1]);
    } else
        printf("%s too small to contain dfu suffix\n", argv[1]);
    exit(0);
    }
/* append a suffix to the file? */
if (_write_suffix) {
/* append a DFU suffix */
fseek(_fp, 0L, SEEK_END);
/* write the suffix while iterating the CRC */
for (_i = 0; _i < sizeof(_suffix) - 4; _i++) {
_crc(_fullcrc, _suffix[_i]);
fputc(_suffix[_i], _fp);
}
/* and write the CRC, lo to hi */
/* printf("full crc: 0x%08lX\n", _fullcrc); */
for (_i = 0; _i < 4; _i++) {
fputc((unsigned char) (_fullcrc & 0x000000ff), _fp);
_fullcrc /= 256;
}
/* warm fuzzies */
printf("dfu suffix appended to %s\n", argv[1]);
}
/* finished */
fclose(_fp);
}
/* eof */

void printHelp(void)
{
    printf("usage: bin2dfu <inputBinFlash.bin> <inputBinQspi.bin>\r\n");
    printf("\toptional: -o <outputFile.bin>\r\n");

}

/*
output file format
offset  size    value
------------------------
0               4       magic nr(0x4168b18f)

4               4       target address of the flash binary (should normally be 0x08000000)

8               4       size in bytes of the flash content (SIZE_FLASH)

SIZE_FLASH+8    4       target address of the qspi memory, relative to the qspi memory, so normally 0

SIZE_FLASH+12   4       size in bytes of the qspi content (SIZE_QSPI)

SIZE_FLASH+
SIZE_QSPI+16    16      dfu file suffix (see usb dfu spec)

*/

#define CHUNK_SIZE 256
int main(int argc,char ** argv)
{
    char *inputFileNameBinFlash;
    char *inputFileNameBinQspi;
    const char * outputFileName=0;

    FILE * inputFileBinFlash;
    FILE * inputFileBinQspi;
    FILE * outputFile;

    uint8_t idxOutputFile=0xFE;
    uint8_t inputFileCnt=0;
    uint32_t magicNr = 0x4168b18f;
    uint32_t wordBfr;
    uint8_t bytesBfr[CHUNK_SIZE];
    uint32_t crc;
    if (argc < 3)
    {
        printf("no enough parameters\r\n");
        printHelp();
        return 0;
    }
    if (argc == 5)
    {
        for (uint8_t c=1;c<argc-1;c++)
        {
            if (strcmp(argv[c],"-o")==0)
            {
                idxOutputFile = c;
                outputFileName = argv[c+1];
                break;
            }   
        }
        
    }
    for (uint8_t c=1;c<argc;c++)
    {
        if (c!= idxOutputFile && c!=idxOutputFile+1)
        {
            if (inputFileCnt == 0)
            {
                inputFileNameBinFlash= argv[c];
                inputFileCnt++;
            }
            else
            {
                inputFileNameBinQspi = argv[c];
            }
        }
    }
    if (outputFileName == 0)
    {
        outputFileName = "PiPicoFX.dfu";
    }
    inputFileBinFlash = fopen(inputFileNameBinFlash,"rb");
    inputFileBinQspi = fopen(inputFileNameBinQspi,"rb");
    outputFile = fopen(outputFileName,"wb");

    fwrite(&magicNr,4,1,outputFile);
    wordBfr = 0x08000000;
    fwrite(&wordBfr,4,1,outputFile);
    fseek(inputFileBinFlash,0,SEEK_END);
    wordBfr = (uint32_t)ftell(inputFileBinFlash);
    fseek(inputFileBinFlash,0,SEEK_SET);
    fwrite(&wordBfr,4,1,outputFile);
    for (uint32_t c=0;c<((wordBfr/CHUNK_SIZE)+1);c++)
    {
        uint32_t bytesRemaining = wordBfr - c*CHUNK_SIZE;
        uint32_t bytesToTransfer;
        if (bytesRemaining > CHUNK_SIZE)
        {
            bytesToTransfer = CHUNK_SIZE;
        }
        else
        {
            bytesToTransfer = bytesRemaining;
            
        }
        fread(bytesBfr,bytesToTransfer,1,inputFileBinFlash);
        fwrite(bytesBfr,bytesToTransfer,1,outputFile);
    }
    wordBfr=0;
    fwrite(&wordBfr,4,1,outputFile);
    fseek(inputFileBinQspi,0,SEEK_END);
    wordBfr = (uint32_t)ftell(inputFileBinQspi);
    fseek(inputFileBinQspi,0,SEEK_SET);
    fwrite(&wordBfr,4,1,outputFile);
    for (uint32_t c=0;c<((wordBfr/CHUNK_SIZE)+1);c++)
    {
        uint32_t bytesRemaining = wordBfr - c*CHUNK_SIZE;
        uint32_t bytesToTransfer;
        if (bytesRemaining > CHUNK_SIZE)
        {
            bytesToTransfer = CHUNK_SIZE;
        }
        else
        {
            bytesToTransfer = bytesRemaining;
            
        }
        fread(bytesBfr,bytesToTransfer,1,inputFileBinQspi);
        fwrite(bytesBfr,bytesToTransfer,1,outputFile);
    }

    // write dfu suffix except crc
    bytesBfr[0] = 0;
    bytesBfr[1] = 0;
    bytesBfr[2] = 0;
    bytesBfr[3] = 0; //dwCRC
    bytesBfr[4] = 16; //bLength
    bytesBfr[5] = 'D';
    bytesBfr[6] = 'F';
    bytesBfr[7] = 'U'; //ucDfuSignature
    bytesBfr[8] = 0x1;
    bytesBfr[9] = 0x1a; //bcdDfu
    bytesBfr[10] = __HIBYTE(USB_VENDOR_ID);
    bytesBfr[11] = __LOBYTE(USB_VENDOR_ID);
    bytesBfr[12] = __HIBYTE(USB_PRODUCT_ID);
    bytesBfr[13] = __LOBYTE(USB_PRODUCT_ID);
    bytesBfr[14] = 0x1;
    bytesBfr[15] = 0x0;

    for (uint8_t c=15;c>3;c--)
    {
        fputc(bytesBfr[c],outputFile);
    }
    fclose(outputFile);

    // open output file to generate the crc
    outputFile = fopen(outputFileName,"rb");

    fseek(outputFile, 0L, SEEK_END);
    wordBfr = (uint32_t)ftell(outputFile);
    rewind(outputFile);
    crc = 0xffffffff;  
    for (; wordBfr; wordBfr--)
        _crc(crc, (unsigned char) fgetc(outputFile));

    fclose(outputFile);
    outputFile =fopen(outputFileName,"ab");
    fseek(outputFile, 0, SEEK_END);    
    fputc((uint8_t)(crc & 0xFF),outputFile);
    fputc((uint8_t)((crc >> 8) & 0xFF),outputFile);
    fputc((uint8_t)((crc >> 16) & 0xFF),outputFile);
    fputc((uint8_t)((crc >> 24) & 0xFF),outputFile);






    fclose(inputFileBinQspi);
    fclose(inputFileBinFlash);
    fclose(outputFile);
    return 0;
}