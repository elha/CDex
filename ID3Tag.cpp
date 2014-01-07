/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006, 2007, 2008 Georgy Berdyshev
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

#include "ID3Tag.h"
#include "config.h"
#include "resource.h"
#include <id3/reader.h>
#include <id3/writer.h>
#include <assert.h>

#include <iostream>
#include <stdio.h>

#include "taglib\fileref.h"
#include "taglib\tag.h"

using namespace std;

#include "taglib\mpeg\mpegfile.h"
#include "taglib\mpeg\id3v2\id3v2tag.h"
#include "taglib\mpeg\id3v2\id3v2frame.h"
#include "taglib\mpeg\id3v2\id3v2header.h"
#include "taglib\mpeg\id3v1\id3v1tag.h"
#include "taglib\mpeg\id3v1\id3v1genres.h"
#include "taglib\mpeg\id3v2\frames\UnknownFrame.h"
#include "taglib\mpeg\id3v2\frames\AttachedPictureFrame.h"
#include "taglib\mpeg\id3v2\frames\TextIdentificationFrame.h"

INITTRACE( _T( "ID3Tag" ) );

// CONSRUCTOR
CID3Tag::CID3Tag( CUString& strFileName )
{
	ENTRY_TRACE( _T( "CID3Tag::CID3Tag() with file name %s" ), strFileName );
	// Initialize the variables
	Init();

	// Assign file name
	m_strFileName=strFileName;

	EXIT_TRACE( _T( "CID3Tag::CID3Tag() with file name %s" ), strFileName );
}

// Init CID3Tag variables
void CID3Tag::Init()
{
	ENTRY_TRACE( _T( "CID3Tag::Init()" ) );

	// Initialize all strings and variables
	m_nTagSize		= 0;

	m_bIsV2			= false;
	m_bIsV1			= false;

	EXIT_TRACE( _T( "CID3Tag::Init()" ) );
}


BOOL CID3Tag::OpenFile()
{
	ENTRY_TRACE( _T( "CID3Tag::OpenFile()" ) );

	// open file as binary for read write
	m_pFile = CDexOpenFile( m_strFileName, _W( "r+b" ) );

	// Oeps, something didn't go well
	if ( NULL == m_pFile )
	{
		LTRACE( _T( "CID3Tag::OpenFile() Could not open file %s" ), m_strFileName );
		LTRACE( _T( "CID3Tag::OpenFile() Las error is %d" ), GetLastError() );
		return FALSE;
	}
	EXIT_TRACE( _T( "CID3Tag::OpenFile()" ) );

	return TRUE;
}

BOOL CID3Tag::OpenFile( CUString strFileName )
{
	m_strFileName=strFileName;

	BOOL bReturn = OpenFile();

	CloseFile();

	return bReturn;
}

void CID3Tag::CloseFile()
{
	if (m_pFile)
		fclose(m_pFile);

	m_pFile = NULL;
}


CUString GetEncodedString( const ID3_Tag& myTag, ID3_FrameID frameId )
{
    CUString returnString = _T( "" );
    ID3_Frame* pFrame = NULL;

    if (pFrame = myTag.Find( frameId ) )
    {
        ID3_Field* fld;

        if ( NULL != (fld = pFrame->GetField( ID3FN_TEXT ) ) )
        {
            ID3_TextEnc enc = fld->GetEncoding();
//#ifdef 
#ifdef _UNICODE
            unicode_t* buffer = NULL;
            fld->SetEncoding( ID3TE_UNICODE );
            size_t nText = fld->Size();
            buffer = new unicode_t[ nText / 2 + 1 ];
            fld->Get( buffer, nText + 1);
            fld->SetEncoding( enc );
            returnString = CUString( (wchar_t*)buffer );
#else
            char *buffer = NULL;
            fld->SetEncoding( ID3TE_ASCII );
            size_t nText = fld->Size();
            buffer = new char[nText + 1];
            fld->Get( buffer, nText + 1);
            fld->SetEncoding(enc);
            returnString = CUString( buffer );
#endif
        }
    }
    return returnString;
}


