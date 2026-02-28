#include <windows.h>
#include <iostream>
#include <tlhelp32.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")

using namespace std;

class USBMONITOR {
private:
    volatile bool isRunning;
    volatile bool shouldSuspendExplorer;
    char programPath[MAX_PATH];  // 存储程序所在目录
    
    // 设置程序路径
    void SetProgramPath() {
        GetModuleFileName(NULL, programPath, MAX_PATH);
        // 去掉文件名，只保留目录
        char* lastSlash = strrchr(programPath, '\\');
        if (lastSlash != NULL) {
            *(lastSlash + 1) = '\0';
        }
    }
    
    // 检查文件是否存在（在程序目录下）
    bool CheckFileInProgramDir(const char* filename) {
        char fullPath[MAX_PATH];
        strcpy(fullPath, programPath);
        strcat(fullPath, filename);
        return (GetFileAttributes(fullPath) != INVALID_FILE_ATTRIBUTES);
    }
    
    // 检查是否已设置开机自启
    bool IsAutoStartEnabled() {
        HKEY hKey;
        LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, 
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
            0, KEY_READ, &hKey);
        
        if (result != ERROR_SUCCESS) {
            return false;
        }
        
        char value[1024];
        DWORD valueSize = sizeof(value);
        result = RegQueryValueEx(hKey, "USBMONITOR", NULL, NULL, (LPBYTE)value, &valueSize);
        
