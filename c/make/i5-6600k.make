#############################################################################
#	basic compile build                                                     #
#	target - Intel Core 5i 6600k, Mint										#
############################################################################# 

# set directories
system_BINARIES := ~/Git/HAS/bin
code_DIRECTORY := $(working_DIRECTORY)
program_INCLUDE_DIRS := /home/mam1/Git/HAS/c/headers/
program_LIBRARY_DIRS := $~/Git/HAS/c/lib
program_LIBRARIES := hellow

# scan working directory
program_C_SRCS := $(program_NAME).c
program_CXX_SRCS :=  $(program_NAME).cpp
program_C_OBJS :=  ${program_C_SRCS:.c=.o}
program_CXX_OBJS :=  ${program_CXX_SRCS:.cpp=.o}
program_OBJS :=  $(program_C_OBJS) $(program_CXX_OBJS)

# set parameters
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
# LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
# LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))
CFLAGS  += -Wall
CXXFLAGS += $(CFLAGS)
LDFLAGS += $(CFLAGS) -fno-exceptions      

# set locations for basic gnu tools
CC  := gcc
CXX := g++
LD  := ld
AS  := as
AR  := ar
OBJCOPY := $objcopy

.PHONY: all clean distclean

# create executable - link in project library
all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	gcc hellow.c -o $(system_BINARIES)/$(program_NAME) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
	@echo "\n>>>>> build of $(program_NAME).c completed\n"

# clean up
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)

clean:
	@- $(RM) -v $(program_NAME)
	@- $(RM) -v $(program_OBJS)
	cd .. && $(MAKE) clean

# rebuild the project ibrary from lib-sources 
lib: 
	cd .. && $(MAKE) clean
	cd .. && $(MAKE)

# build all project moduals
project:
	cd .. && $(MAKE) clean
	cd .. && $(MAKE) 
	cd ../CGI && $(MAKE)
	cd ../UI && $(MAKE) t=$(target_MACHINE)
	cd ../daemon && $(MAKE)	t=$(target_MACHINE)
	cd ../../script_lib/OSXMint && $(MAKE)
	sload $(target_MACHINE)

dump:
	@echo CC .................... $(CC)
	@echo cc .................... $(cc)
	@echo LINK .................. $(LINK)
	@echo LINK.cc ............... $(LINK.cc)
	@echo program_NAME .......... $(program_NAME)
	@echo program_INCLUDE_DIRS .. $(program_INCLUDE_DIRS)
	@echo program_C_OBJS ........ $(program_C_OBJS)
	@echo program_OBJS .......... $(program_OBJS)
	@echo LDFLAGS ............... $(LDFLAGS)
	@echo CPPFLAGS .............. $(CPPFLAGS)
	@echo CFLAGS ................ $(CFLAGS)

distclean: cleans