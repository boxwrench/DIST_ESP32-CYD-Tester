# Quick Start: Generate Test Files

## Step 1: Generate EXTERNAL tool output (RGB565)
```powershell
cd C:\GitHub\DIST_ESP32-CYD-Tester
python tools\external_png2rgb565.py test_assets\fish_bluegill_32x32.png test_assets\fish_bluegill_EXTERNAL.h test_assets\fish_blug_EXTERNAL.rgb565
```

## Step 2: Rename current BGR565 file for comparison
```powershell
copy test_assets\fish_bluegill_32x32.rgb565 test_assets\fish_bluegill_CURRENT.rgb565
```

## Step 3: Compare the outputs
```powershell
python tools\compare_conversions.py test_assets\fish_bluegill_CURRENT.rgb565 test_assets\fish_bluegill_EXTERNAL.rgb565
```

## Step 4: Copy BOTH files to SD card
```
SD Card: /sprite_tests/
- fish_bluegill_CURRENT.rgb565
- fish_bluegill_EXTERNAL.rgb565  
```

## Step 5: Flash simplified diagnostic firmware
```powershell
cd sprite_test_firmware_diagnostic
pio run -t upload
```

## Expected Results

The diagnostic firmware will show 4 tests, each for 30 seconds:
1. Reference colors (fillRect)
2. EXTERNAL file + swap=true
3. EXTERNAL file + swap=false
4. CURRENT file + swap=true
5. CURRENT file + swap=false

**Take photos of each screen to identify which shows vibrant colors!**
