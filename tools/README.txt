cd tools
python3 generate_flag_positions.py        (needs numpy, pandas)  -> writes flag_positions.h here
cp flag_positions.h ../src/
Per-flag corrections (OV / TOROV / REMOVE tables) are at the top of the script.
