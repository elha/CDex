#include "stdafx.h"
#include "UString.h"
#include <assert.h>
#include <WCHAR.H>
#include <stdio.h>

// #define DIM(x) ( sizeof((x)) / sizeof((x)[0]) )

//////////////////////////////////////////////////////////////////////
//
// Constructors/Destructor
//
//////////////////////////////////////////////////////////////////////

CUString::CUString()
{
    m_bufferSize = 0;
	AllocString(0);
}

CUString::CUString(const CUString& str)
{
    m_bufferSize = 0;
    m_pszString = NULL;
    int nLen = str.GetLength();
    AllocString(nLen);
    wcsncpy( m_pszString, (LPCWSTR)str, nLen );
}

CUString::CUString(LPCWSTR wideString)
{
    m_bufferSize = 0;
	int nLen = wcslen( wideString );
	AllocString( nLen );
    wcsncpy( m_pszString, wideString, nLen );
}

CUString::CUString( LPCSTR string, int codePage )
{
    m_bufferSize = 0;

	int nLen = strlen( string );

    AllocString( nLen );

    if ( codePage == -1 )
    {
        for ( int i=0;i<nLen;i++ )
        {
            m_pszString[i] = string[i]; 
        }
    }
    else
    {
        // convert from ANSI code page to wide string
        MultiByteToWideChar( codePage, 0, string, -1, m_pszString , nLen );
    }
}

CUString::CUString( LPCWSTR string, int codePage )
{
    m_bufferSize = 0;

	int nLen = wcslen( string );

    CHAR* tmpString = new CHAR[ nLen + 1 ];

    memset( tmpString, 0, nLen + 1 );

    for ( int i=0;i<nLen; i++ )
    {
        tmpString[ i ] = (CHAR)string[ i ];
    }

    AllocString( nLen );

    // convert from ANSI code page to wide string
    MultiByteToWideChar( codePage, 0, tmpString, -1, m_pszString , nLen );
    
    delete [] tmpString;
}

CUString::CUString( WCHAR wch )
{
    m_bufferSize = 0;
	AllocString( 1 );
	m_pszString[ 0 ] = wch;
}



CUString::~CUString()
{
    if (m_bufferSize > 0){
        free (m_pszString);
    }
}


//////////////////////////////////////////////////////////////////////
//
// Memory Manipulation
//
//////////////////////////////////////////////////////////////////////

void CUString::AllocString(int nLen){
    assert(nLen >= 0);

    if ( m_bufferSize == 0 )
    {
        m_bufferSize = nLen + 1;
        m_pszString = (WCHAR*) calloc( m_bufferSize, sizeof(WCHAR) );
    }
    else
    {
        if ( (( nLen + 1 - (int)m_bufferSize )) > 0 )
        {
            m_bufferSize = max( 2 * m_bufferSize, (unsigned int)(nLen + 1 ) );
            m_pszString = (WCHAR*) realloc( m_pszString, m_bufferSize * sizeof(WCHAR));
            memset( &m_pszString[ m_bufferSize / 2 ], 0, (m_bufferSize / 2) * sizeof(WCHAR) );
        }
    }

    m_pszString[ nLen ] = '\0';
}

void CUString::StringCopy(int nSrcLen, LPCWSTR lpszSrcData)
{
	AllocString(nSrcLen);
	memcpy(m_pszString, lpszSrcData, nSrcLen * sizeof(WCHAR));
	m_pszString[nSrcLen] = '\0';
}

void CUString::StringCopy(CUString &str, int nLen, int nIndex, int nExtra) const
{
	int nNewLen = nLen + nExtra;
	if (nNewLen != 0)
	{
		str.AllocString(nNewLen);
		memcpy(str.GetString(), m_pszString+nIndex, nLen * sizeof(WCHAR));
	}
}

void CUString::GetEncodedString( vector<CHAR>& outputCharVector, unsigned int codePage ) const
{
    // get number of chars of encoded string
	int nChars = WideCharToMultiByte( codePage , 0, m_pszString, -1, NULL, 0, NULL, NULL );
    
    if ( (int)outputCharVector.size() < nChars )
    {
        outputCharVector.resize( nChars );
    }

    // do conversion
    WideCharToMultiByte( codePage , 0, m_pszString, -1, (LPSTR)&outputCharVector[0], outputCharVector.size(), NULL, NULL );
}

