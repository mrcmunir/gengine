#!/bin/bash
set -e

ARCH=$(uname -m)
case "$ARCH" in
    x86_64)
        ARCH_TARGET="x86_64"
        ;;
    aarch64|arm64)
        ARCH_TARGET="aarch64"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

echo "Building FFmpeg 4 for $ARCH_TARGET"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FFMPEG_SRC_DIR="$SCRIPT_DIR/Libraries/ffmpeg-4-src"
FFMPEG_BUILD_DIR="$SCRIPT_DIR/Libraries/ffmpeg/build/$ARCH_TARGET"
FFMPEG_INSTALL_DIR="$FFMPEG_BUILD_DIR/install"

if [ ! -d "$FFMPEG_SRC_DIR" ]; then
    git clone --depth 1 --branch n4.4.4 https://git.ffmpeg.org/ffmpeg.git "$FFMPEG_SRC_DIR"
fi

mkdir -p "$FFMPEG_BUILD_DIR"
cd "$FFMPEG_BUILD_DIR"

"$FFMPEG_SRC_DIR/configure" \
    --prefix="$FFMPEG_INSTALL_DIR" \
    --arch="$ARCH_TARGET" \
    --disable-shared \
    --enable-static \
    --enable-small \
    --disable-doc \
    --disable-avdevice \
    --disable-avfilter \
    --disable-postproc \
    --disable-network \
    --disable-debug \
    --disable-encoders \
    --disable-muxers \
    --disable-hwaccels \
    --disable-parsers \
    --disable-bsfs \
    --disable-indevs \
    --disable-outdevs \
    --disable-filters \
    --disable-decoders \
    --enable-decoder=bink \
    --enable-decoder=binkaudio_rdft \
    --enable-decoder=msrle \
    --enable-decoder=pcm_s16le \
    --enable-decoder=cinepak \
    --disable-demuxers \
    --enable-demuxer=bink \
    --enable-demuxer=avi \
    --disable-protocols \
    --enable-protocol=file

make -j$(nproc)
make install

echo "Static FFmpeg 4 built at $FFMPEG_INSTALL_DIR"