// LoadTag method
BOOL CID3Tag::LoadTagProperties()
{
	m_nTagSize = 0;

    BYTE v2Header[ 10 ] = {0,};
    BYTE v1Header[ 128 ] = {0,};
    FILE* pFile = CDexOpenFile( m_strFileName, _W( "rb" ) );
    
    m_nTagSize = 0;

    if ( pFile )
    {
        fread( v2Header, sizeof( v2Header ), 1, pFile );

        if (    ( v2Header[ 0 ] == (BYTE)'I' ) &&
                ( v2Header[ 1 ] == (BYTE)'D' ) &&
                ( v2Header[ 2 ] == (BYTE)'3' ) )
        {
            m_nTagSize =    (((long)v2Header[6]) << 21 ) + (((long)v2Header[7]) << 14 ) +
		                (((long)v2Header[8]) << 7) + (long)v2Header[9];
            m_nTagSize += sizeof( v2Header );
        }

        fseek( pFile, -128, SEEK_END );
        fread( v1Header, sizeof( v1Header ), 1, pFile );
        if ( 0 == memcmp( v1Header, "TAG", 3 ) )
        {
            m_bIsV1 = TRUE;
        }
    }
    if ( pFile )
    {
        fclose( pFile );
    }

    if ( m_nTagSize  > 0 )
    {
        m_bIsV2 = TRUE;
    }


//    TagLib::MPEG::File tagFile( GetDosFileName( m_strFileName ) );
//
//    TagLib::ID3v2::Tag *id3v2tag = tagFile.ID3v2Tag();
//    TagLib::ID3v1::Tag *id3v1tag = tagFile.ID3v1Tag();
//
//    if ( id3v1tag )
//    {
//        m_tagData.SetArtist( CUString( id3v1tag->artist().toCString( true ), CP_UTF8 ) );
//        m_tagData.SetTitle( CUString( id3v1tag->title().toCString( true ), CP_UTF8 ) );
//        m_tagData.SetAlbum( CUString( id3v1tag->album().toCString( true ), CP_UTF8 ) );
//        m_tagData.SetTitle( CUString( id3v1tag->title().toCString( true ), CP_UTF8 ) );
//    
//        if ( id3v1tag->year() > 0 )
//        {
//            CUString strYear;
//            strYear.Format( _W("%04d"), id3v1tag->year() );
//            m_tagData.SetYear( strYear );
//        }
//
//        m_tagData.SetComment( CUString( ( id3v1tag->comment() ).toCString( true ), CP_UTF8 ) );
//
//        m_bIsV1 = true;
//    }
//    if ( id3v2tag )
//    {
//        CUString strValue;
//
//        m_tagData.SetArtist( CUString( ( id3v2tag->artist() ).toCString( true ), CP_UTF8 ) );
//        m_tagData.SetTitle( CUString( ( id3v2tag->title() ).toCString( true ), CP_UTF8 ) );
//        m_tagData.SetAlbum( CUString( ( id3v2tag->album() ).toCString( true ), CP_UTF8  ) );
//        m_tagData.SetTitle( CUString( ( id3v2tag->title() ).toCString( true ), CP_UTF8  ) );
//        
//        if ( id3v2tag->year() > 0 )
//        {
//            CUString strYear;
//            strYear.Format( _W("%04d"), id3v2tag->year() );
//            m_tagData.SetYear( strYear );
//        }
//        
//        m_tagData.SetComment( CUString( ( id3v2tag->comment() ).toCString( true ), CP_UTF8 ) );
//
//        m_bIsV2 = true;
//    }
//
//		//	nTmp = ID3_GetGenreNum( &myTag );
//
//		//	if ( nTmp != 0xFF )
//		//	{
//		//		int nIdx = -1;
//
//		//		// Get genre number
//		//		if ( -1 != ( nIdx = g_GenreTable.SearchID3V1ID( nTmp ) ) )
//		//		{
//		//			m_tagData.SetGenre( g_GenreTable.GetGenre( nIdx ) );
//		//		}
//		//		else
//		//		{
//		//			m_tagData.SetGenre( g_language.GetString( IDS_UNKNOWN ) );
//		//		}
//		//	}
//		//	else
//		//	{
//		//		m_tagData.SetGenre( g_language.GetString( IDS_UNKNOWN )  );
//		//	}
//
//		//	pTmp = ID3_GetGenre( &myTag );
//		//	if ( pTmp )	
//		//	{
//		//		m_tagData.SetGenre( pTmp );
//		//	}
//
//		//	pTmp = ID3_GetEncodedBy( &myTag );
//		//	if ( pTmp )	
//		//	{
//		//		m_tagData.SetEncodedBy( pTmp );
//		//	}
//
//		//	pTmp = ID3_GetMCDI( &myTag );
//		//	if ( pTmp )	
//		//	{
//		//		m_tagData.SetMCDI( pTmp );
//		//		// SetRawToc( pTmp );
//		//	}
//
//		//	m_tagData.SetTrackNumber( ID3_GetTrackNum( &myTag  ) );
//
//		//	m_bIsV2 = myTag.HasV2Tag();
//		//	m_bIsV1 = myTag.HasV1Tag();
//
////		}
//
//	// AF TODO
//	//catch(ID3_Error err)
//	//{
//	//	strLang = g_language.GetString( IDS_CANTLOADID3TAG );
//	//	CDexMessageBox( strLang );
//	//	LTRACE( _T( "CID3Tag::SaveTag_v2() Exception Occured! while writing tag into file %s error desciption %s" ),
//	//				m_strFileName, 
//	//				err.GetErrorDesc() );
//	//	return FALSE;
//	//}
	return TRUE;
}