void CUString::GetAsVector( vector<CHAR>& outputCharVector, unsigned int codePage ) const
{
    // get number of chars (include terminate char
	int nChars = ( GetLength() + 1 ) * sizeof( WCHAR) ;
    
    if ( (int)outputCharVector.size() < nChars )
    {
        outputCharVector.resize( nChars );
    }
	
	// do conversion
	WideCharToMultiByte(codePage, 0, m_pszString, -1, (LPSTR)&outputCharVector[0], outputCharVector.size(), NULL, NULL);
}

void CUString::GetAsVector(vector<WCHAR>& outputCharVector) const
{
	// get number of chars (include terminate char
	int nChars = GetLength() + 1;

	if ((int)outputCharVector.size() < nChars)
	{
		outputCharVector.resize(nChars);
	}
	// peform deep copy
	memcpy(&outputCharVector[0], m_pszString, nChars  * sizeof(WCHAR));
}

void CUString::ConcatCopy(LPCWSTR lpszData)
{
	assert(lpszData != NULL);

    int nDataLen = wcslen( lpszData );
	int nLen     = GetLength();
    int nNewLen  = nLen + nDataLen;
	AllocString( nNewLen );
	memcpy(m_pszString+nLen, lpszData, nDataLen * sizeof(WCHAR));
}

void CUString::ConcatCopy(WCHAR ch)
{
	// Calculate the new string length and realloc memory
	int nNewLen = GetLength() + 1;
	AllocString( nNewLen );
	m_pszString[nNewLen-1] = ch;
}

void CUString::ConcatCopy(LPCWSTR lpszData1, LPCWSTR lpszData2)
{
	assert(lpszData1 != NULL);
	assert(lpszData2 != NULL);
	int nLen1 = wcslen(lpszData1);
	int nLen2 = wcslen(lpszData2);
	int nLen = nLen1 + nLen2;
	AllocString(nLen);
	memcpy(m_pszString, lpszData1, nLen1 * sizeof(WCHAR));
	memcpy(m_pszString+nLen1, lpszData2, nLen2 * sizeof(WCHAR)); 
}
//////////////////////////////////////////////////////////////////////
//
// Accessors for the String as an Array
//
//////////////////////////////////////////////////////////////////////

void CUString::Empty()
{
    memset( m_pszString, 0, m_bufferSize );
}

WCHAR CUString::GetAt(int nIndex)
{
	int nLen = wcslen(m_pszString);
	assert(nIndex >= 0);
	assert(nIndex < nLen);
	return m_pszString[nIndex];
}

WCHAR CUString::operator[] (int nIndex)
{
	int nLen = wcslen(m_pszString);
	assert(nIndex >= 0);
	assert(nIndex < nLen);
	return m_pszString[nIndex];
}

void CUString::SetAt(int nIndex, WCHAR ch)
{
	int nLen = wcslen(m_pszString);
	assert(nIndex >= 0);
	assert(nIndex < nLen);
	m_pszString[nIndex] = ch;	
}

int CUString::GetLength() const
{ 
	return wcslen(m_pszString); 
}

bool CUString::IsEmpty() const
{ 
	return (GetLength() > 0) ? false : true; 
}


//////////////////////////////////////////////////////////////////////
//
// Conversions
//
//////////////////////////////////////////////////////////////////////

void CUString::MakeUpper()
{
	_wcsupr(m_pszString);
}

void CUString::MakeLower()
{
	_wcslwr(m_pszString);
}

void CUString::MakeReverse()
{
	_wcsrev(m_pszString);
}

void CUString::TrimLeft()
{
	LPWSTR lpsz = m_pszString;

    int charsToMove  = GetLength()+1;
	
    while ( iswspace(*lpsz) )
    {
        charsToMove--;
        lpsz++;
    }

	if (lpsz != m_pszString)
	{
		memmove( m_pszString, lpsz, ( charsToMove * sizeof(WCHAR) ) );
	}
}

void CUString::TrimRight()
{
	LPWSTR lpsz = m_pszString;

    int trimChars = 0;
    for ( int i = GetLength()-1; i >= 0; i-- )
    {
        if ( !iswspace( m_pszString[ i ] ) ) 
        {
            break;
        }
        trimChars++;
    }


    if ( trimChars > 0 )
    {
        m_pszString[ GetLength() - trimChars ] = _W( '\0' );
    }
}


//////////////////////////////////////////////////////////////////////
//
// Searching
//
//////////////////////////////////////////////////////////////////////

int CUString::Find(WCHAR ch) const
{
	return Find(ch, 0);
}

int CUString::Find(WCHAR ch, int nStart) const
{
	if (nStart >= GetLength())
		return -1;
	LPWSTR lpsz = wcschr( m_pszString + nStart, ch);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pszString);
}

int CUString::Find(LPCWSTR lpszSub)
{
	return Find(lpszSub, 0);
}

