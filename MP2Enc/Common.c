/*
** Copyright (C) 2000 Albert L. Faber
** 
** Floating-Point processeing was added by DSPguru on March 2002.
** Modified Source-code can be found at http://DSPguru.doom9.net
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Leeser General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include    "common.h"
#include    "tables.h"

//#undef _BLADEDLL

// Local function prototypes
static int GetAllocTable(int nTable,al_table** pAllocTable);



/***********************************************************************
*
*  Global Variable Definitions
*
***********************************************************************/

char *mode_names[4] = { "stereo", "j-stereo", "dual-ch", "single-ch" };
char *HEADER_names[3] = { "I", "II", "III" };
char *version_names[2] = { "MPEG-2 LSF", "MPEG-1" };

// 1: MPEG-1, 0: MPEG-2 LSF
double  s_freq[2][4] =
{	
	{22.05, 24, 16, 0},		// MPEG-2 LSF
	{44.1, 48, 32, 0}		// MPEG-1
};

// 1: MPEG-1, 0: MPEG-2 LSF
int     bitrate[2][3][15] = 
{
	{	// MPEG-2 LSF
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256},		// Layer 1
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160},			// Layer 2
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160}			// Layer 3
	},
	{
		// MPEG-1
		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},	// Layer 1
		{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},		// Layer 2
		{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}		// Layer 3
	}
};

// TABLE 3-B.1. Layer I,II scalefactors
double multiple[64] =
{
	2.00000000000000, 1.58740105196820, 1.25992104989487,
	1.00000000000000, 0.79370052598410, 0.62996052494744, 0.50000000000000,
	0.39685026299205, 0.31498026247372, 0.25000000000000, 0.19842513149602,
	0.15749013123686, 0.12500000000000, 0.09921256574801, 0.07874506561843,
	0.06250000000000, 0.04960628287401, 0.03937253280921, 0.03125000000000,
	0.02480314143700, 0.01968626640461, 0.01562500000000, 0.01240157071850,
	0.00984313320230, 0.00781250000000, 0.00620078535925, 0.00492156660115,
	0.00390625000000, 0.00310039267963, 0.00246078330058, 0.00195312500000,
	0.00155019633981, 0.00123039165029, 0.00097656250000, 0.00077509816991,
	0.00061519582514, 0.00048828125000, 0.00038754908495, 0.00030759791257,
	0.00024414062500, 0.00019377454248, 0.00015379895629, 0.00012207031250,
	0.00009688727124, 0.00007689947814, 0.00006103515625, 0.00004844363562,
	0.00003844973907, 0.00003051757813, 0.00002422181781, 0.00001922486954,
	0.00001525878906, 0.00001211090890, 0.00000961243477, 0.00000762939453,
	0.00000605545445, 0.00000480621738, 0.00000381469727, 0.00000302772723,
	0.00000240310869, 0.00000190734863, 0.00000151386361, 0.00000120155435,
	1E-20
};


/***********************************************************************
*
*  Global Function Definitions
*
***********************************************************************/

int js_bound(int lay,int m_ext)
{
	static int jsb_table[3][4] ={	{ 4, 8, 12, 16 }, 
									{ 4, 8, 12, 16},
									{ 0, 4, 8, 16}
								};  // lay+m_e -> jsbound

    if(lay<1 || lay >3 || m_ext<0 || m_ext>3) 
	{
		MP2LibError("js_bound bad HEADER/modext (%d/%d)\n", lay, m_ext);
    }
    return(jsb_table[lay-1][m_ext]);
}

// interpret data in hdr str to fields in pFrame
void hdr_to_frps(PFRAME pFrame)
{
	PHEADER pHeader = pFrame->pHeader;     // (or pass in as arg?)

    pFrame->actual_mode = pHeader->mode;
    pFrame->nChannels = (pHeader->mode == MPG_MD_MONO) ? 1 : 2;

    if (pHeader->lay == 2)
	{
		pFrame->sblimit = pick_table(pFrame);
	}

    if(pHeader->mode == MPG_MD_JOINT_STEREO)
        pFrame->jsbound = js_bound(pHeader->lay, pHeader->mode_ext);
    else
        pFrame->jsbound = pFrame->sblimit;
    // alloc, tab_num set in pick_table
}

