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


#ifndef ID3TAGINCLUDED
#define ID3TAGINCLUDED

#include "Config.h"
#include <id3.h>
#include <id3/tag.h>
#include <id3/misc_support.h>
#include "TagData.h"

#pragma pack(push,1)

#define FORMAT_LEN 512
#define STD_BUF_LEN 2048
#define PLAY_BUF_LEN 4096
#define DECODER "l3dec"
#define DECODEOPTS "-sti","-sto","-sa"
#define AUDIO "/dev/dsp"
#define FILENAME_LEN 256

/* MPEG Header Definitions - Mode Values */

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

#define         MPG_MD_LR_LR             0
#define         MPG_MD_LR_I              1
#define         MPG_MD_MS_LR             2
#define         MPG_MD_MS_I              3



// ID3 TAG Format, as it is stored in the MP3 files
// Warning: No zero terminiated strings!, they are just fixed
typedef struct 
{
    char	tag[3];
    char	title[30];
    char	artist[30];
    char	album[30];
    char	year[4];
    char	comment[30];
    BYTE	genre;

} ID3FILETAG;


class CID3Tag 
{
public:

	// CONSTRUCTOR
	CID3Tag(CUString& strFileName);
	CID3Tag() {};

	// MUTATORS
	BOOL	LoadTagProperties();
	BOOL	SaveTag_v1();
	BOOL	SaveTag_v2();
	BOOL	SaveTag( );

	static FILE* SaveInitialV2Tag( const CUString& strFileName, DWORD dwPadSize );
	static BOOL  CopyTags( const CUString& strSrc,const CUString& strDst, BOOL  copyV1Tag, BOOL  copyV2Tag );

	BOOL	OpenFile();
	BOOL	OpenFile( CUString strFileName );
	void	CloseFile();

    size_t	GetTagSize() const { return m_nTagSize; }

    void    SetTagData( const CTagData& value ) { m_tagData = value; }
    CTagData GetTagData() const { return m_tagData;}
    CTagData& GetTagDataReference() { return m_tagData;}

	void Init( );

	void SetTagSize( size_t nValue )				{ m_nTagSize = nValue; }
	void SetCommentDescr( const CUString& strNew )	{ m_strCommentDescr = strNew; }
    static BOOL CopyInitialV2Tag( const CUString& strSrc, const CUString& strDst );

	bool IsV2Tag() { return m_bIsV2; }
	bool IsV1Tag() { return m_bIsV1; }

	void IsV1Tag( bool bValue ) { m_bIsV1 = bValue; }
	void IsV2Tag( bool bValue ) { m_bIsV2 = bValue; }

private:
	CUString	m_strFileName;
	FILE*	m_pFile;
	size_t	m_nTagSize;
	CUString	m_strCommentDescr;
	bool	m_bIsV2;
	bool	m_bIsV1;
	CUString	m_strLengthInMs;
	RAW_TOC_TYPE	m_btaRawToc;
    CTagData    m_tagData;
};



#if 0
class Layer 
{
public:
	char *mode_name();
	char *layer_name();
	char *version_name();
	char *version_num();
	unsigned int bitrate();
	unsigned int sfreq();
	BOOL get(FILE * file);
	int version;
	int lay;
	int error_protection;
	int bitrate_index;
	int sampling_frequency;
	int padding;
	int extension;
	int mode;
	int mode_ext;
	int copyright;
	int original;
	int emphasis;
	int stereo;
};


class Mpfile {
    public:
    Mpfile();
    ~Mpfile();
    BOOL mpopen(char *name, BOOL write);
    void mpclose();
    BOOL initfile(BOOL gettag);
    void calc();
    BOOL mpformat(char *dest, char *format, size_t maxlen, int canslash);
    BOOL frename(char *format);
    BOOL execute(char *format);
    BOOL writetag();
    BOOL wipetag();
#ifdef PLAYER
    BOOL play(int threads);
#endif
    void tagcopy(Tag * from);

    Tag *tag;
    Layer *layer;
    BOOL hastag;
    BOOL haslayer;

    protected:
    BOOL openwrite;
    BOOL parseif(char what);
    FILE *file;
    size_t filelen;
    char filename[FILENAME_LEN];
    unsigned long length;
};

#endif

/* From tag.c */
//extern char *genres[];
//extern int genre_largest;

#pragma pack(pop)

#endif