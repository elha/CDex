#include <stdio.h>
#include <stdlib.h>

#include "machine.h"
#include "MP2Enc.h"

#include "TimeEval.h"
#include "common.h"
#include "tables.h"
#include "sndfile.h"
#include "subband.h"

#define MP2_ENC_VERSION "0.10"


const int DFLT_SFQ=44100;
const int DFLT_MOD=MP2ENC_MD_STEREO;
const int DFLT_PSY=2;
const int DFLT_BRT=192;
#define DFLT_EXT ".MP2"


/************************************************************************
*
* usage
*
* PURPOSE:  Writes command line syntax to the file specified by #stderr#
*
************************************************************************/

void usage(void)  /* print syntax & exit */
{
  /* FIXME: maybe have an option to display better definitions of help codes, and
            long equivalents of the flags */
  fprintf(stdout,"\nMP2Enc version %s (http://cdexos.sourceforge.net)\n", MP2_ENC_VERSION);
  fprintf(stdout,"MPEG Audio Layer II encoder\n\n");
  fprintf(stdout,"usage: \n");
  fprintf(stdout, "\t%s [options] <input> <output>\n\n","MP2Enc");

  fprintf(stdout,"Options:\n");
  fprintf(stdout,"Input\n");
  fprintf(stdout,"\t-s sfrq  input smpl rate in kHz   (dflt %4.1f)\n",DFLT_SFQ);
  fprintf(stdout,"\t-a       downmix from stereo to mono\n");
  fprintf(stdout,"\t-x       force byte-swapping of input\n");
  fprintf(stdout,"Output\n");
  fprintf(stdout,"\t-m mode  channel mode : s/d/j/m   (dflt %4c)\n",DFLT_MOD);
  fprintf(stdout,"\t-p psy   psychoacoustic model 1/2 (dflt %4u)\n",DFLT_PSY);
  fprintf(stdout,"\t-b br    total bitrate in kbps    (dflt 192)\n");
  fprintf(stdout,"\t-v lev   vbr mode\n");
  fprintf(stdout,"Operation\n");
  fprintf(stdout,"\t-f       fast mode (turns off psy model)\n");
  fprintf(stdout,"\t-q num   quick mode. only calculate psy model every num frames\n");
  fprintf(stdout,"Misc\n");
//  fprintf(stdout,"\t-d emp   de-emphasis n/5/c        (dflt %4c)\n",DFLT_EMP);
  fprintf(stdout,"\t-c       mark as copyright\n");
  fprintf(stdout,"\t-o       mark as original\n");
  fprintf(stdout,"\t-e       add error protection\n");
  fprintf(stdout,"\t-r       force padding bit/frame off\n");
  fprintf(stdout,"Files\n");
  fprintf(stdout,"\tinput    input sound file. (WAV,AIFF,PCM or use '/dev/stdin')\n");
  fprintf(stdout,"\toutput   output bit stream of encoded audio\n");
  fprintf(stdout,"\n\tAllowable bitrates for 16, 22.05 and 24kHz sample input\n");
  fprintf(stdout,"\t8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160\n");
  fprintf(stdout,"\n\tAllowable bitrates for 32, 44.1 and 48kHz sample input\n");
  fprintf(stdout,"\t32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 384\n");
  exit(1);
}


/********************************************************************
new_ext()
Puts a new extension name on a file name <filename>.
Removes the last extension name, if any.
***********************************************************************/
static void new_ext(char *filename, char *extname, char *newname)
{
  int found, dotpos;

  /* First, strip the extension */
  dotpos=strlen(filename);
  found=0;
  do
    {
      switch (filename[dotpos])
        {
        case '.' :
          found=1;
          break;
        case '\\':
        case '/' :
        case ':' :
          found=-1;
          break;
        default  :
          dotpos--;
          if (dotpos<0) found=-1;
          break;
        }
    }
  while (found==0);
  if (found==-1) strcpy(newname,filename);
  if (found== 1)
    {
      strncpy(newname,filename,dotpos);
      newname[dotpos]='\0';
    }
  strcat(newname,extname);
}


/************************************************************************
*
* parse_args
*
* PURPOSE:  Sets encoding parameters to the specifications of the
* command line.  Default settings are used for parameters
* not specified in the command line.
*
* SEMANTICS:  The command line is parsed according to the following
* syntax:
*
* -m  is followed by the mode
* -p  is followed by the psychoacoustic model number
* -s  is followed by the sampling rate
* -b  is followed by the total bitrate, irrespective of the mode
* -d  is followed by the emphasis flag
* -c  is followed by the copyright/no_copyright flag
* -o  is followed by the original/not_original flag
* -e  is followed by the error_protection on/off flag
* -f  turns off psy model (fast mode)
* -q <i>  only calculate psy model every ith frame
* -a  downmix from stereo to mono 
* -r  turn off padding bits in frames.
* -x  force byte swapping of input
*
* If the input file is in AIFF format, the sampling frequency is read
* from the AIFF header.
*
* The input and output filenames are read into #inpath# and #outpath#.
*
************************************************************************/

