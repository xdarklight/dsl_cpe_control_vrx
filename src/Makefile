RGSRC=../../../../../../
include $(RGSRC)/envir.mak

DSL_CPE_CONTROL_DAEMON=dsl_cpe_control
TARGET+=$(DSL_CPE_CONTROL_DAEMON)
RAMDISK_BIN_FILES+=$(DSL_CPE_CONTROL_DAEMON)

LDFLAGS_$(DSL_CPE_CONTROL_DAEMON)+=-lpthread -lrt

ifndef CONFIG_LSP_DIST
  LDFLAGS_$(DSL_CPE_CONTROL_DAEMON)+=-lopenrg_gpl
endif

O_OBJS_$(DSL_CPE_CONTROL_DAEMON)+=dsl_cpe_cli.o dsl_cpe_cli_pipe.o \
  dsl_cpe_cli_access.o dsl_cpe_cli_console.o dsl_cpe_control.o \
  dsl_cpe_init_cfg.o dsl_cpe_linux.o dsl_cpe_debug.o dsl_cpe_dti.o \
  dsl_cpe_cli_vrx.o dsl_cpe_debug_vrx.o dsl_cpe_bnd_vrx.o

CFLAGS+=-DHAVE_CONFIG_H -include rg_config.h

L_OBJS_$(DSL_CPE_CONTROL_DAEMON)=\
    $(BUILDDIR)/vendor/lantiq/xway/ifxos/src/libifxos.a

INTERNAL_HEADERS+=dsl_cpe_cli_console.h dsl_cpe_cli.h \
  dsl_cpe_control.h dsl_cpe_debug.h dsl_cpe_debug_vrx.h \
  dsl_cpe_dti.h dsl_cpe_init_cfg.h dsl_cpe_linux.h dsl_cpe_os.h \
  dsl_cpe_os_lint_map.h dsl_cpe_rtems.h dsl_cpe_simulator.h dsl_cpe_soap_H.h \
  dsl_cpe_soap_Stub.h dsl_cpe_vxworks.h dsl_cpe_win32.h stdsoap2.h \
  dsl_cpe_config.h dsl_cpe_bnd_vrx.h

JPKG_EXPORTED_FILES+=dsl_cpe_config.h

include $(RGMK)
