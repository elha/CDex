/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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


#include "StdAfx.h"
#include "MCI_CD.h"
#include "Config.h"
#include <mmsystem.h>

INITTRACE( _T( "MCI_CD" ) );

// if (quickbuf[5+j*8] & 4) vDataTrack[i] = 1; else vDataTrack[i] = 0;

/*	if (nHighest > 2) {
	    if (vDataTrack[nHighest] && !vDataTrack[nHighest-1]) {
			bool done = false;
            int thisdrive = Form8->ComboBox2->ItemIndex;
 			j = vStopSector[1];
            if ((unsigned char)cdromunits[thisdrive] != 0xFF) done = ANALOGGetLastAudioTrackLenght(cdromunits[thisdrive],nHighest - 1, j);
			i = 0;
            while (!done && i < 27) {
	 			j = vStopSector[1];
    	        if (((unsigned char)cdromunits[i] != 0xFF) && i != thisdrive) done = ANALOGGetLastAudioTrackLenght(cdromunits[i],nHighest - 1, j);
                i++;
            }
        	if (done) {
            	vStopSector[nHighest-1] = vStartSector[nHighest-1]+j;
                multimediadisc = vStopSector[nHighest-1];
            }
        }
    } */



static BOOL GetMCILastudioTrackLenght(char chDriveLetter,int tracknumber, int &tracklength)
{
	MCI_INFO_PARMS sMCIInfo;
	TCHAR szMCIReturnString[80];
	MCI_OPEN_PARMS sMCIOpen;
    MCI_STATUS_PARMS mciStatusParms;
	TCHAR zDevice[4];
    DWORD nErr;
    int firsttracklength;

	// Create device string
    _stprintf(zDevice, _T( "%c:" ), chDriveLetter );

	// Set MCI open parameters
    sMCIOpen.lpstrDeviceType = (LPCTSTR) MCI_DEVTYPE_CD_AUDIO;
    sMCIOpen.lpstrElementName = zDevice;

	// Try to open the device
    nErr = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT,(DWORD)&sMCIOpen);
    
	if (nErr) 
		return false;

    sMCIInfo.lpstrReturn = szMCIReturnString;
	sMCIInfo.dwRetSize = 79;
    mciStatusParms.dwItem = MCI_STATUS_POSITION;
//    mciStatusParms.dwTrack = 2;
	mciStatusParms.dwTrack = tracknumber;

    if (mciSendCommand(sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD)&mciStatusParms))
		goto errorjumpout;

    firsttracklength = (DWORD)MCI_MSF_MINUTE(mciStatusParms.dwReturn) * 4500 +
				       (DWORD)MCI_MSF_SECOND(mciStatusParms.dwReturn) * 75 +
				   	   (DWORD)MCI_MSF_FRAME(mciStatusParms.dwReturn)-151;
    
	if (firsttracklength != tracklength)
		goto errorjumpout;

	mciStatusParms.dwItem = MCI_STATUS_LENGTH;

	if (mciSendCommand(sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK,(DWORD)&mciStatusParms)) 
		goto errorjumpout;

	tracklength = (DWORD)MCI_MSF_MINUTE(mciStatusParms.dwReturn) * 4500 +
					 (DWORD)MCI_MSF_SECOND(mciStatusParms.dwReturn) * 75 +
					 (DWORD)MCI_MSF_FRAME(mciStatusParms.dwReturn);

    mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
    return true;
errorjumpout:
	mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, 0, NULL);
	return false;
}


//---------------------------------------------------------------------------
short MCIGetDiskInfo(	char	chDriveLetter,
							int&	nHighest,
							LPINT	vStartSector,
							LPINT	vStopSector,
							DWORD&	dwTotalTime,
							DWORD&	dwDiscID
							)
{
	int					i;
	short				vDataTrack[100];
	MCI_INFO_PARMS		sMCIInfo;
	TCHAR				szMCIReturnString[80];
	MCI_OPEN_PARMS		sMCIOpen;
    MCI_STATUS_PARMS	mciStatusParms;
	TCHAR				zDevice[4];
    DWORD				nErr;

    _stprintf(zDevice, _T( "%c:" ), chDriveLetter );

    sMCIOpen.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;

    sMCIOpen.lpstrElementName = zDevice;

	memset(vStartSector,0x00,101*sizeof(INT));
	memset(vStopSector,0x00,101*sizeof(INT));

	// Try to open the device
    nErr = mciSendCommand(NULL,MCI_OPEN,MCI_OPEN_TYPE|MCI_OPEN_SHAREABLE|MCI_OPEN_TYPE_ID|MCI_OPEN_ELEMENT,(DWORD)&sMCIOpen);
    if (nErr)
    {
        nErr = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT,(DWORD)&sMCIOpen);
	    if (nErr)
		{
		    TCHAR zError[256];
            mciGetErrorString(nErr, zError, 255);
			LTRACE( zError ); 
        }
    }

    sMCIInfo.lpstrReturn = szMCIReturnString;
	sMCIInfo.dwRetSize = 79;
    mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;

    if (mciSendCommand(sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms))
	{
        mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, 0, NULL);
		return 0;
    }

    nHighest = (BYTE)mciStatusParms.dwReturn;

