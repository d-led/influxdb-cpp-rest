# GNU Make project makefile autogenerated by Premake

ifndef config
  config=debug_x32
endif

ifndef verbose
  SILENT = @
endif

.PHONY: clean prebuild prelink

ifeq ($(config),debug_x32)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x32/Debug
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x32/Debug/test-influxdb-cpp-auth
  DEFINES += -D_DEBUG
  INCLUDES += -I../../../deps/fmt/include -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I../../../src/influxdb-c-rest -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m32 -fPIC -g -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m32 -fPIC -g -std=c++14
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x32/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Debug/libinflux-c-rest.so ../../../bin/linux/gmake/x32/Debug/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread -lboost_system -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x32/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Debug/libinflux-c-rest.so ../../../bin/linux/gmake/x32/Debug/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib32 -Wl,-rpath,'$$ORIGIN' -m32
  LINKCMD = $(CXX) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),debug_x64)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x64/Debug
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x64/Debug/test-influxdb-cpp-auth
  DEFINES += -D_DEBUG
  INCLUDES += -I../../../deps/fmt/include -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I../../../src/influxdb-c-rest -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m64 -fPIC -g -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m64 -fPIC -g -std=c++14
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x64/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Debug/libinflux-c-rest.so ../../../bin/linux/gmake/x64/Debug/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread -lboost_system -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x64/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Debug/libinflux-c-rest.so ../../../bin/linux/gmake/x64/Debug/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib64 -Wl,-rpath,'$$ORIGIN' -m64
  LINKCMD = $(CXX) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),release_x32)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x32/Release
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x32/Release/test-influxdb-cpp-auth
  DEFINES +=
  INCLUDES += -I../../../deps/fmt/include -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I../../../src/influxdb-c-rest -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m32 -O2 -fPIC -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m32 -O2 -fPIC -std=c++14
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x32/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Release/libinflux-c-rest.so ../../../bin/linux/gmake/x32/Release/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread -lboost_system -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x32/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Release/libinflux-c-rest.so ../../../bin/linux/gmake/x32/Release/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib32 -Wl,-rpath,'$$ORIGIN' -m32 -s
  LINKCMD = $(CXX) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),release_x64)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x64/Release
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x64/Release/test-influxdb-cpp-auth
  DEFINES +=
  INCLUDES += -I../../../deps/fmt/include -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I../../../src/influxdb-c-rest -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m64 -O2 -fPIC -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m64 -O2 -fPIC -std=c++14
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x64/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Release/libinflux-c-rest.so ../../../bin/linux/gmake/x64/Release/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread -lboost_system -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x64/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Release/libinflux-c-rest.so ../../../bin/linux/gmake/x64/Release/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib64 -Wl,-rpath,'$$ORIGIN' -m64 -s
  LINKCMD = $(CXX) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: prebuild prelink $(TARGET)
	@:

endif

OBJECTS := \
	$(OBJDIR)/auth_test.o \
	$(OBJDIR)/fixtures.o \

RESOURCES := \

CUSTOMFILES := \

SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
	SHELLTYPE := msdos
endif

$(TARGET): $(GCH) ${CUSTOMFILES} $(OBJECTS) $(LDDEPS) $(RESOURCES) | $(TARGETDIR)
	@echo Linking test-influxdb-cpp-auth
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(CUSTOMFILES): | $(OBJDIR)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning test-influxdb-cpp-auth
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(OBJECTS): $(GCH) $(PCH) | $(OBJDIR)
$(GCH): $(PCH) | $(OBJDIR)
	@echo $(notdir $<)
	$(SILENT) $(CXX) -x c++-header $(ALL_CXXFLAGS) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
else
$(OBJECTS): | $(OBJDIR)
endif

$(OBJDIR)/auth_test.o: ../../../src/auth_test/auth_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/fixtures.o: ../../../src/test/fixtures.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

-include $(OBJECTS:%.o=%.d)
ifneq (,$(PCH))
  -include $(OBJDIR)/$(notdir $(PCH)).d
endif