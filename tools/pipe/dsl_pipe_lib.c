/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef LINUX

#include "dsl_pipe_client.h"

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_void_t *DSL_PIPE_Malloc(DSL_uint32_t size)
{
   DSL_void_t *memblock;

   memblock = (DSL_void_t*) malloc((size_t)size);

   return (memblock);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
void DSL_PIPE_Free(DSL_void_t *memblock)
{
   free(memblock);
}

/**
   Open a file.
*/
FILE *DSL_PIPE_FOpen(const DSL_char_t *name,  const DSL_char_t *mode)
{
   return fopen(name, mode);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_PIPE_FClose(FILE *fd)
{
   if(fd != 0)
      return fclose(fd);

   return -1;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_PIPE_FRead(DSL_void_t *buf, DSL_uint32_t size,  DSL_uint32_t count, FILE *stream)
{
   return fread(buf, size,  count, stream);
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_PIPE_FPrintf(FILE *stream, const DSL_char_t *format, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nRet = 0;

   va_start(ap, format);   /* set ap pointer to 1st unnamed arg */

   nRet = vfprintf(stream, format, ap);
   fflush(stream);

   va_end(ap);

   return nRet;
}
#endif /* #ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

#endif /* LINUX */