//    DWORD dwTime=GetTickCount();

	for(i = 0; i < nHighest; i++)
	{
//		LTRACE("A %d",GetTickCount()-dwTime);

        mciStatusParms.dwItem = MCI_STATUS_POSITION;
		mciStatusParms.dwTrack = i+1;

		// Open the device
        if (mciSendCommand(sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD)(LPVOID) &mciStatusParms))
		{
			mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, 0, NULL);
            return 0;
        }

//		LTRACE("B %d",GetTickCount()-dwTime);

        vStartSector[i] = (DWORD)MCI_MSF_MINUTE(mciStatusParms.dwReturn) * 4500 +
				   	  (DWORD)MCI_MSF_SECOND(mciStatusParms.dwReturn) * 75 +
				   	  (DWORD)MCI_MSF_FRAME(mciStatusParms.dwReturn)-150;
        int nMin=MCI_MSF_MINUTE(mciStatusParms.dwReturn);
        int nSec=MCI_MSF_SECOND(mciStatusParms.dwReturn);
        int nFrm=MCI_MSF_FRAME(mciStatusParms.dwReturn);

		mciStatusParms.dwItem = MCI_STATUS_LENGTH;
		mciStatusParms.dwTrack = i+1;

		if (mciSendCommand(sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK,(DWORD)&mciStatusParms)) 
		{
			mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, 0, NULL);
            return 0;
		}

//		LTRACE("C %d",GetTickCount()-dwTime);

        vStopSector[i] = (DWORD)MCI_MSF_MINUTE(mciStatusParms.dwReturn) * 4500 +
				   	  (DWORD)MCI_MSF_SECOND(mciStatusParms.dwReturn) * 75 +
				   	  (DWORD)MCI_MSF_FRAME(mciStatusParms.dwReturn)-1;

		vStopSector[i]+=vStartSector[i];

//		if (i > 0)
//			vStopSector[i-1] = vStartSector[i] - 1;

	    mciStatusParms.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
    	mciStatusParms.dwTrack = i;
    	mciSendCommand (sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD)(LPVOID)&mciStatusParms);

//		LTRACE("D %d",GetTickCount()-dwTime);

		if (mciStatusParms.dwReturn != MCI_CDA_TRACK_AUDIO)
		{
			vDataTrack[i] = 1;
			LTRACE( _T( "Track %d is not an Audio Track" ), i + 1 );
		}
		else
		{
			vDataTrack[i] = 0;
		}
    }

//	mciStatusParms.dwItem = MCI_STATUS_LENGTH;
//	mciStatusParms.dwTrack = nHighest;

	// Open the MCI device
//	if (mciSendCommand(sMCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK, (DWORD)&mciStatusParms))
//	{
//		mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, 0, NULL);
//		return 0;
//	}

//	vStopSector[nHighest] = vStartSector[nHighest] +(DWORD)MCI_MSF_MINUTE(mciStatusParms.dwReturn) * 4500 +
//					 (DWORD)MCI_MSF_SECOND(mciStatusParms.dwReturn) * 75 +
//					 (DWORD)MCI_MSF_FRAME(mciStatusParms.dwReturn) - vStopSector[0];
	if (nHighest>0)
	{
		vStartSector[nHighest]=vStopSector[nHighest-1]+2;
		vStopSector[nHighest]=150;
	}

	// Set total time
    dwTotalTime = vStartSector[nHighest];

	// Get volume information
    GetVolumeInformation (zDevice,NULL,0,&dwDiscID,NULL,NULL,NULL,0);

    sMCIInfo.lpstrReturn = szMCIReturnString;
	sMCIInfo.dwRetSize = 79;

	// Get the CDPlayer string from the MCI device
    nErr = mciSendCommand (sMCIOpen.wDeviceID, MCI_INFO, MCI_INFO_MEDIA_IDENTITY | MCI_WAIT,(DWORD)&sMCIInfo);
    if (!nErr)
	{
		dwDiscID = _ttol(sMCIInfo.lpstrReturn);
    	LTRACE( _T( "Decimal disc ID %d" ), dwDiscID );
	}

	// Close MCI device
    mciSendCommand(sMCIOpen.wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);

    return 0x0100;
}


