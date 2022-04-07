#include "Utility.h"

//
#include <filesystem>

//
#if defined(_WIN32)
#include "Windows.h"
#endif

namespace
{
	char dummyChar;
}

std::string getCurrentDllPath()
{
#if defined(_WIN32)
	char path[_MAX_PATH];
	HMODULE phModule = NULL;

	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
								GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
							(LPCSTR)&dummyChar,
							&phModule))
	{
		// Some error
	}
	GetModuleFileNameA(phModule, path, sizeof(path));
	std::filesystem::path res(path);
	return res.parent_path().u8string();
#elif
	return std::string();
#endif
}