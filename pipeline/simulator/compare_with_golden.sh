sup=--suppress-common-line
#sup= 
set -x
./pipeline
cp ./iimage.bin ../golden/iimage.bin
cp ./dimage.bin ../golden/dimage.bin
(cd ../golden/ ; ./pipeline)
diff ./snapshot.rpt ../golden/snapshot.rpt --side-by-side ${sup}
diff ./error_dump.rpt ../golden/error_dump.rpt --side-by-side ${sup}
