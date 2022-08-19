SOURCES	= $(wildcard *.cpp)
BINARIES = $(SOURCES:.cpp=)

clean:
	rm $(BINARIES)

%: %.cpp Makefile
	clang++ -o $@ $<

.PHONY: all
all: $(BINARIES)