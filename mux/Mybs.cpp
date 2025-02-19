#include "../inc/mux/Mybs.h"

void bs_init( bs_t *s, void *p_data, int i_data )
{
	s->p_start = (unsigned char *)p_data;		
	s->p       = (unsigned char *)p_data;		
	s->p_end   = s->p + i_data;	                
	s->i_left  = 8;				                
}


int bs_read( bs_t *s, int i_count )
{
	static int i_mask[33] ={0x00,
		0x01,      0x03,      0x07,      0x0f,
		0x1f,      0x3f,      0x7f,      0xff,
		0x1ff,     0x3ff,     0x7ff,     0xfff,
		0x1fff,    0x3fff,    0x7fff,    0xffff,
		0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
		0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
		0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
		0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};

	int      i_shr;			   
	int i_result = 0;	        
	while( i_count > 0 )	   
	{
		if( s->p >= s->p_end )	
		{						
			break;
		}

		if( ( i_shr = s->i_left - i_count ) >= 0 )
		{					
			i_result |= ( *s->p >> i_shr )&i_mask[i_count];

			s->i_left -= i_count; 
			if( s->i_left == 0 )	
			{
				s->p++;				
				s->i_left = 8;		
			}
			return( i_result );		
		}
		else	
		{			
			i_result |= (*s->p&i_mask[s->i_left]) << -i_shr;

			i_count  -= s->i_left;	
			s->p++;	
			s->i_left = 8;	
		}
	}

	return( i_result );
}

int bs_read1( bs_t *s )
{
	if( s->p < s->p_end )	
	{
		unsigned int i_result;

		s->i_left--;                           
		i_result = ( *s->p >> s->i_left )&0x01;
		if( s->i_left == 0 )                  
		{
			s->p++;			                   
			s->i_left = 8;                     
		}
		return i_result;                       
	}

	return 0;                                  
}

int bs_read_ue( bs_t *s )
{
	int i = 0;
	while( bs_read1( s ) == 0 && s->p < s->p_end && i < 32 )	
	{
		i++;
	}

	return( ( 1 << i) - 1 + bs_read( s, i ) );	
}