void SetFrame( const CUString& tagValue, ID3_Tag& tag, ID3_FrameID frameId, bool forceAscii = false )
{
    ID3_Frame* pFrame = new ID3_Frame( frameId );

    if ( pFrame )
    {
        CUStringConvert strCnv;
        
        if ( forceAscii || AsciiOnly( strCnv.ToT( tagValue ) ) )
        {
            pFrame->GetField( ID3FN_TEXT )->SetEncoding( ID3TE_ASCII );
            pFrame->GetField( ID3FN_TEXT )->Set( strCnv.ToACP( tagValue ) );
        }
        else
        {
            pFrame->GetField( ID3FN_TEXT )->SetEncoding( ID3TE_UNICODE );
            pFrame->GetField( ID3FN_TEXT )->Set( (unicode_t*)(LPCWSTR)tagValue );
            pFrame->GetField( ID3FN_TEXTENC )->Set( ID3TE_UNICODE );
        }
        tag.AttachFrame( pFrame );
    }
}

unsigned int GetPictureSize( const CUString& strFileName )
{
    FILE* pictureFile = CDexOpenFile( strFileName, _W( "rb" ) );
    unsigned int fileSize = 0;

    if ( NULL != pictureFile )
    {
        fseek( pictureFile, 0, SEEK_END );
        fileSize = (unsigned int)ftell( pictureFile );
        fclose( pictureFile );
    }

    return fileSize;
}

