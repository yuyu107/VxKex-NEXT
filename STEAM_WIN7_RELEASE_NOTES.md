# Steam on Windows 7 — VxKex NEXT touch API compatibility fix v0.1

> **Pre-release / 预发行测试版** · Independent fork of [VxKex NEXT](https://github.com/YuZhouRen86/VxKex-NEXT).

## 中文说明

本测试版用于解决部分 **2026 年 10 月版 Steam** 在 **Windows 7 SP1** 上启用 VxKex NEXT 后，因缺少触摸 API 而导致 SteamUI 无法加载的问题（可能提示 `Failed to load steamui.dll`）。

### 修改内容

在 `KxUser.dll` 中补充以下函数的导出和兼容入口：

- `InitializeTouchInjection`
- `InjectTouchInput`

Windows 7 原生的 `USER32.dll` 不提供这些 Windows 8 API。本测试版中的两个函数是**占位兼容实现**：返回 `FALSE`，并将 `GetLastError()` 设置为 `ERROR_CALL_NOT_IMPLEMENTED`。**它们不支持真正的模拟触摸输入**。目标是让具有上述静态导入的 SteamUI 在特定配置下完成加载，不意味着 Steam 的全部功能已经兼容。

### 已验证

- Windows 7 SP1 **64 位**；
- 使用**未修改的** 2026 年 10 月版 `SteamUI.dll`；
- Steam 可进入主界面；
- 游戏下载和游戏启动在测试环境中正常。

### 已知问题

- **Steam 游戏内界面 / Steam Overlay（Shift+Tab）仍不可用**；
- 可能仍弹出 **Steam Client Service 需要维护** 的提示；
- 不提供触摸注入功能；
- **32 位 Windows 7、Windows 8/8.1 以及其他 Steam 更新版本尚未充分测试**；
- Steam 其他新 API 依赖或后续更新仍可能造成新的兼容性问题。

### 安装与回退

1. **请先备份现有 VxKex NEXT 配置，并建议在虚拟机或有系统还原点的设备上测试。**
2. 当前安装器**不能可靠地覆盖安装相同版本**：若提示“VxKex NEXT 已经安装”，先从控制面板卸载现有版本，卸载时选择**保留兼容性设置**，再安装本测试版。不要尝试将两个版本并行安装。
3. 安装完成后，在需要的 Steam 程序上启用 VxKex NEXT；不要盲目给所有 Steam 子进程或游戏启用兼容层。
4. 使用官方 Steam 安装文件和未修改的 `SteamUI.dll` 进行验证。本补丁**不包含 Steam 文件**。
5. 如果 Steam 更新后出现新的错误，先确认客户端版本是否仍与本次测试相同。此前使用过的 `steam.cfg` 更新抑制设置与本修复是两件独立的事。
6. 遇到问题时，卸载测试版、保留配置并重新安装先前备份的 VxKex NEXT 版本。

**请注意：** GitHub Actions 编译成功与在单台测试机器上启动成功，不代表适用于所有 Windows 7 设备。欢迎在 Issues 中提供 Windows 版本、Steam 版本、报错截图及日志。

## English summary

This experimental build adds `InitializeTouchInjection` and `InjectTouchInput` exports to `KxUser.dll` to address missing Windows 8 touch API imports used by some October 2026 SteamUI builds on Windows 7 SP1 x64.

Both functions are **nonfunctional compatibility stubs**, returning `FALSE` with `ERROR_CALL_NOT_IMPLEMENTED`. They do **not** emulate touch input.

Confirmed in a Windows 7 SP1 x64 test: an unmodified SteamUI.dll loads, the Steam UI opens, and game download/launch functionality works. Steam Overlay is still unavailable and a Steam Client Service maintenance prompt may appear.

Existing VxKex NEXT installations may need to be uninstalled first; select the option to preserve settings and back up before switching. This build has not been broadly validated on 32-bit systems or other Windows versions.

### Download

Use the **Release** build of the installer (not Debug), attached to the GitHub Release. Source changes: `01-Extended DLLs/KxUser/pointer.c` and `kxuser.def`.

Upstream credit: [YuZhouRen86/VxKex-NEXT](https://github.com/YuZhouRen86/VxKex-NEXT).
