#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "../inc/flv/FlvParser.h"

using namespace std;

#define CheckBuffer(x) { if ((nBufSize-nOffset)<(x)) { nUsedLen = nOffset; return 0;} }

int CAudioTag::_aacProfile;
int CAudioTag::_sampleRateIndex;
int CAudioTag::_channelConfig;

static const unsigned int nH264StartCode = 0x01000000;


CVideoSEI::CVideoSEI()
{
}

CVideoSEI::~CVideoSEI()
{	
	for (size_t i = 0; i < _vVSEI.size(); i++)	
		delete _vVSEI[i].szUD;	
}

int CVideoSEI::Process(unsigned char *pNalu, int nNaluLen, int nTimeStamp)
{
	if (pNalu[4] != 0x06 || pNalu[5] != 0x05)
		return 0;
	unsigned char *p = pNalu + 4 + 2;
	while (*p++ == 0xff);
	const char *szVideojjUUID = "XITONGRTMPSTREAMERUUID";
	char *pp = (char *)p;
	for (size_t i = 0; i < strlen(szVideojjUUID); i++)
	{
		if (pp[i] != szVideojjUUID[i])
			return 0;
	}	
	VSEI sei;
	sei.nTimeStamp = nTimeStamp;
	sei.nLen = nNaluLen - (pp - (char *)pNalu) - 16 - 1;
	sei.szUD = new char[sei.nLen];
	memcpy(sei.szUD, pp + 16, sei.nLen);
	_vVSEI.push_back(sei);

	return 1;
}

CFlvParser::CFlvParser()
{
    _pFlvHeader = NULL;
	_vsei = new CVideoSEI();
}

CFlvParser::~CFlvParser()
{
	for (size_t i = 0; i < _vpTag.size(); i++)
	{
		DestroyTag(_vpTag[i]);
		delete _vpTag[i];
	}
	if (_vsei != NULL)
		delete _vsei;
}

int CFlvParser::Parse(unsigned char *pBuf, int nBufSize, int &nUsedLen)
{
	int nOffset = 0;

	if (_pFlvHeader == 0)
	{
		CheckBuffer(9);
		_pFlvHeader = CreateFlvHeader(pBuf+nOffset);
		nOffset += _pFlvHeader->nHeadSize;
	}

	while (1)
	{
		CheckBuffer(15);
		//int nPrevSize = ShowU32(pBuf + nOffset);
		nOffset += 4;

		Tag *pTag = CreateTag(pBuf + nOffset, nBufSize-nOffset);
		if (pTag == NULL)
		{
			nOffset -= 4;
			break;
		}
		nOffset += (11 + pTag->_header.nDataSize);

		_vpTag.push_back(pTag);
	}
	nUsedLen = nOffset;

	return 0;
}

int CFlvParser::PrintInfo()
{
	Stat();

	cout << "video num: " << _sStat.nVideoNum << " , audio num: "
	     << _sStat.nAudioNum << " , meta num: " << _sStat.nMetaNum << endl;
	cout << "maxTimeStamp: " << _sStat.nMaxTimeStamp << " ,nLengthSize: " << _sStat.nLengthSize << endl;
	cout << "Video SEI num: " << _vsei->_vVSEI.size() << endl;
	for (size_t i = 0; i < _vsei->_vVSEI.size(); i++)
		cout << "SEI time : " << _vsei->_vVSEI[i].nTimeStamp << endl;
	return 1;
}

int CFlvParser::DumpH264(const std::string &path)
{
	fstream f;
	f.open(path.c_str(), ios_base::out|ios_base::binary);

	vector<Tag *>::iterator it_tag;
	for (it_tag = _vpTag.begin(); it_tag != _vpTag.end(); it_tag++)
	{
		if ((*it_tag)->_header.nType != 0x09)
			continue;

		f.write((char *)(*it_tag)->_pMedia, (*it_tag)->_nMediaLen);
	}
	f.close();

	return 1;
}

int CFlvParser::DumpAAC(const std::string &path)
{
	fstream f;
	f.open(path.c_str(), ios_base::out | ios_base::binary);
	vector<Tag *>::iterator it_tag;
	for (it_tag = _vpTag.begin(); it_tag != _vpTag.end(); it_tag++)
	{
		if ((*it_tag)->_header.nType != 0x08)
			continue;
		CAudioTag *pAudioTag = (CAudioTag *)(*it_tag);
		if (pAudioTag->_nSoundFormat != 10)
			continue;
		if (pAudioTag->_nMediaLen!=0)
			f.write((char *)(*it_tag)->_pMedia, (*it_tag)->_nMediaLen);
	}
	f.close();

	return 1;
}