BOOL CID3Tag::SaveTag_v2()
{
    int version = g_config.GetID3Version();
    
    bool id3V1 = ( version & 0x01  ) != 0 ;
    bool id3V23 = ( version & 0x02 ) != 0 ;
    bool id3V24 = ( version & 0x04 ) != 0 ;

    CUStringConvert strCnv;

    if ( id3V24 )
    {
        LTRACE( _T( "CID3Tag::SaveTag_v2() writing tag version 2.4 to file %s" ), strCnv.ToT( m_strFileName ) );

        TagLib::ID3v2::FrameFactory::instance()->setDefaultTextEncoding( TagLib::String::Type::UTF8 );
        TagLib::ID3v2::Tag v2Tag;
        TagLib::ByteVector rendered_tag;

        v2Tag.setArtist( TagLib::String( m_tagData.GetArtist(), TagLib::String::Type::UTF16LE ) );
        v2Tag.setAlbum(  TagLib::String( m_tagData.GetAlbum(), TagLib::String::Type::UTF16LE )  );
        v2Tag.setTitle(  TagLib::String( m_tagData.GetTitle(), TagLib::String::Type::UTF16LE )  );
        v2Tag.setYear( (unsigned int)_wtoi( m_tagData.GetYear() ) );
        v2Tag.setGenre( TagLib::String( m_tagData.GetGenre(), TagLib::String::Type::UTF16LE )  );
        v2Tag.setComment( TagLib::String( m_tagData.GetComment(), TagLib::String::Type::UTF16LE )  );
        v2Tag.setTrack( m_tagData.GetTrackNumber() + m_tagData.GetTrackOffset() );

        TagLib::String frameId( "MCDI" );
        TagLib::ByteVector tocData;
        tocData.setData( (const char*)m_btaRawToc, 804 );

        TagLib::ID3v2::UnknownFrame* myFrame = new TagLib::ID3v2::UnknownFrame( frameId.data( TagLib::String::Type::Latin1 ) );
        myFrame->setFieldata( tocData );
        v2Tag.addFrame( myFrame );

        CUString pictureComment = g_config.GetTagPictureComment();
        CUString pictureFileName = g_config.GetTagPictureFile();
        BOOL bAddPicture = g_config.GetAddTagPicture();

        unsigned int fileSize = GetPictureSize( pictureFileName );

        FILE* pictureFile = CDexOpenFile( pictureFileName, _W( "rb" ) );

        if ( bAddPicture && ( pictureFile != NULL ) )
        {
            TagLib::ID3v2::AttachedPictureFrame* picFrame = new TagLib::ID3v2::AttachedPictureFrame();
            picFrame->setDescription(  TagLib::String( m_tagData.GetGenre(), TagLib::String::Type::UTF16LE ) );

            // get the picture data
            TagLib::ByteVector pictureData( fileSize );

            fread( &pictureData[ 0 ], fileSize, 1, pictureFile );
            
            
            fclose( pictureFile );



            if (    ( pictureFileName.Find( _W( ".jpg" ) ) > 0 ) ||
                    ( pictureFileName.Find( _W( ".jpeg" ) ) > 0 ) )
            {
                picFrame->setMimeType( "image/jpg" );
            } else if ( pictureFileName.Find( _W( ".png" ) ) )
            {
                picFrame->setMimeType( "image/png" );
            } else
            {
                picFrame->setMimeType( "image/unknown" );
            }

            // picFrame->FrontCover();
            picFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
            picFrame->setPicture( pictureData );
            v2Tag.addFrame( picFrame );
        }

		if ( m_tagData.GetLengthInMs().GetLength() > 0 )
		{
            TagLib::ID3v2::TextIdentificationFrame * textFrame = new TagLib::ID3v2::TextIdentificationFrame( "TLEN", TagLib::String::Latin1 );
            textFrame->setText( TagLib::String( m_tagData.GetLengthInMs(), TagLib::String::Type::UTF16LE ) );
            v2Tag.addFrame( textFrame );
		}


        rendered_tag = v2Tag.render();
        
        char* data = rendered_tag.data();
        int size = rendered_tag.size();

        FILE* pFile = NULL;

        // AF TODO CHECK IF TAG IS NOT LARGER THAN INITIAL PADDING SIZE
        pFile = CDexOpenFile( m_strFileName, _W( "rb+" ) );

        fwrite( data, size, 1, pFile );

        fclose( pFile );

    }

    if ( id3V23 )
    {
        LTRACE( _T( "CID3Tag::SaveTag_v2() writing tag version 2.3 to file %s" ), strCnv.ToT( m_strFileName ) );

        // get short path name
        ID3_Tag myTag;

        myTag.Link( GetDosFileName( m_strFileName ) );

		myTag.SetPadding( true );

        SetFrame( m_tagData.GetArtist(), myTag, ID3FID_LEADARTIST );
        SetFrame( m_tagData.GetAlbum(), myTag, ID3FID_ALBUM );
        SetFrame( m_tagData.GetTitle(), myTag, ID3FID_TITLE );
        SetFrame( m_tagData.GetYear(), myTag, ID3FID_YEAR );
        SetFrame( m_tagData.GetComment(), myTag, ID3FID_COMMENT );
        SetFrame( m_tagData.GetEncodedBy(), myTag, ID3FID_ENCODEDBY );

        ID3_Frame frame;
            
        frame.SetID( ID3FID_CDID );

        frame.Field( ID3FN_DATA ).Set(	m_tagData.GetRawToc(), 804 );
        myTag.AddFrame( frame );

        ID3_AddTrack( &myTag, (uchar)m_tagData.GetTrackNumber() +  (uchar)m_tagData.GetTrackOffset(), (uchar)m_tagData.GetTotalTracks() + (uchar)m_tagData.GetTrackOffset(), true );

        CUString strTrack;
        switch ( g_config.GetID3V2TrackNumber() )
		{
			case 0: strTrack.Format( _W("%lu"), (luint) m_tagData.GetTrackNumber() + m_tagData.GetTrackOffset() ); break;
			case 1: strTrack.Format( _W("%lu/%lu"), (luint) m_tagData.GetTrackNumber(), (luint) m_tagData.GetTotalTracks() + m_tagData.GetTrackOffset() ); break;
			case 2: strTrack.Format( _W("%02lu"), (luint) m_tagData.GetTrackNumber() + m_tagData.GetTrackOffset() ); break;
			case 3: strTrack.Format( _W("%02lu/%02lu"), (luint) m_tagData.GetTrackNumber() + m_tagData.GetTrackOffset(), (luint) m_tagData.GetTotalTracks() + m_tagData.GetTrackOffset() ); break;
			default:
				assert( false );
        }

        CUString pictureComment = g_config.GetTagPictureComment();
        CUString pictureFileName = g_config.GetTagPictureFile();
        BOOL bAddPicture = g_config.GetAddTagPicture();

        if ( bAddPicture && CDexFileExist( pictureFileName ) )
        {
            frame.SetID(ID3FID_PICTURE);
            if (    ( pictureFileName.Find( _W( ".jpg" ) ) > 0 ) ||
                    ( pictureFileName.Find( _W( ".jpeg" ) ) > 0 ) )
            {
                frame.GetField(ID3FN_MIMETYPE)->Set( "image/jpeg" );
            } else if ( pictureFileName.Find( _W( ".png" ) ) )
            {
                frame.GetField(ID3FN_MIMETYPE)->Set("image/png");
            } else
            {
                frame.GetField(ID3FN_MIMETYPE)->Set("image/unknown");
            }
            frame.GetField(ID3FN_PICTURETYPE)->Set(03);
            frame.GetField(ID3FN_DESCRIPTION)->Set( strCnv.ToACP( pictureComment ) );
            frame.GetField(ID3FN_DATA)->FromFile( strCnv.ToACP( pictureFileName ) );
            
            myTag.AddFrame( frame );
        }

		// set genre string for V2 tag
        SetFrame( strCnv.ToT( m_tagData.GetGenre() ), myTag, ID3FID_CONTENTTYPE );

		if ( m_tagData.GetLengthInMs().GetLength() > 0 )
		{
            SetFrame( strCnv.ToT( m_tagData.GetLengthInMs() ), myTag, ID3FID_SONGLEN );
		}

		// Update V2 tag
		myTag.Update( ID3TT_ID3V2 );


	}
    if ( id3V1 )
    {
        LTRACE( _T( "CID3Tag::SaveTag_v2() writing tag version 1.0 to file %s" ), strCnv.ToT( m_strFileName ) );

        // get short path name
        ID3_Tag myTag;

        myTag.Link( GetDosFileName( m_strFileName ) );

        SetFrame( m_tagData.GetArtist(), myTag, ID3FID_LEADARTIST, true );
        SetFrame( m_tagData.GetAlbum(), myTag, ID3FID_ALBUM, true );
        SetFrame( m_tagData.GetTitle(), myTag, ID3FID_TITLE, true );
        SetFrame( m_tagData.GetYear(), myTag, ID3FID_YEAR, true );
        SetFrame( m_tagData.GetComment(), myTag, ID3FID_COMMENT, true );

        ID3_AddTrack( &myTag, (uchar)( m_tagData.GetTrackNumber() + m_tagData.GetTrackOffset() ) , (uchar)( m_tagData.GetTotalTracks() + m_tagData.GetTrackOffset() ), true );

        BYTE nGenreIdx = g_GenreTable.SearchGenre( m_tagData.GetGenre() );
		BYTE nGenre    = g_GenreTable.GetID3V1ID( nGenreIdx );

		// set genre number for V1 tag
		ID3_AddGenre( &myTag, nGenre, true );

		// Update V1 tag
		myTag.Update( ID3TT_ID3V1 );

		// strip V2 tag if it is not requested
		if ( ( !id3V23 && !id3V24 ) && myTag.HasV2Tag() ) 
		{
			myTag.Strip( ID3TT_ID3V2 );
		}
    }
	return TRUE;
}



