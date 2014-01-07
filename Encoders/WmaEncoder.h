#include <windows.h>
#include <mmsystem.h>

#ifndef CompressionH
#define CompressionH

enum ENCompressionOpenResult {COROk, CORFileError, CORMemoryError, CORCodecError, CORForce_enum_size_to_32bits_ = 0x80000000};
enum ENCompressBlockResult {COBOk, COBFileError, COBMemoryError, COBCodecError, COBForce_enum_size_to_32bits_ = 0x80000000};
//INT_MAX <limits.h> or climits
#define MaxDMCBufferLen 32768	// the maximum buffer sent to compression DLLS

#endif

//-----------------------------------------------------------------
// Typedefs for acessing the output dlls
// See waveout.cpp for a description
// of the functions.
//-----------------------------------------------------------------

extern "C" {
	//----------------------------------------------------------
	//---------------------------------------------------------------------------
	// Creates an output device
	// OpenResult is the returned open result (see Compression.h)
	//---------------------------------------------------------------------------
	typedef void * DCreateNewCompressionObject(char *Filename, ENCompressionOpenResult &ReturnError, WAVEFORMATEX *WFXOutputFormat);
	typedef DCreateNewCompressionObject FAR *LPCreateNewCompressionObject;
	//----------------------------------------------------------
	//===========================================================================
	// delete a Compression object
	//===========================================================================
	typedef void DDeleteCompressionObject(void *Output);
	typedef DDeleteCompressionObject FAR *LPDeleteCompressionObject;
	//----------------------------------------------------------
	//----------------------------------------------------------
	//---------------------------------------------------------------------------
	// A new data block has arrived (pData is a pointer to a buffer)
	// IsEOF indicates if is the last data block
	// Result is the compression result
	//---------------------------------------------------------------------------
	typedef void DANewDataBlockArrived (void *Output, char *pData, DWORD DataLength, bool IsEOF, ENCompressBlockResult &Result);
	typedef DANewDataBlockArrived FAR *LPANewDataBlockArrived;
	//----------------------------------------------------------
	//===========================================================================
	// Called to Show the configuration settings on the main form
	//===========================================================================
	typedef void DShowConfigBit(HWND OnForm);
	typedef DShowConfigBit FAR *LPShowConfigBit;
	//===========================================================================
	// Called to RemoveConfigBit the configuration settings on the main form
	//===========================================================================
	typedef void DRemoveConfigBit(HWND OnForm);
	typedef DRemoveConfigBit FAR *LPRemoveConfigBit;

	//---------------------------------------------------------------------------
	// Returns the Extension this compression routine uses
	// ie ".wav"
	//---------------------------------------------------------------------------
	typedef void DGetExtensionYouCreate(char *RetExt);
	typedef DGetExtensionYouCreate FAR *LPGetExtensionYouCreate;	

	//===========================================================================
	// Sets an element in the ID Tag [Optional Function]
	// Element is the name - ie ARTIST / TITLE / ALBUM / GENRE / COMMENT / YEAR
	// TagVal is the new value
	// NB this is called BEFORE conversion - store any values needed
	// ALSO this function should not be present in any Compression DLL that does not require it (will slow down conversion)
	//===========================================================================
	typedef void DSetCompIDTagElement(char *Element, char *TagVal);
	typedef DSetCompIDTagElement FAR *LPSetCompIDTagElement;

	//===========================================================================
	// [Optional] Sets an Profile
	// A profile is basically a name that any settings this compression
	// dll uses should be saved under, this will NOT be called if the default 
	// profile is being used!
	//===========================================================================
	typedef void DSetProfile(char *ProfileName);
	typedef DSetProfile FAR *LPSetProfile;

	//===========================================================================
	// [Optional] Returns If Raw Files should be sent to compression codec
	// that is without decompressing (Sveta Portable Audio uses it)
	// If this routine returns true then CreateNewCompressionObject will be called
	// with the Input filename!
	//===========================================================================
	typedef bool DSendRawUnCompressedFiles (void);
	typedef DSendRawUnCompressedFiles FAR *LPSendRawUnCompressedFiles;

	typedef void DSetIDTagElement(char *FileName, char *Element, char *TagVal);
	typedef DSetIDTagElement FAR *LPSetIDTagElement;

	typedef void DSetSettings (DWORD SWMABitRate, DWORD SIsStereo, DWORD SWMASampleRate, DWORD SProtectWMA);
	typedef DSetSettings FAR *LPSetSettings;
};

/*#define NumMonoWMAFormats 13
STAWMAFormat MonoWMAFormats[NumMonoWMAFormats] = { {5, 8000 },
           {6, 8000 },
           {8, 8000 },
           {8, 11025 },
           {10, 11025 },
           {10, 16000 },
           {12, 16000 },
           {16, 16000 },
           {16, 22050 },
           {20, 22050 },
           {20, 32000 },
           {32, 44100 },
           {48, 44100 },
          };

#define NumStereoWMAFormats 18
STAWMAFormat StereoWMAFormats[NumStereoWMAFormats] = { {12, 8000 },
           {20, 16000 },
           {22, 22050 },
           {32, 22050 },
           {32, 32000 },
           {32, 44100 },
           {40, 32000 },
           {48, 32000 },
           {48, 44100 },
           {64, 44100 },
           {80, 44100 },
           {96, 44100 },
           {128, 44100 },
           {128, 48000 },
           {160, 44100 },
           {160, 48000 },
           {192, 44100 },
           {192, 48000 },
          };
  */
