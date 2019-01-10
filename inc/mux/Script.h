#pragma once

#include "FlvFunc.h"
#include "ScriptTag.h"

#define  ONE_SCRIPT_FRAME_SIZE  1024 * 1024
#define  MAX_ECMAARAY_NAME_LENGH      100

int AllocStruct_Script_Tag(Script_Tag ** scripttag);
int FreeStruct_Script_Tag(Script_Tag * scripttag);
int ReadStruct_Script_Tag(unsigned char * Buf , unsigned int length ,Script_Tag * tag);