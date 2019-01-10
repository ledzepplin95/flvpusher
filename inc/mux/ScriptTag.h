#pragma once

#include "FlvFunc.h"

#define  MAX_SCRIPT_TAG_BUF_SIZE  1024 * 100
#define  SCRIPT_TAG_HEADER_LENGTH  11

typedef struct Tag_Script_Tag                           
{
	unsigned char Type ;                       
	unsigned int  DataSize;                    
	unsigned int  Timestamp;                   
	unsigned char TimestampExtended;           
	unsigned int  StreamID;                    

	unsigned char Type_1;
	unsigned int  StringLength;                
	unsigned char Type_2;
	unsigned int  ECMAArrayLength;             
	double duration;                                    
	double width;							           
	double height;							           
	double videodatarate;					          
	double framerate;						                
	double videocodecid;					          
	double audiosamplerate;					            
	double audiodatarate;					           
	double audiosamplesize;					            
	int    stereo;							           
	double audiocodecid;					         
	double filesize;						             
	double lasttimetamp;					         
	double lastkeyframetimetamp;               
	double filepositions[1000];                
	double times[1000];                        
	unsigned char * Data;                      
}Script_Tag ;

double char2double(unsigned char * buf,unsigned int size);
void   double2char(unsigned char * buf,double val);
int WriteStruct_Script_Tag(unsigned char * buf,double duration,double width,
	double height,double framerate,double audiosamplerate,int stereo,double filesize,
	unsigned int filepositions_times_length ,double * filepositions,double *times);

