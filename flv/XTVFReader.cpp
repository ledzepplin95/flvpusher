#include <string>
#include <iostream>
#include <algorithm>
#include <stdint.h>
#include <string.h>

#include "../inc/flv/FlvParser.h"
#include "../inc/flv/XTVFReader.h"

#pragma pack(push,1)
typedef struct tagHeader {  
	uint32_t frameLength; 
	byte frameType; 
	uint32_t timeStamp;  	 
} TAG_HEADER;


XTVFReader::XTVFReader(void)
{
	frameWidth=0;
	frameHeight=0;
	frameRate=0;
}

XTVFReader::~XTVFReader(void)
{
	//if(m_file) m_file.close();
	aac.clear();
	h264.clear();
	flv.clear();
}

void XTVFReader::close()
{
	if(m_file) m_file.close();
	frameWidth=0;
	frameHeight=0;
	frameRate=0;
	aac.clear();
	h264.clear();
	flv.clear();
}

inline string getStringLastNChar(string str, int lastN)
{
	return str.substr(str.size() - lastN);
}

inline string getStringNChar(string str,int n)
{
	return str.substr(0,n);
}

XTVFReader::XTVFReader(const char*fileName)
{
	string ending("xtvf");
	string ext=getStringLastNChar(string(fileName),4);
	transform(ext.begin(),ext.end(),ext.begin(),(int(*)(int))std::tolower);
	if(ext!=ending)
	{
		cout<< "file extension not correct!" <<endl;
		return;
	}
	m_file.open(fileName, ios_base::in | ios_base::binary);
	if (!m_file) 
	{
		printf("can't open .xtvf file!\n");
		return;
	}
    flv=getStringNChar(string(fileName),strlen(fileName)-4);
        
	aac="/tmp/"+flv+"aac";
	h264="/tmp/"+flv+"264";
	flv="/tmp/"+flv+"flv";	
}

void XTVFReader::xtvf2flv()
{
	header();	
	process();
}

void XTVFReader::header()
{
	m_file.seekg(0,std::ios::beg);
	char buf[5];
	m_file.read(buf,5);
	if(buf[0]!='X'|| buf[1]!='T'
		|| buf[2]!='V' || buf[3]!='F')
	{
		cout << "file header not right!"<< endl;
	}
}


bitset<56> XTVFReader::getADTSHeader(byte header[],unsigned int len)
{
	bitset<56> b;
	b.reset();
    for(size_t i=0;i<12;i++)
		b[i]=1;
	b[12]=0; b[13]=0; b[14]=0; b[15]=1;

	if( ((header[0]&(byte)0xf)>>1&(byte)1)==true ) b[16]=1;
	if( ((header[0]&(byte)0xf)&(byte)1)==true ) b[17]=1;

	if( (((header[0]&(byte)0xf0)>>3)&(byte)1)==true ) b[18]=1;
    if( (((header[0]&(byte)0xf0)>>2)&(byte)1)==true ) b[19]=1;
	if( (((header[0]&(byte)0xf0)>>1)&(byte)1)==true ) b[20]=1;
	if( ((header[0]&(byte)0xf0)&(byte)1)==true ) b[21]=1;

	b[22]=0;
	//b[23]=((header[1]&0xf)>>2)&1;
	//b[24]=((header[1]&0xf)>>1)&1;
	//b[25]=(header[1]&0xf)&1;
	b[23]=1; b[24]=1; b[25]=0; 

	b[26]=0;
	b[27]=0;
	b[28]=0;
	b[29]=0;
	short frameLength=len+7;
	if( (frameLength>>12)&1 ) b[30]=1;
	if( (frameLength>>11)&1 ) b[31]=1;
	if( (frameLength>>10)&1 ) b[32]=1;
    if( (frameLength>>9)&1 ) b[33]=1;
	if( (frameLength>>8)&1 ) b[34]=1;
	if( (frameLength>>7)&1 ) b[35]=1;
	if( (frameLength>>6)&1 ) b[36]=1;
	if( (frameLength>>5)&1 ) b[37]=1;
	if( (frameLength>>4)&1 ) b[38]=1;
	if( (frameLength>>3)&1 ) b[39]=1;
	if( (frameLength>>2)&1 ) b[40]=1;
	if( (frameLength>>1)&1 ) b[41]=1;
    if( frameLength&1 ) b[42]=1;

	for(size_t i=0;i<11;i++)
		b[i+43]=1;
	b[54]=0;
	b[55]=0;

	return b;
}

