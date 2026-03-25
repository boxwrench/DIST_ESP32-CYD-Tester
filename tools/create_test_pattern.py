#!/usr/bin/env python3
"""
Generate a vibrant test pattern RGB565 file for ESP32 CYD sprite color debugging.
Creates a 32x32 pattern with pure saturated colors.
"""

import struct
import os

# RGB565 pure colors (as they would appear in big-endian format before byte swap)
# These are the standard TFT_eSPI color values
COLORS = {
    'RED':     0xF800,  # 11111 000000 00000
    'GREEN':   0x07E0,  # 00000 111111 00000
    'BLUE':    0x001F,  # 00000 000000 11111
    'YELLOW':  0xFFE0,  # 11111 111111 00000
    'CYAN':    0x07FF,  # 00000 111111 11111
    'MAGENTA': 0xF81F,  # 11111 000000 11111
    'WHITE':   0xFFFF,  # 11111 111111 11111
    'BLACK':   0x0000,  # 00000 000000 00000
}

def create_test_pattern(output_path, width=32, height=32, bgr=False):
    """
    Create a 32x32 test pattern with 8 color bands
    """
    # RGB values for pure colors
    # (Red, Green, Blue)
    pure_colors = [
        (255, 0, 0),    # RED
        (0, 255, 0),    # GREEN
        (0, 0, 255),    # BLUE
        (255, 255, 0),  # YELLOW
        (0, 255, 255),  # CYAN
        (255, 0, 255),  # MAGENTA
        (255, 255, 255),# WHITE
        (0, 0, 0),      # BLACK
    ]
    
    band_height = height // len(pure_colors)
    
    with open(output_path, 'wb') as f:
        for y in range(height):
            color_idx = min(y // band_height, len(pure_colors) - 1)
            r, g, b = pure_colors[color_idx]
            
            # Convert to 5-6-5
            r5, g6, b5 = (r >> 3) & 0x1F, (g >> 2) & 0x3F, (b >> 3) & 0x1F
            
            if bgr:
                # BGR565: B in high bits, R in low bits
                color = (b5 << 11) | (g6 << 5) | r5
            else:
                # RGB565: R in high bits, B in low bits
                color = (r5 << 11) | (g6 << 5) | b5
            
            for x in range(width):
                f.write(struct.pack('<H', color))
    
    print(f"Created: {output_path} ({'BGR565' if bgr else 'RGB565'})")

def create_simple_rgb_test(output_path, width=30, height=30, bgr=False):
    """
    Create a simple 3-stripe test: RED | GREEN | BLUE
    """
    stripe_width = width // 3
    
    with open(output_path, 'wb') as f:
        for y in range(height):
            for x in range(width):
                if x < stripe_width:
                    r, g, b = (255, 0, 0)
                elif x < stripe_width * 2:
                    r, g, b = (0, 255, 0)
                else:
                    r, g, b = (0, 0, 255)
                
                r5, g6, b5 = (r >> 3) & 0x1F, (g >> 2) & 0x3F, (b >> 3) & 0x1F
                if bgr:
                    color = (b5 << 11) | (g6 << 5) | r5
                else:
                    color = (r5 << 11) | (g6 << 5) | b5
                    
                f.write(struct.pack('<H', color))
    
    print(f"Created RGB stripe test: {output_path} ({'BGR565' if bgr else 'RGB565'})")

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Test Pattern Generator')
    parser.add_argument('output_dir', nargs='?', default='.', help='Output directory')
    parser.add_argument('--bgr', action='store_true', help='Use BGR565 bit order')
    
    args = parser.parse_args()
    
    # Create 8-band color pattern
    pattern_path = os.path.join(args.output_dir, 'test_pattern_32x32.rgb565')
    create_test_pattern(pattern_path, bgr=args.bgr)
    
    # Create simple RGB stripe test
    rgb_path = os.path.join(args.output_dir, 'test_rgb_30x30.rgb565')
    create_simple_rgb_test(rgb_path, bgr=args.bgr)
    
    print("\nCopy these files to SD card /sprite_tests/ folder")

