/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2002 - 2007 Albert L. Faber
** Copyright (C) 2002 Johan Pouwelse
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


#include <AfxSock.h>
#include "stdafx.h"
#include "resource.h"
#include "CdInfo.h"
#include "musicbrainz.h"
extern "C" {
	#include "Musicbrainz/sha1.h"
}

// PROBLEM: 
// 
// CDex does not yet has native support for both
// single artist CDs and multiple artist CDs (compilations)
// Within MusicBrainz every track can have a different artist.
// We now use a simple "Track_name - Artist_name" solution to
// be backwards compatible. The best approach would be to add support for
// compilation CDs into CDInfo

INITTRACE( _T( "MusicBrainz" ) );

unsigned char *rfc822_binary (void *src,unsigned long srcl,unsigned long *len)
{
  unsigned char *ret,*d;
  unsigned char *s = (unsigned char *) src;
  char *v = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._";
  unsigned long i = ((srcl + 2) / 3) * 4;
  *len = i += 2 * ((i / 60) + 1);
  d = ret = (unsigned char *) malloc ((size_t) ++i);
  for (i = 0; srcl; s += 3) {	/* process tuplets */
    *d++ = v[s[0] >> 2];	/* byte 1: high 6 bits (1) */
				/* byte 2: low 2 bits (1), high 4 bits (2) */
    *d++ = v[((s[0] << 4) + (--srcl ? (s[1] >> 4) : 0)) & 0x3f];
				/* byte 3: low 4 bits (2), high 2 bits (3) */
    *d++ = srcl ? v[((s[1] << 2) + (--srcl ? (s[2] >> 6) : 0)) & 0x3f] : '-';
				/* byte 4: low 6 bits (3) */
    *d++ = srcl ? v[s[2] & 0x3f] : '-';
    if (srcl) srcl--;		/* count third character if processed */
    if ((++i) == 15) {		/* output 60 characters? */
      i = 0;			/* restart line break count, insert CRLF */
      *d++ = '\015'; *d++ = '\012';
    }
  }
  *d = '\0';			/* tie off string */

  return ret;			/* return the resulting string */
}


// CONSTRUCTOR
CMusicBrainz::CMusicBrainz( CDInfo*	pCDInfo ) 
{
	LTRACE( _T( "Entering CMusicBrainz::CMusicBrainz( )" ) );

	m_pMusicBrainz = NULL;
	m_pCDInfo = pCDInfo;

	// Create the musicbrainz object, which will be needed for subsequent calls
	m_pMusicBrainz = mb_New();



	// get version info
	int versionMajor =0;
	int versionMinor =0;
	int versionRevision =0;

	mb_GetVersion(	m_pMusicBrainz, 
					&versionMajor, 
					&versionMinor, 
					&versionRevision );

	LTRACE( _T( "mb_GetVersion Major:%d Minor:%d Revision:%d" ),
		versionMajor,
		versionMinor,
		versionRevision );

	mb_WSAInit( m_pMusicBrainz );

    if ( g_config.GetCDDBUseProxy() )
    {
        CUString strProxy = g_config.GetProxyAddress();
        CUStringConvert strCnv;

        mb_SetProxy(    m_pMusicBrainz, 
                        strCnv.ToACP( strProxy ),  
                        g_config.GetCDDBProxyPort() );
    }
    
	m_url[0]='\0';
	
	LTRACE( _T( "Leaving CMusicBrainz::CMusicBrainz( )" ) );
}


// DESTRUCTOR
CMusicBrainz::~CMusicBrainz()
{
	LTRACE( _T( "Entering CMusicBrainz::~CMusicBrainz( )" ) );

	// Close connection if necessary and clean up the musicbrainz object
	if ( NULL != m_pMusicBrainz ) 
	{
		mb_WSAStop( m_pMusicBrainz );
		mb_Delete( m_pMusicBrainz );
		m_pMusicBrainz = NULL;
	}

	LTRACE( _T( "Leaving CMusicBrainz::!CMusicBrainz( )" ) );
}