int CUString::Find(LPCWSTR lpszSub, int nStart)
{
	assert(wcslen(lpszSub) > 0);

	if (nStart > GetLength())
		return -1;

	LPWSTR lpsz = wcsstr( m_pszString + nStart, lpszSub);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pszString);
}

int CUString::FindOneOf(LPCWSTR lpszCharSet) const
{
	LPWSTR lpsz = wcspbrk(m_pszString, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pszString);
}



int CUString::ReverseFind( WCHAR ch ) const
{

   int nPos = GetLength();

   while ( ( nPos >= 0 ) && ( m_pszString[ nPos ] != ch ) )
   {
       nPos --;
   }
    return nPos;
}


int CUString::ReverseFind( LPCWSTR lpszSub, int nStart)
{
   // we're gonna need this several times, but there's no point in calling it 
   // more than once.
   int nLength = GetLength();
   // compensate for the reverse direction of the find
   nStart = nLength - nStart;
   // make sure we're not gonna be wasting our time
	if (wcslen(lpszSub) <= 0 || nStart > nLength || nStart < 0)
   {
      return -1;
   }

   // Create a new string so we can reverse it without taking a chance on munging 
   // up the original. We already know the length of the string being searched, 
   // so we don't need to retrieve it again - just reverse the string for the 
   // search
   CUString newStr(m_pszString);
   newStr.MakeReverse();

   // Create a new substring to find. We do this so we can reverse it in 
   // preparation for the search. We're also gonna need the size of this substring 
   // to help calculate the correct position.
   CUString newSub(lpszSub);
   int nSubLength = newSub.GetLength();
   newSub.MakeReverse();

   // now we're ready to search
   LPWSTR lpsz = LPWSTR(wcsstr(newStr + nStart, newSub));
   int nResult = (lpsz == NULL) ? -1 : (int)(lpsz - newStr);
   // if the search resulted in a hit, we need to re-calculate the resulting 
   // position for use with the unreversed version of the string.
   if (nResult != -1)
   {
      nResult = nLength - (nSubLength + nResult) + 1;
   }
	return nResult;
}

//////////////////////////////////////////////////////////////////////
//
// Assignment Operations
//
//////////////////////////////////////////////////////////////////////

CUString& CUString::operator=(const CUString& strSrc)
{
//	if (m_pszString != strSrc.GetString())

    StringCopy( strSrc.GetLength(), strSrc.GetString() );

	return *this;
}

const CUString& CUString::operator=(LPCWSTR wideString )
{
	assert(wideString != NULL);
	StringCopy( wcslen(wideString), wideString );
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
// Concatenation
//
//////////////////////////////////////////////////////////////////////

CUString operator+(CUString& strSrc1, CUString& strSrc2)
{
	CUString s;
	s.ConcatCopy((LPCWSTR) strSrc1, (LPCWSTR) strSrc2);
	return s;
}

CUString operator+(CUString& strSrc, LPCWSTR lpsz)
{
	CUString s;
	s.ConcatCopy((LPCWSTR) strSrc, lpsz);
	return s;
}

CUString operator+(LPCWSTR lpsz, CUString& strSrc)
{
	CUString s;
	s.ConcatCopy(lpsz, (LPCWSTR) strSrc);
	return s;
}


const CUString& CUString::operator+=(CUString& strSrc)
{
	ConcatCopy((LPCWSTR) strSrc);
	return *this;
}

const CUString& CUString::operator+=(LPCWSTR wideString)
{
	ConcatCopy( wideString );
	return *this;
}
const CUString& CUString::operator+=(WCHAR ch)
{
	ConcatCopy(ch);
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
// Extraction
//
//////////////////////////////////////////////////////////////////////

CUString CUString::Mid(int nFirst) const
{
	return Mid(nFirst, GetLength() - nFirst);	
}

CUString CUString::Mid(int nFirst, int nCount) const
{
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetLength())
		nCount = GetLength() - nFirst;
	if (nFirst > GetLength())
		nCount = 0;

    CUString newStr;

    assert( nCount >= 0 );

    if ( nCount > 0 )
    {
	    assert(nFirst >= 0);
	    assert(nFirst + nCount <= GetLength());

	    if (nFirst == 0 && nFirst + nCount == GetLength())
 		    return *this;

	    StringCopy(newStr, nCount, nFirst, 0);
    }

	return newStr;
}

CUString CUString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetLength())
		return *this;

	CUString newStr;
	StringCopy(newStr, nCount, 0, 0);
	return newStr;
}

CUString CUString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetLength())
		return *this;

	CUString newStr;
	StringCopy(newStr, nCount, GetLength() - nCount, 0);
	return newStr;
}

