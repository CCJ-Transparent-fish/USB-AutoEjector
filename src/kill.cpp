#include <windows.h>
#include <tlhelp32.h>
#include <string>

using namespace std;

// 结束指定进程名的所有进程
bool KillProcessByName(const char* processName) {
    bool found = false;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, processName) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    found = true;
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    
    CloseHandle(hSnapshot);
    return found;
}

int main() {
    // 隐藏控制台窗口
    HWND hConsole = GetConsoleWindow();
    ShowWindow(hConsole, SW_HIDE);
    
    // 要结束的进程列表
    const char* processes[] = {
        "USBMONITOR.exe",
        "USBGuard.exe"
    };
    
    int processCount = sizeof(processes) / sizeof(processes[0]);
    
    // 结束所有目标进程
    for (int i = 0; i < processCount; i++) {
        KillProcessByName(processes[i]);
    }
    
    return 0;
}
