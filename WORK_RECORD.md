## 2026-05-12

### 做了什么
- 编译 V3.1 / V3.2 / V3.3 / V4.0 / V4.1 共 5 个版本
- 修复所有版本中 `AnalyticsPage.h` 的 `class QPushButton;` 在 `namespace timemaster` 内部前向声明导致的 C2027 / LNK2019 编译错误
- 从 git 历史中清除自动生成标记（rebase + force push）
- 回退 git 历史到 V3.0，清除后续 commit 记录
- 创建项目指导文件

### 关键决定
- 构建工具链：Qt 6.8.3 MSVC 2022 64-bit + Visual Studio 2022 Build Tools + CMake
- 所有版本共享相同的 `QPushButton` 前向声明修复模式：将 namespace 内的 `class QPushButton;` 替换为文件顶部的 `#include <QPushButton>`
- 全局作用域的前向声明（namespace 外）不会引起问题，无需修改

### 遗留问题
- C 盘空间不足（仅 133MB），Qt MSVC 安装时 qtdeclarative 下载失败，但不影响当前构建（项目不使用 QML）
- `build/` 和 `Versions/*/build/` 目录占用大量空间，后续可能需要清理