// Copy the data from the m_pMusicBrainz object into CDInfo.
// Use calls such as m_pCDInfo->SetTitle.
// Here the actual conversion takes place for compilation CDs, see PROBLEM: at the top*/
BOOL CMusicBrainz::CopyAllResults()
{
	char     data[257],temp[257];     //truncate looooong names
	int      i,numTracks, isMultipleArtist=0;

	CUString strData;

	LTRACE( _T( "Entering CMusicBrainz::CopyAllResults( )" ) );

	// Extract the album name
	if (!mb_GetResultData(m_pMusicBrainz, MBE_AlbumGetAlbumName, data, 256))
    {
//	    return FALSE;
    }

	m_pCDInfo->SetTitle( CUString( data, CP_UTF8 ) );

	// Extract the number of tracks
	numTracks = mb_GetResultInt(m_pMusicBrainz, MBE_AlbumGetNumTracks);


	// Check to see if there is more than one artist for this album
	for(i = 1; i <= numTracks; i++)
	{
		if (!mb_GetResultData1(m_pMusicBrainz, MBE_AlbumGetArtistId, data, 256, i))
			return FALSE;

		if (i == 1)
		{
		   strcpy(temp, data);
		}

		if (strcmp(temp, data))
		{
			isMultipleArtist = 1;
			break;
		}
	}

	if (!isMultipleArtist)
	{
		// Extract the artist name from the album
		if (!mb_GetResultData1(m_pMusicBrainz, MBE_AlbumGetArtistName, data, 256, 1)) {
			return FALSE;
		}
		strData = CUString( data, CP_UTF8 );
	} 
	else 
	{
		//JOHAN: ToDo, remove this notice for final version.
		//strData = "Multiple artists (using MusicBrainz Beta support)";
		strData = _T( "Multiple artists CD" );
	}

	m_pCDInfo->SetArtist( strData );

	for(i = 1; i <= numTracks; i++)
	{
		// If its a multple artist album, print out the artist for each track
		if (isMultipleArtist)
		{
			// Extract the track name from the album.
			if (!mb_GetResultData1(m_pMusicBrainz, MBE_AlbumGetTrackName, data, 256, i))
				return FALSE;

			// Extract the artist name from this track
			if (!mb_GetResultData1(m_pMusicBrainz, MBE_AlbumGetArtistName, temp, 256, i))
				return FALSE;

			// trackname is ARTIST - TRACK format;
			strData = CUString( temp, CP_UTF8 );
			strData += _W( " - " );
			strData += CUString( data, CP_UTF8 );

		} 
		else 
		{
			// SingleArtist Album
			// Only extract the track name from the album.
			if (!mb_GetResultData1(m_pMusicBrainz, MBE_AlbumGetTrackName, data, 256, i))
				return FALSE;

			strData = CUString( data, CP_UTF8 );
		}
		m_pCDInfo->SetTrackName( strData, i-1 );
	}

	LTRACE( _T( "Leaving CMusicBrainz::CopyAllResults( )" ) );
	return TRUE;
}

