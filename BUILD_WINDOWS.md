# 在 Windows 上编译《时间管理大师》

> ⚠️ 由于源码是在 Linux 环境下打包的，本压缩包**不含 Windows 可执行文件**。
> 按以下步骤即可在自己的 Windows 机器上编译出 `TimeMaster.exe`。

## 一、一次性环境准备（10-20 分钟）

### 1. 安装 Qt 6（带 MSVC 组件）

打开 <https://www.qt.io/download-open-source>，下载 **Qt Online Installer**，安装时勾选：

```
Qt 6.7.x（或更新版本）
  └ MSVC 2022 64-bit            ← 必选
  └ Qt Network                  ← AI 调用需要
  └ Qt SQL                      ← 数据库需要
Developer and Designer Tools
  └ CMake                       ← 通常会自动勾上
  └ Ninja
```

默认路径是 `C:\Qt`。安装完毕后无需配置环境变量，构建脚本会自动在 `C:\Qt` 下查找。

> 如果你嫌官方安装器太慢，也可以用 [aqtinstall](https://github.com/miurahr/aqtinstall)：
> ```powershell
> pip install aqtinstall
> aqt install-qt windows desktop 6.7.3 win64_msvc2022_64 -O C:\Qt
> ```

### 2. 安装 Visual Studio 2022（或 2019）

下载 [Visual Studio Community 2022](https://visualstudio.microsoft.com/zh-hans/vs/community/)（免费），安装时**只勾选**：

```
工作负载 → 使用 C++ 的桌面开发
```

其余可以全部跳过。

## 二、编译

1. 把压缩包解压到任意目录，例如 `D:\code\time-master\`。
2. 双击 `build_windows.bat`，或者在文件管理器里 Shift+右键 → 在此处打开 PowerShell：
   ```powershell
   .\build_windows.bat
   ```
3. 等待几分钟，看到 `Build succeeded` 即完成。

生成产物：

```
build\Release\
  ├ TimeMaster.exe        ← 主程序，双击运行
  ├ Qt6Core.dll
  ├ Qt6Widgets.dll
  ├ Qt6Network.dll
  ├ Qt6Sql.dll
  ├ platforms\
  ├ tls\
  └ sqldrivers\
```

> 这个 `Release` 文件夹是**自包含**的，可以直接打包发给别人，对方不需要装 Qt 就能跑。

## 三、首次运行

1. 双击 `build\Release\TimeMaster.exe`。
2. 进入应用后点左下角的 **⚙ 设置**，填写 DeepSeek API Key（[在这里申请](https://platform.deepseek.com/api_keys)）。
3. 在日历页顶部的 AI 输入框试试：
   > 「明天下午 3 点项目评审；周三上午健身一小时」
4. AI 误识别？点 **🕐 导入历史**，选中那一批 → **撤销整批**。

## 四、用户数据位置

```
%LOCALAPPDATA%\TimeMaster\timemaster.db
```

把这个文件备份起来就完整保留了所有日程。

## 五、常见问题

### Qt 找不到怎么办？

把 Qt 路径作为参数传给脚本：
```bat
build_windows.bat C:\Qt\6.7.3\msvc2022_64
```

### 编译时报 cl.exe 找不到

打开「**x64 Native Tools Command Prompt for VS 2022**」（开始菜单里搜），然后在该窗口里 `cd` 到项目目录再跑 `build_windows.bat`。

### 中文乱码 / 编码错误

确保 Windows 区域设置的「Beta 版：使用 UTF-8 提供全球语言支持」**不要**勾选（默认就是不勾），CMake 已经显式指定了 `/utf-8`。

### 运行后窗口空白 / 字体方块

请确认系统已安装「微软雅黑」（YaHei UI）。Windows 自带，一般无需处理。
