#pragma once

#include "Information.h"

extern FILE * pVideo_H264_File;
extern FILE * pAudio_Aac_File;

extern FILE * pVideo_Audio_Flv_File;

FILE * OpenFile(char * FileName,char * OpenMode);                        
void   CloseFile(FILE * pFile);                                          
int    ReadFile(FILE * pFile ,unsigned char * Buffer,int BufferSize);    
int    WriteFile(FILE * pFile ,char * Buffer,int BufferSize);            