int CFlvParser::DumpFlv(const std::string &path)
{
    fstream f;
    f.open(path.c_str(), ios_base::out | ios_base::binary);    
    f.write((char *)_pFlvHeader->pFlvHeader, _pFlvHeader->nHeadSize);
    unsigned int nLastTagSize = 0;    
    vector<Tag *>::iterator it_tag;
    for (it_tag = _vpTag.begin(); it_tag < _vpTag.end(); it_tag++)
    {
        unsigned int nn = WriteU32(nLastTagSize);
        f.write((char *)&nn, 4);        
        if ((*it_tag)->_header.nType == 0x09 && *((*it_tag)->_pTagData + 1) == 0x01) {
            bool duplicate = false;
            unsigned char *pStartCode = (*it_tag)->_pTagData + 5 + _nNalUnitLength;            
            unsigned nalu_len = 0;
            unsigned char *p_nalu_len=(unsigned char *)&nalu_len;
            switch (_nNalUnitLength) {
            case 4:
                nalu_len = ShowU32((*it_tag)->_pTagData + 5);
                break;
            case 3:
                nalu_len = ShowU24((*it_tag)->_pTagData + 5);
                break;
            case 2:
                nalu_len = ShowU16((*it_tag)->_pTagData + 5);
                break;
            default:
                nalu_len = ShowU8((*it_tag)->_pTagData + 5);
                break;
            }
            //unsigned char *pStartCodeRecord = pStartCode;
            size_t i;
            for (i = 0; i < (size_t)((*it_tag)->_header.nDataSize - 5 - _nNalUnitLength - 4); ++i) {
                if (pStartCode[i] == 0x00 && pStartCode[i+1] == 0x00 && pStartCode[i+2] == 0x00 &&
                        pStartCode[i+3] == 0x01) {
                    if (pStartCode[i+4] == 0x67) {                  
                        i += 4;
                        continue;
                    }
                    else if (pStartCode[i+4] == 0x68) {                        
                        i += 4;
                        continue;
                    }
                    else if (pStartCode[i+4] == 0x06) {                        
                        i += 4;
                        continue;
                    }
                    else {
                        i += 4;                        
                        duplicate = true;
                        break;
                    }
                }
            }

            if (duplicate) {
                nalu_len -= i;
                (*it_tag)->_header.nDataSize -= i;
                unsigned char *p = (unsigned char *)&((*it_tag)->_header.nDataSize);
                (*it_tag)->_pTagHeader[1] = p[2];
                (*it_tag)->_pTagHeader[2] = p[1];
                (*it_tag)->_pTagHeader[3] = p[0];     

                f.write((char *)(*it_tag)->_pTagHeader, 11);
                switch (_nNalUnitLength) {
                case 4:
                    *((*it_tag)->_pTagData + 5) = p_nalu_len[3];
                    *((*it_tag)->_pTagData + 6) = p_nalu_len[2];
                    *((*it_tag)->_pTagData + 7) = p_nalu_len[1];
                    *((*it_tag)->_pTagData + 8) = p_nalu_len[0];
                    break;
                case 3:
                    *((*it_tag)->_pTagData + 5) = p_nalu_len[2];
                    *((*it_tag)->_pTagData + 6) = p_nalu_len[1];
                    *((*it_tag)->_pTagData + 7) = p_nalu_len[0];
                    break;
                case 2:
                    *((*it_tag)->_pTagData + 5) = p_nalu_len[1];
                    *((*it_tag)->_pTagData + 6) = p_nalu_len[0];
                    break;
                default:
                    *((*it_tag)->_pTagData + 5) = p_nalu_len[0];
                    break;
                }         
                f.write((char *)(*it_tag)->_pTagData, pStartCode - (*it_tag)->_pTagData);        
                f.write((char *)pStartCode + i, (*it_tag)->_header.nDataSize - (pStartCode - (*it_tag)->_pTagData));             
            } else {
                f.write((char *)(*it_tag)->_pTagHeader, 11);
                f.write((char *)(*it_tag)->_pTagData, (*it_tag)->_header.nDataSize);
            }
        } else {
            f.write((char *)(*it_tag)->_pTagHeader, 11);
            f.write((char *)(*it_tag)->_pTagData, (*it_tag)->_header.nDataSize);
        }
        nLastTagSize = 11 + (*it_tag)->_header.nDataSize;
    }
    unsigned int nn = WriteU32(nLastTagSize);
    f.write((char *)&nn, 4);
    f.close();

    return 1;
}

int CFlvParser::Stat()
{
	for (size_t i = 0; i < _vpTag.size(); i++)
	{
		switch (_vpTag[i]->_header.nType)
		{
		case 0x08:
			_sStat.nAudioNum++;
			break;
		case 0x09:
			StatVideo(_vpTag[i]);
			break;
		case 0x12:
			_sStat.nMetaNum++;
			break;
		default:
			;
		}
	}

	return 1;
}

