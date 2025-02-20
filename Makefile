SHELL=cmd.exe
USE_DEBUG = NO
USE_UNICODE = NO

ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
#TOOLS=c:\mingw\bin
TOOLS=c:\mingw\bin
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -O3 -c
LFLAGS = -s -O3
endif

# RES=image.rc resource.h

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
LFLAGS += -dUNICODE -d_UNICODE 
endif

#LIBS=-lmpr -lshlwapi -lole32 -luuid

CPPSRC=plus42_skin_editor.cpp 

OBJS = $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

#*************************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) $<

BIN = plus42_skin_editor.exe

all: $(BIN)

clean:
	rm -f *.o *.exe *~ *.zip

dist:
	rm -f p42_skin_editor.zip
	zip p42_skin_editor.zip $(BIN) README.md

wc:
	wc -l *.cpp

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) $(CPPSRC)"

depend: 
	makedepend $(CSRC) $(CPPSRC)

$(BIN): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE
