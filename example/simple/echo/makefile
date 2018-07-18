TARGETNAME := echo
TARGETPATH := ../../bin
OBJECTPATH := ../../obj/$(TARGETNAME)
# 0 -- Application; 1 -- Static library; 2 -- Dynamic library
TARGETTYPE = 0
TARGETINCS = -I ../../../baseCom/ -I ../../../netWork/


CPP = g++
DEBUG = -g -DDEBUG -D_REENTRANT -w -std=c++11
RELEASE = -s -O2
CPPFLAGS  = $(TARGETINCS)
LINKFLAGS = $(TARGETLIBS) -lpthread -lrt -lnsl -lz -lgcc_s -lodbc
TARGETSRCS = $(wildcard *.c *.cpp)
OBJS = $(addprefix $(OBJECTPATH)/, $(patsubst %.c,%.o,$(TARGETSRCS:.cpp=.o)))

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
	
