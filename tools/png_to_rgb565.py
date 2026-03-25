#!/usr/bin/env python3
"""
PNG to RGB565 Raw Binary Converter
Converts PNG images to raw RGB565 format for ESP32 CYD sprite testing
"""

from PIL import Image
import struct
import sys
import os

def png_to_rgb565(png_path, output_path=None, bgr=False):
    """
    Convert PNG to raw RGB565/BGR565 binary file
    
    Args:
        png_path: Path to input PNG file
        output_path: Path to output .rgb565 file
        bgr: If True, pack as BGR565 instead of RGB565
    """
    if not os.path.exists(png_path):
        print(f"Error: File not found: {png_path}")
        return False
    
    if output_path is None:
        base = os.path.splitext(png_path)[0]
        output_path = base + ('.bgr565' if bgr else '.rgb565')
    
    try:
        # Load image and handle transparency
        img_orig = Image.open(png_path)
        width, height = img_orig.size
        
        if img_orig.mode in ('RGBA', 'LA') or (img_orig.mode == 'P' and 'transparency' in img_orig.info):
            img = Image.new('RGB', img_orig.size, (0, 0, 0))
            if img_orig.mode == 'P':
                img_orig = img_orig.convert('RGBA')
            img.paste(img_orig, mask=img_orig.split()[3] if img_orig.mode == 'RGBA' else None)
        else:
            img = img_orig.convert('RGB')
        
        print(f"Converting {png_path} to {'BGR565' if bgr else 'RGB565'}")
        
        with open(output_path, 'wb') as f:
            for y in range(height):
                for x in range(width):
                    r, g, b = img.getpixel((x, y))
                    
                    # Convert 8-bit to 5- or 6-bit
                    r5 = (r >> 3) & 0x1F
                    g6 = (g >> 2) & 0x3F
                    b5 = (b >> 3) & 0x1F
                    
                    # Pack bits
                    if bgr:
                        # BGR565: B in high bits, R in low bits
                        packed = (b5 << 11) | (g6 << 5) | r5
                    else:
                        # RGB565: R in high bits, B in low bits
                        packed = (r5 << 11) | (g6 << 5) | b5
                    
                    # Write as LITTLE-endian uint16
                    f.write(struct.pack('<H', packed))
        
        print(f"  ✓ Saved to {output_path}")
        return True
        
    except Exception as e:
        print(f"Error converting {png_path}: {e}")
        return False

def batch_convert(directory, bgr=False):
    """Convert all PNG files in a directory"""
    converted = 0
    for filename in os.listdir(directory):
        if filename.lower().endswith('.png'):
            if png_to_rgb565(os.path.join(directory, filename), bgr=bgr):
                converted += 1
    print(f"\nBatch conversion complete: {converted} files")

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='PNG to RGB565/BGR565 Converter')
    parser.add_argument('input', help='Input PNG file or directory')
    parser.add_argument('output', nargs='?', help='Output filename (optional)')
    parser.add_argument('--batch', action='store_true', help='Batch convert directory')
    parser.add_argument('--bgr', action='store_true', help='Use BGR565 bit order')
    
    args = parser.parse_args()
    
    if args.batch:
        batch_convert(args.input, bgr=args.bgr)
    else:
        png_to_rgb565(args.input, args.output, bgr=args.bgr)
