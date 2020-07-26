#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define EXIT_SUCCESS	0
#define EXIT_ERROR_GENERIC -1
#define EXIT_ERROR_PARAMS	1
#define EXIT_ERROR_PROCESSING	2

void DisplayError(LPTSTR lpszFunction);

// Marc: Supprime l'expansion des jokers (wildcards)
int _dowildcard = 0;	// Disable wildcard expansion

auto strPgmName = TEXT("WADel");
auto strPgmVersion = TEXT("1.0");

enum class RunMode { help, process, error };

TCHAR chOvr = TEXT('A');
bool fDeletion = false;

string pathmask;

void version() {
		_tprintf(TEXT("%s, version %s\n"), strPgmName, strPgmVersion);
}

void usage() {
		_tprintf(TEXT("Usage 1:\n"));
		_tprintf(TEXT("\twadel\t-h\t\tAsk help\n"));
		_tprintf(TEXT("Usage 2:\n"));
		_tprintf(TEXT("\twadel\t<pathmask>\tProcess file matching <pathmask>\n"));
		_tprintf(TEXT("\t\t[-c:<char>]\tOverride with characters <char> ('A' is the default)\n"));
		_tprintf(TEXT("\t\t[-d]\t\tDelete the file\n"));
}

void display_arguments(int argc, char*argv[]) {
		for (auto i=1; i<argc; ++i) {
			_tprintf(TEXT("argv[%i] = %s\n"), i , argv[i]);
		}
}

RunMode parse_args(int argc, char* argv[]) {
		vector<string> r_arg(argc);
		bool fHelp = false;
		bool fProcess = false;
		for (int i=0; i<argc; ++i) {
				r_arg[i] = argv[i];
				
				if (i == 0) continue;
				
				// Check if help option
				if (r_arg[i].compare("-h") == 0)
						fHelp = true;
				else {
						fProcess = true;
						if (r_arg[i].compare(0, 3, "-c:") == 0) {
								if (r_arg[i].size() > 4)
										return RunMode::error;
								else {
										chOvr = r_arg[i][3];
								}
						} else if (r_arg[i].compare(0, 2, "-d") == 0) {
								if (r_arg[i].size() > 2)
										return RunMode::error;
								else {
										fDeletion = true;
								}
						} else {
								size_t length_of_arg;

 								StringCchLength(argv[1], MAX_PATH, &length_of_arg);
								if (length_of_arg > (MAX_PATH - 1))
										return RunMode::error;

								pathmask = r_arg[i];			
						}
				}
		}
		if ((argc <= 1) || fHelp) {
			usage();
			return ((argc <= 1) || fProcess)? RunMode::error: RunMode::help;
		}

		return RunMode::process;				
}

int proc_individual_file(LPCTSTR szFilename, LARGE_INTEGER sz_in_bytes) {
		
		if (sz_in_bytes.HighPart != 0) {
				_tprintf(TEXT("\nIgnoring too big file file %s"), szFilename);
				return EXIT_SUCCESS;
		}

		_tprintf(TEXT("\nProcessing file %s"), szFilename);	
		ofstream ofs(szFilename, ios_base::binary);
		for (int i=0; i < (sz_in_bytes.LowPart / sizeof(chOvr)); ++i)
			ofs.write(&chOvr, sizeof(chOvr));
		ofs.close();

		if (fDeletion)
			if (remove(szFilename) != 0) {
				_tprintf(TEXT("\n\tError deleting file %s"), szFilename);
				return EXIT_ERROR_PROCESSING;
			} else {
				_tprintf(TEXT("... File deleted"));
			}
			
		return EXIT_SUCCESS;				
}

int proc(LPCTSTR szPathmask) {
		WIN32_FIND_DATA ffd;
		LARGE_INTEGER filesize;
		TCHAR szDir[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;
		DWORD dwError=0;
		TCHAR szFindFirstFile[] { TEXT("FindFirstFile") };
   
		// Prepare string for use with FindFile functions.  First, copy the
		// string to a buffer, then append '\*' to the directory name.

		StringCchCopy(szDir, MAX_PATH, szPathmask);

		// Find the first file in the directory.
		hFind = FindFirstFile(szDir, &ffd);

		if (INVALID_HANDLE_VALUE == hFind) {
				DisplayError(szFindFirstFile);
				return EXIT_ERROR_PROCESSING;
		} 

		// List all the files in the directory with some info about them.
		do {
		  if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		  		// Nothing to do with directories
		  		continue;
		  else {
		      filesize.LowPart = ffd.nFileSizeLow;
		      filesize.HighPart = ffd.nFileSizeHigh;
		     
		      int iResult = proc_individual_file(ffd.cFileName, filesize);
		      if (iResult != EXIT_SUCCESS)
		      		return iResult;
		  }
		} while (FindNextFile(hFind, &ffd) != 0);

		dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES) {
	  		DisplayError(szFindFirstFile);
	  		return EXIT_ERROR_PROCESSING;
		}

		FindClose(hFind);

		return EXIT_SUCCESS;				
}

int _tmain(int argc, TCHAR *argv[])
{
		// Affiche le nom du programme et sa version
		version();
		
		// Analyse les paramètres passés en ligne de commande
		auto aPA = parse_args(argc, argv);
		switch(aPA) {
		case RunMode::error:
				return EXIT_ERROR_PARAMS;
		case RunMode::help:
				return EXIT_SUCCESS;
		case RunMode::process:
				break;
		default: 
				return EXIT_ERROR_GENERIC;
		}
		
		return proc(argv[1]);
}


void DisplayError(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and clean up

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
        
    _tprintf("\n\t%s", (LPCTSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}