FILE* CID3Tag::SaveInitialV2Tag( const CUString& strFileName, DWORD dwPadSize )
{
    // create output file
	FILE* pFile = CDexOpenFile( strFileName, _W( "wb+" ) );

	if ( ( NULL != pFile ) && ( dwPadSize > 10 ) )
	{
        CUString pictureFileName = g_config.GetTagPictureFile();
        BOOL bAddPicture = g_config.GetAddTagPicture();

        // reserve extra tag space if picture has to be attached
        if ( bAddPicture )
        {
            unsigned int pictureSize = GetPictureSize( pictureFileName );
            dwPadSize += pictureSize;
        }

        DWORD	dwTagSize = dwPadSize - 10;
		char	strHeader[10] = {'\0',};
		unsigned int		i = 0;

		// Tag identification
		strHeader[0] = 'I';
		strHeader[1] = 'D';
		strHeader[2] = '3';
	
		// Version number
		strHeader[3] = 3;
		strHeader[4] = 0;

		// Clear Flags byte
		strHeader[5] = 0;

		// Write tag length
		strHeader[6] = (char)((dwTagSize >> 21) & 0x7F );
		strHeader[7] = (char)((dwTagSize >> 14) & 0x7F );
		strHeader[8] = (char)((dwTagSize >>  7) & 0x7F );
		strHeader[9] = (char)((dwTagSize      ) & 0x7F );

		// Write header  
		fwrite( strHeader, sizeof( strHeader ), 1, pFile );

		// Write padding data
		for ( i=0;i< dwTagSize; i++ )
		{
			strHeader[0] = 0;
			fwrite( strHeader, 1, 1, pFile );
		}
	}
	return pFile;
}


