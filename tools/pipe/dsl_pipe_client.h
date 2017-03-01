/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef DSL_PIPE_CLIENT_H
#define DSL_PIPE_CLIENT_H

#include "drv_dsl_cpe_api_types.h"
#include "drv_dsl_cpe_api_error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/sem.h>

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT

#include <stdarg.h>
#include <sys/stat.h>

#else

/** IFXOS includes*/
#include "ifxos_common.h"
#include "ifxos_print_io.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_device_access.h"
#include "ifxos_file_access.h"
#include "ifxos_time.h"
#include "ifxos_lock.h"
#include "ifxos_thread.h"
#include "ifxos_termios.h"
#include "ifxos_pipe.h"
#include "ifxos_misc.h"
#include "ifxos_socket.h"

#endif /* INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

#if defined(INCLUDE_DSL_CPE_IFXOS_SUPPORT)
   /* support other OS only through lib_ifxos */
   #ifndef USE_LIB_IFXOS
      #define USE_LIB_IFXOS 1
   #endif
#endif /** INCLUDE_DSL_CPE_IFXOS_SUPPORT*/


#ifndef __BIG_ENDIAN
   #error please define the __BIG_ENDIAN macro
#endif

#ifndef __BYTE_ORDER
   #error please specify the endianess of your target system
#endif

/**
   Carriage Return + Line Feed, maybe overwritten by compile switches
   or OS-specific adaptation */
#ifndef DSL_PIPE_CRLF
#define DSL_PIPE_CRLF                    "\n\r"
#endif

#if defined(USE_LIB_IFXOS) && (USE_LIB_IFXOS == 1)

/*
   common defines - ifxos_common.h"
*/

#define DSL_BYTE_ORDER               IFXOS_BYTE_ORDER
#define DSL_LITTLE_ENDIAN            IFXOS_LITTLE_ENDIAN
#define DSL_BIG_ENDIAN               IFXOS_BIG_ENDIAN

#ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN             IFXOS_BIG_ENDIAN
   #define __LITTLE_ENDIAN          IFXOS_LITTLE_ENDIAN
   #define __BYTE_ORDER             IFXOS_BYTE_ORDER
#endif

#ifndef __BYTE_ORDER
   #define __BYTE_ORDER       __LITTLE_ENDIAN
#endif

/*
   Function map - stdio, string
*/
#define DSL_PIPE_FPrintf                  IFXOS_FPrintf

/*
   Function map - Memory Functions.
*/
#define DSL_PIPE_Malloc                   IFXOS_MemAlloc
#define DSL_PIPE_Free                     IFXOS_MemFree

/*
   Function map - Device handling (open, close ...).
*/
#define DSL_PIPE_FOpen                    IFXOS_FOpen
#define DSL_PIPE_FClose                   IFXOS_FClose
#define DSL_PIPE_FRead                    IFXOS_FRead

#else /* (USE_LIB_IFXOS == 1) */

#ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN       1
   #define __LITTLE_ENDIAN    2
#endif

#ifndef __BYTE_ORDER
   #define __BYTE_ORDER       __BIG_ENDIAN
#endif

/**
   Allocates a memory block

   \param size Bytes to allocate

   \return
   returns a DSL_void_t pointer to the allocated space, NULL if there's not
   sufficient memory space available.
*/
DSL_void_t *DSL_PIPE_Malloc(DSL_uint32_t size);

/**
   Deallocates a memory block

   \param memblock Previously allocated memory block that should be freed

*/
void DSL_PIPE_Free(DSL_void_t *memblock);

/**
   Open a file.
*/
FILE *DSL_PIPE_FOpen(const DSL_char_t *name,  const DSL_char_t *mode);

/**
   Close the file or memory file.
*/
DSL_int_t DSL_PIPE_FClose(FILE *fd);

/**
   Read from file, stdin .
*/
DSL_int_t DSL_PIPE_FRead(DSL_void_t *buf, DSL_uint32_t size,  DSL_uint32_t count, FILE *stream);

/**
   Print out .
*/
DSL_int_t DSL_PIPE_FPrintf(FILE *stream, const DSL_char_t *format, ...);

#endif /* (USE_LIB_IFXOS == 1) */

#endif /* DSL_PIPE_CLIENT_H */
