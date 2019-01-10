#pragma once

#include <fstream>
#include <string>
#include <bitset> 

using namespace std;

typedef bitset<8> byte;

class XTVFReader
{
public:
	XTVFReader(void);
	~XTVFReader(void);
	XTVFReader(const char*fileName);

	void xtvf2flv();
	void close();
	string getAACName()const;
	string getH264Name()const;
	string getFlvName()const;
	unsigned int getFrameHeight()const;
	unsigned int getFrameWidth()const;
	unsigned int getFrameRate()const;

private:
	void header();
	void process();
	bitset<56> getADTSHeader(byte header[],unsigned int len);

private:
	fstream m_file;

	string aac,h264,flv;
	char adts_header[2];
	unsigned int frameWidth,frameHeight;
	unsigned int frameRate;
};

