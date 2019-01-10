
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "inc/flv/XTVFReader.h"
#include "inc/mux/Mux.h"

#include <librtmp/log.h>
#include <librtmp/rtmp.h>

#include <sys/socket.h>
#include <unistd.h>  
#include <netinet/in.h>  
#include <arpa/inet.h> 

#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
	(x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

int rtmpFd;

int ReadU8(uint32_t *u8,FILE*fp){
	if(fread(u8,1,1,fp)!=1)
		return 0;
	return 1;
}

int ReadU16(uint32_t *u16,FILE*fp){
	if(fread(u16,2,1,fp)!=1)
		return 0;
	*u16=HTON16(*u16);
	return 1;
}

int ReadU24(uint32_t *u24,FILE*fp){
	if(fread(u24,3,1,fp)!=1)
		return 0;
	*u24=HTON24(*u24);
	return 1;
}

int ReadU32(uint32_t *u32,FILE*fp){
	if(fread(u32,4,1,fp)!=1)
		return 0;
	*u32=HTON32(*u32);
	return 1;
}

int PeekU8(uint32_t *u8,FILE*fp){
	if(fread(u8,1,1,fp)!=1)
		return 0;
	fseek(fp,-1,SEEK_CUR);
	return 1;
}

int ReadTime(uint32_t *utime,FILE*fp){
	if(fread(utime,4,1,fp)!=1)
		return 0;
	*utime=HTONTIME(*utime);
	return 1;
}

int InitSockets()
{
#ifdef WIN32
	WORD version;
	WSADATA wsaData;
	version=MAKEWORD(2,2);
	return (WSAStartup(version, &wsaData) == 0);
#endif   

    return 1;
}

void CleanupSockets()
{
#ifdef WIN32
	WSACleanup();
#endif   
}

int testPublish(char*flv,char*url){

	RTMP *rtmp=NULL;			
	RTMPPacket *packet=NULL;
	uint32_t start_time=0;
	uint32_t now_time=0;
	long pre_frame_time=0; 
	long lasttime=0;
	int bNextIsKey=1;
	uint32_t preTagsize=0;	
	uint32_t type=0;			
	uint32_t datalength=0;		
	uint32_t timestamp=0;		
	uint32_t streamid=0;

	if (!InitSockets()){
		RTMP_LogPrintf("Init Socket Err\n");
		return -1;
	}
	rtmp=RTMP_Alloc();	
	RTMP_Init(rtmp);	
	//rtmp->Link.timeout=5;	
	if(!RTMP_SetupURL(rtmp,url))
	{
		RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}	
	RTMP_EnableWrite(rtmp);	
	if (!RTMP_Connect(rtmp,NULL)){
		RTMP_Log(RTMP_LOGERROR,"Connect Err\n");
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}
	if (!RTMP_ConnectStream(rtmp,0)){
		RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}
	packet=(RTMPPacket*)malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet,1024*64);
	RTMPPacket_Reset(packet);
	packet->m_hasAbsTimestamp = 0;	
	packet->m_nChannel = 0x04;	
	packet->m_nInfoField2 = rtmp->m_stream_id;

	FILE*fp=NULL;
	fp=fopen(flv,"rb");
	if (!fp){
		RTMP_LogPrintf("Open File Error.\n");
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}
	RTMP_LogPrintf("Start to send data ...\n");
	fseek(fp,9,SEEK_SET);	
	fseek(fp,4,SEEK_CUR);	
	start_time=RTMP_GetTime();
	while(1)
	{
		if((((now_time=RTMP_GetTime())-start_time)<(pre_frame_time)) && bNextIsKey){		
				if(pre_frame_time>lasttime){
					RTMP_LogPrintf("TimeStamp:%8lu ms\n",pre_frame_time);
					lasttime=pre_frame_time;
				}
				usleep(20*1000);
				continue;
		}		
		if(!ReadU8(&type,fp))	
			break;		
		if(!ReadU24(&datalength,fp))
			break;		
		if(!ReadTime(&timestamp,fp))
			break;		
		if(!ReadU24(&streamid,fp))
			break;	
		if (type!=0x08&&type!=0x09){		
			fseek(fp,datalength+4,SEEK_CUR);
			continue;
		}		
		if(fread(packet->m_body,1,datalength,fp)!=datalength)
			break;
		packet->m_headerType = RTMP_PACKET_SIZE_LARGE; 
		packet->m_nTimeStamp = timestamp; 
		packet->m_packetType = type;
		packet->m_nBodySize  = datalength;
		pre_frame_time=timestamp;
		if (!RTMP_IsConnected(rtmp)){
			RTMP_Log(RTMP_LOGERROR,"rtmp is not connect\n");
			break;
		}		
		if (!RTMP_SendPacket(rtmp,packet,0)){
			RTMP_Log(RTMP_LOGERROR,"Send Error\n");
			break;
		}		
		if(!ReadU32(&preTagsize,fp))
			break;		
		if(!PeekU8(&type,fp))
			break;
		if(type==0x09){
			if(fseek(fp,11,SEEK_CUR)!=0)
				break;
			if(!PeekU8(&type,fp)){
				break;
			}
			if(type==0x17)
				bNextIsKey=1;
			else
				bNextIsKey=0;
			fseek(fp,-11,SEEK_CUR);
		}
	}    
	RTMP_LogPrintf("\nSend Data Over\n");
	if(fp) fclose(fp);

	if (rtmp!=NULL){
		RTMP_Close(rtmp);	
		RTMP_Free(rtmp);	
		rtmp=NULL;
	}
	if (packet!=NULL){
		RTMPPacket_Free(packet);	
		free(packet);
		packet=NULL;
	}
	CleanupSockets();

	return 1;
}   

void Mux(char*aacFile,char*h264File,char*flvFile,
	double width,double height,double frameRate)
{	
	pVideo_H264_File = OpenFile(h264File,(char*)"rb");
        if(pVideo_H264_File==NULL) return;
	pAudio_Aac_File = OpenFile(aacFile,(char*)"rb");
        if(pAudio_Aac_File==NULL) return;
	pVideo_Audio_Flv_File = OpenFile(flvFile,(char*)"wb");
        if(pVideo_Audio_Flv_File ==NULL) return;

	WriteBuf2File(width,height,frameRate);

	if (pVideo_H264_File)
	{
		CloseFile(pVideo_H264_File);
		pVideo_H264_File = NULL;
	}
	if (pAudio_Aac_File)
	{
		CloseFile(pAudio_Aac_File);
		pAudio_Aac_File = NULL;
	}
	if (pVideo_Audio_Flv_File)
	{
		CloseFile(pVideo_Audio_Flv_File);
		pVideo_Audio_Flv_File = NULL;
	}		
}


int main(int argc, char* argv[])
{
	if(argc!=3) return 0;

	for(;;)
	{
		XTVFReader reader(argv[1]);
		reader.xtvf2flv();	
                Mux((char*)reader.getAACName().c_str(),(char*)reader.getH264Name().c_str(),
                    (char*)reader.getFlvName().c_str(),reader.getFrameWidth(),
                    reader.getFrameHeight(),reader.getFrameRate());
		testPublish(const_cast<char*>(reader.getFlvName().c_str()),argv[2]);
		//testPublish((char*)"33.flv",argv[2]);
		reader.close();		
	}

	return 0;
}

