CXX = gcc

INCLUDES = -I./ 
CXXFLAGS = -O3 -Wall -DNDEBUG $(INCLUDES)
LDFLAGS = -lpacket_handler -L./

TARGETS = dce-sub-raw-socket 

SRC = $(wildcard ./*.c)
OBJS = $(SRC:%.c=%.o)

#防止all clean文件时，make all或者make clean执行失败
.PHONY: all clean $(TARGETS)

all : $(TARGETS)

dce-sub-raw-socket: dce-sub-raw-socket.o
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGETS) *.o

