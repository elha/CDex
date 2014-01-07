#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "machine.h"


#define MPEGFRAMESIZE 1152


enum byte_order { order_unknown, order_bigEndian, order_littleEndian };


#define PROTO_ARGS

#ifndef EXIT_SUCCESS
	#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
	#define EXIT_FAILURE 1
#endif

/***********************************************************************
*
*  Global Definitions
*
***********************************************************************/

#ifndef FALSE
	#define         FALSE                   0
#endif

#ifndef TRUE
	#define         TRUE                    1
#endif

#define         NULL_CHAR               '\0'

#define         MAX_U_32_NUM            0xFFFFFFFF
#ifndef PI
	#define         PI                      3.14159265358979
#endif
#define         PI4                     PI/4
#define         PI64                    PI/64
#define         LN_TO_LOG10             0.2302585093

#define         VOL_REF_NUM             0
#define         MPEG_AUDIO_ID           1
#define			MPEG_PHASE2_LSF			0		// Low Sample frequencies MPEG-2
#define         MAC_WINDOW_SIZE         24

#define         MONO                    1
#define         STEREO                  2
#define         BITS_IN_A_BYTE          8
//#define         WORD                    16
#define         MAX_NAME_SIZE           81
#define         SBLIMIT                 32
#define         SSLIMIT                 18
#define         FFT_SIZE                1024
#define         HAN_SIZE                512
#define         SCALE_BLOCK             12
#define         SCALE_RANGE             64
#define         SCALE                   32768
#define         CRC16_POLYNOMIAL        0x8005

// MPEG Header Definitions - Mode Values
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

// Mode Extention
#define         MPG_MD_LR_LR             0
#define         MPG_MD_LR_I              1
#define         MPG_MD_MS_LR             2
#define         MPG_MD_MS_I              3


// "bit_stream.h" Definitions
#define         MINIMUM         4    // Minimum size of the buffer in bytes
#define         MAX_LENGTH      32   // Maximum length of word written or read from bit stream
                                        
#define         READ_MODE       0
#define         WRITE_MODE      1
#define         ALIGNING        8
#define         BINARY          0
#define         ASCII           1

#ifndef BS_FORMAT
#define         BS_FORMAT       BINARY	// BINARY or ASCII = 2x bytes
#endif

//#define         BUFFER_SIZE     8192
#define         BUFFER_SIZE     16000

#define         MIN(A, B)       ((A) < (B) ? (A) : (B))
#define         MAX(A, B)       ((A) > (B) ? (A) : (B))


// Psychoacoustic Model 2 Definitions
#define LOGBLKSIZE      10
#define BLKSIZE         1024
#define HBLKSIZE        513
#define CBANDS          63
#define LXMIN           32.0


/***********************************************************************
*
*  Global Type Definitions
*
***********************************************************************/


// Header Information Structure
typedef struct HEADER_TAG
{
    int version;
    int lay;
    int bCRC;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int bPrivate;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
} HEADER, *PHEADER;

// Parent Structure Interpreting some Frame Parameters in Header
// "bit_stream.h" Type Definitions

typedef struct  BITSTREAM_TAG
{
    BYTE*		buf;			// bit stream buffer
    long        totbit;         // bit counter of bit stream
    int         buf_byte_idx;   // pointer to top byte in buffer
    int         buf_bit_idx;    // pointer to top bit of top byte in buffer
} BITSTREAM, *PBITSTREAM;


typedef struct {
	unsigned int    steps;
	unsigned int    bits;
	unsigned int    group;
	unsigned int    quant;
} sb_alloc, *alloc_ptr;

typedef sb_alloc		ALLOC_16[16];
typedef sb_alloc        al_table[SBLIMIT][16];


typedef struct FRAME_TAG
{
    PHEADER		pHeader;        // raw header information
    int         actual_mode;    // when writing IS, may forget if 0 chs
    al_table*	alloc;			// bit allocation table read in
    int         tab_num;		// number of table as loaded
    int         nChannels;		// 1 for mono, 2 for stereo
    int         jsbound;        // first band of joint stereo coding
    int         sblimit;        // total number of sub bands
} FRAME, *PFRAME;


// Structure for Reading Layer II Allocation Tables from File
typedef struct 
{
		int        dummy;
		int        line;
		double     bark, hear, x;
} g_thres, *g_ptr;


typedef struct ALLOCTABLE_TAG
{
	int				i,j;
	sb_alloc		alloc;
} ALLOCTABLE;

#endif