int CFlvParser::StatVideo(Tag *pTag)
{
	_sStat.nVideoNum++;
	_sStat.nMaxTimeStamp = pTag->_header.nTimeStamp;

	if (pTag->_pTagData[0] == 0x17 && pTag->_pTagData[1] == 0x00)
	{
		_sStat.nLengthSize = (pTag->_pTagData[9] & 0x03) + 1;
	}

	return 1;
}

CFlvParser::FlvHeader *CFlvParser::CreateFlvHeader(unsigned char *pBuf)
{
	FlvHeader *pHeader = new FlvHeader;
	pHeader->nVersion = pBuf[3];
	pHeader->bHaveAudio = (pBuf[4] >> 2) & 0x01;
	pHeader->bHaveVideo = (pBuf[4] >> 0) & 0x01;
	pHeader->nHeadSize = ShowU32(pBuf + 5);

	pHeader->pFlvHeader = new unsigned char[pHeader->nHeadSize];
	memcpy(pHeader->pFlvHeader, pBuf, pHeader->nHeadSize);

	return pHeader;
}

int CFlvParser::DestroyFlvHeader(FlvHeader *pHeader)
{
	if (pHeader == NULL)
		return 0;
	delete pHeader->pFlvHeader;

	return 1;
}

void Tag::Init(TagHeader *pHeader, unsigned char *pBuf, int nLeftLen)
{
	memcpy(&_header, pHeader, sizeof(TagHeader));
	_pTagHeader = new unsigned char[11];
	memcpy(_pTagHeader, pBuf, 11);
	_pTagData = new unsigned char[_header.nDataSize];
	memcpy(_pTagData, pBuf + 11, _header.nDataSize);
}

CVideoTag::CVideoTag(TagHeader *pHeader, unsigned char *pBuf, int nLeftLen, CFlvParser *pParser)
{
	Init(pHeader, pBuf, nLeftLen);

	unsigned char *pd = _pTagData;
	_nFrameType = (pd[0] & 0xf0) >> 4;
	_nCodecID = pd[0] & 0x0f;
	if (_header.nType == 0x09 && _nCodecID == 7)	
		ParseH264Tag(pParser);	
}

CAudioTag::CAudioTag(TagHeader *pHeader, unsigned char *pBuf, int nLeftLen, CFlvParser *pParser)
{
	Init(pHeader, pBuf, nLeftLen);

	unsigned char *pd = _pTagData;
	_nSoundFormat = (pd[0] & 0xf0) >> 4;
	_nSoundRate = (pd[0] & 0x0c) >> 2;
	_nSoundSize = (pd[0] & 0x02) >> 1;
	_nSoundType = (pd[0] & 0x01);
	if (_nSoundFormat == 10) // AAC
	{
		ParseAACTag(pParser);
	}
}

int CAudioTag::ParseAACTag(CFlvParser *pParser)
{
	unsigned char *pd = _pTagData;
	int nAACPacketType = pd[1];

	if (nAACPacketType == 0)	
		ParseAudioSpecificConfig(pParser, pd);	
	else if (nAACPacketType == 1)	
		ParseRawAAC(pParser, pd);	
	else
	{
	}

	return 1;
}

int CAudioTag::ParseAudioSpecificConfig(CFlvParser *pParser, unsigned char *pTagData)
{
	unsigned char *pd = _pTagData;

	_aacProfile = ((pd[2]&0xf8)>>3) - 1;
	_sampleRateIndex = ((pd[2]&0x07)<<1) | (pd[3]>>7);
	_channelConfig = (pd[3]>>3) & 0x0f;

	_pMedia = NULL;
	_nMediaLen = 0;

	return 1;
}

int CAudioTag::ParseRawAAC(CFlvParser *pParser, unsigned char *pTagData)
{
	uint64_t bits = 0;
	int dataSize = _header.nDataSize - 2;

	CFlvParser::WriteU64(bits, 12, 0xFFF);
	CFlvParser::WriteU64(bits, 1, 0);
	CFlvParser::WriteU64(bits, 2, 0);
	CFlvParser::WriteU64(bits, 1, 1);
	CFlvParser::WriteU64(bits, 2, _aacProfile);
	CFlvParser::WriteU64(bits, 4, _sampleRateIndex);
	CFlvParser::WriteU64(bits, 1, 0);
	CFlvParser::WriteU64(bits, 3, _channelConfig);
	CFlvParser::WriteU64(bits, 1, 0);
	CFlvParser::WriteU64(bits, 1, 0);
	CFlvParser::WriteU64(bits, 1, 0);
	CFlvParser::WriteU64(bits, 1, 0);
	CFlvParser::WriteU64(bits, 13, 7 + dataSize);
	CFlvParser::WriteU64(bits, 11, 0x7FF);
	CFlvParser::WriteU64(bits, 2, 0);

	_nMediaLen = 7 + dataSize;
	_pMedia = new unsigned char[_nMediaLen];
	unsigned char p64[8];
	p64[0] = (unsigned char)(bits >> 56);
	p64[1] = (unsigned char)(bits >> 48);
	p64[2] = (unsigned char)(bits >> 40);
	p64[3] = (unsigned char)(bits >> 32);
	p64[4] = (unsigned char)(bits >> 24);
	p64[5] = (unsigned char)(bits >> 16);
	p64[6] = (unsigned char)(bits >> 8);
	p64[7] = (unsigned char)(bits);
	memcpy(_pMedia, p64+1, 7);
	memcpy(_pMedia + 7, pTagData + 2, dataSize);

	return 1;
}

