#!/usr/bin/env python3
"""
Wrapper/comparison script for external PNG-to-RGB565 converter
Downloads and uses jimmywong2003/PNG-to-RGB565 tool for comparison
"""

import subprocess
import sys
import os
from pathlib import Path

# External tool source
EXTERNAL_TOOL_URL = "https://raw.githubusercontent.com/jimmywong2003/PNG-to-RGB565/master/png2rgb565.py"
EXTERNAL_TOOL_PATH = Path(__file__).parent / "external_png2rgb565.py"

def download_external_tool():
    """Download the verified external PNG converter"""
    import urllib.request
    
    if EXTERNAL_TOOL_PATH.exists():
        print(f"✓ External tool already downloaded: {EXTERNAL_TOOL_PATH}")
        return True
    
    try:
        print(f"Downloading external tool from {EXTERNAL_TOOL_URL}...")
        urllib.request.urlretrieve(EXTERNAL_TOOL_URL, EXTERNAL_TOOL_PATH)
        print(f"✓ Downloaded to {EXTERNAL_TOOL_PATH}")
        return True
    except Exception as e:
        print(f"✗ Failed to download: {e}")
        return False

def run_external_converter(png_path, output_path):
    """Run the external converter"""
    if not EXTERNAL_TOOL_PATH.exists():
        if not download_external_tool():
            return False
    
    # External tool expects: python png2rgb565.py <image_file> <output_include_file> <output_binary_file>
    h_file = output_path.replace('.rgb565', '.h')
    
    try:
        cmd = [sys.executable, str(EXTERNAL_TOOL_PATH), png_path, h_file, output_path]
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ External tool succeeded")
            print(result.stdout)
            return True
        else:
            print(f"✗ External tool failed:")
            print(result.stderr)
            return False
    except Exception as e:
        print(f"✗ Error running external tool: {e}")
        return False

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python run_external_converter.py <input.png> <output.rgb565>")
        sys.exit(1)
    
    png_path = sys.argv[1]
    output_path = sys.argv[2]
    
    if not os.path.exists(png_path):
        print(f"✗ Input file not found: {png_path}")
        sys.exit(1)
    
    success = run_external_converter(png_path, output_path)
    sys.exit(0 if success else 1)
