#pragma once

#include "VideoTag.h"

#define ONE_VIDEO_FRAME_SIZE  1024 * 1024
#define MAX_SPS_FRAME_SIZE 1024
#define MAX_PPS_FRAME_SIZE 1024

int AllocStruct_H264_Tag(Video_Tag ** videotag);
int FreeStruct_H264_Tag(Video_Tag * videotag);
int ReadStruct_H264_Tag(unsigned char * Buf , unsigned int length ,Video_Tag * tag);