BOOL GetMCIToc(int vStartSector[101],int vStopSector[101],int& nTocEntries,DWORD& dwVolumeID)
{
	BOOL	bFound=FALSE;
	int		vASPIStartSector[101];
	int		i;

   	ENTRY_TRACE( _T( "GetMCIToc" ) );

	if (nTocEntries==0)
		return FALSE;

	// Initialze nTocEntries;
	nTocEntries=0;

	// Make a copy of the original start sectors
	memcpy(vASPIStartSector,vStartSector,sizeof(vASPIStartSector));

	// Loop through all CDROM devices
	for (char chDrive='C';(chDrive<='Z') && (bFound==FALSE);chDrive++)
	{
		// Construct root directory drive letter
		CUString strRoot = CUString(chDrive) + _W( ":\\" );

        CUStringConvert strCnv;

		// Get logical drive number
		UINT lDrive = GetDriveType( strCnv.ToT( strRoot ));

		// Check if this is a CDROM-DRIVE
		if (lDrive == DRIVE_CDROM)
		{
			DWORD dwTotalTime=0;
			DWORD dwVolID=0;
			int	nHighest=0;

			LTRACE( _T( "CD-DRIVE %c is a CDROM device" ), chDrive );

			if (MCIGetDiskInfo(chDrive,nHighest,vStartSector,vStopSector,dwTotalTime,dwVolID))
			{
				int nLastTrackLength=0;
//				BOOL bRet=GetMCILastudioTrackLenght(chDrive,nHighest,nLastTrackLength);

				// Check if this is the one we searched for
				// If we have crossed a session border, we get an additional leadout
				// Session lead-out = 1m30s => 6750 sectors
				// Session lead-in  = 1m00s => 4500 sectors
				// Thus a total gap op 6750 + 4500 = 11250 sectors
				if ( 
					( (vASPIStartSector[1]==vStartSector[1]) ||
					  (vASPIStartSector[1]==vStartSector[1]+11250) ))
//				if (vASPIStartSector[1]==vStartSector[1])
				{
					LTRACE( _T( "Found matching TOC for drive %c Tracks from %d to %d" ),
								chDrive,
								0,
								nHighest );
					bFound=TRUE;
					dwVolumeID=dwVolID;
					LTRACE( _T( "Found matching TOC for drive %c " ), chDrive );
					for (i=0;i<=nHighest;i++)
					{
    					LTRACE( _T( "Track %d: Start %d Stop %d" ), 
								i + 1,
								vStartSector[ i ],
								vStopSector[ i ] );
						
					}
    				LTRACE( _T( "Total Lenght: %d" ), dwTotalTime );
					LTRACE( _T( "Disc ID: %08X" ), dwVolumeID );
					nTocEntries = nHighest;
					LTRACE( _T( "NumTOC Entries is : %d" ), nTocEntries );
				} 
			}
		}
	}


   	EXIT_TRACE( _T( "GetMCIToc" ) );

	return bFound;
}


/*

int GetAnalogDiskInfo(char chDriveLetter)
{
	int i;
	int	nLowest=0;
	int	nHighest=0;

	int vStartSector[101],vStopSector[101];
	DWORD dwTotalTime=0;
	DWORD dwDiscID=0;

    if (MCIGetDiskInfo(	chDriveLetter,
							nLowest,nHighest,vStartSector,vStopSector,dwTotalTime,dwDiscID))
	{
	    for (i=nLowest;i<=nHighest;i++)
		{
    		LTRACE( _T( "Track %d: Start %d" ) ,i,vStartSector[i]);
			
	    }
    	LTRACE( _T( "Total Lenght: %d" ), dwTotalTime );
        LTRACE( _T( "Disc ID: %08X" ), dwDiscID );
    } else {
    	if (!nLowest) LTRACE("Usage: TOC <CD-ROM driveletter>");
        else LTRACE("No disc inserted or disc in use by other program");
    }
	return 0;
}
//---------------------------------------------------------------------------

*/