static void ParseArgs
(
		int				argc,
	    char**			argv,
		PMP2ENC_CONFIG	pConfig,
		BOOL*			pbDownMixToMono,
		BOOL*			pbByteSwap,
		CHAR*			lpszInPath,
		CHAR*			lpszOutPath
	)
{
	int		i=0;
	BOOL	bErr=FALSE;
	CHAR*	lpszProgName=argv[0];

	FLOAT	srate;
	int		brate;
//	long samplerate;

	// clear everything
	memset(lpszInPath,0,MAX_NAME_SIZE);
	memset(lpszOutPath,0,MAX_NAME_SIZE);
	memset(pConfig,0,sizeof(MP2ENC_CONFIG));

	// set defaults
	pConfig->dwVersion	= MP2ENC_CUR_IF_VERSION;
	pConfig->dwSize		= sizeof(MP2ENC_CONFIG);

	pConfig->dwSampleRate		= DFLT_SFQ;
	pConfig->dwMode				= DFLT_MOD;
	pConfig->dwBitrate			= DFLT_BRT;
							
	pConfig->bPrivate			= FALSE;		
	pConfig->bCRC				= FALSE;	
	pConfig->bCopyright			= FALSE;
	pConfig->bOriginal			= FALSE;

	pConfig->bUseVbr			= FALSE;
	pConfig->dwVbrQuality		= 0;

	pConfig->dwPsyModel			= 2;
	pConfig->dwQuickMode		= 1;

	pConfig->bWriteAncil		= FALSE;
	pConfig->bNoPadding			= FALSE;

	*pbDownMixToMono=FALSE;
	*pbByteSwap=FALSE;


	// process args
	while( ( ++i < argc ) && (bErr == FALSE) )
	{
		char c, *token, *arg, *nextArg;
		int  argUsed=0;

		token = argv[i];

		if(*token++ == '-')
        {
			if(i+1 < argc) nextArg = argv[i+1];
			else           nextArg = "";

			if (!*token)
			{
				// The user wants to use stdin and/or stdout
				if(lpszInPath[0] == '\0')       strncpy(lpszInPath, argv[i],MAX_NAME_SIZE);
				else if(lpszOutPath[0] == '\0') strncpy(lpszOutPath, argv[i],MAX_NAME_SIZE);
			} 

			while( ( c = *token++ ) )
            {
				if(*token /* NumericQ(token) */) arg = token;
				else                             arg = nextArg;

				switch(c)
                {
					case 'm':
						argUsed = 1;
						switch (*arg)
						{
							case 's': pConfig->dwMode = MP2ENC_MD_STEREO;		break;
							case 'd': pConfig->dwMode = MP2ENC_MD_DUALCHANNEL;	break;
							case 'j': pConfig->dwMode = MP2ENC_MD_JSTEREO;		break;
							case 'm': pConfig->dwMode = MP2ENC_MD_MONO;			break;
							default:
								MP2LibError("%s: -m mode must be s/d/j/m not %s\n",lpszProgName, arg);
								bErr= TRUE;
						}
					break;

					case 'p':
						pConfig->dwPsyModel = atoi(arg);
						argUsed = 1;
						if(pConfig->dwPsyModel<0 || pConfig->dwPsyModel>2)
						{
							MP2LibError("%s: -p model must be 1 or 2, not %s\n",lpszProgName, arg);
							bErr=TRUE;;
						}
					break;

					case 's':
						argUsed = 1;
						srate = (float)atof( arg );

						/* samplerate = rint( 1000.0 * srate ); $A  */
						pConfig->dwSampleRate = (long) (( 1000.0 * srate ) + 0.5);

						if (	(pConfig->dwSampleRate != 48000 ) ||
								(pConfig->dwSampleRate != 44100 ) ||
								(pConfig->dwSampleRate != 32000 ) ||
								(pConfig->dwSampleRate != 24000 ) ||
								(pConfig->dwSampleRate != 22050 ) ||
								(pConfig->dwSampleRate != 16000 ) )
						{
							MP2LibError("%s: -s invalid sampling frequency %d\n",lpszProgName, pConfig->dwSampleRate);
							bErr=TRUE;
						}
					break;

					case 'b': 
						argUsed = 1;
						pConfig->dwBitrate = atoi(arg);
					break;
					case 'c': pConfig->bCopyright = TRUE; break;
					case 'o': pConfig->bOriginal  = TRUE; break;
					case 'e': pConfig->bCRC		  = TRUE; break;
					case 'f': pConfig->dwPsyModel = 0   ; break;
					case 'r': pConfig->bNoPadding = TRUE; break;
					case 'q':
						argUsed = 1;
						pConfig->dwQuickMode = atoi (arg);
					case 'a':
						*pbDownMixToMono = TRUE;
						pConfig->dwMode  = MP2ENC_MD_MONO;
					break;
					case 'x': *pbByteSwap= TRUE;	break;
					case 'v':
						argUsed = 1;
						pConfig->bUseVbr = TRUE;
						pConfig->dwVbrQuality = atoi(arg);
					break;
					case 'h':
						usage();
					break;
					default:
						MP2LibError("%s: unrec option %c\n",lpszProgName, c);
						bErr = TRUE;
					break;
                }

				if(argUsed)
                {
                  if(arg == token)    token = "";   /* no more from token */
                  else                ++i;          /* skip arg we used */
                  arg = "";
                  argUsed = 0;
                }
            }
        }
		else
		{
			if(lpszInPath[0] == '\0')       strcpy(lpszInPath, argv[i]);
			else if(lpszOutPath[0] == '\0') strcpy(lpszOutPath, argv[i]);
			else
			{
				MP2LibError("%s: excess arg %s\n", lpszProgName, argv[i]);
				bErr=TRUE;
			}
		}
    }

  /* check for a valid bitrate */
//  if ( brate == 0 )
//    brate = bitrate[info->version][10];

//  if( (info->bitrate_index = BitrateIndex(brate, info->version)) < 0) err=1;

	if( bErr || lpszInPath[0] == '\0')
		usage();  /* If no infile defined, or err has occured, then call usage() */

	if( lpszOutPath[0] == '\0')
	{
		/* replace old extension with new one, 1992-08-19, 1995-06-12 shn */
		new_ext(lpszInPath, DFLT_EXT, lpszOutPath);
	}
}