Tag *CFlvParser::CreateTag(unsigned char *pBuf, int nLeftLen)
{
	TagHeader header;
	header.nType = ShowU8(pBuf+0);
	header.nDataSize = ShowU24(pBuf + 1);
	header.nTimeStamp = ShowU24(pBuf + 4);
	header.nTSEx = ShowU8(pBuf + 7);
	header.nStreamID = ShowU24(pBuf + 8);
	header.nTotalTS = (unsigned int)((header.nTSEx << 24)) + header.nTimeStamp;
	//cout << "total TS : " << header.nTotalTS << endl;	
	if ((header.nDataSize + 11) > nLeftLen)	
		return NULL;	

	Tag *pTag;
	switch (header.nType) {
	case 0x09:
		pTag = new CVideoTag(&header, pBuf, nLeftLen, this);
		break;
	case 0x08:
		pTag = new CAudioTag(&header, pBuf, nLeftLen, this);
		break;
	default:
		pTag = new Tag();
		pTag->Init(&header, pBuf, nLeftLen);
	}
	
	return pTag;
}

int CFlvParser::DestroyTag(Tag *pTag)
{
	if (pTag->_pMedia != NULL)
		delete []pTag->_pMedia;
	if (pTag->_pTagData!=NULL)
		delete []pTag->_pTagData;
	if (pTag->_pTagHeader != NULL)
		delete []pTag->_pTagHeader;

	return 1;
}

int CVideoTag::ParseH264Tag(CFlvParser *pParser)
{
	unsigned char *pd = _pTagData;
	int nAVCPacketType = pd[1];
	//int nCompositionTime = CFlvParser::ShowU24(pd + 2);

	if (nAVCPacketType == 0)	
		ParseH264Configuration(pParser, pd);	
	else if (nAVCPacketType == 1)	
		ParseNalu(pParser, pd);	
	else
	{
	}

	return 1;
}

int CVideoTag::ParseH264Configuration(CFlvParser *pParser, unsigned char *pTagData)
{
	unsigned char *pd = pTagData;

	pParser->_nNalUnitLength = (pd[9] & 0x03) + 1;
	int sps_size, pps_size;
	sps_size = CFlvParser::ShowU16(pd + 11);
	pps_size = CFlvParser::ShowU16(pd + 11 + (2 + sps_size) + 1);	
	_nMediaLen = 4 + sps_size + 4 + pps_size;
	_pMedia = new unsigned char[_nMediaLen];
	memcpy(_pMedia, &nH264StartCode, 4);
	memcpy(_pMedia + 4, pd + 11 + 2, sps_size);
	memcpy(_pMedia + 4 + sps_size, &nH264StartCode, 4);
	memcpy(_pMedia + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);

	return 1;
}

int CVideoTag::ParseNalu(CFlvParser *pParser, unsigned char *pTagData)
{
	unsigned char *pd = pTagData;
	int nOffset = 0;
	_pMedia = new unsigned char[_header.nDataSize+10];
	_nMediaLen = 0;
	nOffset = 5;
	while (1)
	{
		if (nOffset >= _header.nDataSize)
			break;

		int nNaluLen;
		switch (pParser->_nNalUnitLength)
		{
		case 4:
			nNaluLen = CFlvParser::ShowU32(pd + nOffset);
			break;
		case 3:
			nNaluLen = CFlvParser::ShowU24(pd + nOffset);
			break;
		case 2:
			nNaluLen = CFlvParser::ShowU16(pd + nOffset);
			break;
		default:
			nNaluLen = CFlvParser::ShowU8(pd + nOffset);
		}
		memcpy(_pMedia + _nMediaLen, &nH264StartCode, 4);
		memcpy(_pMedia + _nMediaLen + 4, pd + nOffset + pParser->_nNalUnitLength, nNaluLen);
		pParser->_vsei->Process(_pMedia+_nMediaLen, 4+nNaluLen, _header.nTotalTS);
		_nMediaLen += (4 + nNaluLen);
		nOffset += (pParser->_nNalUnitLength + nNaluLen);
	}

	return 1;
}
