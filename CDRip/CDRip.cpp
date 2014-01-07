/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
**
** http://cdexos.sourceforge.net/
** http://sourceforge.net/projects/cdexos 
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <string.h>
#include <assert.h>

#include <math.h>

#include "CDRip.h"
#include "CDExtract.h"
#include "AspiDebug.h"


CCDExtract* pExtract=NULL;

// Get the DLL version number
LONG CCONV CR_GetCDRipVersion()
{

// Get CDex version
//	CFileVersion myVersion;
//	char lpszModuleFileName[MAX_PATH];
//	GetModuleFileName(NULL,lpszModuleFileName,sizeof(lpszModuleFileName));
//	myVersion.Open(lpszModuleFileName);
//	CString strVersion=myVersion.GetProductVersion();
//	return atoi(strVersion);

	return 117;
}

DLLEXPORT CDEX_ERR CCONV CR_DeInit( )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_DeInit()" ) );

	if ( NULL != pExtract )
	{
		pExtract->Clear();
		delete pExtract;
		pExtract = NULL;
	}

	(void)DeInitAspiDll( );

	EXIT_TRACE( _T( "CR_DeInit(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT CDEX_ERR CCONV CR_Init( int transportLayer, int debugLevel )
{
	CDEX_ERR bReturn = CDEX_OK;

	// set log filename TODO extract from ini filename
	LOG_SetLogFileName( LOG_DEFAULT_FILENAME );
	
	SetDebugLevel( debugLevel );

	ENTRY_TRACE( _T( "CR_Init, ini file name = transportLayer =%d, debugLevel = %d " ) , transportLayer, debugLevel );

	CR_DeInit();

	assert( NULL == pExtract );

	// create new extract model
	pExtract = new CCDExtract;
    pExtract->SetTransportLayer( transportLayer );

	if ( pExtract == NULL )
	{
		LTRACE( _T( "pExtract creation failed!" ) );
		bReturn = CDEX_OUTOFMEMORY;
	}
	else
	{
		bReturn = pExtract->Init();
	}


	// Check if low level CD-ROM drivers are intialized properly
	if ( CDEX_OK == bReturn )
	{

		if ( ( NULL != pExtract ) && 
			 pExtract->IsAvailable() )
		{
			// Obtain the specs of the SCSI devices and select the proper CD Device
			pExtract->GetCDRomDevices();
		}
		else
		{
			bReturn = CDEX_ERROR;
		}
	}

	if ( CDEX_OK == bReturn )
	{
		if ( CR_GetNumCDROM() < 1 )
		{
			bReturn = CDEX_NOCDROMDEVICES;
			pExtract->SetAvailable( false );
		}
		else
		{
			// Set drive zero as default
			CR_SetActiveCDROM( 0 );
		}
	}

	EXIT_TRACE( _T( "CR_Init, return value %d" ), bReturn );

	return bReturn;
}


DLLEXPORT LONG CCONV CR_GetNumCDROM()
{
	LONG lReturn = 0L;

	ENTRY_TRACE( _T( "CR_GetNumCDROM()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn = pExtract->GetNumDrives();
	}

	EXIT_TRACE( _T( "CR_GetNumCDROM(), return value: %ld" ), lReturn );

	return lReturn;

}

//DLLFUNCTION void CR_SetActiveCDROM(LONG nActiveDrive)
DLLEXPORT void CCONV CR_SetActiveCDROM( LONG nActiveDrive )
{
	ENTRY_TRACE( _T( "CR_SetActiveDrive(%ld)"), nActiveDrive );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		pExtract->SetActiveCDROM( (BYTE)nActiveDrive );
	}

	EXIT_TRACE( _T( "CR_SetActiveDrive()" ) );
}


DLLEXPORT LONG CCONV CR_GetActiveCDROM()
{
	LONG lReturn = 0;

	ENTRY_TRACE( _T( "CR_GetActiveCDROM()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn = pExtract->GetActiveCDROM();
	}
	else
	{
		lReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_GetActiveCDROM(), return value: %ld" ), lReturn );

	return lReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_SelectCDROMType( DRIVETYPE cdType )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_SelectCDROMType(%d)" ), cdType );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		pExtract->SetDriveType( cdType );
		pExtract->UpdateDriveSettings();
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_SelectCDROMType(), return value: %d" ), bReturn );

	return bReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_GetCDROMParameters( CDROMPARAMS* pParam)
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_GetCDROMParameters(%p)" ), pParam );

	// Clear structure
	memset( pParam, 0x00, sizeof( CDROMPARAMS ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
        memcpy( pParam, &( pExtract->GetActiveParameters() ), sizeof( CDROMPARAMS ) );
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_GetCDROMParameters(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT CDEX_ERR CCONV CR_SetCDROMParameters( CDROMPARAMS* pParam )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_SetCDROMParameters(%p)" ), pParam );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
        pExtract->SetActiveParameters( *pParam );

		if ( CUSTOMDRIVE != pParam->DriveTable.DriveType )
		{
			CR_SelectCDROMType( pParam->DriveTable.DriveType );
		}
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_SetCDROMParameters(), return value: %d" ), bReturn );

	return bReturn;
}




DLLEXPORT CDEX_ERR CCONV CR_OpenRipper(	LONG* plBufferSize,
										LONG dwStartSector,
										LONG dwEndSector,
                                        BOOL bExcludeNullSamplesInCRC
										)
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_OpenRipper(%p, %d, %d, %d )" ), plBufferSize, dwStartSector, dwEndSector, bExcludeNullSamplesInCRC );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
        pExtract->InitCRC( bExcludeNullSamplesInCRC );

		// Set Extract paramters, dwEndSector is inclusive !
		// thus if startsector=0 and endsector 1649, 1650 sectors are extracted
		switch ( pExtract->GetRippingMode() )
		{
			case CR_RIPPING_MODE_NORMAL:
				pExtract->SetupTrackExtract( dwStartSector, dwEndSector + 1 );
			break;
			case CR_RIPPING_MODE_PARANOIA:
				pExtract->SetupTrackExtractParanoia( dwStartSector, dwEndSector + 1 );
			break;
			default:
				assert( FALSE );
				return CDEX_ERROR;
		}


		// Start Thread
		//pExtract->StartThread(pExtract->ThreadFunc,pExtract);
		*plBufferSize= pExtract->GetNumReadSectors() * CB_CDDASECTORSIZE;
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_OpenRipper(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT CDEX_ERR CCONV CR_CloseRipper( ULONG* pCRC )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_CloseRipper()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		// Set Extract paramters
		pExtract->EndTrackExtract();
        if ( NULL != pCRC )
        {
            *pCRC = pExtract->GetCRC();
        }
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_CloseRipper(), return value: %d, crcValue= %08X" ), bReturn, *pCRC );

	return bReturn;
}


DLLEXPORT CDEX_ERR CCONV CR_RipChunk(BYTE* pbtStream,LONG* pNumBytes, BOOL& bAbort)
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_RipChunk(%p, %p, %d" ), pbtStream, pNumBytes, bAbort );

	assert( pNumBytes );
	assert( pbtStream );

	*pNumBytes=0;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		switch ( pExtract->GetRippingMode() )
		{
			case CR_RIPPING_MODE_NORMAL:
				bReturn = pExtract->RipChunk( pbtStream, pNumBytes, bAbort );
			break;

			case CR_RIPPING_MODE_PARANOIA:
				bReturn = pExtract->RipChunkParanoia( pbtStream, pNumBytes, bAbort );
			break;

			default:
				assert( FALSE );
				bReturn = CDEX_ERROR;
			break;
		}
	}
	else
	{
		assert( FALSE );
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_RipChunk(), bAbort = %d, return %d" ), bAbort, bReturn );

	return bReturn;

}


DLLEXPORT LONG CCONV CR_GetPeakValue()
{
	LONG lReturn = 0;

	ENTRY_TRACE( _T( "CR_GetPeakValue()") );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn = pExtract->GetPeakValue();
	}

	EXIT_TRACE( _T( "CR_GetPeakValue(), return value: %d" ), lReturn );

	return lReturn;
}




DLLEXPORT LONG CCONV CR_GetPercentCompleted()
{
	LONG lReturn = 0;

	ENTRY_TRACE( _T( "CR_GetPercentCompleted()") );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn = min( pExtract->GetPercentCompleted(), 99 );
	}

	EXIT_TRACE( _T( "CR_GetPercentCompleted(), return value: %d" ), lReturn );

	return lReturn;
}


DLLEXPORT LONG CCONV CR_GetNumberOfJitterErrors()
{
	LONG lReturn = 0;

	ENTRY_TRACE( _T( "CR_GetNumberOfJitterErrors()") );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn =  pExtract->GetJitterErrors();
	}

	EXIT_TRACE( _T( "CR_GetNumberOfJitterErrors(), return value: %d" ), lReturn );

	return lReturn;
}




DLLEXPORT CDEX_ERR CCONV CR_ReadToc()
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_ReadToc()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->ReadToc();
	}	

	EXIT_TRACE( _T( "CR_ReadToc(), return value: %d" ), bReturn );

	return bReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_ReadCDText(BYTE* pbtBuffer,int nBufferSize,LPINT pnCDTextSize)
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_ReadCDText( %p, %d, %p)" ), pbtBuffer, nBufferSize, pnCDTextSize );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() && 
		 ( TRUE == pExtract->GetUseCDText() ) )
	{
		bReturn = pExtract->ReadCDText( pbtBuffer, nBufferSize, pnCDTextSize );
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_ReadCDText(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT LONG CCONV CR_GetNumTocEntries()
{
	LONG lReturn = 0;

	ENTRY_TRACE( _T( "CR_GetNumTocEntries()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn = pExtract->GetToc().GetNumTracks();
	}

	EXIT_TRACE( _T( "CR_GetNumTocEntries(), return value: %d" ), lReturn );

	return lReturn;
}

DLLEXPORT TOCENTRY CCONV CR_GetTocEntry(LONG nTocEntry)
{
	TOCENTRY TocEntry;

	ENTRY_TRACE( _T( "CR_GetTocEntry(%d)" ), nTocEntry );

	memset( &TocEntry, 0x00, sizeof( TocEntry ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		TocEntry.dwStartSector = pExtract->GetToc().GetStartSector( nTocEntry );
		TocEntry.btFlag = pExtract->GetToc().GetFlags( nTocEntry );
		TocEntry.btTrackNumber = pExtract->GetToc().GetTrackNumber( nTocEntry );
	}

	EXIT_TRACE( _T( "CR_GetTocEntry" ) );

	return TocEntry;
}

DLLEXPORT void CCONV CR_NormalizeChunk(SHORT* pbsStream,LONG nNumSamples,DOUBLE dScaleFactor )
{
	int i;

	ENTRY_TRACE( _T( "CR_NormalizeChunk(%p, %d, %f)" ), pbsStream, nNumSamples, dScaleFactor );

	for ( i = 0; i < nNumSamples; i++)
	{
		pbsStream[ i ] = (short)( (double)pbsStream[ i ] * dScaleFactor );
	}

	EXIT_TRACE( _T( "CR_NormalizeChunk" ) );
}




DLLEXPORT BOOL CCONV CR_IsUnitReady()
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_IsUnitReady()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->IsUnitReady();
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_IsUnitReady(), return value: %d" ), bReturn );

	return bReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_IsMediaLoaded( CDMEDIASTATUS& IsMediaLoaded )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_IsMediaLoaded()" ) );

	IsMediaLoaded = CDMEDIA_NOT_PRESENT;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		IsMediaLoaded = pExtract->IsMediaLoaded();
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_IsMediaLoaded( %d ), return value: %d" ), IsMediaLoaded, bReturn );

	return bReturn;
}

DLLEXPORT BOOL CCONV CR_EjectCD( BOOL bEject )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_EjectCD( %d )" ), bEject );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		pExtract->PreventMediaRemoval( FALSE );
		bReturn = pExtract->EjectCD( bEject );
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_EjectCD(), return value: %d" ), bReturn );

	return bReturn;
}

DLLEXPORT void CCONV CR_LockCD( BOOL bLock )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_LockCD( %d )" ), bLock );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		pExtract->PreventMediaRemoval( bLock );
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CR_LockCD(), return value: %d" ), bReturn );
}


DLLEXPORT BOOL CCONV CR_IsAudioPlaying()
{
	BOOL bReturn = FALSE;

	ENTRY_TRACE( _T( "CR_IsAudioPlaying()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->IsAudioPlaying();
	}

	EXIT_TRACE( _T( "CR_IsAudioPlaying(), return value: %d" ), bReturn );

	return bReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_PlayTrack( int nTrack )
{
	BOOL bReturn = CDEX_ERROR;

	ENTRY_TRACE( _T( "CR_PlayTrack(%d)" ), nTrack );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		int nNumTocEntries = CR_GetNumTocEntries();
		
		for ( int i = 0; i < nNumTocEntries; i++ )
		{
			TOCENTRY myTocEntry = CR_GetTocEntry( i );
			TOCENTRY myTocEntry1 = CR_GetTocEntry( i + 1 );

			if ( pExtract && myTocEntry.btTrackNumber == nTrack )
			{
				pExtract->PlayTrack(	myTocEntry.dwStartSector, 
										myTocEntry1.dwStartSector - 1 );
				bReturn = CDEX_OK;
			}
		}
	}

	EXIT_TRACE( _T( "CR_PlayTrack(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT CDEX_ERR CCONV CR_PlaySection(LONG lStartSector,LONG lEndSector)
{
	LTRACE( _T( "CR_PlaySection" ) );
	if ( !pExtract )
		return CDEX_ERROR;

	pExtract->PlayTrack(lStartSector,lEndSector);
	return CDEX_OK;
}



DLLEXPORT CDEX_ERR CCONV CR_StopPlayTrack()
{
	BOOL bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_StopPlayTrack()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->StopPlayTrack();
	}

	EXIT_TRACE( _T( "CR_StopPlayTrack(), return value: %d" ), bReturn );

	return bReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_PauseCD( BOOL bPause )
{
	BOOL bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_PauseCD(%d)" ), bPause );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->PauseCD( bPause );
	}

	EXIT_TRACE( _T( "CR_PauseCD(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT CDSTATUSINFO CCONV CR_GetCDStatusInfo()
{
	return g_CDStatusInfo;
}


DLLEXPORT CDEX_ERR CCONV CR_GetPlayPosition(DWORD& dwRelPos,DWORD& dwAbsPos)
{
	BOOL bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_GetPlayPosition(%d,%d)" ), dwRelPos, dwAbsPos );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->CurrentPosition(dwRelPos,dwAbsPos);
	}

	EXIT_TRACE( _T( "CR_GetPlayPosition(%d,%d), return value: %d" ), dwRelPos, dwAbsPos, bReturn );

	return bReturn;
}

DLLEXPORT CDEX_ERR CCONV CR_SetPlayPosition(DWORD dwAbsPos)
{
	BOOL bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CR_SetPlayPosition(%d)" ), dwAbsPos );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->Seek( dwAbsPos );
	}

	EXIT_TRACE( _T( "CR_SetPlayPosition(), return value: %d" ), bReturn );

	return bReturn;
}


DLLEXPORT DRIVETYPE CCONV CR_GetCDROMType()
{
	DRIVETYPE retDriveType = GENERIC;

	ENTRY_TRACE( _T( "CR_GetCDROMType(" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		retDriveType = pExtract->GetDriveType();
	}

	EXIT_TRACE( _T( "CR_GetCDROMType(), retDriveType = %d" ), retDriveType );

	return retDriveType;
}


DLLEXPORT LONG CCONV CR_GetJitterPosition()
{
	LONG lReturn = 50;

	ENTRY_TRACE( _T( "CR_GetJitterPosition()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		lReturn = pExtract->GetJitterPosition();
	}

	EXIT_TRACE( _T( "CR_GetJitterPosition(), return value: %d" ), lReturn );

	return lReturn;
}


DLLEXPORT void CCONV CR_GetLastJitterErrorPosition(DWORD& dwStartSector,DWORD& dwEndSector)
{
	dwStartSector=0;
	dwEndSector=0;

	ENTRY_TRACE( _T( "CR_GetJitterPosition()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		pExtract->GetLastJitterErrorPosition( dwStartSector, dwEndSector );
	}

	EXIT_TRACE( _T( "CR_GetJitterPosition(%d,%d)"), dwStartSector, dwEndSector );
}



DLLEXPORT void CCONV  CR_GetSubChannelTrackInfo(	
										int&	nReadIndex,
										int&	nReadTrack,
										DWORD&	dwReadPos )
{
	ENTRY_TRACE( _T( "CR_GetSubChannelTrackInfo()" ) );

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		pExtract->GetSubChannelTrackInfo( nReadIndex, nReadTrack, dwReadPos );
	}

	EXIT_TRACE( _T( "CR_GetSubChannelTrackInfo(%d,%d,%d)"), nReadIndex, nReadTrack, dwReadPos );
}

DLLEXPORT CDEX_ERR CCONV CR_ScanForC2Errors(	
	DWORD	dwStartSector,
	DWORD	dwNumSectors,
	DWORD&	dwErrors,
	DWORD*	pdwErrorSectors )
{
	CDEX_ERR bReturn = CDEX_ERROR;

	if ( pExtract )
	{
		bReturn = pExtract->ScanForC2Errors(	dwStartSector, 
												dwNumSectors,
												dwErrors,
												pdwErrorSectors );
	}

	return bReturn;
}

DLLEXPORT DWORD CCONV CR_GetCurrentRipSector()
{
	return 0;
}

DLLEXPORT CDEX_ERR CCONV CR_GetDetailedDriveInfo( 
	LPSTR lpszInfo,
	DWORD dwInfoSize )
{
	CDEX_ERR bReturn = CDEX_OK;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		bReturn = pExtract->GetDetailedDriveInfo(	lpszInfo, 
													dwInfoSize );
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	return bReturn;
}



BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			LTRACE( _T( "CDRIP.DLL Initializing!\n" ) );
		break;
		case DLL_THREAD_ATTACH:
		break;
		case DLL_THREAD_DETACH:
		break;
		case DLL_PROCESS_DETACH:
			LTRACE( _T( "CDRIP.DLL Terminating!\n" ) );
			LTRACE( _T( "DllMain DLL_PROCESS_DETACH" ) );
			CR_DeInit();
		break;
    }
    return TRUE;
}

DLLEXPORT CDEX_ERR CCONV CR_ReadBlockDirect( BYTE* pbtStream,DWORD numBytes, LONG sector, DWORD numSectors   )
{
	CDEX_ERR returnValue = CDEX_OK;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		returnValue = pExtract->ReadBlockDirect( pbtStream, numBytes, sector, numSectors   );
	}
	else
	{
		returnValue = CDEX_ERROR;
	}

	return returnValue;
}


DLLEXPORT CDEX_ERR CCONV CR_ReadSubChannel( BYTE* pbtStream,DWORD numBytes,BYTE format,  BYTE btTrack )
{
	CDEX_ERR returnValue = CDEX_OK;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
        returnValue = pExtract->ReadSubChannel( format, pbtStream, numBytes, btTrack  );
	}
	else
	{
		returnValue = CDEX_ERROR;
	}

	return returnValue;
}

DLLEXPORT CDEX_ERR CCONV CR_ReadSubchannelData( BYTE* pbtStream,DWORD numBytes, LONG sector, DWORD numSectors   )
{
	CDEX_ERR returnValue = CDEX_OK;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
		returnValue = pExtract->ReadSubChannelData( pbtStream, numBytes, sector, numSectors   );
	}
	else
	{
		returnValue = CDEX_ERROR;
	}

	return returnValue;
}

DLLEXPORT CDEX_ERR CCONV CR_ReadAndGetISRC( MCISRC* mcisrc )
{
	CDEX_ERR returnValue = CDEX_OK;

	if ( ( NULL != pExtract ) && 
		 pExtract->IsAvailable() )
	{
        BYTE buffer[ 0x18 ];

        memset( mcisrc, 0, sizeof( MCISRC ) );

		INT numTracks = pExtract->GetToc().GetNumTracks();
        pExtract->ReadSubChannel( 2, buffer, sizeof( buffer ), 0 );

        memcpy( mcisrc->mmCatalog, &buffer[ 9 ] , sizeof( buffer ) - 9 );
        mcisrc->mmCatalog[ sizeof( buffer ) - 9 + 1 ] = '\0';

        for ( INT track = 0; track < numTracks; track++ )
        {
            pExtract->ReadSubChannel( 3, buffer, sizeof( buffer ), track + 1 );
            memcpy( mcisrc->isrc[ track ], &buffer[ 9 ] , sizeof( buffer ) - 9 );
            mcisrc->isrc[ track ][ sizeof( buffer ) - 9 + 1 ] = '\0';
        }

        //		returnValue = pExtract->ReadBlockDirect( pbtStream, numBytes, sector, numSectors   );
	}
	else
	{
		returnValue = CDEX_ERROR;
	}

	return returnValue;
}
