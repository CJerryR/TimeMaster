# 在 Mac 上编译 时智 (TimePlan)

本文档讲解如何在 macOS 上从源码编译出 `TimePlan.app`。

## 1. 前置依赖

### 1.1 安装 Xcode 命令行工具

```bash
xcode-select --install
```

如已安装会跳过。

### 1.2 安装 Homebrew (如未安装)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 1.3 安装 Qt 6 + CMake

```bash
brew install qt@6 cmake
```

> 整个 Qt 6 大约 1.5 GB，下载需要几分钟。

安装完成后确认版本：

```bash
$(brew --prefix qt@6)/bin/qmake6 -v
# 应该看到 Qt version 6.x.x
cmake --version
# 应该 ≥ 3.16
```

## 2. 编译

进入解压后的项目目录：

```bash
cd path/to/timeplan-cpp
```

### 2.1 配置（生成构建文件）

```bash
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) -DCMAKE_BUILD_TYPE=Release
```

参数说明：
- `-B build`：把生成的 Makefile / Ninja 文件放在 `./build/` 目录
- `-DCMAKE_PREFIX_PATH=...`：让 CMake 找到 brew 安装的 Qt
- `-DCMAKE_BUILD_TYPE=Release`：开优化（默认是 Debug，跑起来会卡）

### 2.2 编译

```bash
cmake --build build -j
```

`-j` 会自动用满 CPU 多核，Apple Silicon 上大约 30 秒到 2 分钟。

### 2.3 运行

```bash
open build/TimePlan.app
```

或双击 `build/TimePlan.app`。

## 3. 打包独立 .app（可选，分发用）

如果要把 `.app` 给其他没装 Qt 的 Mac，需要把 Qt 框架打包进 bundle：

```bash
cmake --install build --prefix dist
```

完成后 `dist/TimePlan.app` 是自包含的，可直接拷贝到其他 Mac。

如果上面命令找不到 `macdeployqt`，可以手动跑：

```bash
$(brew --prefix qt@6)/bin/macdeployqt build/TimePlan.app
```

## 4. 常见问题

### 4.1 `Qt6 not found`

CMake 找不到 Qt。检查路径：

```bash
echo $(brew --prefix qt@6)
# 应该输出类似 /opt/homebrew/opt/qt@6 (Apple Silicon) 或 /usr/local/opt/qt@6 (Intel)
ls $(brew --prefix qt@6)/lib/cmake/Qt6
# 应该能看到 Qt6Config.cmake
```

如果路径不对，换成：

```bash
export Qt6_DIR=/opt/homebrew/opt/qt@6/lib/cmake/Qt6
cmake -B build
```

### 4.2 应用启动后白屏 / 闪退

通常是字体问题。本应用默认使用 PingFang SC，macOS 自带，应该不会有问题。如果有，编辑 `src/main.cpp` 注释掉字体设置那几行重新编译即可。

### 4.3 "已损坏，无法打开"

如果分发 .app 给别人，未签名的 app 在新 Mac 上首次打开会被 Gatekeeper 拦截。运行：

```bash
xattr -cr /path/to/TimePlan.app
```

或在 设置 → 隐私与安全性 中允许打开。

### 4.4 Apple Silicon vs Intel

Brew 在两种架构上的 Qt 路径不同：
- Apple Silicon: `/opt/homebrew/opt/qt@6`
- Intel: `/usr/local/opt/qt@6`

`$(brew --prefix qt@6)` 自动展开为正确路径。

### 4.5 想编译为 Universal Binary（同时支持 Intel + ARM）

```bash
cmake -B build \
    -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

需要 brew 的 Qt 也是 Universal 才能成功。

## 5. 卸载（清除应用数据）

```bash
rm -rf "$HOME/Library/Application Support/TimeplanCpp"
defaults delete com.timeplan.app 2>/dev/null
defaults delete TimeplanCpp 2>/dev/null
```

---

如有编译问题，请检查：
- Qt 版本 ≥ 6.2
- CMake 版本 ≥ 3.16
- C++ 17 编译器（Clang 11+，Xcode 12+ 自带满足）
