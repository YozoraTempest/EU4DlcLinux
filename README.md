# EU4DlcLinux

面向 Steam 原生 Linux 版 Europa Universalis IV 1.37 的 DLC 接口适配项目，目标平台是 **x86_64 + glibc**。当前为 0.1.0 开发版：已完成交叉编译与部分本机测试，尚未完成 Linux 动态加载测试和 EU4 游戏实测。

本项目基于 [anticitizn/creamlinux](https://github.com/anticitizn/creamlinux) 的 `b69b818691983df30b86341be3c8680973295c6d`，保留完整历史、MIT 许可证及第三方声明。上游说明文档保存在 [docs/UPSTREAM_README.md](docs/UPSTREAM_README.md)。

## 补丁分析与迁移范围

提供的 7z 中有 `cream_api.ini`、两个替换 Steam DLL、两个附带的 Steam DLL；配置中的 AppID 为 `236850`，`[dlc]` 有 88 项。文件内容符合 CreamAPI 的 Steam 配置形式，包名中的 ScreamAPI 不能作为其实际实现的依据。

本项目提取这 88 条 DLC ID 和名称，使用现有 Linux 接口实现生成 `lib64Creamlinux.so`。原始 DLL 为 Windows PE 文件，未执行，也未作为 Linux 产物分发。

迁移保留显式 DLC 清单模式。`orgapi`、`orgapi64` 等 Windows DLL 路径没有 Linux 对应关系；其余 Windows 平台选项也未实现逐项等价。详情见 [迁移记录](docs/MIGRATION.md) 与 [静态检查清单](docs/archive-manifest.json)。

## 构建

需要 Linux x86_64 环境、C++14 编译器、CMake 3.20+ 和系统构建工具。项目随附 mINI、spdlog 和现有 Steamworks 头文件，构建过程不下载依赖。

Ubuntu / Debian 示例：

```sh
sudo apt-get install build-essential cmake
sh ./build.sh
```

脚本依次配置、编译、执行测试并将产物安装到 `output/`。也可以分别执行：

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$PWD/output"
```

已有 Docker 时可运行 `docker compose run --rm creamlinux-build`。CI 配置会在 Ubuntu 22.04 上构建、测试并上传构建产物；本地建仓尚未触发远程 CI。

## 安装与运行

运行依赖 Steam Linux 客户端、原生 Linux 游戏目录中的原版 `libsteam_api.so` 和实际 DLC 数据文件。这份补丁不包含 DLC 资源，也不实现完整 Steam 模拟器。

1. 将 `output/` 中的 `lib64Creamlinux.so`、`cream.sh`、`cream_api.ini` 复制到原生 Linux 游戏根目录。
2. 在 Steam 中确认游戏使用原生 Linux 版本。
3. 设置启动选项：

```text
sh ./cream.sh %command%
```

也可以在游戏目录中直接运行（Steam 客户端需已启动）：

```sh
sh ./cream.sh ./eu4
```

启动脚本以自身目录定位配置和共享库，保留游戏参数与退出码。它将两个库复制到每次启动独立的 `/tmp/eu4-dlc.*` 目录后预加载，结束时清理该目录；这样支持含空格的游戏路径，并避免固定临时文件名冲突。强制终止或断电可能留下临时目录。

`CREAM_CONFIG_PATH` 表示 **INI 文件完整路径**。`CREAM_STEAM_API` 表示 **原版 Steam 共享库文件完整路径**。游戏根目录没有 Steam 库时会递归查找；找到多个候选时要求显式指定。

```sh
CREAM_CONFIG_PATH="/path with spaces/cream_api.ini" \
CREAM_STEAM_API="/path/to/game/libsteam_api.so" \
sh ./cream.sh ./eu4
```

脚本会检查两个库是否为 x86_64 ELF。移除 Steam 启动选项即可停止注入。

## 验证状态

详见 [docs/VALIDATION.md](docs/VALIDATION.md)。当前已通过 Windows 上的 CMake → Zig Linux 交叉编译、配置及缓冲区测试、Git Bash 下的启动脚本测试。Linux 动态加载测试程序已编译，尚未执行；EU4 1.37 游戏运行、Steam 集成、启动器、多人成就等行为尚未实测。

仓库中的测试覆盖 CRLF 配置、88 项预置清单、无效 AppID、索引和缓冲区边界，以及启动脚本路径、参数、退出码、清理、平台检查和共享库选择。Linux 接口测试另覆盖原函数转发、重复初始化、虚函数接口、flat API 和 `dlsym` 路径。

## 来源与许可

见 [LICENSE](LICENSE) 和 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。Steamworks 头文件保留 Valve 原始声明，并未由本项目重新许可。
