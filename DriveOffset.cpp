/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2006 Albert L. Faber
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


#include "stdafx.h"
#include ".\driveoffset.h"
#include "Util.h"

DriveOffset::DriveOffset(void)
{
    FILE* file = CDexOpenFile( _W( "cdrom_drive_offsets.txt" ) , _W( "r" ) );

    if ( NULL == file )
    {
        file = CDexOpenFile( _W( "..\\cdrom_drive_offsets.txt" ) , _W( "r" ) );
    }

    if ( NULL != file )
    {
        char line[ 255 ] = {'\0',};

        CUString strLine;

        while ( NULL != fgets( line, sizeof(line) - 1, file ) )
        {
			if ( line[ strlen(line) - 1 ] == 0x0A )
			{
                line[ strlen(line) - 1 ] = '\0';
            }

            if ( line[0] != '#' )
            {
                DRIVEOFFSETTABLE newEntry;

                newEntry.deviceName = _T( "" );
                newEntry.driveOffset = 0;
                newEntry.submits = 0;
                newEntry.confidenceFactor = 0;

                char* token = strtok( line, "\t" );
                
                DWORD tokens = 0;

                if ( NULL != token )
                {
                    newEntry.deviceName = CUString( token, CP_UTF8 );

                    tokens++;
                    while ( NULL != token )
                    {
                        token = strtok( NULL, "\t" );

                        if ( NULL != token )
                        {
                            switch ( tokens )
                            {
                                case 1:
                                    newEntry.driveOffset = atoi( token );
                                    break;
                                case 2:
                                    newEntry.submits = atoi( token );
                                    break;
                                case 3:
                                    newEntry.confidenceFactor = atoi( token );
                                    break;
                                default:
                                    ASSERT( FALSE );
                                    break;
                            }
                            tokens++;
                        }
                    }
                    driveOffsetTable.push_back( newEntry );
                }
            }
        }
    }
}

DriveOffset::~DriveOffset(void)
{
}


bool DriveOffset::GetDriveOffset( CUString& deviceName, LONG& driveOffset )
{
    bool foundDevice = false;
    DWORD index = 0;

    driveOffset = 0;

    CUString search1 = deviceName;

//    CUString search2 = 
    search1.Replace( _W("-"), _W( " " ) );
    search1.Replace( _W("/"), _W( " " ) );
    search1.Replace( _W(" "), _W( "" ) );
    search1.Trim();

    for ( index = 0; index < driveOffsetTable.size(); index++ )
    {
        CUString search2 = driveOffsetTable[ index ].deviceName;
        
        search2.Replace( _W("-"), _W( " " ) );
        search2.Replace( _W("/"), _W( " " ) );
	    search2.Replace( _W(" "), _W( "" ) );
        search2.Trim();

        if ( search1 == search2 )
        {
            foundDevice = true;
            driveOffset = driveOffsetTable[ index ].driveOffset;
            break;
        }
    }
    return foundDevice;
}
