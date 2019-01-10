#pragma once

#include "FlvFunc.h"

#define AUDIO_TAG_HEADER_LENGTH    11
#define ADTS_HEADER_LENGTH         7
#define MAX_AUDIO_TAG_BUF_SIZE     1024 * 100

extern unsigned int decode_audio_done;

typedef struct
{
	unsigned int syncword;  
	unsigned int id;        
	unsigned int layer;     
	unsigned int protection_absent;  
	unsigned int profile;            
	unsigned int sf_index;           
	unsigned int private_bit;        
	unsigned int channel_configuration;  
	unsigned int original;               
	unsigned int home;                   
	
	unsigned int copyright_identification_bit;   
	unsigned int copyright_identification_start; 
	unsigned int aac_frame_length;              
	unsigned int adts_buffer_fullness;  	
	unsigned int no_raw_data_blocks_in_frame;    
} ADTS_HEADER;


typedef struct Tag_Audio_ASC
{
	unsigned char audioObjectType;              
	unsigned char samplingFrequencyIndex;       
	unsigned char channelConfiguration;         
	unsigned char framelengthFlag;              
	unsigned char dependsOnCoreCoder;           
	unsigned char extensionFlag;                
}Audio_ASC;


typedef struct Tag_Audio_Tag                           
{
	unsigned char Type ;                       
	unsigned int  DataSize;                    
	unsigned int  Timestamp;                   
	unsigned char TimestampExtended;           
	unsigned int  StreamID;                    

	unsigned char SoundFormat ;                
	
	unsigned char SoundRate ;                  
	
	unsigned char SoundSize;                   
	
	unsigned char SoundType;                   
	
	unsigned char AACPacketType;               
	
	unsigned char * Data; 
	Audio_ASC * audioasc;
}Audio_Tag;

int   Detach_Head_Aac(ADTS_HEADER * adtsheader);                                 
void  Create_AudioSpecificConfig(unsigned char * buf,
							   unsigned char profile,
							   unsigned char SoundRate,
							   unsigned char SoundType );
int WriteStruct_Aac_Tag(unsigned char * Buf,unsigned int  Timestamp,unsigned char AACPacketType);