CUString CUString::SpanIncluding(LPCWSTR lpszCharSet) const
{
	assert(lpszCharSet != NULL);
#ifdef _UNICODE
//	assert(::IsBadStringPtrW(lpszCharSet, -1) == 0);
#else
//	assert(::IsBadStringPtrA(lpszCharSet, -1) == 0);
#endif
	return Left(wcsspn(m_pszString, lpszCharSet));
}

CUString CUString::SpanExcluding(LPCWSTR lpszCharSet) const
{
	assert(lpszCharSet != NULL);
#ifdef _UNICODE
//	assert(::IsBadStringPtrW(lpszCharSet, -1) == 0);
#else
//	assert(::IsBadStringPtrA(lpszCharSet, -1) == 0);
#endif
	return Left(wcscspn(m_pszString, lpszCharSet));
}


//////////////////////////////////////////////////////////////////////
//
// Comparison
//
//////////////////////////////////////////////////////////////////////
	
int CUString::Compare( const CUString& str) const
{
	assert((LPCWSTR) str != NULL);
#ifdef _UNICODE
	// assert(::IsBadStringPtrW((LPCWSTR) str, -1) == 0);
#else
	// assert(::IsBadStringPtrA((LPCWSTR) str, -1) == 0);
#endif
	return wcscmp(m_pszString, (LPCWSTR) str);	
}

int CUString::Compare(LPCWSTR lpsz) const
{
	assert(lpsz != NULL);
#ifdef _UNICODE
	// assert(::IsBadStringPtrW(lpsz, -1) == 0);
#else
	// assert(::IsBadStringPtrA(lpsz, -1) == 0);
#endif
	return wcscmp(m_pszString, lpsz);	
}

int CUString::CompareNoCase(CUString& str) const
{
	assert((LPCWSTR) str != NULL);
#ifdef _UNICODE
	// assert(::IsBadStringPtrW((LPCWSTR) str, -1) == 0);
#else
	// assert(::IsBadStringPtrA((LPCWSTR) str, -1) == 0);
#endif
	return _wcsicmp(m_pszString, (LPCWSTR) str);	
}

int CUString::CompareNoCase(LPCWSTR lpsz) const
{
	assert(lpsz != NULL);
#ifdef _UNICODE
	// assert(::IsBadStringPtrW(lpsz, -1) == 0);
#else
	// assert(::IsBadStringPtrA(lpsz, -1) == 0);
#endif
	return _wcsicmp(m_pszString, lpsz);	
}

int CUString::Collate(LPCWSTR lpsz) const
{
	assert(lpsz != NULL);
#ifdef _UNICODE
	//assert(::IsBadStringPtrW(lpsz, -1) == 0);
#else
	// assert(::IsBadStringPtrA(lpsz, -1) == 0);
#endif
	return wcscoll(m_pszString, lpsz);	
}

int CUString::Collate(CUString &str) const
{
	assert((LPCWSTR) str != NULL);
#ifdef _UNICODE
	// assert(::IsBadStringPtrW((LPCWSTR) str, -1) == 0);
#else
	// assert(::IsBadStringPtrA((LPCWSTR) str, -1) == 0);
#endif
	return wcscoll(m_pszString, (LPCWSTR) str);	
}


//////////////////////////////////////////////////////////////////////
//
// Formatting
//
//////////////////////////////////////////////////////////////////////
	
void CUString::Format( LPCWSTR pszFormat, ...)
{
	va_list vl;
	va_start(vl, pszFormat);

	int nBufferSize = 32;
	int nRetVal = -1;

	do {
		nBufferSize *= 2;
        AllocString( nBufferSize );

		if( m_pszString == NULL )
        {
			return;
        }

        nRetVal = _vsnwprintf( m_pszString, m_bufferSize - 1, pszFormat, vl);

    } while (nRetVal < 0);

	va_end(vl);
}


//////////////////////////////////////////////////////////////////////
//
// Replacing
//
//////////////////////////////////////////////////////////////////////

int CUString::Replace(WCHAR chOld, WCHAR chNew)
{
	int nCount = 0;

	if (chOld != chNew)
	{
		LPWSTR psz = m_pszString;
		LPWSTR pszEnd = psz + GetLength();
		while(psz < pszEnd)
		{
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz++;
		}
	}

	return nCount;
}

