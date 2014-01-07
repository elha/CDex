/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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


#include "CDRomSettings.h"
#include "Aspi.h"
#include <math.h>
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include <assert.h>
#include <limits.h>
#include "CDRip.h"


void CToc::FillEntry(BYTE* btStream)
{

//	m_reserved1=btStream[0];
	m_btaFlags[m_nNumTracks]=btStream[1];
	m_btaTrackNumber[m_nNumTracks]=btStream[2];
//	m_reserved2=btStream[3];
	memcpy(&m_dwaStartSector[m_nNumTracks],&btStream[4],sizeof(DWORD));

	// Swap DWORD order
    Swap (&m_dwaStartSector[m_nNumTracks], 4);

	// Increase the number of total tracks
	m_nNumTracks++;
}

// CONSTRUCTOR
CToc::CToc()
{
	// Initialize variables
	Clear();
}


void CToc::Clear()
{
	m_nNumTracks=0;
	memset(m_dwaStartSector,0x00,sizeof(m_dwaStartSector));
	memset(m_btaFlags,0x00,sizeof(m_btaFlags));
	memset(m_btaTrackNumber,0x00,sizeof(m_btaTrackNumber));
}


// DESTRUCTOR
CToc::~CToc()
{
}



void CToc::DumpToc(LPBYTE pbtBuffer,int nSize)
{
/*
	int			i;
	DWORD		dw;
	unsigned	mins;
	double		secnds;
	extern double fmod(double,double);


	// Open Dump File
	CADebug myLogFile(CString("C:\\CDEX_TOC.txt"));

	// A header would not hurt
	myLogFile.printf("Start of TOC dump\n\n");

	// Dump binary data
	myLogFile.printf("Dumping binary data first, buffer size=%d =>%d toc entries\n\n",nSize,nSize/8);
	
	myLogFile.DumpBinaray(pbtBuffer,nSize);

	dw = GetStartSector(m_wNumTracks-1);

	// Calculate number of minutes
	mins= dw / (TRACKSPERSEC*60L);

	secnds= (fmod(dw , 60*TRACKSPERSEC) / (DOUBLE)TRACKSPERSEC);


	myLogFile.printf("\n\nDumping Toc Entries\n");
	for (i=0; i<m_wNumTracks; i++) 
	{
		myLogFile.printf("Toc Entry=%2d  StartSector=%8d Flags=%2d",i,m_TrackInfo[i].GetStartSector(),m_TrackInfo[i].GetFlags());
		// Skip nonaudio tracks 
//		if (m_TrackInfo[i].GetFlags() != AUDIOTRKFLAG) 
//			continue;					


		if (m_TrackInfo[i].GetTrack() <= MAXTRK) 
		{
			DWORD dw2;

			dw2= m_TrackInfo[i].GetStartSector();

			dw = m_TrackInfo[i+1].GetStartSector();

			mins= (dw - dw2) / (TRACKSPERSEC*60L);
			secnds= fmod(dw-dw2 , 60*TRACKSPERSEC) / TRACKSPERSEC;


			CString strDuration=GetTrackDuration(i+1);
			CString strStart=GetStartTime(i+1);

			CString strNew;
			myLogFile.printf(" => Start time=%s Duration=%s\n",strStart,strDuration);
		}

	}
	myLogFile.printf("\n\nEnd of TOC dump\n");
*/
}

BOOL CToc::IsAudioTrack(int p_track)
{
	if ( !( m_btaFlags[p_track] & CDROMDATAFLAG) )
		return TRUE;
	return FALSE;
}

DWORD CToc::GetSize(int nTrack)
{
	DWORD dwSectors = GetStartSector( nTrack + 1 ) - GetStartSector( nTrack );
	return dwSectors * CB_CDDASECTORSIZE;
}




CDRomSettings::CDRomSettings()
{
	memset( m_lpszIniFname, 0x00, sizeof( m_lpszIniFname ) );
    m_nTransportLayer = TRANSPLAYER_ASPI;
    // m_nTransportLayer = TRANSPLAYER_NTSCSI;
	m_nActive = 0;
}

CDRomSettings::~CDRomSettings()
{
}

// CDROMPARAMS GetDefaultParameters
CDROMPARAMS CDRomSettings::GetDefaultParameters()
{
	CDROMPARAMS returnValue;
    
    memset( &returnValue, 0, sizeof( returnValue ) );

	strcpy( returnValue.lpszCDROMID, "DONTKNOW" );
	returnValue.nOffsetStart = 0;
	returnValue.nOffsetEnd = 0;	
	returnValue.nSpeed = 0;
	returnValue.nSpinUpTime = 0;
	returnValue.bJitterCorrection = 0;
	returnValue.bSwapLefRightChannel = 0;
	returnValue.nNumOverlapSectors = 7;
	returnValue.nNumReadSectors = 26;	
	returnValue.nNumCompareSectors = 1;	
	returnValue.nMultiReadCount = 0;
	returnValue.bMultiReadFirstOnly = FALSE;
	returnValue.bFUA = FALSE;
    returnValue.nSampleOffset = LONG_MIN;
	returnValue.bLockDuringRead = TRUE;
	returnValue.bUseCDText = TRUE;
    returnValue.bReadC2Errors = FALSE;

	returnValue.btTargetID=0;
	returnValue.btAdapterID=0;
	returnValue.btLunID=0;
	returnValue.nAspiRetries=0;
	returnValue.nAspiTimeOut=3000;


	returnValue.DriveTable.DriveType=GENERIC;
	returnValue.DriveTable.ReadMethod=READMMC;
	returnValue.DriveTable.SetSpeed=SPEEDMMC;
	returnValue.DriveTable.Endian=ENDIAN_LITTLE;
	returnValue.DriveTable.EnableMode=ENABLENONE;
	returnValue.DriveTable.nDensity=0;
	returnValue.DriveTable.bAtapi=TRUE;

	returnValue.bAspiPosting=FALSE;
	returnValue.nRippingMode = 0;
	returnValue.nParanoiaMode = 0;

    return returnValue;
}