void WriteHdr(PFRAME pFrame,FILE *s)
{
	PHEADER pHeader = pFrame->pHeader;

	fprintf(s, "HDR: s=FFF, id=%X, l=%X, ep=%s, br=%X, sf=%X, pd=%X, ",
		   pHeader->version, pHeader->lay, ((pHeader->bCRC) ? "on" : "off"),
		   pHeader->bitrate_index, pHeader->sampling_frequency, pHeader->padding);
	fprintf(s, "pr=%X, m=%X, js=%X, c=%X, o=%X, e=%X\n",
		   pHeader->bPrivate, pHeader->mode, pHeader->mode_ext,
		   pHeader->copyright, pHeader->original, pHeader->emphasis);
	fprintf(s, "alg.=%s, HEADER=%s, tot bitrate=%d, sfrq=%.1f\n",
		   version_names[pHeader->version], HEADER_names[pHeader->lay-1],
	   bitrate[pHeader->version][pHeader->lay-1][pHeader->bitrate_index],
		   s_freq[pHeader->version][pHeader->sampling_frequency]);
	fprintf(s, "mode=%s, sblim=%d, jsbd=%d, ch=%d\n",
		   mode_names[pHeader->mode], pFrame->sblimit, pFrame->jsbound, pFrame->nChannels);
   fflush(s);
}

void WriteBitAlloc(UINT bit_alloc[2][SBLIMIT],FRAME *f_p,FILE *s)
{
	int i,j;
	int st = f_p->nChannels;
	int sbl = f_p->sblimit;
	int jsb = f_p->jsbound;

	fprintf(s, "BITA ");
	for(i=0; i<sbl; ++i)
	{
		if(i == jsb) fprintf(s,"-");
			for(j=0; j<st; ++j)
				fprintf(s, "%1x", bit_alloc[j][i]);
	}
	fprintf(s, "\n");
	fflush(s);
}

void WriteScale(UINT bit_alloc[2][SBLIMIT],
				UINT scfsi[2][SBLIMIT],
				UINT scalar[2][3][SBLIMIT],
				FRAME* pFrame,
				FILE*	s)
{
	int stereo  = pFrame->nChannels;
	int sblimit = pFrame->sblimit;
	int lay     = pFrame->pHeader->lay;
	int i,j,k;

	if(lay == 2)
	{
		fprintf(s, "SFSI ");
		for (i=0;i<sblimit;i++)
			for (k=0;k<stereo;k++)
				if (bit_alloc[k][i])
					fprintf(s,"%d",scfsi[k][i]);
		fprintf(s, "\nSCFs ");
		for (k=0;k<stereo;k++)
		{
			for (i=0;i<sblimit;i++)
				if (bit_alloc[k][i])
					switch (scfsi[k][i])
					{
						case 0:
								for (j=0;j<3;j++)
								fprintf(s,"%2d%c",scalar[k][j][i],(j==2)?';':'-');
						break;
						case 1:
						case 3:
								fprintf(s,"%2d-",scalar[k][0][i]);
								fprintf(s,"%2d;",scalar[k][2][i]);
						break;
						case 2:
								fprintf(s,"%2d;",scalar[k][0][i]);
					}
			fprintf(s, "\n");
		}
	}
	else
	{ 
		// lay == 1
		fprintf(s, "SCFs ");
		for (i=0;i<sblimit;i++)
			for (k=0;k<stereo;k++)
				if (bit_alloc[k][i])
					fprintf(s,"%2d;",scalar[k][0][i]);
		fprintf(s, "\n");
	}
}

void WriteSamples(	int ch,
					UINT sample[SBLIMIT],
					UINT bit_alloc[SBLIMIT],
					FRAME* pFrame,
					FILE *s
				)
{
	int i;
	int stereo = pFrame->nChannels;
	int sblimit = pFrame->sblimit;

	fprintf(s, "SMPL ");

	for (i=0;i<sblimit;i++)
		if ( bit_alloc[i] != 0)
			fprintf(s, "%d:", sample[i]);
	if(ch==(stereo-1) )
		fprintf(s, "\n");
	else
		fprintf(s, "\t");
}

// convert bitrate in kbps to index
int BitrateIndex(int layr,int bRate,int version)
{
	int     index = 0;
	int     found = 0;

	while(!found && index<15)
	{
		if(bitrate[version][layr-1][index] == bRate)
			found = 1;
		else
			++index;
	}
	if(found)
		return(index);
	else
	{
		fprintf(stderr, "BitrateIndex: %d (HEADER %d) is not a legal bitrate\n",bRate, layr);
		// Error!
		return(-1);
	}
}