BOOL CID3Tag::SaveTag( )
{
	return SaveTag_v2();
}

BOOL CID3Tag::CopyInitialV2Tag( 
                        const CUString& strSrc,
						const CUString& strDst )
{
    // read source file, and determine if there is a ID3V2 Tag
    BYTE v2Header[ 10 ];
    FILE* pFile = CDexOpenFile( strSrc, _W( "rb" ) );
    
    long v2Size = 0;

    if ( pFile )
    {
        fread( v2Header, sizeof( v2Header ), 1, pFile );

        if (    ( v2Header[ 0 ] == (BYTE)'I' ) &&
                ( v2Header[ 1 ] == (BYTE)'D' ) &&
                ( v2Header[ 2 ] == (BYTE)'3' ) )
        {
            v2Size =    (((long)v2Header[6]) << 21 ) + (((long)v2Header[7]) << 14 ) +
		                (((long)v2Header[8]) << 7) + (long)v2Header[9];
        }

        if ( v2Size > 0 )
        {
            FILE* pOutFile = CDexOpenFile( strDst, _W( "wb" ) );

            if ( pOutFile )
            {
                BYTE copyBuffer[ 1024 ];

		        // Write header  
                fwrite( &v2Header[0], sizeof( v2Header ), 1, pOutFile );

                long bytesCopyLeft = v2Size;
                do
                {
                    long bytesCopy = min( bytesCopyLeft, sizeof( copyBuffer ) );
                    fread( copyBuffer, 1, bytesCopy, pFile ); 
                    fwrite( copyBuffer, 1, bytesCopy, pOutFile ); 
                    bytesCopyLeft -= bytesCopy;
                } while ( bytesCopyLeft > 0 );

                fclose( pOutFile );
            }
        }
        fclose( pFile );
    }
    return TRUE;
}


BOOL CID3Tag::CopyTags( const CUString& strSrc,
						const CUString& strDst,
                        BOOL  copyV1Tag,
                        BOOL  copyV2Tag )
{
    CUString strTmp;
    
    strTmp =  strDst + CUString( _W( ".tagged" ) );

    if ( copyV2Tag )
    {
        CopyInitialV2Tag( strSrc, strTmp );

        FILE* pFileDst = CDexOpenFile( strDst,  _W( "rb" ) );
        FILE* pFileTmp = CDexOpenFile( strTmp,  _W( "rb+" ) );

        if ( pFileDst && pFileTmp )
        {
            fseek( pFileTmp, 0, SEEK_END );
            BYTE copyBuffer[1024];
            DWORD bytesRead = 0;
            do
            {
                bytesRead = fread( copyBuffer, 1, sizeof( copyBuffer ), pFileDst ); 
                fwrite( copyBuffer, 1, bytesRead, pFileTmp ); 
            } while ( bytesRead > 0 );
        }

        if ( pFileDst ) 
        {
            fclose( pFileDst );
        }

        if ( pFileTmp ) 
        {
            fclose( pFileTmp );
        }

        CDexDeleteFile( strDst );
        CDexMoveFile( strTmp, strDst );
    }
    if ( copyV1Tag )
    {
        CopyInitialV2Tag( strSrc, strTmp );

        FILE* pFileSrc = CDexOpenFile( strSrc,  _W( "rb" ) );
        FILE* pFileDst = CDexOpenFile( strDst,  _W( "rb+" ) );

        if ( pFileDst && strSrc )
        {
            BYTE v1Header[ 128 ];

            fseek( pFileDst, 0, SEEK_END );
            fseek( pFileSrc, -128, SEEK_END );

            fread( v1Header, sizeof( v1Header ), 1, pFileSrc );
            fwrite( v1Header, sizeof( v1Header ), 1, pFileDst );
        }

        if ( pFileDst ) 
        {
            fclose( pFileDst );
        }

        if ( pFileSrc ) 
        {
            fclose( pFileSrc );
        }

    }

	return TRUE;
}


