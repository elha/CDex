#include "config.h"
#include "common.h"
#include "interface.h"
#include "MpgLibDLL.h"

MPSTR mp;
plotting_data *mpg123_pinfo=NULL;
//int* pretab=NULL;

DLLEXPORT MPGLIB_ERR MpgLib_OpenStream(H_STREAM phStream)
{
	/* clear mp structure */
	memset(&mp,0x00,sizeof(mp));

	/* set stream parameter, not used right now */
	phStream=1;

	/* call decoder init function */
	return InitMP3(&mp);
}

DLLEXPORT MPGLIB_ERR MpgLib_DecodeChunk(H_STREAM hStream,PBYTE pInStream,INT nInSize,PBYTE pOutStream,INT nOutSize,PINT pnOutBytes)
{
	/* call decode chunk functions */
	if ( (mp.bsize==0) && (nInSize==0) )
		return MPGLIB_NEED_MORE;
	return decodeMP3(&mp,(CHAR*)pInStream,nInSize,(CHAR*)pOutStream,nOutSize,pnOutBytes);
}


DLLEXPORT MPGLIB_ERR MpgLib_CloseStream(H_STREAM hStream)
{
	/* deinitialize the decoder */
	ExitMP3(&mp);

	/* return success */
	return MPGLIB_OK;
}

DLLEXPORT VOID MpgLib_Version(CHAR* lpszVersionString,INT nSize)
{
	/* return version string if possible */
	if (lpszVersionString!=NULL && nSize>13)
		strcpy(lpszVersionString,"MPG Lib 0.01");
}

DLLEXPORT INT MpgLib_GetBitRate(VOID)
{
    switch(mp.fr.lay)
    {
      case 1:
			return tabsel_123[mp.fr.lsf][0][mp.fr.bitrate_index] * 1000;
//			fr->framesize /= freqs[fr->sampling_frequency];
      break;
      case 2:
			return tabsel_123[mp.fr.lsf][1][mp.fr.bitrate_index] * 1000;
        break;
      case 3:
			return tabsel_123[mp.fr.lsf][2][mp.fr.bitrate_index] * 1000;
        break; 
	}
      return (0);
}


DLLEXPORT INT MpgLib_Flush(VOID)
{
	while (mp.head!=NULL && mp.tail!=NULL)
	{
		remove_buf(&mp);
	}
	mp.bsize=0;
	return (0);
}
