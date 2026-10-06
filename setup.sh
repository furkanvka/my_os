#!/bin/bash
# =============================================================================
# my_os - Cross Compiler & Bağımlılık Kurulum Betiği
# OSDev Wiki rehberine uygun olarak i686-elf-gcc cross-compiler derler.
# https://wiki.osdev.org/GCC_Cross-Compiler
# =============================================================================

set -e

# Renkli çıktı
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info() { echo -e "${GREEN}[INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }


# --- 2. Adım: Cross-compiler zaten var mı kontrol et ---
if command -v i686-elf-gcc &>/dev/null; then
    info "i686-elf-gcc zaten kurulu: $(i686-elf-gcc --version | head -1)"
    info "Kurulum tamamlandı!"
    exit 0
fi

# --- 3. Adım: Kaynak kodları indir ve derle ---
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

BINUTILS_VERSION="2.41"
GCC_VERSION="13.2.0"
SRC_DIR="$HOME/src-cross-compiler"

mkdir -p "$SRC_DIR"
cd "$SRC_DIR"

# Binutils
if [ ! -f "binutils-${BINUTILS_VERSION}.tar.xz" ]; then
    info "Binutils ${BINUTILS_VERSION} indiriliyor..."
    wget -q "https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
fi

if [ ! -d "binutils-${BINUTILS_VERSION}" ]; then
    info "Binutils açılıyor..."
    tar xf "binutils-${BINUTILS_VERSION}.tar.xz"
fi

# GCC
if [ ! -f "gcc-${GCC_VERSION}.tar.xz" ]; then
    info "GCC ${GCC_VERSION} indiriliyor..."
    wget -q "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"
fi

if [ ! -d "gcc-${GCC_VERSION}" ]; then
    info "GCC açılıyor..."
    tar xf "gcc-${GCC_VERSION}.tar.xz"
fi

# --- Binutils derleme ---
info "Binutils derleniyor (bu biraz sürebilir)..."
mkdir -p build-binutils
cd build-binutils
../"binutils-${BINUTILS_VERSION}/configure" \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror
make -j$(nproc)
make install
cd "$SRC_DIR"

# --- GCC derleme ---
info "GCC derleniyor (bu uzun sürebilir, ~10-20 dk)..."
mkdir -p build-gcc
cd build-gcc
../"gcc-${GCC_VERSION}/configure" \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c \
    --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
cd "$SRC_DIR"

# --- 4. Adım: PATH'e ekle ---
info "PATH ayarlanıyor..."

SHELL_RC=""
if [ -f "$HOME/.bashrc" ]; then
    SHELL_RC="$HOME/.bashrc"
elif [ -f "$HOME/.zshrc" ]; then
    SHELL_RC="$HOME/.zshrc"
fi

if [ -n "$SHELL_RC" ]; then
    if ! grep -q 'opt/cross/bin' "$SHELL_RC"; then
        echo "" >> "$SHELL_RC"
        echo '# i686-elf Cross Compiler' >> "$SHELL_RC"
        echo 'export PATH="$HOME/opt/cross/bin:$PATH"' >> "$SHELL_RC"
        info "PATH, $SHELL_RC dosyasına eklendi."
    else
        info "PATH zaten $SHELL_RC içinde tanımlı."
    fi
fi

# Doğrulama
export PATH="$HOME/opt/cross/bin:$PATH"
info "========================================="
info "Kurulum tamamlandı!"
info "i686-elf-gcc: $(i686-elf-gcc --version | head -1)"
info "i686-elf-as:  $(i686-elf-as --version | head -1)"
info "========================================="
info ""
info "Yeni terminalde veya 'source $SHELL_RC' ile PATH'i yükle,"
info "sonra projeye dönüp 'make run' çalıştır."
