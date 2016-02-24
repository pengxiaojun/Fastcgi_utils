BINTARGET = backend.fcgi

# export OSTYPE
ifeq ($(OSTYPE), cygwin)
CFLAGS += $(GRUTIL_INCS)
CXXFLAGS += $(GRUTIL_INCS)
else
CFLAGS += $(GRUTIL_INCS) -fPIC -rdynamic
CXXFLAGS += $(GRUTIL_INCS) -fPIC -rdynamic
endif
CXXFLAGS += -I .
CXXFLAGS += -I ../../../sharedinc

LDFLAGS += -L../../../output64/ -lgrutil -rdynamic

LDFLAGS += -ldl -luninet -lfcgi -lsqlite3 -lpthread -lprotobuf

#CXXFLAGS += -DENABLE_TRACE

CXXSRC += $(wildcard handler/*.cpp\
		  protobuf/*.cpp\
		  nvr/*.cpp\
		  tvw/*.cpp)

$(shell protobuf/make.sh)

include ../../../sharedinc/Makefile.inc
