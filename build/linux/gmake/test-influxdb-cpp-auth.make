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
  DEFINES += -D_DEBUG -D_GLIBCXX_USE_CXX11_ABI=0
  INCLUDES += -I../../../deps/fmt -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/include -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -g -m32 -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CFLAGS)
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x32/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Debug/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread-mt -lboost_system-mt -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x32/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Debug/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib32 -L/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/lib -L../../../~/.linuxbrew/lib64 -m32
  LINKCMD = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),debug_x64)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x64/Debug
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x64/Debug/test-influxdb-cpp-auth
  DEFINES += -D_DEBUG -D_GLIBCXX_USE_CXX11_ABI=0
  INCLUDES += -I../../../deps/fmt -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/include -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -g -m64 -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CFLAGS)
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x64/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Debug/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread-mt -lboost_system-mt -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x64/Debug/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Debug/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib64 -L/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/lib -L../../../~/.linuxbrew/lib64 -m64
  LINKCMD = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),release_x32)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x32/Release
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x32/Release/test-influxdb-cpp-auth
  DEFINES += -D_GLIBCXX_USE_CXX11_ABI=0
  INCLUDES += -I../../../deps/fmt -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/include -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m32 -O2 -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CFLAGS)
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x32/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Release/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread-mt -lboost_system-mt -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x32/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x32/Release/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib32 -L/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/lib -L../../../~/.linuxbrew/lib64 -s -m32
  LINKCMD = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),release_x64)
  RESCOMP = windres
  TARGETDIR = ../../../bin/linux/gmake/x64/Release
  TARGET = $(TARGETDIR)/test-influxdb-cpp-auth
  OBJDIR = ../../../obj/linux/gmake/x64/Release/test-influxdb-cpp-auth
  DEFINES += -D_GLIBCXX_USE_CXX11_ABI=0
  INCLUDES += -I../../../deps/fmt -I../../../deps/rxcpp/Rx/v2/src/rxcpp -I../../../src/influxdb-cpp-rest -I/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/include -I../../../deps/catch/single_include -I../../../src/test
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m64 -O2 -std=c++14
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CFLAGS)
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += ../../../bin/linux/gmake/x64/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Release/libfmt.a -lssl -lcrypto -lboost_random -lboost_chrono -lboost_thread-mt -lboost_system-mt -lboost_regex -lboost_filesystem -lcpprest -lpthread
  LDDEPS += ../../../bin/linux/gmake/x64/Release/libinfluxdb-cpp-rest.a ../../../bin/linux/gmake/x64/Release/libfmt.a
  ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib64 -L/home/linuxbrew/.linuxbrew/Cellar/cpprestsdk/2.9.1/lib -L../../../~/.linuxbrew/lib64 -s -m64
  LINKCMD = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

endif

OBJECTS := \
	$(OBJDIR)/auth_test.o \
	$(OBJDIR)/fixtures.o \

RESOURCES := \

CUSTOMFILES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES) ${CUSTOMFILES}
	@echo Linking test-influxdb-cpp-auth
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

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
$(OBJECTS): $(GCH) $(PCH)
$(GCH): $(PCH)
	@echo $(notdir $<)
	$(SILENT) $(CXX) -x c++-header $(ALL_CXXFLAGS) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
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