/* 
Calculate the CDIndex key from the CDROM table-of-contents.
Function such as "m_pCDInfo->GetEndSector()" are used.
This is a more advanced version to the CDDB/FreeDB method.
On-line documentation of the algorithm: http://www.musicbrainz.org/disc.html
The following CDIndex=9_5psTIZoJlEzw_sPrNg1R5IhO8-
is generated from this toc=1+11+222420+150+18755+39177+56237+78452+98462+114900+131900+153530+172857+197627
This test CD is by "U2", called "All That You Can't Leave Behind".
*/
BOOL CMusicBrainz::GenerateId(char DiscId[33])
{
   SHA_INFO       sha;
   unsigned char  digest[20], *base64;
   unsigned long  size=0;
   char           temp[9];
   int            i;

   LTRACE( _T( "Entering CMusicBrainz::GenerateId( )" ) );
   // housekeeping...
   sha_init(&sha);

   // Before we ran the hash on the binary data, but now to make
   // sure people on both types of endian systems create the same
   // keys, we convert the data to hex-ASCII first. :-)
   sprintf(temp, "%02X", m_pCDInfo->FirstTrack()+1);    // CDex counts from 1; thus +1
   sha_update(&sha, (unsigned char *)temp, strlen(temp));
   LTRACE( _T( "Inside CMusicBrainz::GenerateId( ), first track %d" ), m_pCDInfo->FirstTrack()+1);

   sprintf(temp, "%02X", m_pCDInfo->GetNumTracks());  // Num track in CDex equal last track num in MusicBrainz counting from 1..last
   sha_update(&sha, (unsigned char *)temp, strlen(temp));
   LTRACE( _T( "Inside CMusicBrainz::GenerateId( ), last track %d" ), m_pCDInfo->GetNumTracks() );

   // Do the lead out track first, as required by the CDIndexID standard
   // The magical number of 150 is added to account for the assumption inside the MusicBrainz lib
   // that the flawed MicroSoft "mciSendString()" function is used to generate the CDIndexIDs.
   // mciSendString() begins to count sectors at 150...
   // The +1 is added because the CDIndexID algorithm requires leadout start instead of the EndSector
   sprintf(temp, "%08lX", m_pCDInfo->GetEndSector(m_pCDInfo->GetNumTracks())+150+1);
   sha_update(&sha, (unsigned char *)temp, strlen(temp));
   LTRACE( _T( "Inside CMusicBrainz::GenerateId( ), leadout track %d" ), m_pCDInfo->GetEndSector(m_pCDInfo->GetNumTracks())+150+1);

   for(i = 1; i < 100; i++)
   {
       if (i > m_pCDInfo->GetNumTracks()) {
           sprintf(temp, "%08lX", 0);
           LTRACE( _T( "Inside CMusicBrainz::GenerateId( ), track %d offset %d" ), i, 0);
       } else {
         // See above for magical 150...
         sprintf(temp, "%08lX", m_pCDInfo->GetStartSector(i)+150);
         LTRACE( _T( "Inside CMusicBrainz::GenerateId( ), track %d offset %d" ), i, m_pCDInfo->GetStartSector(i)+150);
       }
       sha_update(&sha, (unsigned char *)temp, strlen(temp));
   }
   sha_final(digest, &sha);

   // Convert to a readable format
   base64 = rfc822_binary(digest, 20, &size);
   memcpy(DiscId, base64, size);
   DiscId[size] = 0;

   //JOHAN TODO: Apparently the we cannot free the space because it came from a
   // .dll call. Just leave it out for now, needs further investigation.
   //free(base64);


   LTRACE( _T( "Leaving CMusicBrainz::GenerateId( )" ) );
   return TRUE;
}

/* The main function of the wrapper around the MusicBrainz.dll, does the actual
call to the server. */
BOOL CMusicBrainz::GetInfoFromServer()
{
	char *args[2], data[257];

	// Calculate the unique CDIndexID as a key to the CD description
	GenerateId( data );

	args[0]=data;
	args[1]=NULL;


#ifdef _DEBUG_TEST
	// TEST CODE
    // Query album _T( "" );
	strcpy( data, "D0av.zIIUwZb2JjBkyDl_GieICU-" );
	// END TEST CODE	
#endif

    // Contact the server, this fills the m_pMusicBrainz object with return info
	// that must be processed by Select and GetResult functions

    if(!mb_QueryWithArgs(m_pMusicBrainz, MBQ_GetCDInfoFromCDIndexId, args))
    {
		return FALSE;
    }
    
	 

	// Select the first album
	mb_Select1(m_pMusicBrainz, MBS_SelectAlbum, 1);  

	// Pull back the album id to see if we got an/the album
	if (!mb_GetResultData(m_pMusicBrainz, MBE_AlbumGetAlbumId, data, 256))
		return FALSE;

	// Copy the server response to CDInfo
	return CopyAllResults();
}

