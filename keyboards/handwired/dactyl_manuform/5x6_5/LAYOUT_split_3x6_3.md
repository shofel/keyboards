# LAYOUT_split_3x6_3 Layout Guide

## Overview
This layout maps the standard `LAYOUT_split_3x6_3` (42 keys: 3 rows × 6 cols × 2 hands + 3 thumb keys × 2 hands) to the Dactyl Manuform 5x6_5 physical matrix.

## Matrix Mapping

### Main Keys (36 keys)
- **split_3x6_3 row 0** → 5x6_5 matrix rows **1** (left) and **7** (right)
- **split_3x6_3 row 1** → 5x6_5 matrix rows **2** (left) and **8** (right)
- **split_3x6_3 row 2** → 5x6_5 matrix rows **3** (left) and **9** (right)

### Thumb Keys (6 keys)
- **Left**: `[4,2]`, `[4,3]`, `[4,4]` (first 3 accessible positions from row 4)
- **Right**: `[11,2]`, `[11,0]`, `[10,1]` (first 3 accessible thumb positions)

## Reference Files
- **Standard layout**: `~/workspaces-one/qmk_firmware/layouts/default/split_3x6_3/info.json`
- **Keyboard structure**: `~/workspaces-one/qmk_firmware/keyboards/handwired/dactyl_manuform/5x6_5/keyboard.json`

## Notes
- Row 0 of 5x6_5 is unused (top row)
- Row 4 left side only has thumb keys, not full 6-key row
- **x/y coordinates are inherited from Dactyl's keyboard.json** - look up each matrix position in `keyboard.json` to get the correct coordinates