        RegCloseKey(hKey);
        return (result == ERROR_SUCCESS);
    }
    
    // 设置开机自启
    bool SetAutoStart() {
        char exePath[MAX_PATH];
        GetModuleFileName(NULL, exePath, MAX_PATH);
        
        HKEY hKey;
        LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, 
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
            0, KEY_WRITE, &hKey);
        
        if (result != ERROR_SUCCESS) {
            return false;
        }
        
        result = RegSetValueEx(hKey, "USBMONITOR", 0, REG_SZ, 
            (const BYTE*)exePath, strlen(exePath) + 1);
        
        RegCloseKey(hKey);
        return (result == ERROR_SUCCESS);
    }
    
    // 检查开机自启文件是否存在
    bool CheckAutoStartFile() {
        return CheckFileInProgramDir("开机自启");
    }
    
    // 检查关机文件是否存在
    bool CheckShutdownFile() {
        return CheckFileInProgramDir("关机");
    }
    
    // 检查重启文件是否存在
    bool CheckRestartFile() {
        return CheckFileInProgramDir("重启");
    }
    
    // 检查挂起文件是否存在
    bool CheckSuspendFile() {
        return CheckFileInProgramDir("挂起");
    }
    
    // 检查强制挂起文件是否存在
    bool CheckForceSuspendFile() {
        return CheckFileInProgramDir("强制挂起");
    }
    
    // 执行关机
    void PerformShutdown() {
        Sleep(1000);
        system("shutdown /s /f /t 0");
    }
    
    // 执行重启
    void PerformRestart() {
        Sleep(1000);
        system("shutdown /r /f /t 0");
    }
    
    // 挂起进程树
    void SuspendExplorerProcessTree() {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return;
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (strcmp(pe.szExeFile, "explorer.exe") == 0) {
                    // 挂起explorer.exe进程
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
                    if (hProcess != NULL) {
                        // 挂起进程的所有线程
                        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                        if (hThreadSnapshot != INVALID_HANDLE_VALUE) {
                            THREADENTRY32 te;
                            te.dwSize = sizeof(THREADENTRY32);
                            
                            if (Thread32First(hThreadSnapshot, &te)) {
                                do {
                                    if (te.th32OwnerProcessID == pe.th32ProcessID) {
                                        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                                        if (hThread != NULL) {
                                            SuspendThread(hThread);
                                            CloseHandle(hThread);
                                        }
                                    }
                                } while (Thread32Next(hThreadSnapshot, &te));
                            }
                            CloseHandle(hThreadSnapshot);
                        }
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        
        CloseHandle(hSnapshot);
    }
    
    // 恢复进程树
    void ResumeExplorerProcessTree() {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return;
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (strcmp(pe.szExeFile, "explorer.exe") == 0) {
                    // 恢复explorer.exe进程
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
                    if (hProcess != NULL) {
                        // 恢复进程的所有线程
                        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                        if (hThreadSnapshot != INVALID_HANDLE_VALUE) {
                            THREADENTRY32 te;
                            te.dwSize = sizeof(THREADENTRY32);
                            
                            if (Thread32First(hThreadSnapshot, &te)) {
                                do {
                                    if (te.th32OwnerProcessID == pe.th32ProcessID) {
                                        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                                        if (hThread != NULL) {
                                            ResumeThread(hThread);
                                            CloseHandle(hThread);
                                        }
                                    }
                                } while (Thread32Next(hThreadSnapshot, &te));
                            }
                            CloseHandle(hThreadSnapshot);
                        }
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        
        CloseHandle(hSnapshot);
    }
    
    // 禁用文件资源管理器窗口
    void DisableExplorerWindows() {
        // 枚举所有窗口，找到文件资源管理器窗口并禁用
        EnumWindows(EnumWindowsProc, (LPARAM)this);
    }
    
    // 恢复文件资源管理器窗口
    void EnableExplorerWindows() {
        // 枚举所有窗口，找到文件资源管理器窗口并启用
        EnumWindows(EnumWindowsProcEnable, (LPARAM)this);
    }
    
    // 枚举窗口回调 - 禁用文件资源管理器
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
        USBMONITOR* monitor = (USBMONITOR*)lParam;
        
        char className[256];
        GetClassName(hwnd, className, sizeof(className));
        
        // 文件资源管理器窗口类名
        if (strcmp(className, "CabinetWClass") == 0 || 
            strcmp(className, "ExploreWClass") == 0) {
            // 禁用窗口
            EnableWindow(hwnd, FALSE);
        }
        
        return TRUE;
    }
    
    // 枚举窗口回调 - 启用文件资源管理器
    static BOOL CALLBACK EnumWindowsProcEnable(HWND hwnd, LPARAM lParam) {
        USBMONITOR* monitor = (USBMONITOR*)lParam;
        
        char className[256];
        GetClassName(hwnd, className, sizeof(className));
        
        // 文件资源管理器窗口类名
        if (strcmp(className, "CabinetWClass") == 0 || 
            strcmp(className, "ExploreWClass") == 0) {
            // 启用窗口
            EnableWindow(hwnd, TRUE);
        }
        
        return TRUE;
    }
    
    // 持续监控并挂起文件资源管理器
    static DWORD WINAPI PersistentSuspendThread(LPVOID lpParam) {
        USBMONITOR* monitor = (USBMONITOR*)lpParam;
        
        while (monitor->isRunning) {
            if (monitor->shouldSuspendExplorer) {
                // 持续禁用文件资源管理器窗口
                monitor->DisableExplorerWindows();
            }
            Sleep(100); // 每100毫秒检查一次，确保新窗口也被禁用
        }
        return 0;
    }
    
    // 处理开机自启
    void HandleAutoStart() {
        if (CheckAutoStartFile()) {
            if (!IsAutoStartEnabled()) {
                if (SetAutoStart()) {
                    // 开机自启设置成功
                }
            }
        }
    }
    
    // 检查守护进程是否运行
    bool IsGuardRunning() {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return false;
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (strcmp(pe.szExeFile, "USBGuard.exe") == 0) {
                    CloseHandle(hSnapshot);
                    return true;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        
        CloseHandle(hSnapshot);
        return false;
    }
    
    // 启动守护进程
    void StartGuardProcess() {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // 使用完整路径启动保护程序
        char guardPath[MAX_PATH];
        strcpy(guardPath, programPath);
        strcat(guardPath, "USBGuard.exe");
        
        if (CreateProcess(guardPath, NULL, NULL, NULL, FALSE, 0, NULL, programPath, &si, &pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
    }
    
    // 监控守护进程的线程
    static DWORD WINAPI MonitorGuardThread(LPVOID lpParam) {
        USBMONITOR* monitor = (USBMONITOR*)lpParam;
        while (monitor->isRunning) {
            if (!monitor->IsGuardRunning()) {
                monitor->StartGuardProcess();
            }
            Sleep(150);
        }
        return 0;
    }
    
    // 弹出USB设备
    bool EjectUSBDevice(char driveLetter) {
        char volumePath[] = "\\\\.\\X:";
        volumePath[4] = driveLetter;
        
        HANDLE hVolume = CreateFile(
            volumePath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );
        
        if (hVolume == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        DWORD bytesReturned = 0;
        BOOL success = FALSE;
        
        success = DeviceIoControl(
            hVolume,
            IOCTL_STORAGE_EJECT_MEDIA,
            NULL, 0, NULL, 0, &bytesReturned, NULL
        );
        
        if (!success) {
            DeviceIoControl(
                hVolume,
                FSCTL_DISMOUNT_VOLUME,
                NULL, 0, NULL, 0, &bytesReturned, NULL
            );
            
            DeviceIoControl(
                hVolume,
                FSCTL_LOCK_VOLUME,
                NULL, 0, NULL, 0, &bytesReturned, NULL
            );
            
            success = DeviceIoControl(
                hVolume,
                IOCTL_STORAGE_EJECT_MEDIA,
                NULL, 0, NULL, 0, &bytesReturned, NULL
            );
        }
        
        CloseHandle(hVolume);
        return success != FALSE;
    }
    
    // 检查是否有U盘插入
    bool IsUSBInserted() {
        for (char c = 'A'; c <= 'Z'; c++) {
            char drive[] = "X:";
            drive[0] = c;
            
            UINT type = GetDriveType(drive);
            if (type == DRIVE_REMOVABLE) {
                return true;
            }
        }
        return false;
    }
    
    // 处理U盘插入事件
    void HandleUSBInsertion() {
        // 检查关机文件
        if (CheckShutdownFile()) {
            PerformShutdown();
            return;
        }
        
        // 检查重启文件
        if (CheckRestartFile()) {
            PerformRestart();
            return;
        }
        
        // 检查挂起文件
        if (CheckSuspendFile()) {
            // 开始持续挂起文件资源管理器
            shouldSuspendExplorer = true;
            return;
        }
        
        // 检查强制挂起文件
        if (CheckForceSuspendFile()) {
            // 强制挂起文件资源管理器进程树
            SuspendExplorerProcessTree();
            return;
        }
        
        // 如果没有特殊文件，正常弹出U盘
        for (char c = 'A'; c <= 'Z'; c++) {
            char drive[] = "X:";
            drive[0] = c;
            
            UINT type = GetDriveType(drive);
            if (type == DRIVE_REMOVABLE) {
                bool ejected = EjectUSBDevice(c);
                
                if (ejected) {
                    for (int i = 0; i < 50; i++) {
                        if (!IsUSBInserted()) {
                            break;
                        }
                        Sleep(100);
                    }
                }
            }
        }
    }
    
    // 处理U盘拔出事件
    void HandleUSBRemoval() {
        // 如果之前处于挂起状态，停止持续挂起
        if (shouldSuspendExplorer) {
            shouldSuspendExplorer = false;
            // 恢复文件资源管理器窗口
            EnableExplorerWindows();
        }
        
        // 恢复文件资源管理器进程树（如果之前被强制挂起）
        ResumeExplorerProcessTree();
    }
    
    // 主循环线程 - 处理U盘
    static DWORD WINAPI MainLoopThread(LPVOID lpParam) {
        USBMONITOR* monitor = (USBMONITOR*)lpParam;
        bool lastUSBState = false;
        
        while (monitor->isRunning) {
            bool currentUSBState = monitor->IsUSBInserted();
            
            // U盘插入事件
            if (currentUSBState && !lastUSBState) {
                monitor->HandleUSBInsertion();
            }
            // U盘拔出事件
            else if (!currentUSBState && lastUSBState) {
                monitor->HandleUSBRemoval();
            }
            
            lastUSBState = currentUSBState;
            Sleep(500);
        }
        return 0;
    }

public:
    USBMONITOR() : isRunning(false), shouldSuspendExplorer(false) {
        SetProgramPath();  // 初始化时设置程序路径
    }
    
    void Start() {
        isRunning = true;
        
        // 处理开机自启
        HandleAutoStart();
        
        // 启动时立即检查并启动USBGuard.exe
        if (!IsGuardRunning()) {
            StartGuardProcess();
        }
        
        // 启动USB处理监控线程
        CreateThread(NULL, 0, MainLoopThread, this, 0, NULL);
        
        // 启动守护进程监控线程
        CreateThread(NULL, 0, MonitorGuardThread, this, 0, NULL);
        
        // 启动持续挂起监控线程
        CreateThread(NULL, 0, PersistentSuspendThread, this, 0, NULL);
        
        // 消息循环
        MSG msg;
        while (isRunning && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    void Stop() {
        // 停止前恢复所有窗口
        if (shouldSuspendExplorer) {
            shouldSuspendExplorer = false;
            EnableExplorerWindows();
        }
        // 停止前恢复文件资源管理器进程树
        ResumeExplorerProcessTree();
        isRunning = false;
    }
};

int main() {
    // 隐藏控制台窗口
    HWND hConsole = GetConsoleWindow();
    ShowWindow(hConsole, SW_HIDE);
    
    USBMONITOR monitor;
    monitor.Start();
    
    return 0;
}
