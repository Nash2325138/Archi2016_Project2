CC = g++ -std=c++11 -Wall
ALL_OBJ = simulator.o instruction.o regfile.o memory.o
simulator: ${ALL_OBJ}
	${CC} -o single_cycle ${ALL_OBJ}
allObj: ./*cpp
	${CC} -c ./*.cpp
simulator.o: simulator.cpp
	${CC} -c simulator.cpp
instruction.o: instruction.cpp
	${CC} -c instruction.cpp
regfile.o: regfile.cpp
	${CC} -c regfile.cpp
memory.o: memory.cpp
	${CC} -c memory.cpp
clean:
	rm -f single_cycle ${ALL_OBJ}

snapshot_golden_compare: snapshot.rpt ./TA_golden/snapshot.rpt
	diff --side-by-side --suppress-common-lines snapshot.rpt ./TA_golden/snapshot.rpt
error_dump_golden_compare: error_dump.rpt ./TA_golden/error_dump.rpt
	diff --side-by-side --suppress-common-lines error_dump.rpt ./TA_golden/error_dump.rpt
redo_simulate:
	rm -f snapshot.rpt error_dump.rpt
	rm -f iimage.bin dimage.bin
	cp ./image_generator/iimage.bin ./iimage.bin
	cp ./image_generator/dimage.bin ./dimage.bin
	./single_cycle
	rm -f ./TA_golden/snapshot.rpt ./TA_golden/error_dump.rpt
	rm -f ./TA_golden/iimage.bin ./TA_golden/dimage.bin
	cp ./image_generator/iimage.bin ./TA_golden/iimage.bin
	cp ./image_generator/dimage.bin ./TA_golden/dimage.bin
	(cd TA_golden ; ./single_cycle)
rpt_image_clean:
	rm -f snapshot.rpt error_dump.rpt
	rm -f iimage.bin dimage.bin
	rm -f ./TA_golden/snapshot.rpt ./TA_golden/error_dump.rpt
	rm -f ./TA_golden/iimage.bin ./TA_golden/dimage.bin
