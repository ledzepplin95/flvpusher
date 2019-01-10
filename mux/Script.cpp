#include "../inc/mux/Script.h"

int AllocStruct_Script_Tag(Script_Tag ** scripttag)
{
	Script_Tag * scripttag_t = * scripttag;
	if ((scripttag_t = (Script_Tag *)calloc(1,sizeof(Script_Tag))) == NULL)
	{
		printf ("Error: Allocate Meory To AllocStruct_Script_Tag Buffer Failed ");
		return getchar();
	} 
	if ((scripttag_t->Data = (unsigned char * )calloc(ONE_SCRIPT_FRAME_SIZE,sizeof(unsigned char))) == NULL)
	{
		printf ("Error: Allocate Meory To scripttag_t->Data Buffer Failed ");
		return getchar();
	}
	* scripttag = scripttag_t;

	return 1;
}

int FreeStruct_Script_Tag(Script_Tag * scripttag)
{
	if (scripttag)
	{
		if (scripttag->Data)
		{
			free(scripttag->Data);
			scripttag->Data = NULL;
		}
		free(scripttag);
		scripttag = NULL;
	}

	return 1;
}

int ReadStruct_Script_Tag(unsigned char * Buf , unsigned int length ,Script_Tag * tag)
{
	int Script_Tag_pos = 0;
	int Arry_byte_length;
	unsigned char Arry_Name[MAX_ECMAARAY_NAME_LENGH];
	unsigned char Arry_InFomation;
	unsigned char Arry_InFomation_framekey;
	unsigned int  Arry_Name_framekey_Arry_length;

	tag->Type = Buf[0];
	tag->DataSize = 
		Buf[1]  << 16 |
		Buf[2]  << 8  |
		Buf[3];
	tag->Timestamp = 
		Buf[4]  << 16 |
		Buf[5]  << 8  |
		Buf[6];
	tag->TimestampExtended = Buf[7];
	tag->StreamID = 
		Buf[8]  << 16 |
		Buf[9]  << 8  |
		Buf[10];
	Script_Tag_pos += 11;
	
	tag->Type_1 = Buf[Script_Tag_pos];
	Script_Tag_pos ++;
	if (tag->Type_1 == 0x02)
	{
		tag->StringLength = 
			Buf[Script_Tag_pos]   << 8 |
			Buf[Script_Tag_pos+1];
		Script_Tag_pos +=2;

		Script_Tag_pos +=tag->StringLength;
	}
	
	tag->Type_1 = Buf[Script_Tag_pos];
	Script_Tag_pos ++;
	if (tag->Type_1 == 0x08)
	{
		tag->ECMAArrayLength =                   
			Buf[Script_Tag_pos]     << 24 |
			Buf[Script_Tag_pos+1]   << 16 |
			Buf[Script_Tag_pos+2]   << 8  |
			Buf[Script_Tag_pos+3];
		Script_Tag_pos += 4;
	}

	for (size_t i = 0 ; i< tag->ECMAArrayLength ; i++)  
	{		
	    if (Buf[Script_Tag_pos]  == 0x00 && Buf[Script_Tag_pos + 1]  == 0x00
			&& Buf[Script_Tag_pos + 2]  == 0x00 && Buf[Script_Tag_pos + 3]  == 0x09)
		{
			break;
		}
		
loop:	Arry_byte_length = 
			Buf[Script_Tag_pos]   << 8  |
			Buf[Script_Tag_pos+1];
		Script_Tag_pos +=2;

		memcpy(Arry_Name,Buf + Script_Tag_pos , Arry_byte_length);  
		Script_Tag_pos += Arry_byte_length;

		Arry_InFomation = Buf[Script_Tag_pos];                      
		Script_Tag_pos ++;		

		if (strstr((char *)Arry_Name,"duration") != NULL)           
		{
			//Arry_InFomation == 0
			tag->duration= char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"width") != NULL)
		{
			//Arry_InFomation == 0;
			tag->width= char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"height") != NULL)
		{
			//Arry_InFomation == 0;
			tag->height = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"videodatarate") != NULL)
		{
			//Arry_InFomation == 0;
			tag->videodatarate = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"framerate") != NULL)
		{
			//Arry_InFomation == 0;
			tag->framerate = char2double(&Buf[Script_Tag_pos],8);	
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"videocodecid") != NULL)
		{
			//Arry_InFomation == 0;
			tag->videocodecid = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"audiosamplerate") != NULL)
		{
			//Arry_InFomation == 0;
			tag->audiosamplerate = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"audiodatarate") != NULL)
		{
			//Arry_InFomation == 0;
			tag->audiodatarate = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"audiosamplesize") != NULL)
		{
			//Arry_InFomation == 0;
			tag->audiosamplesize = char2double(&Buf[Script_Tag_pos ],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"stereo") != NULL)
		{
			//Arry_InFomation == 1;
			tag->stereo = Buf[Script_Tag_pos];
			Script_Tag_pos ++;
		}
		else if (strstr((char *)Arry_Name,"audiocodecid") != NULL)
		{
			//Arry_InFomation == 0;
			tag->audiocodecid = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"filesize") != NULL)
		{
			//Arry_InFomation == 0;
			tag->filesize = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"lasttime") != NULL)
		{
			//Arry_InFomation == 0;
			tag->lasttimetamp = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if (strstr((char *)Arry_Name,"lastkeyframetime") != NULL)
		{
			//Arry_InFomation == 0;
			tag->lastkeyframetimetamp = char2double(&Buf[Script_Tag_pos],8);
			Script_Tag_pos += 8;
		}
		else if ((strstr((char *)Arry_Name,"keyframe") != NULL) && Arry_InFomation == 0x03)   
		{
			//Arry_InFomation == 0x03; 
			goto loop;
		}
		else if ((strstr((char *)Arry_Name,"filepositions") != NULL)&& Arry_InFomation == 0x0A)
		{
			//Arry_InFomation == 0x0A; 		
			Arry_Name_framekey_Arry_length = 
				Buf[Script_Tag_pos]      << 24 |
				Buf[Script_Tag_pos + 1]  << 16 |
				Buf[Script_Tag_pos + 2]  << 8  |
				Buf[Script_Tag_pos + 3];
			Script_Tag_pos += 4;			
			for (size_t k = 0 ; k < Arry_Name_framekey_Arry_length ; k ++ )
			{
				Arry_InFomation_framekey =    Buf[Script_Tag_pos];               
				//Arry_InFomation_framekey == 0x00;
				Script_Tag_pos ++;
				tag->filepositions[i]= char2double(&Buf[Script_Tag_pos],8);      
				Script_Tag_pos += 8;
			}		
			i --;
		}
		else if ((strstr((char *)Arry_Name,"times") != NULL) && Arry_InFomation == 0x0A)
		{
			//Arry_InFomation == 0x0A; 			
			Arry_Name_framekey_Arry_length = 
				Buf[Script_Tag_pos]      << 24 |
				Buf[Script_Tag_pos + 1]  << 16 |
				Buf[Script_Tag_pos + 2]  << 8  |
				Buf[Script_Tag_pos + 3];
			Script_Tag_pos += 4;			
			for (size_t k = 0 ; k < Arry_Name_framekey_Arry_length ; k ++ )
			{
				Arry_InFomation_framekey = Buf[Script_Tag_pos];          
				//Arry_InFomation_framekey == 0x00;
				Script_Tag_pos ++;
				tag->times[i]= char2double(&Buf[Script_Tag_pos],8);     
				Script_Tag_pos += 8;
			}			
			i --;
		}
		else
		{			
			switch (Arry_InFomation)
			{
			case 0x00:
				Script_Tag_pos += 8;
				break;
			case 0x01:
				Script_Tag_pos ++;
				break;
			case 0x02:
				Script_Tag_pos += 
					Buf[Script_Tag_pos]  << 8 |
					Buf[Script_Tag_pos+1];
				Script_Tag_pos +=2;
				break;
			case 0x03:
				goto loop;
				break;
			case 0x04:				
				break;
			case 0x07:
				Script_Tag_pos += 2;
				break;
			case 0x08:				
				break;
			case 0x0A:
				Script_Tag_pos += 4;
				break;
			case 0x0B:
				Script_Tag_pos += 10;
				break;
			case 0x0C:				
				break;
			default:              
				printf("Arry_InFomation\n");
				break;
			}
		}
	}
	memcpy(tag->Data,Buf + Script_Tag_pos,length - Script_Tag_pos );

	return 1;
}