// convert samp frq in Hz to index
// legal rates 16000, 22050, 24000, 32000, 44100, 48000
int SmpFrqIndex(LONG sRate,INT* version)  
{
	if (sRate == 44100L)
	{
		*version = MPEG_AUDIO_ID;
		return(0);
	}
	else if (sRate == 48000L)
	{
		*version = MPEG_AUDIO_ID;
		return(1);
	}
	else if (sRate == 32000L)
	{
		*version = MPEG_AUDIO_ID;
		return(2);
	}
	else if (sRate == 24000L)
	{
		*version = MPEG_PHASE2_LSF;
		return(1);
	}
	else if (sRate == 22050L)
	{
		*version = MPEG_PHASE2_LSF;
		return(0);
	}
	else if (sRate == 16000L)
	{
		*version = MPEG_PHASE2_LSF;
		return(2);
	}
	else
	{
		// Error!
		MP2LibError("SmpFrqIndex: %ld is not a legal sample rate\n", sRate);
		return(-1);
	}
}

/*******************************************************************************
*
*  Allocate number of bytes of memory equal to "block".
*
*******************************************************************************/

void  *mem_alloc(ULONG block,char* item)
{
    void    *ptr;
//	int* pNull=NULL;
//	throw ;
//	*pNull=676;
//	float f1=0.0F;
//	float f2=0.0F;
//	float f3;
//	f3=f1/f2;

//    ptr = (PVOID) malloc(block<<1); /* allocate twice as much memory as needed. fixes dodgy
//					    memory problem on most systems */

	// Allocate memory
    ptr = (PVOID) malloc(block<<1);

	// Clear memory
    if (ptr != NULL)
	{
        memset(ptr, 0, block);
    }
    else
	{
        MP2LibError("Unable to allocate %s\n", item);
    }
    return(ptr);
}


/****************************************************************************
*
*  Free memory pointed to by "*ptr_addr".
*
*****************************************************************************/

void mem_free(void** ptr_addr)
{

    if (*ptr_addr != NULL)
	{
        free(*ptr_addr);
        *ptr_addr = NULL;
    }

}


/*******************************************************************************
*
*  Check block of memory all equal to a single byte, else return FALSE
*
*******************************************************************************/

// but only tested as a char (bottom 8 bits)
int memcheck(char *array,int test,int num)
{
	int i=0;

	while (array[i] == test && i<num) 
		i++;

	if (i==num)
		return TRUE;
	else 
		return FALSE;
}

static char *he = "0123456789ABCDEF";

int putmask[9]={0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};


// Write 1 bit from the bit stream 
// bs=bit stream structure
// bit=bit to write into the buffer
void put1bit(
				BITSTREAM *bs,
				int bit
			)
{
	// Bump up the total number of bits
	bs->totbit++;

	// Decrement the bit index position by one
	bs->buf_bit_idx--;

	// Shift in the bit a the correct position
	bs->buf[bs->buf_byte_idx] |= (bit&0x1) << (bs->buf_bit_idx);

	// are there any bits left in this byte ?
	if (!bs->buf_bit_idx)
	{
		// No, start over again with a new bytes
		bs->buf_bit_idx = 8;

		// increment the byte index
		bs->buf_byte_idx++;

		// clear all bits for this byte index
		bs->buf[bs->buf_byte_idx] = 0;
	}
}

// write N bits into the bit stream
// bit stream structure
// val to write into the buffer
// number of bits of val
void putbits(BITSTREAM *bs,UINT val,int N)
{
	register int j = N;
	register int k, tmp;

	if (N > MAX_LENGTH)
		printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);
	
	bs->totbit += N;
	while (j > 0)
	{
		k = MIN(j, bs->buf_bit_idx);

		tmp = val >> (j-k);

		bs->buf_bit_idx -= k;

		bs->buf[bs->buf_byte_idx] |= (tmp&putmask[k]) << (bs->buf_bit_idx);

		if (!bs->buf_bit_idx)
		{
			bs->buf_bit_idx = 8;
			bs->buf_byte_idx++;
			bs->buf[bs->buf_byte_idx] = 0;
		}
		j -= k;
	}
}



/*****************************************************************************
*
*  CRC error protection package
*
*****************************************************************************/

void update_CRC(UINT data,UINT length,UINT* crc)
{
	UINT  masking, carry;

	masking = 1 << length;

	while((masking >>= 1))
	{
		carry = *crc & 0x8000;
		*crc <<= 1;
		if (!carry ^ !(data & masking))
			*crc ^= CRC16_POLYNOMIAL;
	}
	*crc &= 0xffff;
}

