#!/bin/bash
# Build Release Script for OpenSailingRC-Display
# Author: Philippe Hubert
# License: GPL v3.0

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="OpenSailingRC-Display"
BUILD_ENV="m5stack-core2"

# Functions
print_step() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Check if version is provided
if [ $# -eq 0 ]; then
    print_error "Usage: $0 <version> [description]"
fi

VERSION=$1
DESCRIPTION=${2:-"Release $VERSION"}

# Validate version format (basic check)
if [[ ! $VERSION =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    print_warning "Version format should be vX.Y.Z (e.g., v1.0.0)"
fi

print_step "Building release $VERSION for $PROJECT_NAME"

# Check if PlatformIO is available
if ! command -v platformio &> /dev/null; then
    print_error "PlatformIO not found. Please install PlatformIO CLI."
fi

# Clean previous build
print_step "Cleaning previous build..."
platformio run --target clean

# Build the project
print_step "Building firmware..."
platformio run

# Check if build was successful
if [ ! -f ".pio/build/$BUILD_ENV/firmware.bin" ]; then
    print_error "Build failed. firmware.bin not found."
fi

# Create release directory
RELEASE_DIR="releases/$VERSION"
print_step "Creating release directory: $RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Copy binaries
print_step "Copying binaries..."
cp ".pio/build/$BUILD_ENV/firmware.bin" "$RELEASE_DIR/"
cp ".pio/build/$BUILD_ENV/bootloader.bin" "$RELEASE_DIR/"
cp ".pio/build/$BUILD_ENV/partitions.bin" "$RELEASE_DIR/"

# Copy firmware.elf for debugging (optional)
if [ -f ".pio/build/$BUILD_ENV/firmware.elf" ]; then
    cp ".pio/build/$BUILD_ENV/firmware.elf" "$RELEASE_DIR/"
    print_step "Debug symbols (firmware.elf) included"
fi

# Generate checksums
print_step "Generating checksums..."
cd "$RELEASE_DIR"
sha256sum *.bin > checksums.sha256
cd - > /dev/null

# Get file sizes
FIRMWARE_SIZE=$(ls -lh "$RELEASE_DIR/firmware.bin" | awk '{print $5}')
BOOTLOADER_SIZE=$(ls -lh "$RELEASE_DIR/bootloader.bin" | awk '{print $5}')
PARTITIONS_SIZE=$(ls -lh "$RELEASE_DIR/partitions.bin" | awk '{print $5}')

# Update flash instructions with checksums
print_step "Updating documentation..."
if [ -f "releases/v1.0.0/FLASH_INSTRUCTIONS.md" ]; then
    cp "releases/v1.0.0/FLASH_INSTRUCTIONS.md" "$RELEASE_DIR/"
    # Update checksums in the documentation
    sed -i.bak "s/\*À générer après build\*/$(sha256sum $RELEASE_DIR/firmware.bin | cut -d' ' -f1)/" "$RELEASE_DIR/FLASH_INSTRUCTIONS.md" 2>/dev/null || true
fi

# Create a simple release info file
cat > "$RELEASE_DIR/release_info.txt" << EOF
OpenSailingRC-Display Release Information
========================================

Version: $VERSION
Date: $(date '+%Y-%m-%d %H:%M:%S')
Build Environment: $BUILD_ENV
Description: $DESCRIPTION

File Sizes:
-----------
firmware.bin: $FIRMWARE_SIZE
bootloader.bin: $BOOTLOADER_SIZE
partitions.bin: $PARTITIONS_SIZE

Build Info:
-----------
PlatformIO Version: $(platformio --version)
Build Date: $(date -Iseconds)
Git Commit: $(git rev-parse HEAD 2>/dev/null || echo "Not available")
Git Branch: $(git branch --show-current 2>/dev/null || echo "Not available")

Checksums (SHA256):
------------------
$(cat $RELEASE_DIR/checksums.sha256)
EOF

# Create archive
print_step "Creating release archive..."
ARCHIVE_NAME="${PROJECT_NAME}-${VERSION}.zip"
cd releases
zip -r "$ARCHIVE_NAME" "$VERSION/"
cd - > /dev/null

print_success "Release $VERSION created successfully!"
echo
print_step "Release contents:"
ls -la "$RELEASE_DIR/"
echo
print_step "Archive created: releases/$ARCHIVE_NAME"
echo
print_step "Next steps:"
echo "1. Test the binaries on target hardware"
echo "2. Create GitHub release with releases/$ARCHIVE_NAME"
echo "3. Update README.md with new version info"
echo "4. Tag the commit: git tag $VERSION && git push origin $VERSION"
echo
print_success "Build completed!"