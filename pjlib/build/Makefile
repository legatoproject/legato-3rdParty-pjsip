include ../../build.mak
include ../../version.mak
include $(PJDIR)/build/common.mak

export LIBDIR := ../lib
export BINDIR := ../bin

RULES_MAK := $(PJDIR)/build/rules.mak

export PJLIB_LIB := libpj-$(TARGET_NAME)$(LIBEXT)

ifeq ($(PJ_SHARED_LIBRARIES),)
else
export PJLIB_SONAME := libpj.$(SHLIB_SUFFIX)
export PJLIB_SHLIB := $(PJLIB_SONAME).$(PJ_VERSION_MAJOR)
endif

###############################################################################
# Gather all flags.
#
export _CFLAGS 	:= $(CC_CFLAGS) $(OS_CFLAGS) $(HOST_CFLAGS) $(M_CFLAGS) \
		   $(CFLAGS) $(CC_INC)../include
export _CXXFLAGS:= $(_CFLAGS) $(CC_CXXFLAGS) $(OS_CXXFLAGS) $(M_CXXFLAGS) \
		   $(HOST_CXXFLAGS) $(CXXFLAGS)
export _LDFLAGS := $(CC_LDFLAGS) $(OS_LDFLAGS) $(M_LDFLAGS) $(HOST_LDFLAGS) \
		   $(APP_LDFLAGS) $(LDFLAGS) 


###############################################################################
# Defines for building PJLIB library
#
export PJLIB_SRCDIR = ../src/pj
export PJLIB_OBJS += $(OS_OBJS) $(M_OBJS) $(CC_OBJS) $(HOST_OBJS) \
	activesock.o array.o atomic_slist.o atomic_queue.o config.o ctype.o \
	errno.o except.o \
	fifobuf.o guid.o hash.o ip_helper_generic.o list.o lock.o log.o \
	os_time_common.o os_info.o pool.o pool_buf.o pool_caching.o pool_dbg.o \
	rand.o rbtree.o sock_common.o sock_qos_common.o \
	ssl_sock_common.o ssl_sock_ossl.o ssl_sock_gtls.o ssl_sock_dump.o \
	ssl_sock_darwin.o ssl_sock_mbedtls.o string.o timer.o types.o unittest.o
export PJLIB_CFLAGS += $(_CFLAGS)
export PJLIB_CXXFLAGS += $(_CXXFLAGS)
export PJLIB_LDFLAGS += $(_LDFLAGS)

###############################################################################
# Defines for building test application
#
export TEST_SRCDIR = ../src/pjlib-test
export TEST_OBJS += activesock.o atomic.o atomic_slist.o \
		    echo_clt.o errno.o exception.o \
		    fifobuf.o file.o hash_test.o ioq_perf.o ioq_udp.o \
		    ioq_stress_test.o ioq_unreg.o ioq_tcp.o ioq_iocp_unreg_test.o \
		    list.o mutex.o os.o pool.o pool_perf.o rand.o rbtree.o \
		    select.o sleep.o sock.o sock_perf.o ssl_sock.o \
		    string.o test.o thread.o timer.o timestamp.o \
		    udp_echo_srv_sync.o udp_echo_srv_ioqueue.o \
		    unittest_test.o util.o
export TEST_CFLAGS += $(_CFLAGS)
export TEST_CXXFLAGS += $(_CXXFLAGS)
export TEST_LDFLAGS += $(PJLIB_LDLIB) $(_LDFLAGS)
ifeq ($(EXCLUDE_APP),0)
export TEST_EXE := pjlib-test-$(TARGET_NAME)$(HOST_EXE)
endif

export CC_OUT CC AR RANLIB HOST_MV HOST_RM HOST_RMDIR HOST_MKDIR OBJEXT LD LDOUT 
###############################################################################
# Main entry
#
# $(TARGET) is defined in os-$(OS_NAME).mak file in current directory.
#

all: $(TARGETS) $(TARGETS_EXE)

lib: $(TARGETS)

doc:
	cd .. && rm -rf docs/$(PJ_VERSION) && doxygen docs/doxygen.cfg
	@if [ -n "$(WWWDIR)" ] && ! [ -d "$(WWWDIR)/docs/$(PJ_VERSION)/pjlib/docs/html" ] ; then \
		echo "Creating docs/$(PJ_VERSION)/pjlib/docs/html" ; \
		mkdir -p $(WWWDIR)/docs/$(PJ_VERSION)/pjlib/docs/html ; \
	fi 
	@if [ -n "$(WWWDIR)" ] && [ -d "$(WWWDIR)/docs/$(PJ_VERSION)/pjlib/docs/html" ] ; then \
		echo "Copying docs/$(PJ_VERSION) to $(WWWDIR)/docs/$(PJ_VERSION)/pjlib/docs/html.." ; \
		cp -v -a ../docs/$(PJ_VERSION)/html/* $(WWWDIR)/docs/$(PJ_VERSION)/pjlib/docs/html/ ; \
	fi
print:
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib print_lib
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test print_bin
	
depend: ../include/pj/config_site.h
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib depend
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test depend
	echo '$(BINDIR)/$(TEST_EXE): $(LIBDIR)/$(PJLIB_LIB)' >> .pjlib-test-$(TARGET_NAME).depend


.PHONY: all dep depend clean realclean distclean
.PHONY: $(TARGETS)
.PHONY: $(PJLIB_LIB) $(PJLIB_SONAME)
.PHONY: $(TEST_EXE)

dep: depend

pjlib: $(PJLIB_LIB)
$(PJLIB_LIB): ../include/pj/config_site.h

$(PJLIB_SONAME): $(PJLIB_LIB)
$(PJLIB_LIB) $(PJLIB_SONAME):
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib $(subst /,$(HOST_PSEP),$(LIBDIR)/$@)

../include/pj/config_site.h:
	touch ../include/pj/config_site.h

pjlib-test: $(TEST_EXE)
$(TEST_EXE): $(PJLIB_LIB) $(PJLIB_SONAME)
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test $(subst /,$(HOST_PSEP),$(BINDIR)/$@)

.PHONY: pjlib.ko
pjlib.ko:
	echo Making $@
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib $(subst /,$(HOST_PSEP),$(LIBDIR)/$@)

.PHONY: pjlib-test.ko
pjlib-test.ko:
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test $(subst /,$(HOST_PSEP),$(LIBDIR)/$@)

clean:
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib clean
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test clean

realclean:
	$(subst @@,$(subst /,$(HOST_PSEP),.pjlib-$(TARGET_NAME).depend),$(HOST_RMR))
	$(subst @@,$(subst /,$(HOST_PSEP),.pjlib-test-$(TARGET_NAME).depend),$(HOST_RMR))
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib realclean
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test realclean

distclean: realclean

gcov-report:
	$(MAKE) -f $(RULES_MAK) APP=PJLIB app=pjlib gcov-report
	$(MAKE) -f $(RULES_MAK) APP=TEST app=pjlib-test gcov-report

