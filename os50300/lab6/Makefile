############################################
#
#  IUPUI CSCI 503 -- Operating Systems Lab 6
#
############################################

## Compiler, tools and options
CC      = gcc
LINK    = gcc
#OPT     = -g

CCFLAGS = $(OPT) -Wall
#LDFLAGS = $(OPT)

## Libraries
LIBS = /home/xiaozhon/OS/papi/lib/libpapi.a
INC  = -I/home/xiaozhon/OS/papi/include

## FILES
OBJECTS1 = papi_function_call.o
TARGET1  = function_call_test

OBJECTS2 = papi_system_call.o
TARGET2  = system_call_test

OBJECTS3 = papi_context_switch.o
TARGET3  = context_switch_test

OBJECTS4 = papi_page_fault.o
TARGET4  = page_fault_test

## Implicit rules
.SUFFIXES: .c
.c.o:
#	$(CC) -c $(INC) $<
	$(CC) -c $(CCFLAGS) $(INC) $<

## Build rules
all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) 

$(TARGET1): $(OBJECTS1)
	$(LINK) -o $@ $(OBJECTS1) $(LIBS)

$(TARGET2): $(OBJECTS2)
	$(LINK) -o $@ $(OBJECTS2) $(LIBS)
	
$(TARGET3): $(OBJECTS3)
	$(LINK) -o $@ $(OBJECTS3) $(LIBS)

$(TARGET4): $(OBJECTS4)
	$(LINK) -o $@ $(OBJECTS4) $(LIBS)

clean:
	rm -f $(OBJECTS1) $(TARGET1)
	rm -f $(OBJECTS2) $(TARGET2)
	rm -f $(OBJECTS3) $(TARGET3)
	rm -f $(OBJECTS4) $(TARGET4)
	rm -f *~ core
