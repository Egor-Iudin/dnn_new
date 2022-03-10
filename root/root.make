libCROOT.a: CTFile.o CTTree.o CTDirectory.o CTRandom.o CTGraph.o 
	rm -f libROOT.a
	ar -cvq libCROOT.a CTFile.o CTTree.o CTDirectory.o CTRandom.o CTGraph.o

CTFile.o: CTFile.h CTFile.cxx
	$(CC) $(CFLAGS) -c CTFile.cxx -o CTFile.o

CTTree.o: CTTree.h CTTree.cxx
	$(CC) $(CFLAGS) -c CTTree.cxx -o CTTree.o

CTDirectory.o: CTDirectory.h CTDirectory.cxx
	$(CC) $(CFLAGS) -c CTDirectory.cxx -o CTDirectory.o

CTRandom.o: CTRandom.h CTRandom.cxx
	$(CC) $(CFLAGS) -c CTRandom.cxx -o CTRandom.o

CTGraph.o: CTGraph.h CTGraph.cxx
	$(CC) $(CFLAGS) -c CTGraph.cxx -o CTGraph.o

CTFile.h:
	ln -s ../root/CTFile.h .

CTFile.cxx:
	ln -s ../root/CTFile.cxx .

CTTree.h:
	ln -s ../root/CTTree.h .

CTGraph.h :
	ln -s ../root/CTGraph.h .

CTTree.cxx:
	ln -s ../root/CTTree.cxx .

CTDirectory.h:
	ln -s ../root/CTDirectory.h .

CTDirectory.cxx:
	ln -s ../root/CTDirectory.cxx .

CTRandom.h:
	ln -s ../root/CTRandom.h .

CTRandom.cxx:
	ln -s ../root/CTRandom.cxx .

CTGraph.cxx: 
	ln -s ../root/CTGraph.cxx .

