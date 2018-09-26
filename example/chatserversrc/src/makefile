TARGETNAME := chatServer
TARGETPATH := ../bin
OBJECTPATH := ../obj/$(TARGETNAME)
# 0 -- Application; 1 -- Static library; 2 -- Dynamic library
TARGETTYPE = 0
TARGETINCS = -I../include -I../../factory/include -I../../lib/include
TARGETLIBS = ../../factory/netfactory.a ../../lib/lib/libjson.a ../../lib/lib/mysqlLib.a
#TARGETLIBS = ../../lib/lib/GameProvider.a ../../lib/lib/Epoll.a ../../lib/lib/Common.a ../../lib/lib/libaes.a ../../lib/lib/ODBCExt.a ../../lib/lib/libonsclient4cpp.a 

CPP = g++
DEBUG = -g -Wall -D_REENTRANT -w -std=c++0x
RELEASE = -s -O2
CPPFLAGS  = $(TARGETINCS)
LINKFLAGS = $(TARGETLIBS) -lpthread -lrt
#-lrt -lnsl -lz -lgcc_s -lodbc -ldl
TARGETSRCS = ${wildcard *.c *.cpp}
#../../../baseCom/*.cpp ../../../netWork/*.cpp ../../../netWork/poller/*.cpp}

#TARGETSRCS = main.cpp
OBJS = $(addprefix $(OBJECTPATH)/, $(patsubst %.c,%.o,$(notdir $(TARGETSRCS:.cpp=.o))))
#OBJS = $(patsubst %.c,%.o,$(TARGETSRCS:.cpp=.o))

ifeq ($(strip $(TARGETPATH)),)
    TARGETPATH := .
endif

ifeq ($(strip $(OBJECTPATH)),)
    OBJECTPATH := .
endif

ifneq ($(strip $(MAKECMDGOALS)), release)
    CPPFLAGS := $(CPPFLAGS) $(DEBUG)
else
    CPPFLAGS := $(CPPFLAGS) $(RELEASE)
endif

$(OBJECTPATH)/%.o : %.cpp;mkdir -p $(OBJECTPATH);$(CPP) $(CPPFLAGS) -c $< -o $@
$(OBJECTPATH)/%.o : %.c;mkdir -p $(OBJECTPATH);$(CPP) $(CPPFLAGS) -c $< -o $@

ifneq ($(TARGETTYPE), 1)
    ifeq ($(TARGETTYPE), 2)
        TYPE = -shared
        TARGETNAME := $(TARGETNAME).so
    endif
    $(TARGETNAME) : $(OBJS)
	@mkdir -p $(TARGETPATH)
	$(CPP) $(CPPFLAGS) -o $(TARGETPATH)/$@ $^ $(LINKFLAGS) $(TYPE)
else
    TARGETNAME := $(TARGETNAME).a
    $(TARGETNAME) : $(OBJS)
	@mkdir -p $(TARGETPATH)
	$(AR) rcs $(TARGETPATH)/$@ $^
endif

.PHONY: all debug release clean

all debug release: $(TARGETNAME)

clean:
	rm -f $(OBJECTPATH)/*.o $(TARGETPATH)/$(TARGETNAME) $(TARGETPATH)/core*

