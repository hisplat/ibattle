
CXX	:= clang++
CC 	:= clang
LD	:= clang++
# CXX := g++
# CC := gcc
# LD := g++

AR	:= ar
CXXFLAGS := -Wall -Werror -O0 -g -pg -Wno-unused -Wno-unreachable-code -Wno-unused-parameter -Wno-unused-result -Wno-deprecated -fPIC -std=c++0x -fno-rtti
CFLAGS := -Wall -Werror -O0 -g -pg -Wno-unused -Wno-unreachable-code -Wno-unused-parameter -Wno-unused-result -Wno-deprecated -fPIC
INCLUDES = -Isrc/common -Isrc/debugtools -Isrc/base

LINKS	= -g -pg -L. -ldl -rdynamic
LIBS	= -lpthread -lrt


base_src_files := src/base/logging.cpp
base_src_files += src/base/dump.cpp
base_src_files += src/base/buffer.cpp
base_src_files += src/base/helper.cpp
base_src_files += src/base/archive.cpp
base_src_files += src/base/condition.cpp
base_src_files += src/base/thread_pool.cpp
base_src_files += src/base/pipe_waiter.cpp
base_src_files += src/base/lock.cpp
base_src_files += src/base/crc32.cpp
base_src_files += src/base/md5sum.cpp

common_src_files := 


gate_source := src/gate/main.cpp
gate_source += $(base_src_files)
gate_source += $(common_src_files)
gate_objects := $(gate_source:.cpp=.o)


debugtools_source := src/debugtools/main.cpp
debugtools_source += src/debugtools/instant_function.cpp
debugtools_source += src/debugtools/misc.cpp
debugtools_source += $(base_src_files)
debugtools_source += $(common_src_files)

debugtools_objects := $(debugtools_source:.cpp=.o)

all: gate debugtools

gate: $(gate_objects)
	@echo Linking $@ ...
	$(LD) $^ $(LINKS) $(LIBS) -o$@
	@echo $@ done.

debugtools: $(debugtools_objects)
	@echo Linking $@ ...
	$(LD) $^ $(LINKS) $(LIBS) -o$@
	@echo $@ done.


.cpp.o:
	@echo Compling $@ ...
	$(CXX) -c $< $(INCLUDES) $(CXXFLAGS)  -o $@
	@echo -------------------------------------------

.c.o:
	@echo Compling $@ ...
	$(CC) -c $< $(INCLUDES) $(CFLAGS)  -o $@
	@echo -------------------------------------------

%.d:%.cpp
	$(CXX) -MM $< $(INCLUDES) $(CXXFLAGS) -o $@


clean:
	rm -fv gate debugtools
	find . -name '*.o' -exec rm -fv {} \;
	find . -name '*.d' -exec rm -fv {} \;
	rm -fv gmon.out
