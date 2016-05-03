set -x
./assembler -o iimage.bin -a assembly.s
./ge_dimage
cp ./iimage.bin ../simulator/iimage.bin -f
cp ./dimage.bin ../simulator/dimage.bin -f
