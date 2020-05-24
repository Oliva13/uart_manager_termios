
TOP=$(shell pwd)

MKDIR_P = mkdir -p

OBJDIR = ./obj

CC=$(CROSS)gcc
CPP=$(CROSS)g++

INCLUDE_DIRS := . ./include 

SOURCE_DIRS  := . ./src

#CFLAGS   += -c -Wall -g -O0 -I$(APP_INC_DIR)
#CPPFLAGS += -g -std=c++0x  -I$(APP_INC_DIR)

CFLAGS   += -c -Wall -O3 -I$(APP_INC_DIR)
CPPFLAGS += -O3 -std=c++0x  -I$(APP_INC_DIR)

LDLIBS	   =

LDFLAGS = -lpthread

SOURCE_FILES := $(wildcard $(addsuffix /*.c,   $(SOURCE_DIRS) ) )
SOURCE_FILES += $(wildcard $(addsuffix /*.cpp, $(SOURCE_DIRS) ) )

OBJECT_FILES := $(notdir $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE_FILES))))

OBJ = $(addprefix $(OBJDIR)/, $(OBJECT_FILES))

EXECUTABLE=uart_manager2

VPATH := $(SOURCE_DIRS)

.PHONY : all install clean

#all: lib directories 

all: directories $(EXECUTABLE)

#lib:
#	@$(MAKE) -C $(PJBASE)

directories: ${OBJDIR}
		

${OBJDIR}:
	${MKDIR_P} ${OBJDIR}


$(EXECUTABLE): $(OBJECT_FILES)
	@echo "---------- Build ${EXECUTABLE} application"
	$(CC) $(OBJ) -L./ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	echo "---------- Build $<"
	$(CPP) $< -c $(CPPFLAGS) $(addprefix -I, $(INCLUDE_DIRS)) $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -D,$(DEFINES)) -o ./obj/$@

%.o: %.c
	echo "---------- Build $<"
	$(CC) $< -c $(CFLAGS) $(addprefix -I, $(INCLUDE_DIRS)) $(addprefix -I, $(SOURCE_DIRS)) $(addprefix -D,$(DEFINES)) -o ./obj/$@

install:
	@echo -e ${BLUE}'Installing ${EXECUTABLE}...'${WHITE}
	install ${EXECUTABLE} $(APP_OUT_DIR)/bin

clean:
	@echo "---------- Remove ${EXECUTABLE} application"
	@$ if [ -e ${EXECUTABLE} ]; then rm ${EXECUTABLE}; fi;
	@echo "---------- Remove object files"
	@$ rm -f $(OBJ)


