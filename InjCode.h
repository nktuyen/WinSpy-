/***************************************************************
Module name: InjCode.h
Copyright (c) 2003 Robert Kuster

Notice:	If this code works, it was written by Robert Kuster.
		Else, I don't know who wrote it.

		Use it on your own risk. No responsibilities for
		possible damages of even functionality can be taken.
***************************************************************/
#if !defined INJCODE_H
#define INJCODE_H

int GetWindowTextRemoteA (HANDLE hProcess, HWND hWnd, LPSTR  lpString);
int GetWindowTextRemoteW (HANDLE hProcess, HWND hWnd, LPWSTR lpString);


#ifdef UNICODE
#define GetWindowTextRemote GetWindowTextRemoteW
#else
#define GetWindowTextRemote GetWindowTextRemoteA
#endif // !UNICODE

#endif // !defined(INJCODE_H)

