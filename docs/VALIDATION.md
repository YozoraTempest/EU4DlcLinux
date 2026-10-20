# 验证记录

日期：2026-10-20。执行环境：Windows、Git Bash、Zig 0.15.2、Visual Studio 随附 CMake / Ninja。当前机器没有可用的 WSL Linux 发行版。

## 已通过

| 检查 | 结果 |
| --- | --- |
| 原补丁静态检查 | 解压成功；配置为 AppID 236850、88 条 DLC；四个 DLL 均为 PE |
| 清单迁移 | 与原配置逐条比较 ID、名称、次序，无差异 |
| C++ 配置与缓冲区测试 | 使用 Zig 编译为 Windows 可执行程序后实际执行通过 |
| POSIX 启动脚本测试 | 在 Git Bash 中实际执行通过；使用模拟平台查询与 ELF 头部夹具，不加载 Linux 库 |
| CMake 交叉配置与构建 | 全部目标编译链接成功，目标为 x86_64-linux-gnu.2.31 |
| 产物格式 | file 确认为 ELF 64-bit LSB shared object、x86-64 |

C++ 测试包含 CRLF、88 条预置记录、无效 AppID、缺失配置、空清单、无效索引、空指针、0 / 负值 / 单字节缓冲区和字符串截断。

启动脚本测试包含含空格路径、参数原样传递、退出码、临时目录清理、非 Linux 拒绝、配置文件检查、Steam 库歧义检查、显式库选择和 PE 拒绝。测试的目标命令使用 POSIX `.` 内建加载探针，以便在 Windows 上检查脚本逻辑；这不构成 Linux 进程预加载成功的证据。

交叉编译命令使用 CMake 的 Linux 目标配置、Zig 的 C++ 驱动及 `-target x86_64-linux-gnu.2.31`。Zig 安装包从官方 index 获取，并在解压前核对官方 SHA-256。

编译报告有两条随附 fmt 头文件的字面量运算符弃用警告，未影响构建。

## 已编译，尚未执行

- `runtime_tests` 与 `libmock_steam.so`：用于验证真实 ELF 预加载、原始 SteamAPI_Init 转发、重复初始化、ISteamApps 虚函数、flat API、显式 dlsym 查询和无关函数透传。
- `missing_config`：用于验证 Linux 动态库读取错误配置时返回失败，且不调用原始初始化函数。
- Linux 下的 `config_tests`：同一测试源码已在 Windows 执行，但 Linux 目标程序尚未执行。

这些测试已接入 CTest 与 GitHub Actions，尚未触发远程 CI。

## 尚未进行

- 原生 Linux EU4 1.37 启动与 DLC 功能验证。
- 目标游戏实际 Steamworks ABI 版本检查。
- Steam 客户端集成、Paradox 启动器、Steam Runtime / Flatpak、联机、成就和长期运行验证。

交叉编译产物与测试程序不代表游戏实测通过。Linux 上执行 `sh ./build.sh` 会先运行 CTest，再生成本机构建安装包。
