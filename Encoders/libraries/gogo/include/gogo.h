/* -*- TABSIZE = 4 -*- */
/*
 *	for new GOGO-no-coda (1999/09,10)
 *	Copyright (C) 1999 PEN@MarineCat
 */
#ifndef __GOGO_H__
#define __GOGO_H__

#include <limits.h>

typedef	signed int				MERET;
typedef	unsigned long				MPARAM;
typedef	unsigned long				UPARAM;

#ifdef GOGO_DLL_EXPORTS
#define		EXPORT				__declspec(dllexport) 
#else
#define		EXPORT					
#endif


#define	ME_NOERR				(0)	// return normally;
#define	ME_EMPTYSTREAM				(1)	// stream becomes empty;
#define	ME_HALTED				(2)	// stopped by user;
#define	ME_INTERNALERROR			(10)	// internal error;
#define	ME_PARAMERROR				(11)	// parameters error;
#define	ME_NOFPU				(12)	// no FPU;FPU
#define	ME_INFILE_NOFOUND			(13)	// can't open input file;
#define	ME_OUTFILE_NOFOUND			(14)	// can't open output file;
#define	ME_FREQERROR				(15)	// frequency is not good;
#define	ME_BITRATEERROR				(16)	// bitrate is not good;
#define	ME_WAVETYPE_ERR				(17)	// WAV format is not good;
#define	ME_CANNOT_SEEK				(18)	// can't seek;
#define	ME_BITRATE_ERR				(19)	// only for compatibility;
#define	ME_BADMODEORLAYER			(20)	// mode/layer not good;
#define	ME_NOMEMORY				(21)	// fail to allocate memory;
#define	ME_CANNOT_SET_SCOPE			(22)	// thread error;
#define	ME_CANNOT_CREATE_THREAD			(23)	// fail to create thread;
#define	ME_WRITEERROR				(24)	// lock of capacity of disk;


// definition of call-back function for user;
typedef	MERET	(*MPGE_USERFUNC)(void *buf, unsigned long nLength );
#define MPGE_NULL_FUNC (MPGE_USERFUNC)NULL	// for HighC

///////////////////////////////////////////////////////////////////////////
// Configuration
///////////////////////////////////////////////////////////////////////////
// for INPUT
#define		MC_INPUTFILE			(1)
// para1 choice of input device
	#define		MC_INPDEV_FILE		(0)		// input device is file;
	#define		MC_INPDEV_STDIO		(1)		//                 stdin;
	#define		MC_INPDEV_USERFUNC	(2)		//       defined by user;
	// para2
	struct MCP_INPDEV_USERFUNC {
		MPGE_USERFUNC	pUserFunc;	// pointer to user-function for call-back or MPGE_NULL_FUNC if none
		unsigned int	nSize;		// size of file or MC_INPDEV_MEMORY_NOSIZE if unknown
		int				nBit;	// nBit = 8 or 16 ;
		int				nFreq;	// input frequency ;
		int				nChn;	// number of channel(1 or 2) ;
	};
	#define		MC_INPDEV_MEMORY_NOSIZE		(UINT_MAX)

///////////////////////////////////////////////////////////////////////////
// for OUTPUT ( now stdout is not support )
#define		MC_OUTPUTFILE			(2)
// para1 choice of output device
	#define		MC_OUTDEV_FILE		(0)		// output device is file;
	#define		MC_OUTDEV_STDOUT	(1)		//                  stdout;
	#define		MC_OUTDEV_USERFUNC	(2)		//        defined by user;
// para2 pointer to file if necessary ;

///////////////////////////////////////////////////////////////////////////
// mode of encoding ;
#define		MC_ENCODEMODE			(3)
// para1 mode;
	#define		MC_MODE_MONO		(0)		// mono;
	#define		MC_MODE_STEREO		(1)		// stereo;
	#define		MC_MODE_JOINT		(2)		// joint-stereo;
	#define		MC_MODE_MSSTEREO	(3)		// mid/side stereo;
	#define		MC_MODE_DUALCHANNEL	(4)		// dual channel;

///////////////////////////////////////////////////////////////////////////
// bitrate;
#define		MC_BITRATE				(4)
// para1 bitrate;


///////////////////////////////////////////////////////////////////////////
// frequency of input file (force);
#define		MC_INPFREQ				(5)
// para1 frequency;

///////////////////////////////////////////////////////////////////////////
// frequency of output mp3 (force);
#define		MC_OUTFREQ				(6)
// para1 frequency;

///////////////////////////////////////////////////////////////////////////
// size ofheader if you ignore WAV-header (for example cda);
#define		MC_STARTOFFSET			(7)

///////////////////////////////////////////////////////////////////////////
// psycho-acoustics ON/OFF;
#define		MC_USEPSY				(8)
// PARA1 boolean(TRUE/FALSE)

