#ifndef USTRING_H_INCLUDED
#define USTRING_H_INCLUDED

#include <vector>

using namespace std;

#define _W(x)      L ## x


class CUString  
{
protected:
	LPWSTR	m_pszString;
    unsigned int m_bufferSize;

public: 
	CUString();

    CUString(const CUString& str);
	CUString(LPCWSTR wideString);

#ifdef _UNICODE
    CUString( LPCSTR string, int codePage );
#else
    CUString( LPCSTR string, int codePage = CP_ACP );
#endif

    CUString( LPCWSTR string, int codePage );
    CUString(WCHAR wch );

    virtual ~CUString();
	
	// Assignment Operations
	CUString& operator=(const CUString& strSrc);
    LPCWSTR operator&() {return m_pszString;}

	const CUString& operator=(LPCWSTR wideChar );
	// const CUString& operator=(BSTR bstrStr);

    operator LPCWSTR() const	{ return m_pszString; }
	WCHAR*	GetString() const { return m_pszString; } 
    void GetEncodedString( vector<CHAR>& outputString, unsigned int codePage ) const;
    void GetAsVector( vector<CHAR>& outputString ) const;
    //vector<CHAR> GetAcpString() { return GetString( CP_ACP); }
    //vector<CHAR> GetUtf8String() { return GetString( CP_UTF8); }

	// BSTR	AllocSysString();
	
	// Concatenation
	const CUString& operator+=(CUString& strSrc);
	const CUString& operator+=(LPCWSTR wideString);
	// const CUString& operator+=(BSTR bstrStr);
	const CUString& operator+=(WCHAR ch);
	friend CUString operator+(CUString& strSrc1, CUString& strSrc2);
	friend CUString operator+(CUString& strSrc, LPCWSTR lpsz);
	friend CUString operator+(LPCWSTR wideString, CUString& strSrc);
	// friend CUString operator+(CUString& strSrc, BSTR bstrStr);
	//friend CUString operator+(BSTR bstrStr, CUString& strSrc);

	// Accessors for the String as an Array
	int		GetLength() const;
	bool	IsEmpty() const;
	void	Empty();
	WCHAR	GetAt(int nIndex);
	void	SetAt(int nIndex, WCHAR ch);
	WCHAR	operator[] (int nIndex);

	// Conversions
	void	MakeUpper();
	void	MakeLower();
	void	MakeReverse();
	void	TrimLeft();
	void	TrimRight();
    void	Trim() { TrimLeft(); TrimRight(); }

	// Searching
	int		Find(WCHAR ch) const;
	int		Find(WCHAR ch, int nStart) const;
	int		Find(LPCWSTR lpszSub);
	int		Find(LPCWSTR lpszSub, int nStart);
	int		FindOneOf(LPCWSTR lpszCharSet) const;
    int     ReverseFind( LPCWSTR lpszSub, int nStart = 0);
    int     ReverseFind( WCHAR ch ) const;

	// Extraction
	CUString Mid(int nFirst) const;
	CUString Mid(int nFirst, int nCount) const;
	CUString Left(int nCount) const;
	CUString Right(int nCount) const;
	CUString SpanIncluding(LPCWSTR lpszCharSet) const;
	CUString SpanExcluding(LPCWSTR lpszCharSet) const;

	// Comparison
	int Compare(const CUString& str) const;
	int Compare(LPCWSTR wideString) const;
	int CompareNoCase(CUString& str) const;
	int CompareNoCase(LPCWSTR wideString) const;
	int Collate(CUString& str) const;
	int Collate(LPCWSTR wideString) const;

	// Formatting
	void Format(LPCWSTR pszCharSet, ...);

	// Replacing
	int Replace(WCHAR chOld, WCHAR chNew);
	int Replace(LPCWSTR lpszOld, LPCWSTR lpszNew);
	void	ConcatCopy(LPCWSTR lpszData1, LPCWSTR lpszData2);
	
protected:
	void	StringCopy(CUString& str, int nLen, int nIndex, int nExtra) const;
	void	StringCopy(int nSrcLen, LPCWSTR lpszSrcData);
	void	ConcatCopy(LPCWSTR lpszData);
	void	ConcatCopy(WCHAR ch);
	void	AllocString(int nLen);


	// void	ReAllocString(int nLen);
};	

// Compare operations
bool operator==(const CUString& s1, const CUString& s2);
bool operator==(const CUString& s1, LPCWSTR s2);
bool operator==(LPCWSTR s1, const CUString& s2);
bool operator!=(const CUString& s1, const CUString& s2);
bool operator!=(const CUString& s1, LPCWSTR s2);
bool operator!=(LPCWSTR s1, const CUString& s2);

// Compare implementations
inline bool operator==(const CUString& s1, const CUString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool operator==(const CUString& s1, LPCWSTR s2)
	{ return s1.Compare(s2) == 0; }
inline bool operator==(LPCWSTR s1, const CUString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool operator!=(const CUString& s1, const CUString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool operator!=(const CUString& s1, LPCWSTR s2)
	{ return s1.Compare(s2) != 0; }
inline bool operator!=(LPCWSTR s1, const CUString& s2)
	{ return s2.Compare(s1) != 0; }



class CUStringConvert
{
    vector<CHAR> convertBuffer;

public: 
    CUStringConvert( ):
        convertBuffer(100)
        {
        }

    ~CUStringConvert() {};

    LPSTR ToUTF8( const CUString& strConvert ) { 
        strConvert.GetEncodedString( convertBuffer, CP_UTF8 ); 
        return (LPSTR)&convertBuffer[0];
    }
    LPSTR ToACP( const CUString& strConvert ) { 
        strConvert.GetEncodedString( convertBuffer, CP_ACP ); 
        return (LPSTR)&convertBuffer[0];
    }
    LPSTR ToCodePage( const CUString& strConvert, unsigned int codePage ) { 
        strConvert.GetEncodedString( convertBuffer, codePage ); 
        return (LPSTR)&convertBuffer[0];
    }
    LPTSTR ToT( const CUString& strConvert ) { 
#ifdef _UNICODE
        strConvert.GetAsVector( convertBuffer ); 
#else
        strConvert.GetEncodedString( convertBuffer, CP_ACP ); 
#endif
        return (LPTSTR)&convertBuffer[0];
    }
};

#endif

