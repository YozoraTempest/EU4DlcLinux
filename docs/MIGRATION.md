# EU4 1.37 Linux 迁移记录

分析日期：2026-10-20。目标：Steam 原生 Linux / x86_64 / glibc。

## 原始文件

压缩包 `1.37dlc破解补丁ScreamAPI（Steam学习版用）.7z` 为 574600 字节，SHA-256：

```text
8ec39ef984bcd1e9053615a7a9b5bcbfdf176906695ac0cec85df4e6bf9153f3
```

静态解压得到五个文件：

| 文件 | 大小 / 字节 | 静态特征 |
| --- | ---: | --- |
| cream_api.ini | 8198 | Steam AppID 236850；88 条 DLC 配置 |
| steam_api.dll | 737280 | PE x86；包含 cream_api.ini 字符串 |
| steam_api64.dll | 834560 | PE x86_64；包含 cream_api.ini 字符串 |
| steam_api64_o.dll | 295336 | PE x86_64 |
| steam_api64_org_launcher.dll | 250656 | PE x86_64 |

所有文件的 SHA-256 见 `archive-manifest.json`。检查仅涉及文件列表、头部、字符串及配置；没有执行 DLL，没有据此宣称文件真实来源或安全性。压缩包内的说明和配置注释被作为输入数据处理。

原始配置中 `orgapi64` 实际指向 `steam_api64_org_launcher.dll`，而非另一个随附 DLL。Linux 不复用这些 DLL，也不通过修改扩展名将其转成 ELF。

## 技术方案

Steamworks 在 Linux 使用 `libsteam_api.so`。采用上游已存在的 ELF `LD_PRELOAD` 方案，转发原始 Steam API，适配清单内的 DLC 查询。无需重写完整 SDK 或制作 Windows DLL 转换层。

源代码基线：`anticitizn/creamlinux` 的 `b69b818691983df30b86341be3c8680973295c6d`。保留上游历史，远程命名为 `upstream`。该上游 README 列出 EU4 的支持记录，但没有证明当前迁移版与 EU4 1.37 的特定构建兼容。

当前改动：

- `package/cream_api.ini` 将多游戏预置替换为本次提供的 EU4 清单；ID、名称、次序全部保留，使用 LF 换行。
- `src/config.hpp` 复用随附的 mINI，检查配置读取、非空清单和 uint32 AppID，提供受边界检查的 DLC 元数据复制。
- `SteamAPI_Init` 只加载一次配置，重复初始化仍调用原始 Steam 函数，避免 DLC 清单不断累加；配置错误记录日志并返回失败。
- 修正 `SteamAPI_ISteamApps_BGetDLCDataByIndex`，使用随附 `steam_api_flat.h` 声明的 `ISteamApps* self` 首参数；将相关 flat API 与 `dlsym` 查询接到相同实现。
- 启动脚本明确区分配置文件和目录，校验 ELF 架构，以独立临时目录处理路径空格及并发文件名冲突，并仅为目标命令设置新增预加载库。
- CMake 目标限制为 x86_64 Linux，显式链接动态加载与线程依赖，加入接口测试和构建产物安装规则。

## 原配置对应关系

| Windows 配置 | Linux 处理 |
| --- | --- |
| steam.appid = 236850 | 作为目标项目元数据；游戏 AppID 仍由原版 Steam API / 客户端处理 |
| steam.unlockall = false | 保留显式 `[dlc]` 清单模式 |
| dlc 下的 88 条记录 | 逐条导入 |
| orgapi / orgapi64 | 使用游戏自带 Linux `libsteam_api.so` |
| language 等注释选项 | 不启用，语言继续由原版接口返回 |
| forceoffline / extraprotection / steam_misc | 不进行逐项等价移植，继续采用上游现有行为；已启用的 Windows 特有模式不能假定兼容 |

## 尚需确认

没有提供 EU4 Linux 可执行文件与其 Steam 库，不能从 Windows 包推导出 Linux 游戏所用的实际 SDK ABI。当前复用上游已有的 `STEAMAPPS_INTERFACE_VERSION008` 及现存接口转发逻辑；没有虚构额外版本适配。

随附测试 mock 使用同一组 SDK 头文件，能够验证实现自洽，但无法证明真实游戏 ABI 一致。上线前需要在目标 Linux 环境运行 CTest，再启动实际 EU4 1.37 检查日志中的接口版本、DLC 识别和数据文件加载。测试结果须与补丁版本、游戏版本和系统环境一起记录。

## 参考资料

- Linux 实现与支持说明：https://github.com/anticitizn/creamlinux
- Valve Steamworks API 概览：https://partner.steamgames.com/doc/sdk/api
- Valve ISteamApps 接口：https://partner.steamgames.com/doc/api/ISteamApps

以上资料仅用于技术方案与接口核对；本地压缩包的结论来自本次静态检查。
