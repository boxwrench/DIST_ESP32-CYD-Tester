#!/usr/bin/env python3
"""Debug RGB565 conversion by comparing source PNG to converted file"""

from PIL import Image
import struct

# Load source PNG
img = Image.open(r'..\test_assets\fish_bluegill_32x32.png').convert('RGB')
width, height = img.size
print(f"Image size: {width}x{height}")

# Sample some pixels from the fish
test_points = [
    (24, 16, "center"),
    (10, 10, "upper-left body"),
    (35, 16, "right side"),
    (24, 5, "top fin"),
]

print("\n=== Color Comparison ===")
print("Checking if RGB565 conversion preserves color vibrancy...\n")

with open(r'..\test_assets\fish_bluegill_32x32.rgb565', 'rb') as f:
    rgb565_data = f.read()

for x, y, desc in test_points:
    # Get RGB from source PNG
    r, g, b = img.getpixel((x, y))
    
    # Calculate expected RGB565
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    expected = (r5 << 11) | (g6 << 5) | b5
    
    # Read actual from file (little-endian)
    offset = (y * width + x) * 2
    actual = struct.unpack_from('<H', rgb565_data, offset)[0]
    
    # Decode back to RGB for comparison
    actual_r5 = (actual >> 11) & 0x1F
    actual_g6 = (actual >> 5) & 0x3F
    actual_b5 = actual & 0x1F
    actual_r = actual_r5 << 3
    actual_g = actual_g6 << 2
    actual_b = actual_b5 << 3
    
    print(f"[{desc}] at ({x},{y}):")
    print(f"  Source PNG:     RGB({r:3d}, {g:3d}, {b:3d})")
    print(f"  Expected RGB565: 0x{expected:04X}")
    print(f"  Actual file:     0x{actual:04X}")
    
    if expected == actual:
        print(f"  ✓ MATCH - decoded back to RGB({actual_r:3d}, {actual_g:3d}, {actual_b:3d})")
    else:
        # Check if it's byte-swapped
        swapped = ((actual & 0xFF) << 8) | ((actual >> 8) & 0xFF)
        if swapped == expected:
            print(f"  ⚠ BYTE ORDER MISMATCH - file is big-endian!")
        else:
            print(f"  ✗ VALUE MISMATCH!")
            print(f"    Decoded: RGB({actual_r:3d}, {actual_g:3d}, {actual_b:3d})")
    print()

# Check overall color saturation
print("=== Saturation Check ===")
max_r, max_g, max_b = 0, 0, 0
for y in range(height):
    for x in range(width):
        r, g, b = img.getpixel((x, y))
        max_r = max(max_r, r)
        max_g = max(max_g, g)
        max_b = max(max_b, b)

print(f"Max R in source: {max_r} (should be close to 255 for vibrant)")
print(f"Max G in source: {max_g}")
print(f"Max B in source: {max_b}")

if max_r < 200 and max_g < 200 and max_b < 200:
    print("\n⚠ SOURCE IMAGE has muted colors - this may be the issue!")
else:
    print("\n✓ Source image has full color range")