int main(int argc,char* argv[])
{
	DWORD		dwSamples=0;
	DWORD		dwBufferSize=0;
	HMP2_STREAM	hStream=0;

	CHAR		strFNameIn[MAX_NAME_SIZE]={'\0',};
	CHAR		strFNameOut[MAX_NAME_SIZE]={'\0',};

	FILE*		pFileOut=NULL;

	BYTE*		pPCM=NULL;
	BYTE*		pMP2=NULL;

	DWORD		dwRead=0;
	DWORD		dwWrite=0;
	INT			nTime=0;

	BOOL		bSwapBytes=FALSE;
	BOOL		bDownMixToMono=FALSE;

	MP2ENC_CONFIG	mp2Config={0,};
	SNDFILE*		pSndFile=NULL;
	SF_INFO			wfInfo;

	// Parse command line arguments
	ParseArgs(argc,argv,&mp2Config,&bDownMixToMono,&bSwapBytes,strFNameIn,strFNameOut);

	// Clear sound structure
	memset(&wfInfo,0,sizeof(SF_INFO));

	// Open input file
	if (! ( pSndFile = sf_open_read(strFNameIn,&wfInfo)))
	{
		MP2LibError("Could not open input file %s\n",strFNameIn);
		return -1;
	}

	// Open output file
	pFileOut=fopen(strFNameOut,"wb+");

	if (pFileOut==NULL)
	{
		MP2LibError("Could not create output file %s\n",strFNameOut);
		return -1;
	}

	
	// Init the stream
	MP2EncOpen(&mp2Config,&dwSamples,&dwBufferSize,&hStream);

	// Allocate PCM input buffer
	pPCM=(PBYTE)calloc(dwSamples,sizeof(SHORT));

	// Allocate MP2 output buffer
	pMP2=(PBYTE)calloc(dwBufferSize,1);


	TimeEval(0);
	TimeEval(0);

	while ( (dwRead=(sf_readf_short(pSndFile,(PSHORT)pPCM,dwSamples/2)*2)) >0 )
	{
		// Encode it
		MP2EncEncodeFrame(hStream,dwRead,(PSHORT)pPCM, pMP2,&dwWrite);
		
		if (dwWrite)
		{
			fwrite(pMP2,dwWrite,1,pFileOut);
		}
	}


	// Close the encoder
	MP2EncClose(hStream);

	nTime=TimeEval(1);

	printf ("nTime is %d \n",nTime);

	// free PCM input buffer
	if (pPCM)
		free(pPCM);

	// free MP2 output buffer
	if (pMP2)
		free(pMP2);

	// close input file
	if (pSndFile)
		sf_close(pSndFile);

	// close output file
	if (pFileOut)
		fclose(pFileOut);

	return 0;
}