# 字体目录

软件启动时会自动扫描以下位置查找 `.ttf` / `.otf` 字体文件：

1. `assets/fonts/`（这个目录）
2. exe 同级的 `fonts/` 目录（用于发布版）
3. Qt 内嵌资源 `:/fonts/`

找到后会通过 `QFontDatabase` 注册并自动应用为全局字体。

## 推荐字体（接近 Claude 网页的字体观感）

### 英文 / 符号：Inter

- 下载：https://rsms.me/inter/
- 在 `static/` 子目录里取这几个文件丢到本目录：
  - `Inter-Regular.ttf`
  - `Inter-Medium.ttf`
  - `Inter-SemiBold.ttf`
  - `Inter-Bold.ttf`

### 中文：思源黑体 / Noto Sans CJK SC

- 下载：https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/SimplifiedChinese
- 推荐取：
  - `NotoSansCJKsc-Regular.otf`
  - `NotoSansCJKsc-Medium.otf`
  - `NotoSansCJKsc-Bold.otf`

## 不放字体也能跑

如果这里是空的，软件会自动回退到系统字体：

- Windows: Segoe UI Variable + Microsoft YaHei UI
- macOS: SF Pro Text + PingFang SC
- Linux: 系统的 Noto Sans CJK SC

观感会接近系统默认，但与 Claude 网页的字体气质略有差距。

## 如何确认成功加载

启动应用时控制台会打印：
```
[FontLoader] primary= "Inter" cjk= "Noto Sans CJK SC" mono= "..." custom= true
```

`custom= true` 表示从本目录或资源里加载到了自定义字体。
