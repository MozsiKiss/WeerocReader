CXXFLAGS += -Wall -O2 -funroll-loops `root-config --cflags --libs`
LDFLAGS += -lstdc++

WeerocReader: WeerocReader.cc
	g++ WeerocReader.cc -o build/WeerocReader $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f WeerocReader *~ *.o *.a
	rm -f WeerocReaderBaselineSubt *~ *.o *.a
	rm -f build/*
