set -x
./assembler -d iimage.bin -o disassembled_check.s
cat ./disassembled_check.s