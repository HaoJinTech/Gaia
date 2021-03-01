
######### Makefile for gaia ########
EXTENSION = c
CC = gcc
EXE = Gaia
INCDIR = inc
SRCDIR = src
LIBDIR = lib
OBJ = obj
DEP = dep
 
#CPPFLAGS += -I /home/
CFLAGS += -g -Wall -I$(INCDIR) -I./lib/$(INCDIR)
LINKFLAGS +=  -lpthread

CXX_SOURCES = $(foreach dir,$(SRCDIR), $(wildcard $(dir)/*.$(EXTENSION)))
CXX_OBJECTS = $(patsubst  %.$(EXTENSION), $(OBJ)/%.o, $(notdir $(CXX_SOURCES)))
CXX_LIB = $(foreach dir,$(LIBDIR), $(wildcard $(dir)/*.so))

$(EXE): $(CXX_OBJECTS)
	$(CC) $(CXX_OBJECTS) $(CXX_LIB) $(LINKFLAGS) -o $(EXE)
 
$(OBJ)/%.o: $(SRCDIR)/%.$(EXTENSION)
	$(CC) -c $(CPPFLAGS) $< -o $@ $(CFLAGS)
  
.PHONY:clean
clean:
	-rm  -f  $(CXX_OBJECTS)  $(DEP_FILES)  $(EXE)