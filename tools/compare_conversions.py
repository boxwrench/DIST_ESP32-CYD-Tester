#!/usr/bin/env python3
"""
Compare RGB565 conversion outputs - byte-level analysis
Compares our BGR565 output vs external RGB565 output
"""

import struct
import sys
from pathlib import Path

def read_rgb565_file(filepath):
    """Read RGB565 file and return list of uint16 values"""
    with open(filepath, 'rb') as f:
        data = f.read()
    
    # Unpack as little-endian uint16
    pixels = []
    for i in range(0, len(data), 2):
        pixel = struct.unpack('<H', data[i:i+2])[0]
        pixels.append(pixel)
    
    return pixels

def decode_rgb565(pixel):
    """Decode RGB565 to R,G,B components"""
    r = (pixel >> 11) & 0x1F  # 5 bits
    g = (pixel >> 5) & 0x3F   # 6 bits
    b = pixel & 0x1F           # 5 bits
    return (r, g, b)

def decode_bgr565(pixel):
    """Decode BGR565 to R,G,B components"""
    b = (pixel >> 11) & 0x1F  # 5 bits (blue in high bits)
    g = (pixel >> 5) & 0x3F   # 6 bits
    r = pixel & 0x1F           # 5 bits (red in low bits)
    return (r, g, b)

def compare_files(file1, file2, file1_name="File 1", file2_name="File 2"):
    """Compare two RGB565 files"""
    print(f"\n{'='*70}")
    print(f"Comparing: {file1_name} vs {file2_name}")
    print(f"{'='*70}\n")
    
    pixels1 = read_rgb565_file(file1)
    pixels2 = read_rgb565_file(file2)
    
    if len(pixels1) != len(pixels2):
        print(f"⚠️  SIZE MISMATCH: {len(pixels1)} vs {len(pixels2)} pixels")
        return
    
    print(f"File sizes match: {len(pixels1)} pixels ({len(pixels1)*2} bytes)")
    
    # Sample first 10 pixels
    print(f"\n{'Pixel':<8} {'File1 Raw':<12} {'File1 RGB':<15} {'File2 Raw':<12} {'File2 RGB':<15} {'Match'}")
    print("-" * 80)
    
    matches = 0
    for i in range(min(10, len(pixels1))):
        p1 = pixels1[i]
        p2 = pixels2[i]
        
        # Try decoding as RGB565
        rgb1 = decode_rgb565(p1)
        rgb2 = decode_rgb565(p2)
        
        match = "✓" if p1 == p2 else "✗"
        if p1 == p2:
            matches += 1
        
        print(f"{i:<8} 0x{p1:04X}      {rgb1}     0x{p2:04X}      {rgb2}     {match}")
    
    # Check if files are identical
    total_matches = sum(1 for p1, p2 in zip(pixels1, pixels2) if p1 == p2)
    match_pct = (total_matches / len(pixels1)) * 100
    
    print(f"\nOverall: {total_matches}/{len(pixels1)} pixels match ({match_pct:.1f}%)")
    
    if match_pct == 100:
        print("✅ Files are IDENTICAL")
    elif match_pct == 0:
        print("❌ Files are COMPLETELY DIFFERENT")
        # Check if they're byte-swapped versions
        swapped_matches = sum(1 for p1, p2 in zip(pixels1, pixels2) 
                             if ((p1 >> 8) | ((p1 & 0xFF) << 8)) == p2)
        if swapped_matches == len(pixels1):
            print("💡 Files appear to be BYTE-SWAPPED versions of each other")
    else:
        print(f"⚠️  Files are PARTIALLY DIFFERENT ({100-match_pct:.1f}% differ)")
    
    # Check for BGR reversal
    print("\n--- Checking for BGR vs RGB ---")
    bgr_matches = 0
    for i in range(min(10, len(pixels1))):
        p1 = pixels1[i]
        p2 = pixels2[i]
        
        # Decode File1 as BGR, File2 as RGB
        rgb1_as_bgr = decode_bgr565(p1)
        rgb2_as_rgb = decode_rgb565(p2)
        
        if rgb1_as_bgr == rgb2_as_rgb:
            bgr_matches += 1
    
    if bgr_matches > 7:  # >70% match
        print(f"💡 File1 appears to be BGR format, File2 is RGB format")
    
    print()

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python compare_conversions.py <file1.rgb565> <file2.rgb565>")
        print("\nExample:")
        print("  python compare_conversions.py")
        print("    ..\\test_assets\\fish_bluegill_CURRENT.rgb565")
        print("    ..\\test_assets\\fish_bluegill_EXTERNAL.rgb565")
        sys.exit(1)
    
    file1 = Path(sys.argv[1])
    file2 = Path(sys.argv[2])
    
    if not file1.exists():
        print(f"❌ File not found: {file1}")
        sys.exit(1)
    
    if not file2.exists():
        print(f"❌ File not found: {file2}")
        sys.exit(1)
    
    compare_files(file1, file2, file1.name, file2.name)
