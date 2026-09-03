#!/bin/bash
#
# Apollo Kernel Build Script
# For Xiaomi Redmi K30S Ultra (apollo) - Linux 5.10
#

set -e

# === Configuration ===
KERNEL_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_DIR="/home/user/apollo-kernel/output"
TOOLCHAIN_DIR="/home/user/apollo-kernel/toolchains/proton-clang"
DEFCONFIG="apollo_defconfig"
JOBS="${JOBS:-2}"

# === Environment Setup ===
export PATH="$HOME/.local/usr/bin:$TOOLCHAIN_DIR/bin:$PATH"
export LIBRARY_PATH="$HOME/.local/usr/lib/x86_64-linux-gnu:${LIBRARY_PATH:-}"
export CPATH="$HOME/.local/usr/include:${CPATH:-}"
export BISON_PKGDATADIR="$HOME/.local/usr/share/bison"

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export CC=clang
export LD=ld.lld
export AR=llvm-ar
export NM=llvm-nm
export OBJCOPY=llvm-objcopy
export OBJDUMP=llvm-objdump
export STRIP=llvm-strip

MAKEFLAGS="CC=$CC LD=$LD AR=$AR NM=$NM OBJCOPY=$OBJCOPY OBJDUMP=$OBJDUMP STRIP=$STRIP CROSS_COMPILE=$CROSS_COMPILE ARCH=$ARCH"

# === Functions ===
usage() {
    echo "Usage: $0 {defconfig|menuconfig|clean|distclean|dtbs|Image|all|zip}"
    echo ""
    echo "  defconfig  - Generate .config from apollo_defconfig"
    echo "  menuconfig - Run menuconfig"
    echo "  clean      - Remove build artifacts"
    echo "  distclean  - Full clean including .config"
    echo "  dtbs       - Build device tree blobs only"
    echo "  Image      - Build kernel Image only"
    echo "  all        - Build Image + dtbs (default)"
    echo "  zip        - Build and create AnyKernel3 flashable zip"
    echo ""
    echo "Environment variables:"
    echo "  JOBS       - Number of parallel jobs (default: 2)"
    exit 1
}

do_defconfig() {
    echo "==> Generating $DEFCONFIG..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS $DEFCONFIG
    echo "==> .config generated successfully"
}

do_menuconfig() {
    echo "==> Running menuconfig..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS menuconfig
}

do_clean() {
    echo "==> Cleaning build artifacts..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS clean
    echo "==> Clean complete"
}

do_distclean() {
    echo "==> Distcleaning..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS distclean
    echo "==> Distclean complete"
}

do_dtbs() {
    echo "==> Building device trees..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS -j$JOBS dtbs
    echo "==> DTBs built successfully"
}

do_image() {
    echo "==> Building kernel Image..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS -j$JOBS Image
    echo "==> Image built successfully"
}

do_all() {
    echo "==> Building kernel Image + dtbs (jobs=$JOBS)..."
    cd "$KERNEL_DIR"
    make $MAKEFLAGS -j$JOBS Image dtbs
    echo "==> Build complete!"

    # Copy artifacts
    mkdir -p "$OUTPUT_DIR"
    if [ -f "$KERNEL_DIR/arch/arm64/boot/Image" ]; then
        cp "$KERNEL_DIR/arch/arm64/boot/Image" "$OUTPUT_DIR/Image"
        echo "  -> Image copied to $OUTPUT_DIR/"
    fi
    if [ -f "$KERNEL_DIR/arch/arm64/boot/dts/qcom/apollo-sm8250.dtb" ]; then
        cp "$KERNEL_DIR/arch/arm64/boot/dts/qcom/apollo-sm8250.dtb" "$OUTPUT_DIR/apollo-sm8250.dtb"
        echo "  -> apollo-sm8250.dtb copied to $OUTPUT_DIR/"
    fi
    # Also create Image.gz if possible
    if [ -f "$KERNEL_DIR/arch/arm64/boot/Image" ]; then
        gzip -k -f "$KERNEL_DIR/arch/arm64/boot/Image" 2>/dev/null && \
            cp "$KERNEL_DIR/arch/arm64/boot/Image.gz" "$OUTPUT_DIR/Image.gz" 2>/dev/null && \
            echo "  -> Image.gz copied to $OUTPUT_DIR/"
    fi
}

do_zip() {
    do_all
    echo "==> Creating AnyKernel3 flashable zip..."
    ANYKERNEL_DIR="/home/user/apollo-kernel/anykernel"
    if [ ! -d "$ANYKERNEL_DIR" ]; then
        echo "ERROR: AnyKernel3 directory not found at $ANYKERNEL_DIR"
        exit 1
    fi

    # Copy kernel and dtb to AnyKernel3
    cp "$OUTPUT_DIR/Image.gz" "$ANYKERNEL_DIR/Image.gz" 2>/dev/null || \
        cp "$OUTPUT_DIR/Image" "$ANYKERNEL_DIR/Image.gz" 2>/dev/null
    cp "$OUTPUT_DIR/apollo-sm8250.dtb" "$ANYKERNEL_DIR/dtb" 2>/dev/null

    # Create zip
    cd "$ANYKERNEL_DIR"
    ZIP_NAME="apollo-kernel-5.10-$(date +%Y%m%d-%H%M).zip"
    zip -r9 "$OUTPUT_DIR/$ZIP_NAME" . -x "*.git*" "README*" 2>/dev/null
    echo "==> Flashable zip created: $OUTPUT_DIR/$ZIP_NAME"
}

# === Main ===
cd "$KERNEL_DIR"

case "${1:-all}" in
    defconfig)  do_defconfig ;;
    menuconfig) do_menuconfig ;;
    clean)      do_clean ;;
    distclean)  do_distclean ;;
    dtbs)       do_dtbs ;;
    Image)      do_image ;;
    all)        do_all ;;
    zip)        do_zip ;;
    *)          usage ;;
esac
