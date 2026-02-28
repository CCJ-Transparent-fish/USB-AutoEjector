
# USB Auto Ejector（USB自动弹出器）

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Platform-Windows-blue)](https://github.com)

🔌 一个Windows平台下的USB设备自动弹出工具，支持多种触发模式和双进程保护。

## 📋 功能特点

- ✅ **自动弹出** - 检测到USB设备插入时自动弹出
- ✅ **多种模式** - 支持关机、重启、挂起资源管理器等触发方式
- ✅ **双进程保护** - 主程序 + 守护进程，防止被误关闭
- ✅ **开机自启** - 支持添加开机自启动注册表项
- ✅ **轻量级** - 低资源占用，后台静默运行

## 📁 文件结构

```
USB-AutoEjector/
├── src/
│   ├── USBMONITOR.cpp        # 主程序源代码
│   ├── USBGuard.cpp           # 守护进程源代码
│   ├── kill.cpp               # 进程终止工具源代码
│   └── DisableAutoStart.cpp   # 取消开机自启工具源代码
├── docs/
│   └── 使用手册.txt            # 详细使用说明
├── README.md                   # 本文件
└── LICENSE                     # MIT许可证
```

## 🚀 快速开始

### 方法一：直接使用编译好的程序

1. 下载所有exe文件到同一目录
2. 运行 `USBMONITOR.exe`
3. 根据需要创建控制文件（详见下方说明）

### 方法二：自行编译

#### 使用MinGW编译：
```bash
# 编译主程序
g++ -o USBMONITOR.exe src/USBMONITOR.cpp -ladvapi32 -luser32 -static

# 编译守护程序
g++ -o USBGuard.exe src/USBGuard.cpp -ladvapi32 -static

# 编译工具
g++ -o kill.exe src/kill.cpp -static
g++ -o DisableAutoStart.exe src/DisableAutoStart.cpp -ladvapi32 -static
```

#### 使用Visual Studio：
- 创建空项目，添加相应源文件
- 链接依赖库：`advapi32.lib`、`user32.lib`

## 🎮 使用说明

### 控制文件（在程序目录下创建，无后缀名）

| 文件名 | 功能 |
|--------|------|
| `开机自启` | 添加开机自启注册表项 |
| `关机` | U盘插入时关机 |
| `重启` | U盘插入时重启 |
| `挂起` | U盘插入时挂起资源管理器窗口 |
| `强制挂起` | U盘插入时挂起资源管理器进程树 |

### 关闭程序
- 运行 `kill.exe`
- 或重启电脑

## ⚠️ 注意事项

- 程序会操作Windows注册表（开机自启功能）
- 强制挂起功能会影响资源管理器操作，请谨慎使用
- 所有源代码完全开放，无恶意代码
- 建议以管理员身份运行以获得完整功能

## 🔧 依赖库

- Windows SDK
- advapi32.lib
- user32.lib

## 📜 许可证

本项目采用 MIT License 开源。


## ⭐ 免责声明


本程序仅供学习交流使用，请勿用于非法用途。使用本程序所产生的一切后果由使用者自行承担。




