#include "../inc/mux/Demux.h"

File_Header * m_fileheadr  = NULL;
Audio_Tag   * m_audio_tag  = NULL;
Video_Tag   * m_video_tag  = NULL;
Script_Tag  * m_script_tag = NULL;

int AllocStruct()
{	
	AllocStruct_File_Header(&m_fileheadr);
	AllocStruct_Aac_Tag(&m_audio_tag);
	AllocStruct_H264_Tag(&m_video_tag);
	AllocStruct_Script_Tag(&m_script_tag);

	return 1;
}

int FreeStruct()
{	
	FreeStruct_File_Header(m_fileheadr);
	FreeStruct_Aac_Tag(m_audio_tag);
	FreeStruct_H264_Tag(m_video_tag);
	FreeStruct_Script_Tag(m_script_tag);

	return 1;
}

int And_Head_H264(unsigned char * buf, unsigned int size,unsigned char *spsbuffer,unsigned int spslength,
	unsigned char * ppsbuffer,unsigned int  ppslength ,unsigned int IsVideo_I_Frame)
{
	int write_size = 0;
	unsigned char * buffer = NULL;
	unsigned char strcode[4];                
	unsigned int Sei_length_1 = 0;           
	//unsigned int Sei_length_2 = 0;
	buffer = (unsigned char * )calloc(size + 1024,sizeof(char));
	
	if (buf[4] == 0x06)  
	{
		Sei_length_1 =
			buf[2]  << 8  |
			buf[3];
		memcpy(buffer,buf,size);
		buffer[4 + Sei_length_1] =    0x00;
		buffer[4 + Sei_length_1 +1] = 0x00;
		buffer[4 + Sei_length_1 +2] = 0x00;
		buffer[4 + Sei_length_1 +3] = 0x01;
	}
	else
	{
		memcpy(buffer,buf,size);
	}
	if (IsVideo_I_Frame == 1) 
	{
		strcode[0] = 0x00;
		strcode[1] = 0x00;
		strcode[2] = 0x00;
		strcode[3] = 0x01;
		write_size = WriteFile(pVideo_H264_File ,(char *)strcode,4);
		write_size = WriteFile(pVideo_H264_File ,(char *)spsbuffer,spslength);
		write_size = WriteFile(pVideo_H264_File ,(char *)strcode,4);
		write_size = WriteFile(pVideo_H264_File ,(char *)ppsbuffer,ppslength);
	}
	buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	buffer[3] = 0x01;
	
	write_size = WriteFile(pVideo_H264_File ,(char *)buffer,size);
	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}

	return  write_size;
}

int And_Head_Aac(unsigned char * buf, unsigned int size,unsigned int IsAacFrame,
	unsigned char audioObjectType,unsigned char samplerate,unsigned char channelcount)
{
	int write_size = 0;
	if (IsAacFrame)
	{		
		unsigned char  adts_headerbuf[ADTS_HEADER_LENGTH] ;
		unsigned int  aac_frame_length = size + ADTS_HEADER_LENGTH;
		adts_headerbuf[0] = 0xFF;
		adts_headerbuf[1] = 0xF1;

		adts_headerbuf[2] = (0x01 << 6)  | (samplerate << 2)   | (channelcount >> 7) ;
		adts_headerbuf[3] = (channelcount << 6) |  0x00 | 0x00 | 0x00 |0x00 | ((aac_frame_length &  0x1800) >> 11);
		adts_headerbuf[4] = (aac_frame_length & 0x7F8) >> 3;
		adts_headerbuf[5] = (aac_frame_length & 0x7) << 5  |  0x1F;
		adts_headerbuf[6] = 0xFC  | 0x00;
		write_size = WriteFile(pAudio_Aac_File ,(char*)adts_headerbuf,ADTS_HEADER_LENGTH);
	}
	write_size = WriteFile(pAudio_Aac_File ,(char *)buf,size);

	return write_size;
}

