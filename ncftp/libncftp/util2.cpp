/* util2.cpp */

#if defined(WIN32) || defined(_WINDOWS)
#	pragma warning(disable : 4510)	// warning C4510: 'DStr' : default constructor could not be generated
#	pragma warning(disable : 4512)	// warning C4512: 'DStr' : assignment operator could not be generated
#	pragma warning(disable : 4610)	// warning C4610: struct 'DStr' can never be instantiated - user defined constructor required
//#	include "syshdrs.h"
#	ifndef WINVER
#		define WINVER 0x0400
#	endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0400
#	endif
#	include <windows.h>		/* includes <winsock2.h> if _WIN32_WINNT >= 0x400 */
#	include <shlobj.h>
#	include <process.h>

extern "C" void
GetSpecialDir(char *dst, size_t size, int whichDir)
{
	LPITEMIDLIST idl;
	LPMALLOC shl;
	char path[MAX_PATH + 1];
	HRESULT hResult;
	
	memset(dst, 0, size);
	hResult = SHGetMalloc(&shl);
	if (SUCCEEDED(hResult)) {
		hResult = SHGetSpecialFolderLocation(
					NULL,
					whichDir,
					&idl
					);

		if (SUCCEEDED(hResult)) {
			if(SHGetPathFromIDList(idl, path)) {
				(void) strncpy(dst, path, size - 1);
				dst[size - 1] = '\0';
			}
			shl->Free(idl);
		}
		shl->Release();
	}
}	// GetSpecialDir


#endif

