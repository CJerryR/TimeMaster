#!/usr/bin/env bash
# ============================================================
# make_icns.sh — Cadence app icon builder
# 把 src/assets/icon.svg 渲染为多分辨率 PNG 并合成为 .icns
#
# 用法（在 macOS 上）：
#   chmod +x scripts/make_icns.sh
#   ./scripts/make_icns.sh
# 输出：build/icon.icns 和 build/icon.iconset/
#
# 依赖：
#   - 已装 Xcode CLT（提供 iconutil / sips / qlmanage）
#   - 推荐：brew install librsvg  (rsvg-convert, 渲染最准)
#     退而求其次：brew install imagemagick
# ============================================================
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SVG="$ROOT/src/assets/icon.svg"
OUT="$ROOT/build"
ICONSET="$OUT/icon.iconset"

if [ ! -f "$SVG" ]; then
    echo "❌ 找不到 $SVG"
    exit 1
fi

mkdir -p "$ICONSET"

# 检测可用的 SVG 渲染器
RENDERER=""
if command -v rsvg-convert >/dev/null 2>&1; then
    RENDERER="rsvg"
elif command -v magick >/dev/null 2>&1; then
    RENDERER="magick"
elif command -v convert >/dev/null 2>&1; then
    RENDERER="convert"
else
    echo "❌ 需要 rsvg-convert (推荐) 或 imagemagick。"
    echo "   brew install librsvg     # 推荐"
    echo "   brew install imagemagick # 备选"
    exit 1
fi

render() {
    local size="$1"; local out="$2"
    case "$RENDERER" in
        rsvg) rsvg-convert -w "$size" -h "$size" "$SVG" -o "$out" ;;
        magick) magick -background none -density 600 "$SVG" \
                       -resize "${size}x${size}" "$out" ;;
        convert) convert -background none -density 600 "$SVG" \
                          -resize "${size}x${size}" "$out" ;;
    esac
}

# Apple 要求的 iconset 9 张图（mac OS 标准）
echo "📐 渲染各分辨率..."
render 16    "$ICONSET/icon_16x16.png"
render 32    "$ICONSET/icon_16x16@2x.png"
render 32    "$ICONSET/icon_32x32.png"
render 64    "$ICONSET/icon_32x32@2x.png"
render 128   "$ICONSET/icon_128x128.png"
render 256   "$ICONSET/icon_128x128@2x.png"
render 256   "$ICONSET/icon_256x256.png"
render 512   "$ICONSET/icon_256x256@2x.png"
render 512   "$ICONSET/icon_512x512.png"
render 1024  "$ICONSET/icon_512x512@2x.png"

# 合成 .icns
echo "📦 合成 icon.icns ..."
iconutil -c icns -o "$OUT/icon.icns" "$ICONSET"

# 顺手生成一份 1024 PNG，方便 README/截图使用
cp "$ICONSET/icon_512x512@2x.png" "$OUT/icon-1024.png"

echo "✅ 完成"
echo "   $OUT/icon.icns"
echo "   $OUT/icon-1024.png"
