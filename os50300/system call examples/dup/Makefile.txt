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
OBJECTS1 = open.o
TARGET1  = open
OBJECTS2 = dup_1.o
TARGET2  = dup_1
OBJECTS3 = dup_2.o
TARGET3  = dup_2


## Implicit rules
.SUFFIXES: .c
.c.o:
	$(CC) -c $(CCFLAGS) $(INC) $<

## Build rules
all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(OBJECTS1)
	$(LINK) -o $@ $(OBJECTS1) $(LDFLAGS) $(LIBS)

$(TARGET2): $(OBJECTS2)
	$(LINK) -o $@ $(OBJECTS2) $(LDFLAGS) $(LIBS)

$(TARGET3): $(OBJECTS3)
	$(LINK) -o $@ $(OBJECTS3) $(LDFLAGS) $(LIBS)



clean:
	rm -f $(OBJECTS1) $(TARGET1) $(OBJECTS2) $(TARGET2) \
              $(OBJECTS3) $(TARGET3)
	rm -f *~ core
