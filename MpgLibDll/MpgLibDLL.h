#ifndef MPGLIBDLL_H_INCLUDED
#define MPGLIBDLL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#undef DLLEXPORT

#ifdef _MPGLIBDLL
	#define DLLEXPORT __declspec( dllexport )
#else
	#define DLLEXPORT __declspec( dllimport )
#endif

#include <Windows.h>

typedef int MPGLIB_ERR;
typedef int H_STREAM;


#define MPGLIB_ERROR		-1
#define MPGLIB_OK			 0
#define MPGLIB_NEED_MORE	 1

typedef MPGLIB_ERR	(*MPGLIB_OPENSTREAM)	(H_STREAM* phStream);
typedef MPGLIB_ERR	(*MPGLIB_DECODECHUNK)	(H_STREAM	hStream,
											 PBYTE		pInStream,
											 INT		nInSize,
											 PBYTE		pOutStream,
											 INT		nOutSize,
											 PINT		pnOutBytes);

typedef MPGLIB_ERR	(*MPGLIB_CLOSESTREAM)	(H_STREAM hStream);

typedef VOID		(*MPGLIB_VERSION)		(CHAR* lpszVersionString,
											 INT nSize);

typedef INT			(*MPGLIB_GETBITRATE)	(VOID);
typedef INT			(*MPGLIB_FLUSH)	(VOID);


#define	TEXT_MPGLIB_OPENSTREAM	"MpgLib_OpenStream"
#define	TEXT_MPGLIB_DECODECHUNK	"MpgLib_DecodeChunk"
#define	TEXT_MPGLIB_CLOSESTREAM	"MpgLib_CloseStream"
#define	TEXT_MPGLIB_VERSION		"MpgLib_Version"
#define	TEXT_MPGLIB_GETBITRATE	"MpgLib_GetBitRate"
#define	TEXT_MPGLIB_FLUSH		"MpgLib_Flush"


#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */

#endif