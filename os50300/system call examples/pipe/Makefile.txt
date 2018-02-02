########################################
#
#  IUPUI CSCI 503 -- Operating Systems
#
########################################

## Compiler, tools and options
CC      = gcc
LINK    = gcc
OPT     = -g

CCFLAGS = $(OPT) -Wall
LDFLAGS = $(OPT)

## Libraries
LIBS =
INC  = -I.

## FILES
OBJECTS1 = pipe1.o
TARGET1  = pipe1
OBJECTS2 = pipe2.o
TARGET2  = pipe2
OBJECTS3 = pipe3.o
TARGET3  = pipe3
OBJECTS4 = pipe4.o
TARGET4  = pipe4


## Implicit rules
.SUFFIXES: .c
.c.o:
	$(CC) -c $(CCFLAGS) $(INC) $<

## Build rules
all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET1): $(OBJECTS1)
	$(LINK) -o $@ $(OBJECTS1) $(LDFLAGS) $(LIBS)

$(TARGET2): $(OBJECTS2)
	$(LINK) -o $@ $(OBJECTS2) $(LDFLAGS) $(LIBS)

$(TARGET3): $(OBJECTS3)
	$(LINK) -o $@ $(OBJECTS3) $(LDFLAGS) $(LIBS)

$(TARGET4): $(OBJECTS4)
	$(LINK) -o $@ $(OBJECTS4) $(LDFLAGS) $(LIBS)


clean:
	rm -f $(OBJECTS1) $(TARGET1) $(OBJECTS2) $(TARGET2) \
              $(OBJECTS3) $(TARGET3) $(OBJECTS4) $(TARGET4)
	rm -f *~ core
