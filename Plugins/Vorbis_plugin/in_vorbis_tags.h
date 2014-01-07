#ifndef IN_VORBIS_TAGS_H_INCLUDED
#define IN_VORBIS_TAGS_H_INCLUDED

#define VORBIS_TAG_TITLE			"TITLE"
#define VORBIS_TAG_ARTIST			"ARTIST"
#define VORBIS_TAG_ALBUM			"ALBUM"
#define VORBIS_TAG_DATE				"DATE"
#define VORBIS_TAG_COMMENT			"COMMENT"
#define VORBIS_TAG_TRACKNUMBER		"TRACKNUMBER"
#define VORBIS_TAG_RPGAIN_AG		"REPLAYGAIN_ALBUM_GAIN"
#define VORBIS_TAG_RPGAIN_AG_ALT    "RG_AUDIOPHILE"
#define VORBIS_TAG_RPGAIN_TG		"REPLAYGAIN_TRACK_GAIN"
#define VORBIS_TAG_RPGAIN_TG_ALT	"RG_RADIO"




BOOL in_vorbis_tags_is_standard_tag( const PCHAR pszTag );

//typedef const TCHAR* (*TAGFUNC)(const TCHAR *tag,void *p);    // return 0 if not found
//typedef void (*TAGFREEFUNC)(const TCHAR *tag,void *p);


//UINT tagz_format(const TCHAR * spec,TAGFUNC f,TAGFREEFUNC ff,void *fp,T_CHAR * out,UINT max);
//T_CHAR * tagz_format_r(const TCHAR * spec,TAGFUNC f,TAGFREEFUNC ff,void * fp);

extern const char in_vorbis_tags_manual[];


#endif