/*****************************************************************************
*
*  End of CRC error protection package
*
*****************************************************************************/



// **********************************************************************
//
// Read one of the data files ("alloc_*") specifying the bit allocation/
// quatization parameters for each subband in HEADER II encoding
// read in table, return # subbands 
// **********************************************************************

al_table AllocTables[5];
int AllocSbLimits[5];



void InitBitAllocTables()
{
	int nSize=sizeof(AllocTables);
	int nTable=0;

	// Clear all the tables
	memset(AllocTables,0,sizeof(AllocTables));

	for (nTable=0;nTable<5;nTable++)
	{
		ALLOCTABLE * myAlloc;
		int			iIndex=0;
		switch (nTable) 
		{
			case 0: myAlloc=table_ALLOC0;break;
			case 1:	myAlloc=table_ALLOC1;break;
			case 2:	myAlloc=table_ALLOC2;break;
			case 3:	myAlloc=table_ALLOC3;break;
			case 4:	myAlloc=table_ALLOC4;break;
			default: assert(0);
		}

		while (myAlloc[iIndex].i!=-1)
		{
			AllocTables[nTable][myAlloc[iIndex].i][myAlloc[iIndex].j] = myAlloc[iIndex].alloc;
			iIndex++;
		}
		AllocSbLimits[nTable]=myAlloc[iIndex].alloc.steps;

	}
	
}

static int GetAllocTable(int nTable,al_table** pAllocTable)
{
	*pAllocTable=&AllocTables[nTable];
	return  AllocSbLimits[nTable];
}



/***********************************************************************
*
* Using the decoded info the appropriate possible quantization per
* subband table is loaded
*
**********************************************************************/

// choose table, load if necess, return # sb's
int pick_table(FRAME *pFrame)
{
	int table, lay, ws, bsp, br_per_ch, sfrq;

	// return current value if no load
	int sblim = pFrame->sblimit;

	lay = pFrame->pHeader->lay - 1;
	bsp = pFrame->pHeader->bitrate_index;
	br_per_ch = bitrate[pFrame->pHeader->version][lay][bsp] / pFrame->nChannels;
	ws = pFrame->pHeader->sampling_frequency;
	sfrq = (int)s_freq[pFrame->pHeader->version][ws];

	// decision rules refer to per-channel bitrates (kbits/sec/chan)
	if (pFrame->pHeader->version == MPEG_AUDIO_ID)
	{ 
		// MPEG-1
		if ((sfrq == 48 && br_per_ch >= 56) || (br_per_ch >= 56 && br_per_ch <= 80))
			table = 0;
		else if (sfrq != 48 && br_per_ch >= 96) 
				table = 1;
		else if (sfrq != 32 && br_per_ch <= 48)
				table = 2;
		else table = 3;
	}
	else 
	{ 
		// MPEG-2 LSF
		table = 4;
	}
	
	if (pFrame->tab_num != table)
	{
		pFrame->tab_num=table;
		sblim = GetAllocTable(table,&pFrame->alloc);

	}
	return sblim;
}

void II_CRC_calc(
					FRAME *pFrame,
					UINT bit_alloc[2][SBLIMIT],
					UINT scfsi[2][SBLIMIT],
					UINT *crc
				)
{
        int i, k;
        HEADER *info = pFrame->pHeader;
        int stereo  = pFrame->nChannels;
        int sblimit = pFrame->sblimit;
        int jsbound = pFrame->jsbound;
        al_table *alloc = pFrame->alloc;

        *crc = 0xffff; /* changed from '0' 92-08-11 shn */
        update_CRC(info->bitrate_index, 4, crc);
        update_CRC(info->sampling_frequency, 2, crc);
        update_CRC(info->padding, 1, crc);
        update_CRC(info->bPrivate, 1, crc);
        update_CRC(info->mode, 2, crc);
        update_CRC(info->mode_ext, 2, crc);
        update_CRC(info->copyright, 1, crc);
        update_CRC(info->original, 1, crc);
        update_CRC(info->emphasis, 2, crc);

        for (i=0;i<sblimit;i++)
                for (k=0;k<((i<jsbound)?stereo:1);k++)
                        update_CRC(bit_alloc[k][i], (*alloc)[i][0].bits, crc);

        for (i=0;i<sblimit;i++)
                for (k=0;k<stereo;k++)
                        if (bit_alloc[k][i])
                                update_CRC(scfsi[k][i], 2, crc);
}


