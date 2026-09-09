#!/bin/bash

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

BUILD_DIR="$PROJECT_ROOT/build"
LIB_DIR="$PROJECT_ROOT/lib"
PACKAGE_DIR="$PROJECT_ROOT/package"

QT_DIR="$HOME/Qt/6.11.1/macos"
MACDEPLOYQT="$QT_DIR/bin/macdeployqt"

# ============================================================
# 参数检查
# ============================================================

if [ -z "$1" ]; then
    echo "用法:"
    echo "  ./package.sh 01"
    echo "  ./package.sh hello"
    echo "  ./package.sh 01_hello"
    exit 1
fi

T="$1"

# ============================================================
# 查找练习
# ============================================================

EXDIR=""

for d in "$PROJECT_ROOT"/[0-9][0-9]_*; do

    [ -d "$d" ] || continue

    dir="$(basename "$d")"
    number="${dir%%_*}"
    name="${dir#*_}"

    if [ "$T" = "$number" ] || \
       [ "$T" = "$name" ] || \
       [ "$T" = "$dir" ]; then

        EXDIR="$dir"
        break
    fi

done

# ============================================================
# 没找到
# ============================================================

if [ -z "$EXDIR" ]; then

    echo "错误: 找不到练习 '$T'"
    echo ""
    echo "可用练习:"

    for d in "$PROJECT_ROOT"/[0-9][0-9]_*; do

        [ -d "$d" ] || continue

        dir="$(basename "$d")"
        number="${dir%%_*}"
        name="${dir#*_}"

        echo "  $number -> $name"

    done

    exit 1
fi

# ============================================================
# 获取 target 名
#
# 01_hello -> hello
# ============================================================

TARGET="${EXDIR#*_}"

echo "======================================"
echo " Qt Practice Package"
echo "======================================"
echo "练习目录 : $EXDIR"
echo "Target   : $TARGET"
echo ""

# ============================================================
# 1. CMake 配置
# ============================================================

echo "[1/4] CMake 配置..."

cmake -S "$PROJECT_ROOT" \
      -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$QT_DIR"

# ============================================================
# 2. 只编译指定 Target
# ============================================================

echo ""
echo "[2/4] 编译 $TARGET..."

cmake --build "$BUILD_DIR" \
      --target "$TARGET" \
      -j"$(sysctl -n hw.ncpu)"

# ============================================================
# 3. 查找 App
# ============================================================

APP="$LIB_DIR/$EXDIR/$TARGET.app"

if [ ! -d "$APP" ]; then
    echo ""
    echo "错误: 找不到 App:"
    echo "$APP"
    exit 1
fi

echo ""
echo "[3/4] 部署 Qt..."

"$MACDEPLOYQT" "$APP" \
    -always-overwrite

# ============================================================
# 4. ZIP
# ============================================================

mkdir -p "$PACKAGE_DIR"

ZIP="$PACKAGE_DIR/$TARGET.zip"

rm -f "$ZIP"

echo ""
echo "[4/4] 创建 ZIP..."

ditto \
    -c \
    -k \
    --sequesterRsrc \
    --keepParent \
    "$APP" \
    "$ZIP"

echo ""
echo "======================================"
echo " 打包完成"
echo "======================================"
echo ""
echo "App:"
echo "  $APP"
echo ""
echo "ZIP:"
echo "  $ZIP"