///////////////////////////////////////////////////////////////////////////
// 16kHz low-pass filter ON/OFF;
#define		MC_USELPF16				(9)
// PARA1 boolean(TRUE/FALSE)

///////////////////////////////////////////////////////////////////////////
// use special UNIT, para1:boolean;
#define		MC_USEMMX				(10)	// MMX
#define		MC_USE3DNOW				(11)	// 3DNow!
#define		MC_USEKNI				(12)	// SSE(KNI)
#define		MC_USEE3DNOW				(13)	// Enhanced 3D Now!
#define		MC_USESPC1				(14)	// special switch for debug
#define		MC_USESPC2				(15)	// special switch for debug

///////////////////////////////////////////////////////////////////////////
// addition of TAG;
#define		MC_ADDTAG				(16)
// dwPara1  length of TAG
// dwPara2  pointer to TAG

///////////////////////////////////////////////////////////////////////////
// emphasis;
#define		MC_EMPHASIS				(17)	
// para1 type of emphasis;
	#define		MC_EMP_NONE		(0)		// no empahsis;
	#define		MC_EMP_5015MS		(1)		// 50/15ms    ;
	#define		MC_EMP_CCITT		(3)		// CCITT      ;

///////////////////////////////////////////////////////////////////////////
// use VBR;VBR
#define		MC_VBR				(18)

///////////////////////////////////////////////////////////////////////////
// SMP support para1: interger
#define		MC_CPU				(19)

///////////////////////////////////////////////////////////////////////////
// for RAW-PCM;
// byte swapping for 16bitPCM;
#define		MC_BYTE_SWAP			(20)

///////////////////////////////////////////////////////////////////////////
// for 8bit PCM
#define		MC_8BIT_PCM			(21)

///////////////////////////////////////////////////////////////////////////
// for mono PCM
#define		MC_MONO_PCM			(22)

///////////////////////////////////////////////////////////////////////////
// for Towns SND
#define		MC_TOWNS_SND			(23)

///////////////////////////////////////////////////////////////////////////
// BeOS Encode thread priority
#define		MC_THREAD_PRIORITY		(24)

///////////////////////////////////////////////////////////////////////////
// BeOS Read thread priority
//#if	defined(USE_BTHREAD)
#define		MC_READTHREAD_PRIORITY		(25)
//#endif

MERET	EXPORT	MPGE_initializeWork();
MERET	EXPORT	MPGE_setConfigure(MPARAM mode, UPARAM dwPara1, UPARAM dwPara2 );
MERET	EXPORT	MPGE_getConfigure(MPARAM mode, void *para1 );
MERET	EXPORT	MPGE_detectConfigure();
#ifdef USE_BETHREAD
MERET	EXPORT	MPGE_processFrame(int *frameNum);
#else
MERET	EXPORT	MPGE_processFrame();
#endif
MERET	EXPORT	MPGE_closeCoder();
MERET	EXPORT	MPGE_endCoder();
MERET	EXPORT	MPGE_getUnitStates( unsigned long *unit );

// This function is effective for gogo.dll;
MERET	EXPORT	MPGE_getVersion( unsigned long *vercode,  char *verstring );
// vercode = 0x125 ->  version 1.25
// verstring       ->  "ver 1.25 1999/09/25" (allocate abobe 260bytes buffer)



////////////////////////////////////////////////////////////////////////////
// for getting configuration
////////////////////////////////////////////////////////////////////////////

#define		MG_INPUTFILE			(1)	// name of input file ;
#define		MG_OUTPUTFILE			(2)	// name of output file;
#define		MG_ENCODEMODE			(3)	// type of encoding   ;
#define		MG_BITRATE			(4)	// bitrate            ;
#define		MG_INPFREQ			(5)	// input frequency    ;
#define		MG_OUTFREQ			(6)	// output frequency   ;
#define		MG_STARTOFFSET			(7)	// offset of input PCM;
#define		MG_USEPSY			(8)	// psycho-acoustics   ;
#define		MG_USEMMX			(9)	// MMX
#define		MG_USE3DNOW			(10)	// 3DNow!
#define		MG_USEKNI			(11)	// SSE(KNI)
#define		MG_USEE3DNOW			(12)	// Enhanced 3DNow!

#define		MG_USESPC1			(13)	// special switch for debug
#define		MG_USESPC2			(14)	// special switch for debug
#define		MG_COUNT_FRAME			(15)	// amount of frame
#define		MG_NUM_OF_SAMPLES		(16)	// number of sample for 1 frame;
#define		MG_MPEG_VERSION			(17)	// MPEG VERSION
#define		MG_READTHREAD_PRIORITY		(18)	// thread priority to read for BeOS

#endif /* __MUSUI_H__ */
