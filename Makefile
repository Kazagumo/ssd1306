CXX:=aarch64-linux-musl-g++
CC:=aarch64-linux-musl-gcc
LD:=aarch64-linux-musl-ld

FLAGS:=-fPIC -O3 -Ijsoncpp/include -Iu8g2 -Icperiph -Ilinux_header -MMD 
CXXFLAGS:=-std=c++11 -fpermissive 
CFLAGS:=-std=gnu99 -pedantic -Wno-overflow -Wno-stringop-truncation \
		-Wno-overlength-strings -D__ARM_LINUX__ -DU8G2_USE_LARGE_FONTS=1 

FILES:=test.cpp
SRC_DIR:=jsoncpp u8g2 cperiph
BIN_DIR:=bin
CPPSRC := $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.cpp))
CSRC := $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))

all: link

pre:
	$(foreach dir,$(SRC_DIR),$(shell mkdir -p $(BIN_DIR)/$(dir)))

$(CPPSRC): 1
	$(CXX) -c $(FLAGS) $(CXXFLAGS) $@ -o $(BIN_DIR)/$(@:.cpp=.o)

$(CSRC): 1
	$(CC) -c $(FLAGS) $(CFLAGS) $@ -o $(BIN_DIR)/$(@:.c=.o)

$(FILES): 1
	$(CXX) -c $(FLAGS) $(CXXFLAGS) $@ -o $(BIN_DIR)/$(@:.cpp=.o)

1:

link: pre $(CPPSRC) $(CSRC) $(FILES)
	cd $(BIN_DIR); \
	$(CXX) $(CPPSRC:.cpp=.o) $(CSRC:.c=.o) $(FILES:.cpp=.o) -o test.final
	mv $(BIN_DIR)/test.final ssd1306

fast: $(FILES) link

clean:
	rm -rf ./bin ./ssd1306

install: all
	mkdir -p $(DESTDIR)/usr/bin
	cp ssd1306 $(DESTDIR)/usr/bin/ssd1306