#pragma once

#include "FlvFunc.h"
#include "Mybs.h"

#define  MAX_VIDEO_TAG_BUF_SIZE   1024 * 1024
#define  VIDEO_TAG_HEADER_LENGTH  11

extern unsigned int I_Frame_Num ; 
extern unsigned int decode_video_done;

typedef struct Tag_Video_AvcC
{
	unsigned char configurationVersion;  
	unsigned char AVCProfileIndication;  
	unsigned char profile_compatibility; 
	unsigned char AVCLevelIndication;    
	unsigned char reserved_1;            
	unsigned char lengthSizeMinusOne;    
	unsigned char reserved_2;            
	unsigned char numOfSequenceParameterSets;  
	unsigned int sequenceParameterSetLength;   
	unsigned char * sequenceParameterSetNALUnit; 
	unsigned char numOfPictureParameterSets;   
	unsigned int  pictureParameterSetLength;   
	unsigned char * pictureParameterSetNALUnit;
	unsigned char reserved_3;
	unsigned char chroma_format;
	unsigned char reserved_4;
	unsigned char bit_depth_luma_minus8;
	unsigned char reserved_5;
	unsigned char bit_depth_chroma_minus8;
	unsigned char numOfSequenceParameterSetExt;
	unsigned int sequenceParameterSetExtLength;
	unsigned char * sequenceParameterSetExtNALUnit;
}Video_AvcC;

typedef struct Tag_Video_Tag                           
{
	unsigned char Type ;                       
	unsigned int  DataSize;                    
	unsigned int  Timestamp;                   
	unsigned char TimestampExtended;           
	unsigned int  StreamID;                    
	
	unsigned char FrameType;                   
	
	unsigned char CodecID ;                    

	unsigned char AVCPacketType;               
	
	unsigned int CompositionTime;              
	
	Video_AvcC  * video_avcc;
	unsigned char * Data;                     
}Video_Tag;


typedef struct Tag_NALU_t
{
	unsigned char forbidden_bit;           
	unsigned char nal_reference_idc;       
	unsigned char nal_unit_type;           
	unsigned int  startcodeprefix_len;     
	unsigned int  len;                     
	unsigned int  max_size;                
	unsigned char * buf;                   
	unsigned char Frametype;               
	unsigned int  lost_packets;            
} NALU_t;


enum nal_unit_type_e
{
	NAL_UNKNOWN     = 0,
	NAL_SLICE       = 1,
	NAL_SLICE_DPA   = 2,
	NAL_SLICE_DPB   = 3,
	NAL_SLICE_DPC   = 4,
	NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
	NAL_SEI         = 6,    /* ref_idc == 0 */
	NAL_SPS         = 7,
	NAL_PPS         = 8
	/* ref_idc == 0 for 6,9,10,11,12 */
};


enum Frametype_e
{
	FRAME_I  = 15,
	FRAME_P  = 16,
	FRAME_B  = 17
};


unsigned int Create_AVCDecoderConfigurationRecord(unsigned char * buf,unsigned char * spsbuf,
	unsigned int spslength,unsigned char * ppsbuf,unsigned int ppslength);
NALU_t *AllocNALU(int buffersize);  
void FreeNALU(NALU_t * n);           
int FindStartCode2 (unsigned char *Buf);         
int FindStartCode3 (unsigned char *Buf);         
int GetAnnexbNALU (NALU_t *nalu);                
int GetFrameType(NALU_t * n);                    
int TraverseH264File();                          
int WriteStruct_H264_Tag(unsigned char * Buf,unsigned int  Timestamp,
	unsigned char AACPacketType,unsigned int * video_frame_type);
