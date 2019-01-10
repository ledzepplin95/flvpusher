#pragma once

#include "Information.h"
#include "FileIo.h"

#define FILE_HEADER_LENGTH    9

typedef struct Tag_File_Header
{
	unsigned char Signature_1;                 
	unsigned char Signature_2;                 
	unsigned char Signature_3;                 
	unsigned char version  ;                   
	unsigned char TypeFlagsReserved_1;         
	unsigned char TypeFlagsAudio;              
	unsigned char TypeFlagsReserved_2;         
	unsigned char TypeFlagsVideo;              
	unsigned int  DataOffset;                  

}File_Header;

struct Tag_Tag                           
{
	unsigned char Type ;                       
    unsigned int  DataSize;                    
	unsigned int  Timestamp;                   
	unsigned char TimestampExtended;           
	unsigned int  StreamID;                    
	unsigned char * Data;                      
};

int AllocStruct_File_Header(File_Header ** fileheader);
int FreeStruct_File_Header(File_Header * fileheader);
int ReadStruct_File_Header(unsigned char * Buf , unsigned int length ,File_Header * fileheader);
int WriteStruct_File_Header(unsigned char * Buf , unsigned int length);

