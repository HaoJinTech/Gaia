
######### Makefile for gaia ########
EXTENSION = c
CC = gcc
EXE = Gaia
SRCDIR = src
LIBDIR = lib
OBJ = obj
DEP = dep
 
CFLAGS += -g -Wall -I./inc -I./lib/inc -I./src/cmds -I./inc/bll
LINKFLAGS +=  -lpthread

CXX_SOURCES = $(foreach dir,$(SRCDIR), $(wildcard $(dir)/*.$(EXTENSION)))
CXX_SOURCES += $(foreach dir,$(SRCDIR)/bll, $(wildcard $(dir)/*.$(EXTENSION)))

CXX_OBJECTS = $(patsubst  %.$(EXTENSION), $(OBJ)/%.o, $(notdir $(CXX_SOURCES)))
CXX_LIB = $(foreach dir,$(LIBDIR), $(wildcard $(dir)/*.so))

$(EXE): $(CXX_OBJECTS)
	$(CC) $(CXX_OBJECTS) $(CXX_LIB) $(LINKFLAGS) -o $(EXE)
 
$(OBJ)/%.o: $(SRCDIR)/%.$(EXTENSION)
	$(CC) -c $(CPPFLAGS) $< -o $@ $(CFLAGS)

$(OBJ)/%.o: $(SRCDIR)/bll/%.$(EXTENSION)
	$(CC) -c $(CPPFLAGS) $< -o $@ $(CFLAGS)
  
.PHONY:clean
clean:
	-rm  -f  $(CXX_OBJECTS)  $(DEP_FILES)  $(EXE)