#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

#pragma comment(lib, "advapi32.lib")

using namespace std;

class ProcessGuard {
private:
    volatile bool isRunning;
    
    // 检查主进程是否运行
    bool IsMainProcessRunning() {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (strcmp(pe.szExeFile, "USBMONITOR.exe") == 0) {
                    CloseHandle(hSnapshot);
                    return true;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        
        CloseHandle(hSnapshot);
        return false;
    }
    
    // 启动主进程
    void StartMainProcess() {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        if (CreateProcess("USBMONITOR.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
    }
    
    // 监控主进程的线程函数
    static DWORD WINAPI MonitorMainThread(LPVOID lpParam) {
        ProcessGuard* guard = (ProcessGuard*)lpParam;
        while (guard->isRunning) {
            if (!guard->IsMainProcessRunning()) {
                guard->StartMainProcess();
            }
            Sleep(150); // 改为150毫秒检查一次
        }
        return 0;
    }

public:
    ProcessGuard() : isRunning(false) {}
    
    void Start() {
        isRunning = true;
        
        // 启动时立即检查并启动USBMONITOR.exe
        if (!IsMainProcessRunning()) {
            StartMainProcess();
        }
        
        // 启动监控线程
        CreateThread(NULL, 0, MonitorMainThread, this, 0, NULL);
        
        // 简单的消息循环
        MSG msg;
        while (isRunning && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    void Stop() {
        isRunning = false;
    }
};

int main() {
    // 隐藏控制台窗口
    HWND hConsole = GetConsoleWindow();
    ShowWindow(hConsole, SW_HIDE);
    
    ProcessGuard guard;
    guard.Start();
    
    return 0;
}