int ReadStruct()
{
	unsigned char * temporary_buf = NULL;
	unsigned int    temporary_bufsize = 0;
	unsigned int    tag_data_size = 0 ;           
	unsigned char   tag_type;                     
	unsigned int    audiowritedatasize = 0;
	unsigned int    videowritedatasize = 0;
	unsigned int    scriptwritedatasize= 0;
	unsigned int    IsAacFrame = 0;
	unsigned char   spsbuffer[MAX_FRAME_HEAD_SIZE];
	unsigned char   ppsbuffer[MAX_FRAME_HEAD_SIZE];
	unsigned int    spslength = 0;
    unsigned int    ppslength = 0;
	unsigned char   audioObjectType;              
	unsigned char   samplingFrequencyIndex;       
	unsigned char   channelConfiguration;         

	if ((temporary_buf = (unsigned char *)calloc(ONE_FRAME_SIZE,sizeof(char))) == NULL)
	{
		printf ("Error: Allocate Meory To temporary_buf Buffer Failed ");
		return getchar();
	}
	temporary_bufsize = ReadFile(pVideo_Audio_Flv_File ,temporary_buf,9);
	ReadStruct_File_Header(temporary_buf , 9 ,m_fileheadr);
	
    while (!feof (pVideo_Audio_Flv_File) != 0)                         
	{
		temporary_bufsize = ReadFile(pVideo_Audio_Flv_File ,temporary_buf,4);     
		temporary_bufsize = ReadFile(pVideo_Audio_Flv_File ,temporary_buf,11);    
		if (temporary_bufsize == 0)
		{
			if (feof (pVideo_Audio_Flv_File) != 0)   
			{
				break;
			}
		}
		tag_type = temporary_buf[0];
		tag_data_size =
			temporary_buf[1]  << 16 |
			temporary_buf[2]  << 8  |
			temporary_buf[3];
		temporary_bufsize = ReadFile(pVideo_Audio_Flv_File,temporary_buf + 11,tag_data_size);
		switch (tag_type)
		{
		case 0x08:     
			//printf("audio  : tag  size  include tag header : %d\n",(tag_data_size +11));
			audiowritedatasize = ReadStruct_Aac_Tag(temporary_buf ,tag_data_size + 11 ,m_audio_tag);
			if (m_audio_tag->SoundFormat == 0x0A )       
			{
				IsAacFrame = 1;
				if(m_audio_tag->AACPacketType != 0x00)   
				{
					And_Head_Aac(m_audio_tag->Data,audiowritedatasize,
						IsAacFrame,audioObjectType,samplingFrequencyIndex,channelConfiguration);
				}
				else
				{
					audioObjectType = m_audio_tag->audioasc->audioObjectType;
					samplingFrequencyIndex = m_audio_tag->audioasc->samplingFrequencyIndex;
					channelConfiguration = m_audio_tag->audioasc->channelConfiguration;
				}
			}
			else                                         
			{
				IsAacFrame = 0;
				And_Head_Aac(m_audio_tag->Data,audiowritedatasize,IsAacFrame,
					audioObjectType,samplingFrequencyIndex,channelConfiguration);
			}
			break;
		case 0x09:
			//printf("video  : tag  size  include tag header : %d\n",(tag_data_size +11));
			videowritedatasize = ReadStruct_H264_Tag(temporary_buf ,tag_data_size + 11 ,m_video_tag);
			if (m_video_tag->CodecID == 0x07 )  
			{
				if (m_video_tag->AVCPacketType == 0x00)      
				{					
					spslength = m_video_tag->video_avcc->sequenceParameterSetLength;
					ppslength = m_video_tag->video_avcc->pictureParameterSetLength;
					memcpy(spsbuffer,m_video_tag->video_avcc->sequenceParameterSetNALUnit,spslength);
					memcpy(ppsbuffer,m_video_tag->video_avcc->pictureParameterSetNALUnit,ppslength);
				}
				else if(m_video_tag->AVCPacketType != 0x02)  
				{
					And_Head_H264(m_video_tag->Data,videowritedatasize,spsbuffer,spslength,ppsbuffer,ppslength,m_video_tag->FrameType);
				}
			}
			else
			{
				And_Head_H264(m_video_tag->Data,videowritedatasize,spsbuffer,spslength,ppsbuffer,ppslength,m_video_tag->FrameType);
			}
			break;
		case 0x12:
			//printf("script : tag  size  include tag header : %d\n",(tag_data_size +11));
			scriptwritedatasize = ReadStruct_Script_Tag(temporary_buf ,tag_data_size + 11 ,m_script_tag);
			break;
		default:
			//printf("others : tag  reserved\n");
			break;
		}
	}
	printf("duration                        : %lf\n",m_script_tag->duration);
	printf("width                           : %lf\n",m_script_tag->width);
	printf("height                          : %lf\n",m_script_tag->height);
	printf("videodatarate                   : %lf\n",m_script_tag->videodatarate);
	printf("framerate                       : %lf\n",m_script_tag->framerate);
	printf("videocodecid                    : %lf\n",m_script_tag->videodatarate);
	printf("audiosamplerate                 : %lf\n",m_script_tag->audiosamplerate);
	printf("audiodatarate                   : %lf\n",m_script_tag->audiodatarate);
	printf("audiosamplesize                 : %lf\n",m_script_tag->audiosamplesize);
	printf("stereo                          : %d\n",m_script_tag->stereo);
	printf("audiocodecid                    : %lf\n",m_script_tag->audiocodecid);
	printf("filesize                        : %lf\n",m_script_tag->filesize);
	printf("lasttimetamp                    : %lf\n",m_script_tag->lasttimetamp);
	printf("lastkeyframetimetamp            : %lf\n",m_script_tag->lastkeyframetimetamp);
//////////////////////////////////////////////////////////////////////////
	if (temporary_buf)
	{
		free(temporary_buf);
		temporary_buf = NULL;
	}

	return 1;
}
