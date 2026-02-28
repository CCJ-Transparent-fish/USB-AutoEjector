#include <windows.h>
#include <iostream>

#pragma comment(lib, "advapi32.lib")

using namespace std;

// 删除开机自启注册表项
bool RemoveAutoStart() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, 
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
        0, KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegDeleteValue(hKey, "USBMONITOR");
    RegCloseKey(hKey);
    
    return (result == ERROR_SUCCESS);
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

int main() {
    cout << "=== USB监控程序开机自启取消工具 ===" << endl;
    cout << "正在检查开机自启状态..." << endl;
    
    if (IsAutoStartEnabled()) {
        cout << "检测到已设置开机自启" << endl;
        cout << "正在取消开机自启..." << endl;
        
        if (RemoveAutoStart()) {
            cout << "成功取消开机自启！" << endl;
        } else {
            cout << "取消开机自启失败！" << endl;
        }
    } else {
        cout << "未检测到开机自启设置" << endl;
    }
    
    cout << endl;
    cout << "按任意键退出..." << endl;
    cin.get();
    
    return 0;
}
