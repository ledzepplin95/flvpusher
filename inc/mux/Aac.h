#pragma once

#include "FlvFunc.h"
#include "AudioTag.h"

#define ONE_AUDIO_FRAME_SIZE 1024*100

int AllocStruct_Aac_Tag(Audio_Tag ** audiotag);
int FreeStruct_Aac_Tag(Audio_Tag * audiotag);
int ReadStruct_Aac_Tag(unsigned char * Buf , unsigned int length ,Audio_Tag * tag);