int CUString::Replace(LPCWSTR lpszOld, LPCWSTR lpszNew)
{
	int nSourceLen = wcslen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	int nReplaceLen = wcslen(lpszNew);

	int nCount = 0;
	LPWSTR lpszStart = m_pszString;
	LPWSTR lpszEnd = lpszStart + GetLength();
	LPWSTR lpszTarget;

	// Check to see if any changes need to be made
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = wcsstr(lpszStart, lpszOld)) != NULL)
		{
			lpszStart = lpszTarget + nSourceLen;
			nCount++;
		}
		lpszStart += wcslen(lpszStart) + 1;
	}

	// Do the actual work
	if (nCount > 0)
	{
		int nOldLen = GetLength();
		int nNewLen = nOldLen + (nReplaceLen - nSourceLen) * nCount;
		if (GetLength() < nNewLen)
		{
			CUString szTemp = m_pszString;
			AllocString(nNewLen);
			memcpy(m_pszString, (LPCWSTR) szTemp, nOldLen * sizeof(WCHAR));
		}

		lpszStart = m_pszString;
		lpszEnd = lpszStart + GetLength();

		while (lpszStart < lpszEnd)
		{
			while ((lpszTarget = wcsstr(lpszStart, lpszOld)) != NULL)
			{
				int nBalance = nOldLen - (lpszTarget - m_pszString + nSourceLen);
				memmove(lpszTarget + nReplaceLen, lpszTarget + nSourceLen, nBalance * sizeof(WCHAR));
				memcpy(lpszTarget, lpszNew, nReplaceLen * sizeof(WCHAR));
				lpszStart = lpszTarget + nReplaceLen;
				lpszStart[nBalance] = '\0';
				nOldLen += (nReplaceLen - nSourceLen);
			}
			lpszStart += wcslen(lpszStart) + 1;
		}
		assert(m_pszString[GetLength()] == '\0');
	}
	return nCount;
}




#ifdef NOTUSED

CUString::CUString(BSTR bstrString)
{
    m_bAllocated=false;
	int nLen = wcslen(OLE2T(bstrString));
	AllocString(nLen);
	_tcsncpy(m_pszString, OLE2T(bstrString), nLen);
}

CUString::CUString(WCHAR ch, int nRepeat)
{
    m_bAllocated=false;
	if (nRepeat > 0)
	{
		AllocString(nRepeat);
#ifdef _UNICODE
		for (int i=0; i < nRepeat; i++)
			m_pszString[i] = ch;
#else
		memset(m_pszString, ch, nRepeat);
#endif
	}
}

const CUString& CUString::operator=(BSTR bstrStr)
{
	int nLen = wcslen(OLE2T(bstrStr));
	StringCopy(nLen, OLE2T(bstrStr));
	return *this;
}


BSTR CUString::AllocSysString()
{
#ifdef _UNICODE
	BSTR bstr = ::SysAllocStringLen(m_pszString, GetLength());
	if (bstr == NULL)
	{
		assert(0);
		return NULL;
	}
#else
	int nLen = MultiByteToWideChar(CP_ACP, 0, m_pszString, GetLength(), NULL, NULL);
	BSTR bstr = ::SysAllocStringLen(NULL, nLen);
	if (bstr == NULL)
	{
		assert(0);
		return NULL;
	}
	MultiByteToWideChar(CP_ACP, 0, m_pszString, GetLength(), bstr, nLen);
#endif

	return bstr;
}


const CUString& CUString::operator+=(BSTR bstrStr)
{
	ConcatCopy(OLE2T(bstrStr));
	return *this;
}

//void CUString::AllocString(int nLen)
//{
//	assert(nLen >= 0);
//	m_pszString = (WCHAR*) malloc((nLen+1) * sizeof(WCHAR));
//	assert(m_pszString != NULL);
//	m_pszString[nLen] = '\0';
//}

//void CUString::ReAllocString(int nLen)
//{
//	assert(nLen >= 0);
//	m_pszString = (WCHAR*) realloc(m_pszString, (nLen+1) * sizeof(WCHAR));
//	assert(m_pszString != NULL);
//	m_pszString[nLen] = '\0';
//}


CUString operator+(CUString& strSrc, BSTR bstrStr)
{
	CUString s;
	s.ConcatCopy((LPCWSTR) strSrc, OLE2T(bstrStr));
	return s;
}

CUString operator+(BSTR bstrStr, CUString& strSrc)
{
	CUString s;
	s.ConcatCopy(OLE2T(bstrStr), (LPCWSTR) strSrc);
	return s;
}



// added by JMS - 03/31/2003
// returns the zero-based index into our string
int CCOMString::ReverseFind(LPCWSTR lpszSub)
{
	return ReverseFind(lpszSub, _tcslen(lpszSub));
}

// added by JMS - 03/31/2003
// returns the zero-based index into our string

#endif
