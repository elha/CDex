#ifndef OSNDSTREAMWAV_H_INCLUDED
#define OSNDSTREAMWAV_H_INCLUDED

#include "OSndStream.h"
extern "C"
{
	#include "sndfile.h"	
}

class OSndStreamWAV:public OSndStream
{
	SNDFILE*	m_pSndFile;
	int			m_OutputFormat;
public:
	OSndStreamWAV();
	virtual ~OSndStreamWAV();

	virtual BOOL	OpenStream( const CUString& strFileName);
	virtual BOOL	CloseStream();
	virtual BOOL	Write(PBYTE pbData,DWORD dwBytes);
	int				GetOutputFormat() const {return m_OutputFormat;}
	void			SetOutputFormat(int nValue) {m_OutputFormat=nValue;}
};




#endif