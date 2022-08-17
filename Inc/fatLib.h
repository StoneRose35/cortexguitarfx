#ifndef _FAT_LIB_H_
#define _FAT_LIB_H_

//#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "systemChoice.h"

typedef struct  
{
    uint8_t partitionType;
    uint32_t lbaStart;
    uint32_t nrSectors;
} PartitionInfoType;

typedef struct 
{
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t numberOfReservedSectors;
    uint8_t numberOfFats;
    uint32_t sectorsPerFat;
    uint32_t rootDirectoryFirstCluster;
} VolumeIdType;


typedef volatile struct 
{
    char filename[8];
    char fileext[3];
    uint8_t attrib;
    uint32_t firstCluster;
    uint32_t size;
} DirectoryEntryType;

typedef struct 
{
    uint32_t clusterLbaBegin;
    uint32_t fatLbaBegin;
    uint32_t fat2LbaBegin;
} DerivedFATDataType;

typedef struct 
{
    PartitionInfoType partitionInfo;
    VolumeIdType volumeId;
    DerivedFATDataType derivedFATData;

} SdCardInfoType;

typedef union 
{
    uint8_t bytesData[4];
    uint32_t wordValue;
} UInt32Type;

typedef struct 
{
    DirectoryEntryType * dirEntry;
    uint32_t clusterPtr;
    uint16_t sectorPtr; // in sectors, should be increased at every read/write Operation
    uint16_t clusterCntr;
    uint16_t sectorBufferPtr;
    uint8_t sectorBuffer[512];
} FilePointerType;

typedef volatile struct 
{
    DirectoryEntryType * dirEntry;
    uint32_t clusterLbaPtr;
    uint32_t clusterPtr;
    uint16_t sectorPtr; // in sectors, should be increased at every read/write Operation
    uint16_t clusterCntr;
} DirectoryPointerType;

#ifndef HARDWARE
uint8_t readSector(uint8_t*,uint64_t);
#endif
void getPartitionInfo(uint8_t *,uint8_t,PartitionInfoType*);
void getVolumeId(uint8_t *,VolumeIdType* );

uint32_t getFatLbaBegin(PartitionInfoType*,VolumeIdType*);
uint32_t getClusterLbaBegin(PartitionInfoType* ,VolumeIdType*);
uint32_t getClusterLba(uint32_t);
uint8_t entryToSector(uint8_t * sect, uint8_t position,DirectoryPointerType * fp,DirectoryEntryType * entry);
uint16_t getDirectoryEntries(uint8_t*,DirectoryEntryType**);
uint8_t copyDirectoryPointer(DirectoryPointerType ** src,DirectoryPointerType ** dest);
uint16_t getDirectoryContentSize(DirectoryPointerType * fp);
uint8_t createDirectoryPointer(DirectoryPointerType ** dp);
uint8_t destroyDirectoryPointer(DirectoryPointerType ** dp);
uint32_t getNextFreeCluster();

uint32_t getNextCluster(uint8_t*,uint32_t clusterNr);
uint32_t getPreviousCluster(uint8_t*,uint32_t clusterNr);


uint8_t initFatSDCard();
void mountFat32SDCard(DirectoryPointerType ** cwd,DirectoryPointerType ** ndir);
uint8_t openRootDirectory(DirectoryPointerType * fp);
uint8_t openFile(DirectoryPointerType * parentDir,char * filename,FilePointerType * fp);
uint8_t openDirectory(DirectoryPointerType * parentDir,char * dirname,DirectoryPointerType * fp);
uint8_t createDirectory(DirectoryPointerType * fp,char * directoryName);
uint8_t createFile(DirectoryPointerType * fp,char * fileName);
uint8_t deleteDirectory(DirectoryPointerType * parentDir,DirectoryPointerType * fp);
uint16_t readFile(FilePointerType * fp);
uint8_t writeFile(DirectoryPointerType * parentDir,FilePointerType * fp,uint16_t nrbytes);
uint8_t appendToFile(DirectoryPointerType * parentDir,FilePointerType * fp,uint8_t * data, uint16_t datalen);
uint8_t seekEnd(FilePointerType * fp);
uint8_t seekStart(FilePointerType * fp);
uint8_t deleteFile(DirectoryPointerType * parentDir,FilePointerType * fp);

// filename helper functions
uint8_t filenameEquals(DirectoryEntryType * entry,char * fileName);


/******************
 *   ERROR CODES
 ******************/
#define FATLIB_WRONG_MBR_SIG 11
#define FATLIB_WRONG_VOLUMEID_SIG 12
#define FATLIB_WRONG_PART_TYPE 13
#define FATLIB_NO_FILE 14
#define FATLIB_DIRECTORY_NOT_FOUND 15
#define FATLIB_MEMORY_ALLOC_FAILURE 16
#define FATLIB_FOLDERDELETE_TOO_MANY_CLUSTERS 17
#define FATLIB_CREATEDIR_ALREADY_EXISTS 18
#define FATLIB_DIRECTORY_NOT_EMPTY 19
#define FATLIB_FILE_NOT_FOUND 20
#define FATLIB_DISK_FULL 0xFFFFFFFF


/******************
 * SIZE RESTRICTION
 ******************/
#define FATLIB_FOLDER_MAX_CLUSTERS 16 // the maximum number of clusters a folder can span to be writeable and deleteable

#endif