char reverse(char c)
{
	c = (c & 0xaa) >> 1 | (c & 0x55) << 1;
	c = (c & 0xcc) >> 2 | (c & 0x33) << 2;
	c = (c & 0xf0) >> 4 | (c & 0x0f) << 4;

	return c;
}

void bitset2array(const std::bitset<56> &bits,char *buf)
{	
	int n_bytes = 0;
	for(int i=0; i<56; i+= 8)
	{
		char ch;
		for(int j=0; j<8; ++j)
		{
			if (bits.test(i + j))
				ch |= (1 << j);
			else
				ch &= ~(1 << j);
		}
		buf[n_bytes++] = ch;
	}
}



void XTVFReader::process()
{   	
	int nBufSize = 3000 * 1024;
	char*pBuf = new char[nBufSize];	
	fstream aac_file,h264_file;
	aac_file.open(aac.c_str(), ios_base::out | ios_base::binary);
	h264_file.open(h264.c_str(), ios_base::out | ios_base::binary);	
	int frameNo=1;
	int audioFrameNo=1;
	int videoFrameNo=1;
	//int audioTimeStamp=0;
	int videoTimeStamp=0;
	while (!m_file.eof())
	{	
		char startCode[4];
		m_file.read(startCode,4);		
		m_file.read(pBuf,9);	        
		unsigned int frameLength=CFlvParser::ShowU32((unsigned char*)pBuf);
		byte frameType=CFlvParser::ShowU8((unsigned char*)pBuf+4);
		unsigned int timeStamp=CFlvParser::ShowU32((unsigned char*)pBuf+5);
		
		m_file.read(pBuf,frameLength-5);
		if(frameType==0x09)
		{
			byte at=pBuf[0];
			if(at==0xa0)
			{
				adts_header[0]=pBuf[1];
				adts_header[1]=pBuf[2];
				bool isAAC=(adts_header[0]&0xf)==0x02;
				if(!isAAC)
				{
					aac_file.close();
					h264_file.close();
					//m_file.close();
					delete [] pBuf;
					return;
				}				
			}
			else if(at==0xa1)
			{					
				bitset<56> header=getADTSHeader((byte*)&adts_header[0],frameLength-6);			
				char real_header[7];			
				bitset2array(header,&real_header[0]);
				for(int i=0;i<7;i++)
					real_header[i]=reverse(real_header[i]);
				aac_file.write(&real_header[0],7);		
				aac_file.write(&pBuf[1],frameLength-6);
				audioFrameNo++;			
				//audioTimeStamp=timeStamp;
			}
		}
		else if(frameType==0x08)
		{
			byte vt=pBuf[0];
			if(vt==0xa0)
			{
				frameWidth=CFlvParser::ShowU32((unsigned char*)pBuf+1);
				frameHeight=CFlvParser::ShowU32((unsigned char*)pBuf+5);
			}
			else if(vt==0xa1)
			{	
				h264_file.write(startCode,4);
				h264_file.write(&pBuf[1],frameLength-6);
				videoFrameNo++;			
				videoTimeStamp=timeStamp;			;
			}			
		}
		frameNo++;
	}	
	aac_file.close();
	h264_file.close();
	delete [] pBuf;
	frameRate=1000/(videoTimeStamp/videoFrameNo);	
}

string XTVFReader::getAACName()const
{
	return aac;
}

string XTVFReader::getH264Name()const
{
	return h264;
}

string XTVFReader::getFlvName()const
{
	return flv;
}

unsigned int XTVFReader::getFrameHeight()const
{
	return frameHeight;
}

unsigned int XTVFReader::getFrameWidth()const
{
	return frameWidth;
}

unsigned int XTVFReader::getFrameRate()const
{
	return frameRate;
}

#pragma pack(pop)