void CDRomSettings::AddCDRom( LPSTR lpszName, BYTE btAdapterID, BYTE btTargetID, BYTE btLunID )
{
	int nDeviceOffset=0;


	// Loop through Array and see if there are any cd rom with the same name
	for ( unsigned int i=0; i < m_CDParams.size() ; i++ )
	{
		// Check if name to add is equal to current indexed CDROM setting
		if ( strstr(m_CDParams[i].lpszCDROMID,lpszName ) !=NULL )
		{
			// Increase number
			nDeviceOffset++;
		}
	}

	// There appears to be CDROMs with the same name
	if ( nDeviceOffset > 0 )
	{
		char lpszDevNum[4];
		// Add (nDeviceOffset) to strName
		sprintf( lpszDevNum, "(%d)", nDeviceOffset );
		lpszName = strcat( lpszName, lpszDevNum );
	}
	
	// Create new CDROM setting
	CDROMPARAMS newSettings = GetDefaultParameters();


	// Set device name
	strcpy( newSettings.lpszCDROMID,lpszName );
	newSettings.btTargetID = btTargetID;
	newSettings.btAdapterID = btAdapterID;
	newSettings.btLunID = btLunID;

	// Add to array of CDROM settings
	m_CDParams.push_back( newSettings );

	// Also add a new Table of contents
	m_Toc.push_back( CToc() );

}

void CDRomSettings::UpdateDriveSettings()
{
	if (m_CDParams[m_nActive].DriveTable.DriveType!=CUSTOMDRIVE)
	{
		// Set defaults	(SONY)
		m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDNONE;
		m_CDParams[m_nActive].DriveTable.Endian		=ENDIAN_LITTLE;
		m_CDParams[m_nActive].DriveTable.nDensity	=0x00;
		m_CDParams[m_nActive].DriveTable.EnableMode	=ENABLENONE;
		m_CDParams[m_nActive].DriveTable.bAtapi		=FALSE;
	}

	// Setup the specific drive properties
	switch (m_CDParams[m_nActive].DriveTable.DriveType)
	{
		case GENERIC:	
			m_CDParams[m_nActive].DriveTable.ReadMethod	=READMMC;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDMMC;
			m_CDParams[m_nActive].DriveTable.bAtapi		=TRUE;
		break;

		case TOSHIBANEW:	
			m_CDParams[m_nActive].DriveTable.ReadMethod	=READ10;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDTOSHIBA;
			m_CDParams[m_nActive].DriveTable.nDensity	=0x82;
			m_CDParams[m_nActive].DriveTable.EnableMode	=ENABLESTD;
		break;
		case TOSHIBA:	
		case IBM:	
		case DEC:	
			m_CDParams[m_nActive].DriveTable.ReadMethod	=READ10;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDNONE;
			m_CDParams[m_nActive].DriveTable.nDensity	=0x82;
			m_CDParams[m_nActive].DriveTable.EnableMode	=ENABLESTD;
		break;
		case IMSCDD5:	//ReadTocCdrSCSI : ReadTocSCSI; not yet implemented yet
		case IMS:	
		case KODAK:	
		case RICOH:	
		case HP:	
		case PHILIPS:	
		case PLASMON:	
		case GRUNDIGCDR100IPW:
		case MITSUMICDR:
			m_CDParams[m_nActive].DriveTable.ReadMethod	=READ10;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDPHILIPS;
			m_CDParams[m_nActive].DriveTable.EnableMode	=ENABLESTD;
			m_CDParams[m_nActive].DriveTable.Endian		=ENDIAN_BIG;
		break;
		case NRC:
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDNONE;
		break;
		case YAMAHA:
			m_CDParams[m_nActive].DriveTable.EnableMode	=ENABLESTD;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDYAMAHA;
		break;
		case SONY:
		case PLEXTOR:			// beware of type CD-ROM CDU55E can use MMC reading
			m_CDParams[m_nActive].DriveTable.ReadMethod	=READSONY;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDSONY;
		break;
		case NEC:
			m_CDParams[m_nActive].DriveTable.ReadMethod	=READNEC;
			m_CDParams[m_nActive].DriveTable.SetSpeed	=SPEEDNEC;
		break;
		case CUSTOMDRIVE:
		break;
		default:
			assert( FALSE );
	}
}





int CDRomSettings::GetTransportLayer()
{
	return m_nTransportLayer;
}

void CDRomSettings::SetTransportLayer( int nValue )
{
	m_nTransportLayer = nValue;
}



void CDRomSettings::SetActiveCDROM( BYTE nValue )
{
	m_nActive = nValue;
}

void CDRomSettings::SetVendorName( CHAR* lpszValue )
{ 
    strcpy( m_CDParams.back().lpszVendorID, lpszValue );
}

void CDRomSettings::SetType(CHAR* lpszValue )
{ 
    strcpy( m_CDParams.back().lpszProductID, lpszValue ); 
}
