/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL CLI, access function implementation
*/

#include "dsl_cpe_control.h"
#if defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && !defined(INCLUDE_DSL_CPE_CLI_AUTOGEN_SUPPORT)

#include "dsl_cpe_cli.h"
#include "dsl_cpe_cli_console.h"
#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
#include "dsl_cpe_dti.h"
#endif /* #if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_CLI


/* for debugging: */
#ifdef DSL_CLI_LOCAL
#undef DSL_CLI_LOCAL
#endif
#if 1
#define DSL_CLI_LOCAL
#else
#define DSL_CLI_LOCAL static
#endif

DSL_void_t DSL_CPE_CLI_DeviceCommandsRegister ( DSL_void_t );
#ifdef INCLUDE_DSL_ADSL_MIB
DSL_void_t DSL_CPE_CLI_MibCommandsRegister ( DSL_void_t );
#endif /* INCLUDE_DSL_ADSL_MIB*/

#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
const char *sFailureReturn = "nReturn=%d%s";
#else
const char *sFailureReturn = "nReturn=%d";
#endif

#ifndef DSL_CPE_DEBUG_DISABLE
static const DSL_char_t g_sCcaDbgmlg[] =
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_CCA_debugModules_t nCcaDbgModule (dec)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_APP = 1" DSL_CPE_CRLF
   "   DSL_CCA_DBG_OS = 2" DSL_CPE_CRLF
   "   DSL_CCA_DBG_CLI = 3" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PIPE = 4" DSL_CPE_CRLF
   "   DSL_CCA_DBG_SOAP = 5" DSL_CPE_CRLF
   "   DSL_CCA_DBG_CONSOLE = 6" DSL_CPE_CRLF
   "   DSL_CCA_DBG_TCPMSG = 7" DSL_CPE_CRLF
   "   DSL_CCA_DBG_MULTIMODE = 8" DSL_CPE_CRLF
   "   DSL_CCA_DBG_NOTIFICATIONS = 9" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_CCA_debugLevels_t nDbgLevel (hex)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_NONE = 0x00" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PRN = 0x01" DSL_CPE_CRLF
   "   DSL_CCA_DBG_ERR = 0x02" DSL_CPE_CRLF
   "   DSL_CCA_DBG_WRN = 0x40" DSL_CPE_CRLF
   "   DSL_CCA_DBG_MSG = 0x80" DSL_CPE_CRLF
   "   DSL_CCA_DBG_LOCAL = 0xFF" DSL_CPE_CRLF
   DSL_CPE_CRLF "";

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_CCA_DBG_ModuleLevelGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_CCA_debugLevels_t nDbgLvl = DSL_CCA_DBG_NONE;
   DSL_CCA_debugModules_t nDbgModule = DSL_CCA_DBG_NO_BLOCK;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%d", &nDbgModule);

   if ((nDbgModule > DSL_CCA_DBG_NO_BLOCK) && (nDbgModule < DSL_CCA_DBG_LAST_BLOCK))
   {
      nDbgLvl = DSL_CCA_g_dbgLvl[nDbgModule].nDbgLvl;
   }
   else
   {
      ret = -1;
   }

   DSL_CPE_FPrintf (out, DSL_CPE_RET"nDbgLevel=%x" DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret), nDbgLvl);

   return 0;
}
#endif

#ifndef DSL_CPE_DEBUG_DISABLE
static const DSL_char_t g_sCcaDbgmls[] =
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_CCA_debugModules_t nCcaDbgModule (dec)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_APP = 1" DSL_CPE_CRLF
   "   DSL_CCA_DBG_OS = 2" DSL_CPE_CRLF
   "   DSL_CCA_DBG_CLI = 3" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PIPE = 4" DSL_CPE_CRLF
   "   DSL_CCA_DBG_SOAP = 5" DSL_CPE_CRLF
   "   DSL_CCA_DBG_CONSOLE = 6" DSL_CPE_CRLF
   "   DSL_CCA_DBG_TCPMSG = 7" DSL_CPE_CRLF
   "   DSL_CCA_DBG_MULTIMODE = 8" DSL_CPE_CRLF
   "   DSL_CCA_DBG_NOTIFICATIONS = 9" DSL_CPE_CRLF
   "- DSL_CCA_debugLevels_t nDbgLevel (hex)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_NONE = 0x00" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PRN = 0x01" DSL_CPE_CRLF
   "   DSL_CCA_DBG_ERR = 0x02" DSL_CPE_CRLF
   "   DSL_CCA_DBG_WRN = 0x40" DSL_CPE_CRLF
   "   DSL_CCA_DBG_MSG = 0x80" DSL_CPE_CRLF
   "   DSL_CCA_DBG_LOCAL = 0xFF" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_CCA_DBG_ModuleLevelSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_CCA_debugLevels_t nDbgLvl = DSL_CCA_DBG_NONE;
   DSL_CCA_debugModules_t nDbgModule = DSL_CCA_DBG_NO_BLOCK;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%d %x", &nDbgModule, &nDbgLvl);

   if ((nDbgModule > DSL_CCA_DBG_NO_BLOCK) && (nDbgModule < DSL_CCA_DBG_LAST_BLOCK))
   {
      if ((nDbgLvl == DSL_CCA_DBG_PRN) || (nDbgLvl == DSL_CCA_DBG_ERR) ||
         (nDbgLvl == DSL_CCA_DBG_WRN) || (nDbgLvl == DSL_CCA_DBG_MSG) ||
         (nDbgLvl == DSL_CCA_DBG_LOCAL))
      {
         DSL_CCA_g_dbgLvl[nDbgModule].nDbgLvl = nDbgLvl;
      }
      else
      {
         ret = -1;
      }
   }
   else
   {
      ret = -1;
   }

   DSL_CPE_FPrintf (out, DSL_CPE_RET"nDbgLevel=%x" DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret), nDbgLvl);

   return 0;
}
#endif

static const DSL_char_t g_sDBGmlg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_debugModules_t nDbgModule (dec)" DSL_CPE_CRLF
   "   DSL_DBG_CPE_API = 1" DSL_CPE_CRLF
   "   DSL_DBG_G997 = 2" DSL_CPE_CRLF
   "   DSL_DBG_PM = 3" DSL_CPE_CRLF
   "   DSL_DBG_MIB = 4" DSL_CPE_CRLF
   "   DSL_DBG_CEOC = 5" DSL_CPE_CRLF
   "   DSL_DBG_LED = 6 (not used anymore!)" DSL_CPE_CRLF
   "   DSL_DBG_SAR = 7 (not used anymore!)" DSL_CPE_CRLF
   "   DSL_DBG_DEVICE = 8" DSL_CPE_CRLF
   "   DSL_DBG_AUTOBOOT_THREAD = 9" DSL_CPE_CRLF
   "   DSL_DBG_OS = 10" DSL_CPE_CRLF
   "   DSL_DBG_CALLBACK = 11" DSL_CPE_CRLF
   "   DSL_DBG_MESSAGE_DUMP = 12" DSL_CPE_CRLF
   "   DSL_DBG_LOW_LEVEL_DRIVER = 13" DSL_CPE_CRLF
   "   DSL_DBG_MULTIMODE = 14" DSL_CPE_CRLF
   "   DSL_DBG_NOTIFICATIONS = 15" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_debugLevels_t nDbgLevel (hex)" DSL_CPE_CRLF
   "   DSL_DBG_NONE = 0x00" DSL_CPE_CRLF
   "   DSL_DBG_PRN = 0x01" DSL_CPE_CRLF
   "   DSL_DBG_ERR = 0x02" DSL_CPE_CRLF
   "   DSL_DBG_WRN = 0x40" DSL_CPE_CRLF
   "   DSL_DBG_MSG = 0x80" DSL_CPE_CRLF
   "   DSL_DBG_LOCAL = 0xFF" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_ModuleLevelGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_DBG_ModuleLevel_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_DBG_ModuleLevel_t));

   DSL_CPE_sscanf (pCommands, "%u", &(pData.data.nDbgModule));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_MODULE_LEVEL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDbgLevel=%x" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nDbgLevel);
   }

   return 0;
}

static const DSL_char_t g_sDBGmls[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_debugModules_t nDbgModule (dec)" DSL_CPE_CRLF
   "   DSL_DBG_NO_BLOCK = 0 (set all)" DSL_CPE_CRLF
   "   DSL_DBG_CPE_API = 1" DSL_CPE_CRLF
   "   DSL_DBG_G997 = 2" DSL_CPE_CRLF
   "   DSL_DBG_PM = 3" DSL_CPE_CRLF
   "   DSL_DBG_MIB = 4" DSL_CPE_CRLF
   "   DSL_DBG_CEOC = 5" DSL_CPE_CRLF
   "   DSL_DBG_LED = 6 (not used anymore!)" DSL_CPE_CRLF
   "   DSL_DBG_SAR = 7 (not used anymore!)" DSL_CPE_CRLF
   "   DSL_DBG_DEVICE = 8" DSL_CPE_CRLF
   "   DSL_DBG_AUTOBOOT_THREAD = 9" DSL_CPE_CRLF
   "   DSL_DBG_OS = 10" DSL_CPE_CRLF
   "   DSL_DBG_CALLBACK = 11" DSL_CPE_CRLF
   "   DSL_DBG_MESSAGE_DUMP = 12" DSL_CPE_CRLF
   "   DSL_DBG_LOW_LEVEL_DRIVER = 13" DSL_CPE_CRLF
   "   DSL_DBG_MULTIMODE = 14" DSL_CPE_CRLF
   "   DSL_DBG_NOTIFICATIONS = 15" DSL_CPE_CRLF
   "- DSL_debugLevels_t nDbgLevel (hex)" DSL_CPE_CRLF
   "   DSL_DBG_NONE = 0x00" DSL_CPE_CRLF
   "   DSL_DBG_PRN = 0x01" DSL_CPE_CRLF
   "   DSL_DBG_ERR = 0x02" DSL_CPE_CRLF
   "   DSL_DBG_WRN = 0x40" DSL_CPE_CRLF
   "   DSL_DBG_MSG = 0x80" DSL_CPE_CRLF
   "   DSL_DBG_LOCAL = 0xFF" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_ModuleLevelSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_DBG_ModuleLevel_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_DBG_ModuleLevel_t));

   DSL_CPE_sscanf (pCommands, "%u %x", &(pData.data.nDbgModule), &(pData.data.nDbgLevel));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_MODULE_LEVEL_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   }

   return 0;
}

#ifdef INCLUDE_SCRIPT_NOTIFICATION
static const DSL_char_t g_sNsecs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t notify_script[1-256] (Attention: Use absolute firmware path only!)"
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_NotificationScriptExecuteConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_char_t sNotifyScript[256] = { 0 };

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%s", &sNotifyScript);

   /* Get ADSL script file */
   if ( strlen(sNotifyScript) > 0 )
   {
      if (DSL_CPE_IsFileExists(sNotifyScript))
      {
         if (g_sRcScript)
         {
            DSL_CPE_Free(g_sRcScript);
            g_sRcScript = DSL_NULL;
         }

         g_sRcScript = DSL_CPE_Malloc (strlen (sNotifyScript) + 1);

         if (g_sRcScript)
         {
            strcpy (g_sRcScript, sNotifyScript);
         }
      }
      else
      {
         ret = -1;
      }
   }

   DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   if (ret < 0)
   {
      DSL_CPE_FPrintf(out, " (Have you used absolute script path?)" DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* INCLUDE_SCRIPT_NOTIFICATION*/

#ifdef INCLUDE_SCRIPT_NOTIFICATION
static const DSL_char_t g_sNsecg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t notify_script[256]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_NotificationScriptExecuteConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(ret));

   if(g_sRcScript != DSL_NULL)
   {
      DSL_CPE_FPrintf (out, " notify_script=%s", g_sRcScript);
   }
   else
   {
      DSL_CPE_FPrintf (out, " notify_script=n/a");
   }

   DSL_CPE_FPrintf (out, DSL_CPE_CRLF);

   return 0;
}
#endif /* INCLUDE_SCRIPT_NOTIFICATION*/

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sAsecg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t adsl_script[256]" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_char_t vdsl_script[256]" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootScriptExecuteConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(ret));

   if(g_sAdslScript != DSL_NULL)
   {
      DSL_CPE_FPrintf (out, " adsl_script=%s", g_sAdslScript);
   }
   else
   {
      DSL_CPE_FPrintf (out, " adsl_script=n/a");
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if(g_sVdslScript != DSL_NULL)
   {
      DSL_CPE_FPrintf (out, " vdsl_script=%s", g_sVdslScript);
   }
   else
   {
      DSL_CPE_FPrintf (out, " vdsl_script=n/a");
   }
#endif

   DSL_CPE_FPrintf (out, DSL_CPE_CRLF);

   return 0;
}
#endif
#endif

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sAsecs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t adsl_script[1-256]"
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 0)
   " (let empty to clear currently set value,"
   " adsl_script must include the full path to the script)" DSL_CPE_CRLF
#else
   " (enter '.' to let adsl script unchanged)" DSL_CPE_CRLF
   "- DSL_char_t vdsl_script[1-256]"
   " (enter '.' to let vdsl script unchanged, or let empty to clear currently set value,"
   " let both parameters empty to clear currently set values"
   " vdsl_script must include the full path to the script)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootScriptExecuteConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_char_t sAdslScript[256] = { 0 };
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   DSL_char_t sVdslScript[256] = { 0 };
#endif

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_MAX) == DSL_FALSE)
#else
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_MAX) == DSL_FALSE)
#endif
   {
      return -1;
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   DSL_CPE_sscanf (pCommands, "%s %s", &sAdslScript, &sVdslScript);
#else
   DSL_CPE_sscanf (pCommands, "%s", &sAdslScript);
#endif

   /* Get ADSL script file */
   if ( strlen(sAdslScript) > 0 )
   {
      if ( strcmp(sAdslScript, ".") != 0 )
      {
         if (DSL_CPE_IsFileExists(sAdslScript))
         {
            if (g_sAdslScript)
            {
               DSL_CPE_Free(g_sAdslScript);
               g_sAdslScript = DSL_NULL;
            }
            g_sAdslScript = DSL_CPE_Malloc (strlen (sAdslScript) + 1);
            if (g_sAdslScript)
            {
               strcpy (g_sAdslScript, sAdslScript);
            }
         }
         else
         {
            ret = -1;
         }
      }
   }
   else
   {
      DSL_CPE_Free(g_sAdslScript);
      g_sAdslScript = DSL_NULL;
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   /* Get VDSL script file */
   if (strlen(sVdslScript) > 0)
   {
      if ( strcmp(sVdslScript, ".") != 0 )
      {
         if (DSL_CPE_IsFileExists(sVdslScript))
         {
            if (g_sVdslScript)
            {
               DSL_CPE_Free(g_sVdslScript);
               g_sVdslScript = DSL_NULL;
            }
            g_sVdslScript = DSL_CPE_Malloc (strlen (sVdslScript) + 1);
            if (g_sVdslScript)
            {
               strcpy (g_sVdslScript, sVdslScript);
            }
         }
         else
         {
            ret = -1;
         }
      }
   }
   else
   {
      DSL_CPE_Free(g_sVdslScript);
      g_sVdslScript = DSL_NULL;
   }
#endif

   DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   if (ret < 0)
   {
      DSL_CPE_FPrintf(out, " (Have you used absolute script path?)" DSL_CPE_CRLF);
   }

   return 0;
}
#endif
#endif

static const DSL_char_t g_sAcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AutobootCtrlSet_t nCommand" DSL_CPE_CRLF
   "   stop = 0" DSL_CPE_CRLF
   "   start = 1" DSL_CPE_CRLF
   "   restart = 2" DSL_CPE_CRLF
   "   continue = 3" DSL_CPE_CRLF
   "   disable = 4" DSL_CPE_CRLF
   "   enable = 5" DSL_CPE_CRLF
   "   restart full = 6" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootControlSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_AutobootControl_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_AutobootControl_t));

   DSL_CPE_sscanf (pCommands, "%u", &(pData.data.nCommand));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_CONTROL_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sAcfgs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bWaitBeforeConfigWrite" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bWaitBeforeLinkActivation" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bWaitBeforeRestart" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bAutoContinueWaitBeforeConfigWrite (dsl_cpe_control)" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bAutoContinueWaitBeforeLinkActivation (dsl_cpe_control)" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bAutoContinueWaitBeforeRestart (dsl_cpe_control)" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootConfigOptionSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_boolean_t bAdsl = DSL_FALSE, bVdsl = DSL_FALSE;
   DSL_boolean_t bAutoContinueWaitBeforeLinkActivation = DSL_TRUE,
   bAutoContinueWaitBeforeConfigWrite = DSL_TRUE,
   bAutoContinueWaitBeforeRestart = DSL_TRUE,
   bWaitBeforeLinkActivation = DSL_FALSE,
   bWaitBeforeConfigWrite = DSL_FALSE, bWaitBeforeRestart = DSL_FALSE;
   DSL_AutobootConfig_t pData;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   DSL_uint32_t nDevice = 0;
#endif /* #if (DSL_CPE_MAX_DSL_ENTITIES > 1)*/
   DSL_int_t nDeviceNum = -1;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 6, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_AutobootConfig_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u %u %u",
      &bWaitBeforeConfigWrite, &bWaitBeforeLinkActivation, &bWaitBeforeRestart,
      &bAutoContinueWaitBeforeConfigWrite, &bAutoContinueWaitBeforeLinkActivation,
      &bAutoContinueWaitBeforeRestart);

   if ((g_sAdslScript != DSL_NULL) && (strlen(g_sAdslScript) > 0))
   {
      bAdsl = DSL_TRUE;
   }

   if ((g_sVdslScript != DSL_NULL) && (strlen(g_sVdslScript) > 0))
   {
      bVdsl = DSL_TRUE;
   }

   if ( (bAdsl == DSL_TRUE) || (bVdsl == DSL_TRUE) )
   {
      if (!bWaitBeforeLinkActivation && !bWaitBeforeConfigWrite && !bWaitBeforeRestart)
      {
         DSL_CPE_FPrintf (out, "Warning: bWaitBeforeConfigWrite, bWaitBeforeLinkActivation and "
            "bWaitBeforeRestart were set to false "
            "but an autostart script was selected (autostart script "
            "handling only works when one these settings is set to true)" DSL_CPE_CRLF);
         DSL_CPE_FPrintf (out, "Set bWaitBeforeConfigWrite = DSL_TRUE, "
            "bWaitBeforeLinkActivation = DSL_TRUE, bWaitBeforeRestart = DSL_TRUE!" DSL_CPE_CRLF);
         bWaitBeforeConfigWrite = DSL_TRUE;
         bWaitBeforeLinkActivation = DSL_TRUE;
         bWaitBeforeRestart = DSL_TRUE;
      }
   }

   if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
   {
      if (pCtx->bBackwardCompMode)
      {
         nDeviceNum = pCtx->nDevNum;
      }
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
      else
      {
         for (nDevice = 0; nDevice < DSL_CPE_MAX_DSL_ENTITIES; nDevice++)
         {
            if(pCtx->fd[nDevice] == fd)
            {
               nDeviceNum = nDevice;
               break;
            }
         }
      }
#endif /* #if (DSL_CPE_MAX_DSL_ENTITIES > 1)*/
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
      return 0;
   }

   if (nDeviceNum == -1)
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
      return 0;
   }

   g_bWaitBeforeConfigWrite[nDeviceNum]    = bWaitBeforeConfigWrite;
   g_bWaitBeforeLinkActivation[nDeviceNum] = bWaitBeforeLinkActivation;
   g_bWaitBeforeRestart[nDeviceNum]        = bWaitBeforeRestart;

   g_bAutoContinueWaitBeforeConfigWrite[nDeviceNum]    = bAutoContinueWaitBeforeConfigWrite;
   g_bAutoContinueWaitBeforeLinkActivation[nDeviceNum] = bAutoContinueWaitBeforeLinkActivation;
   g_bAutoContinueWaitBeforeRestart[nDeviceNum]        = bAutoContinueWaitBeforeRestart;

   pData.data.nStateMachineOptions.bWaitBeforeConfigWrite =
      bWaitBeforeConfigWrite;

   pData.data.nStateMachineOptions.bWaitBeforeLinkActivation =
      bWaitBeforeLinkActivation;

   pData.data.nStateMachineOptions.bWaitBeforeRestart =
      bWaitBeforeRestart;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   }

   return 0;
}
#endif

#ifdef INCLUDE_DSL_CONFIG_GET
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sAcfgg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bWaitBeforeConfigWrite" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bWaitBeforeLinkActivation" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bWaitBeforeRestart" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bAutoContinueWaitBeforeConfigWrite (dsl_cpe_control)" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bAutoContinueWaitBeforeLinkActivation (dsl_cpe_control)" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bAutoContinueWaitBeforeRestart (dsl_cpe_control)" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootConfigOptionGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_AutobootConfig_t pData;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   DSL_uint32_t nDevice = 0;
#endif /* #if (DSL_CPE_MAX_DSL_ENTITIES > 1)*/
   DSL_int_t nDeviceNum = -1;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_AutobootConfig_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
      {
         if (pCtx->bBackwardCompMode)
         {
            nDeviceNum = pCtx->nDevNum;
         }
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
         else
         {
            for (nDevice = 0; nDevice < DSL_CPE_MAX_DSL_ENTITIES; nDevice++)
            {
               if(pCtx->fd[nDevice] == fd)
               {
                  nDeviceNum = nDevice;
                  break;
               }
            }
         }
#endif /* #if (DSL_CPE_MAX_DSL_ENTITIES > 1)*/
      }
      else
      {
         DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
         return 0;
      }

      if (nDeviceNum == -1)
      {
         DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
         return 0;
      }


      g_bWaitBeforeConfigWrite[nDeviceNum] =
         pData.data.nStateMachineOptions.bWaitBeforeConfigWrite;
      g_bWaitBeforeLinkActivation[nDeviceNum] =
         pData.data.nStateMachineOptions.bWaitBeforeLinkActivation;
      g_bWaitBeforeRestart[nDeviceNum] =
         pData.data.nStateMachineOptions.bWaitBeforeRestart;

      DSL_CPE_FPrintf (out, DSL_CPE_RET"bWaitBeforeConfigWrite=%d "
         "bWaitBeforeLinkActivation=%d bWaitBeforeRestart=%d bAutoContinueWaitBeforeConfigWrite=%d "
         "bAutoContinueWaitBeforeLinkActivation=%d bAutoContinueWaitBeforeRestart=%d"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn), g_bWaitBeforeConfigWrite[nDeviceNum],
         g_bWaitBeforeLinkActivation[nDeviceNum], g_bWaitBeforeRestart[nDeviceNum],
         g_bAutoContinueWaitBeforeConfigWrite[nDeviceNum], g_bAutoContinueWaitBeforeLinkActivation[nDeviceNum],
         g_bAutoContinueWaitBeforeRestart[nDeviceNum]);
   }

   return 0;
}
#endif
#endif

static const DSL_char_t g_sAsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AutobootStatus_t nStatus" DSL_CPE_CRLF
   "   stopped = 0" DSL_CPE_CRLF
   "   starting = 1" DSL_CPE_CRLF
   "   running = 2" DSL_CPE_CRLF
   "   fw wait = 3" DSL_CPE_CRLF
   "   config write wait = 4" DSL_CPE_CRLF
   "   link activation wait = 5" DSL_CPE_CRLF
   "   restart wait = 6" DSL_CPE_CRLF
   "   disabled = 7" DSL_CPE_CRLF
   "- DSL_FirmwareRequestType_t nFirmwareRequestType" DSL_CPE_CRLF
   "   fw request na = 0" DSL_CPE_CRLF
   "   fw request adsl = 1" DSL_CPE_CRLF
   "   fw request vdsl = 2" DSL_CPE_CRLF
   "   fw request xdsl = 3" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_AutobootStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_AutobootStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nStatus=%d nFirmwareRequestType=%d" DSL_CPE_CRLF,
      DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
      pData.data.nStatus, pData.data.nFirmwareRequestType);
   }

   return 0;
}

static const DSL_char_t g_sLsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_LineState_t nLineState (hex)" DSL_CPE_CRLF
   "   not initialized = 0x00000000" DSL_CPE_CRLF
   "   exception = 0x00000001" DSL_CPE_CRLF
   "   not updated = 0x00000010" DSL_CPE_CRLF
   "   disabled = 0x00000020" DSL_CPE_CRLF
   "   idle request = 0x000000ff" DSL_CPE_CRLF
   "   idle = 0x00000100" DSL_CPE_CRLF
   "   silent request = 0x000001ff" DSL_CPE_CRLF
   "   silent = 0x00000200" DSL_CPE_CRLF
   "   handshake = 0x00000300" DSL_CPE_CRLF
#ifdef INCLUDE_DSL_BONDING
   "   bonding clr = 0x00000310" DSL_CPE_CRLF
#endif
   "   t1413 = 0x00000370" DSL_CPE_CRLF
   "   full_init = 0x00000380" DSL_CPE_CRLF
   "   discovery = 0x00000400" DSL_CPE_CRLF
   "   training = 0x00000500" DSL_CPE_CRLF
   "   analysis = 0x00000600" DSL_CPE_CRLF
   "   exchange = 0x00000700" DSL_CPE_CRLF
   "   showtime_no_sync = 0x00000800" DSL_CPE_CRLF
   "   showtime_tc_sync = 0x00000801" DSL_CPE_CRLF
   "   orderly_shutdown_request = 0x0000085F" DSL_CPE_CRLF
   "   orderly_shutdown = 0x00000860" DSL_CPE_CRLF
   "   fastretrain = 0x00000900" DSL_CPE_CRLF
   "   lowpower_l2 = 0x00000a00" DSL_CPE_CRLF
   "   loopdiagnostic active = 0x00000b00" DSL_CPE_CRLF
   "   loopdiagnostic data exchange = 0x00000b10" DSL_CPE_CRLF
   "   loopdiagnostic data request = 0x00000b20" DSL_CPE_CRLF
   "   loopdiagnostic complete = 0x00000c00" DSL_CPE_CRLF
#ifdef INCLUDE_DSL_FILTER_DETECTION
   "   filter detection active = 0x01000050" DSL_CPE_CRLF
   "   filter detection complete = 0x01000060" DSL_CPE_CRLF
#endif
   "   test = 0x01000000" DSL_CPE_CRLF
   "   resync = 0x00000d00" DSL_CPE_CRLF
   "   short init entry = 0x000003c0" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LineStateGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LineState_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LineState_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LINE_STATE_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nLineState=0x%x" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nLineState);
   }

   return 0;
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
static const DSL_char_t g_sRusg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t drvStaticMemUsage" DSL_CPE_CRLF
   "- DSL_uint32_t drvDynamicMemUsage" DSL_CPE_CRLF
   "- DSL_uint32_t appStaticMemUsage" DSL_CPE_CRLF
   "- DSL_uint32_t appDynamicMemUsage" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_ResourceUsageStatisticsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_ResourceUsageStatistics_t pData;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;
   DSL_CPE_ResourceUsageStatisticsData_t resourceUsageStatisticsData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_ResourceUsageStatistics_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_RESOURCE_USAGE_STATISTICS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
      {
         ret = DSL_CPE_ResourceUsageStatisticsGet(
                  pCtx, &resourceUsageStatisticsData);
         if (ret < 0)
         {
            DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(ret));
         }
         else
         {
            DSL_CPE_FPrintf (out,
               DSL_CPE_RET"drvStaticMemUsage=%d drvDynamicMemUsage=%d "
               "appStaticMemUsage=%d appDynamicMemUsage=%d" DSL_CPE_CRLF,
               DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
               pData.data.staticMemUsage, pData.data.dynamicMemUsage,
               resourceUsageStatisticsData.staticMemUsage,
               resourceUsageStatisticsData.dynamicMemUsage);
         }
      }
      else
      {
         DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
      }
   }

   return 0;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sEsmcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_EventType_t nEventType (dec)" DSL_CPE_CRLF
   "   linit failure = 7" DSL_CPE_CRLF
   "   line state = 8" DSL_CPE_CRLF
   "   line powermanagement state = 9" DSL_CPE_CRLF
   "   channel datarate = 10" DSL_CPE_CRLF
   "   firmware error = 11" DSL_CPE_CRLF
   "   init ready = 12" DSL_CPE_CRLF
   "   fe inventory available = 13" DSL_CPE_CRLF
   "   system status = 14" DSL_CPE_CRLF
   "   pm sync = 15" DSL_CPE_CRLF
   "   line transmission status = 16" DSL_CPE_CRLF
   "   showtime logging = 17" DSL_CPE_CRLF
   "   firmware request = 18" DSL_CPE_CRLF
   "   snmp message available = 19" DSL_CPE_CRLF
   "   system interface status = 20" DSL_CPE_CRLF
   "   firmware download status = 21" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bMask" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_EventStatusMaskConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_EventStatusMask_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_EventStatusMask_t));

   DSL_CPE_sscanf (pCommands, "%d", &(pData.data.nEventType));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_EVENT_STATUS_MASK_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"bMask=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.bMask);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

static const DSL_char_t g_sEsmcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_EventType_t nEventType (dec)" DSL_CPE_CRLF
   "   linit failure = 7" DSL_CPE_CRLF
   "   line state = 8" DSL_CPE_CRLF
   "   line powermanagement state = 9" DSL_CPE_CRLF
   "   channel datarate = 10" DSL_CPE_CRLF
   "   firmware error = 11" DSL_CPE_CRLF
   "   init ready = 12" DSL_CPE_CRLF
   "   fe inventory available = 13" DSL_CPE_CRLF
   "   system status = 14" DSL_CPE_CRLF
   "   pm sync = 15" DSL_CPE_CRLF
   "   line transmission status = 16" DSL_CPE_CRLF
   "   showtime logging = 17" DSL_CPE_CRLF
   "   firmware request = 18" DSL_CPE_CRLF
   "   snmp message available = 19" DSL_CPE_CRLF
   "   system interface status = 20" DSL_CPE_CRLF
   "   firmware download status = 21" DSL_CPE_CRLF
   "- DSL_boolean_t bMask" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_EventStatusMaskConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_EventStatusMask_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_EventStatusMask_t));

   DSL_CPE_sscanf (pCommands, "%d %u", &(pData.data.nEventType), &(pData.data.bMask));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_EVENT_STATUS_MASK_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

#ifdef INCLUDE_DSL_FRAMING_PARAMETERS
static const DSL_char_t g_sFpsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nTp" DSL_CPE_CRLF
   "- DSL_uint16_t nMp" DSL_CPE_CRLF
   "- DSL_uint16_t nSEQ" DSL_CPE_CRLF
   "- DSL_uint16_t nBP" DSL_CPE_CRLF
   "- DSL_uint16_t nMSGC" DSL_CPE_CRLF
   "- DSL_uint16_t nMSGLP" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_FramingParameterStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_FramingParameterStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_FramingParameterStatus_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_FRAMING_PARAMETER_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nTp=%hu nMp=%hu nSEQ=%hu "
         "nBP=%hu nMSGC=%hu nMSGLP=%hu"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nTp, pData.data.nMp, pData.data.nSEQ, pData.data.nBP,
         pData.data.nMSGC, pData.data.nMSGLP);
   }

   return 0;
}
#endif /* INCLUDE_DSL_FRAMING_PARAMETERS*/

static const DSL_char_t g_sIsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bEventActivation" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_BF_ResourceActivationType_t nResourceActivationMask (hex)" DSL_CPE_CRLF
   "   cleaned = 0x00000000" DSL_CPE_CRLF
   "   snmp = 0x00000001" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_InstanceStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_InstanceStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_InstanceStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_INSTANCE_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"bEventActivation=%d nEventActivationMask=0x%x"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.bEventActivation, pData.data.nResourceActivationMask);
   }

   return 0;
}

static const DSL_char_t g_sIcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bEventActivation" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_BF_ResourceActivationType_t nResourceActivationMask (hex)" DSL_CPE_CRLF
   "   cleaned = 0x00000000" DSL_CPE_CRLF
   "   snmp = 0x00000001" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_InstanceControlSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_InstanceControl_t pData;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_InstanceControl_t));

   DSL_CPE_sscanf (pCommands, "%u %x", &(pData.data.bEventActivation),
      &(pData.data.nResourceActivationMask));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_INSTANCE_CONTROL_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));

      if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
      {
         if (pData.data.bEventActivation)
         {
            DSL_CPE_FPrintf (out, "STARTING_1" DSL_CPE_CRLF);

            if (!pCtx->bEvtRun)
            {
               DSL_CPE_FPrintf (out, "STARTING_2" DSL_CPE_CRLF);
               /* Start event thread*/
               DSL_CPE_EventHandlerStart(pCtx);
            }
         }
         else
         {
            if (pCtx->bEvtRun)
            {
               pCtx->bEvtRun = DSL_FALSE;
                  /* wait while CPE Event handler thread ends */
               DSL_CPE_ThreadShutdown (&pCtx->EventControl, 1000);
               }
            }
         }
      else
      {
         DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
      }
   }

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sLfcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bTrellisEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bBitswapEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bReTxEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bVirtualNoiseSupport" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_FeatureSupport_t b20BitSupport" DSL_CPE_CRLF
   "   DSL_FEATURE_NA = -1" DSL_CPE_CRLF
   "   DSL_FEATURE_DISABLED = 0" DSL_CPE_CRLF
   "   DSL_FEATURE_ENABLED = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LineFeatureConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LineFeature_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LineFeature_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LINE_FEATURE_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u bTrellisEnable=%u "
         "bBitswapEnable=%u bReTxEnable=%u bVirtualNoiseSupport=%u"
         " b20BitSupport=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.bTrellisEnable, pData.data.bBitswapEnable,
         pData.data.bReTxEnable, pData.data.bVirtualNoiseSupport,
         pData.data.b20BitSupport);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

static const DSL_char_t g_sLfcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bTrellisEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bBitswapEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bReTxEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bVirtualNoiseSupport" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_FeatureSupport_t b20BitSupport" DSL_CPE_CRLF
   "   DSL_FEATURE_NA = -1" DSL_CPE_CRLF
   "   DSL_FEATURE_DISABLED = 0" DSL_CPE_CRLF
   "   DSL_FEATURE_ENABLED = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
      "   upstream = 0" DSL_CPE_CRLF
      "   downstream = 1" DSL_CPE_CRLF
      DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LineFeatureConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LineFeature_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 6, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LineFeature_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u %u %d", &pData.nDirection,
      &pData.data.bTrellisEnable, &pData.data.bBitswapEnable,
      &pData.data.bReTxEnable, &pData.data.bVirtualNoiseSupport, &pData.data.b20BitSupport);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LINE_FEATURE_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);
   }

   return 0;
}

static const DSL_char_t g_sLfsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bTrellisEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bBitswapEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bReTxEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bVirtualNoiseSupport" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_FeatureSupport_t b20BitSupport" DSL_CPE_CRLF
   "   DSL_FEATURE_NA = -1" DSL_CPE_CRLF
   "   DSL_FEATURE_DISABLED = 0" DSL_CPE_CRLF
   "   DSL_FEATURE_ENABLED = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LineFeatureStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LineFeature_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LineFeature_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LINE_FEATURE_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u bTrellisEnable=%u "
         "bBitswapEnable=%u bReTxEnable=%u bVirtualNoiseSupport=%u b20BitSupport=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.bTrellisEnable, pData.data.bBitswapEnable,
         pData.data.bReTxEnable, pData.data.bVirtualNoiseSupport, pData.data.b20BitSupport);
   }

   return 0;
}

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sSe[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t sFileName[1-" _MKSTR(DSL_MAX_COMMAND_LINE_LENGTH) "] (full path to the script)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_ScriptExecute(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_char_t sFileName[DSL_MAX_COMMAND_LINE_LENGTH] = {0};

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%s", sFileName);

   if ((ret = DSL_CPE_ScriptFileParse(DSL_CPE_GetGlobalContext(), -1, sFileName,
                 DSL_SCRIPT_SECTION_COMMON)) < 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(ret));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT */
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */

#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sSicg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   "- DSL_TcLayerSelection_t nTcLayer" DSL_CPE_CRLF
   "   tc unknown = 0" DSL_CPE_CRLF
   "   tc atm = 1" DSL_CPE_CRLF
   "   tc efm = 2" DSL_CPE_CRLF
   "   tc hdlc = 3" DSL_CPE_CRLF
   "   tc auto = 4" DSL_CPE_CRLF
   "   tc efm forced = 5" DSL_CPE_CRLF
   "- DSL_BF_EfmTcConfig_t nEfmTcConfigUs (hex)" DSL_CPE_CRLF
   "   efm tc cleaned = 0x00000000" DSL_CPE_CRLF
   "   efm tc normal = 0x00000001" DSL_CPE_CRLF
   "   efm tc pre emption = 0x00000002"  DSL_CPE_CRLF
   "   efm tc short packets = 0x00000004" DSL_CPE_CRLF
   "- DSL_BF_EfmTcConfig_t nEfmTcConfigDs" DSL_CPE_CRLF
   "   efm tc cleaned = 0x00000000" DSL_CPE_CRLF
   "   efm tc normal = 0x00000001" DSL_CPE_CRLF
   "   efm tc pre emption = 0x00000002"  DSL_CPE_CRLF
   "   efm tc short packets = 0x00000004" DSL_CPE_CRLF
   "- DSL_CPE_SystemInterfaceSelection_t nSystemIf" DSL_CPE_CRLF
   "   systemif unknown = 0" DSL_CPE_CRLF
   "   systemif utopia = 1" DSL_CPE_CRLF
   "   systemif posphy = 2" DSL_CPE_CRLF
   "   systemif mii = 3" DSL_CPE_CRLF
   "   systemif undefined = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_SystemInterfaceConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_SystemInterfaceConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_SystemInterfaceConfig_t));

   DSL_CPE_sscanf (pCommands, "%u", &(pData.nDslMode));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_SYSTEM_INTERFACE_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDslMode=%u nTcLayer=%u "
         "nEfmTcConfigUs=0x%x nEfmTcConfigDs=0x%x nSystemIf=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDslMode,
         pData.data.nTcLayer, pData.data.nEfmTcConfigUs,
         pData.data.nEfmTcConfigDs, pData.data.nSystemIf);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
static const DSL_char_t g_sSics[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   "- DSL_TcLayerSelection_t nTcLayer" DSL_CPE_CRLF
   "   tc unknown = 0" DSL_CPE_CRLF
   "   tc atm = 1" DSL_CPE_CRLF
   "   tc efm = 2" DSL_CPE_CRLF
   "   tc hdlc = 3" DSL_CPE_CRLF
   "   tc auto = 4" DSL_CPE_CRLF
   "   tc efm forced = 5" DSL_CPE_CRLF
   "- DSL_BF_EfmTcConfig_t nEfmTcConfigUs (hex)" DSL_CPE_CRLF
   "   efm tc cleaned = 0x00000000" DSL_CPE_CRLF
   "   efm tc normal = 0x00000001" DSL_CPE_CRLF
   "   efm tc pre emption = 0x00000002"  DSL_CPE_CRLF
   "   efm tc short packets = 0x00000004" DSL_CPE_CRLF
   "- DSL_BF_EfmTcConfig_t nEfmTcConfigDs (hex)" DSL_CPE_CRLF
   "   efm tc cleaned = 0x00000000" DSL_CPE_CRLF
   "   efm tc normal = 0x00000001" DSL_CPE_CRLF
   "   efm tc pre emption = 0x00000002"  DSL_CPE_CRLF
   "   efm tc short packets = 0x00000004" DSL_CPE_CRLF
   "- DSL_CPE_SystemInterfaceSelection_t nSystemIf" DSL_CPE_CRLF
   "   systemif unknown = 0" DSL_CPE_CRLF
   "   systemif utopia = 1" DSL_CPE_CRLF
   "   systemif posphy = 2" DSL_CPE_CRLF
   "   systemif mii = 3" DSL_CPE_CRLF
   "   systemif undefined = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_SystemInterfaceConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_SystemInterfaceConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 5, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_SystemInterfaceConfig_t));

   DSL_CPE_sscanf (pCommands, "%u %u %xu %xu %u", &(pData.nDslMode),
      &(pData.data.nTcLayer), &(pData.data.nEfmTcConfigUs),
      &(pData.data.nEfmTcConfigDs), &(pData.data.nSystemIf));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_SYSTEM_INTERFACE_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDslMode=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDslMode);
   }

   return 0;
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
static const DSL_char_t g_sSisg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_TcLayerSelection_t nTcLayer" DSL_CPE_CRLF
   "   tc unknown = 0" DSL_CPE_CRLF
   "   tc atm = 1" DSL_CPE_CRLF
   "   tc efm = 2" DSL_CPE_CRLF
   "   tc hdlc = 3" DSL_CPE_CRLF
   "   tc auto = 4" DSL_CPE_CRLF
   "   tc efm forced = 5" DSL_CPE_CRLF
   "- DSL_BF_EfmTcConfig_t nEfmTcConfigUs (hex)" DSL_CPE_CRLF
   "   efm tc cleaned = 0x00000000" DSL_CPE_CRLF
   "   efm tc normal = 0x00000001" DSL_CPE_CRLF
   "   efm tc pre emption = 0x00000002"  DSL_CPE_CRLF
   "   efm tc short packets = 0x00000004" DSL_CPE_CRLF
   "- DSL_BF_EfmTcConfig_t nEfmTcConfigDs (hex)" DSL_CPE_CRLF
   "   efm tc cleaned = 0x00000000" DSL_CPE_CRLF
   "   efm tc normal = 0x00000001" DSL_CPE_CRLF
   "   efm tc pre emption = 0x00000002"  DSL_CPE_CRLF
   "   efm tc short packets = 0x00000004" DSL_CPE_CRLF
   "- DSL_CPE_SystemInterfaceSelection_t nSystemIf" DSL_CPE_CRLF
   "   systemif unknown = 0" DSL_CPE_CRLF
   "   systemif utopia = 1" DSL_CPE_CRLF
   "   systemif posphy = 2" DSL_CPE_CRLF
   "   systemif mii = 3" DSL_CPE_CRLF
   "   systemif undefined = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_SystemInterfaceStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_SystemInterfaceStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_SystemInterfaceStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_SYSTEM_INTERFACE_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nTcLayer=%u nEfmTcConfigUs=0x%x "
         "nEfmTcConfigDs=0x%x nSystemIf=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nTcLayer, pData.data.nEfmTcConfigUs,
         pData.data.nEfmTcConfigDs, pData.data.nSystemIf);
   }

   return 0;
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

static const DSL_char_t g_sTmcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_TestModeControlSet_t nTestMode" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   "   showtime_lock = 1" DSL_CPE_CRLF
   "   quiet = 2" DSL_CPE_CRLF
   "   training lock = 3" DSL_CPE_CRLF
   "   lock = 4" DSL_CPE_CRLF
   "   unlock = 5" DSL_CPE_CRLF
   "   sleep = 6" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_TestModeControlSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_TestModeControl_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_TestModeControl_t));

   DSL_CPE_sscanf (pCommands, "%u", &(pData.data.nTestMode));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_TEST_MODE_CONTROL_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sTmsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_TestModeStatus_t nTestMode" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   "   showtime_lock = 1" DSL_CPE_CRLF
   "   quiet = 2" DSL_CPE_CRLF
   "   training lock = 3" DSL_CPE_CRLF
   "   lock = 4" DSL_CPE_CRLF
   "   unlock = 5" DSL_CPE_CRLF
   "   sleep = 6" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_TestModeStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_TestModeStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_TestModeStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_TEST_MODE_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nTestMode=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nTestMode);
   }

   return 0;
}

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
static const DSL_char_t g_sBbsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_uint16_t nBandIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nLimit_firstToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nLimit_lastToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nBorder_firstToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nBorder_lastToneIndex" DSL_CPE_CRLF
   "- ... nParamNr[5] nCount[" _MKSTR(DSL_G997_MAX_NUMBER_OF_BANDS) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BandBorderStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t nBand = 0;
   DSL_BandBorderStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_BandBorderStatus_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BAND_BORDER_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {

      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%d nNumData=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.nNumData);
      DSL_CPE_FPrintf (out,
      "nFormat=(nBandIndex, (nLimit_firstToneIndex, nLimit_lastToneIndex), "
      "(nBorder_firstToneIndex, nBorder_lastToneIndex)) nData=\"" DSL_CPE_CRLF);

      for (nBand = 0; nBand < pData.data.nNumData; nBand++)
      {
         DSL_CPE_FPrintf (out, "(%02d,(%04d,%04d),(%04d,%04d))"DSL_CPE_CRLF, nBand,
         pData.data.nBandLimits[nBand].nFirstToneIndex, pData.data.nBandLimits[nBand].nLastToneIndex,
         pData.data.nBandBorder[nBand].nFirstToneIndex, pData.data.nBandBorder[nBand].nLastToneIndex);
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );

   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
static const DSL_char_t g_sMlsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_MiscLineStatusSelector_t nStatusSelector" DSL_CPE_CRLF
   "   ATTNDR_CODING_GAIN_DS = 0" DSL_CPE_CRLF
   "   ATTNDR_MAX_BITS_PER_SYMBOL_DS = 1" DSL_CPE_CRLF
   "   DUAL_LATENCY_ON_US = 2" DSL_CPE_CRLF
   "   DUAL_LATENCY_ON_DS = 3" DSL_CPE_CRLF
   "   INIT_SNR_DS = 4" DSL_CPE_CRLF
   "   NOMPSD_US = 5" DSL_CPE_CRLF
   "   NOMPSD_DS = 6" DSL_CPE_CRLF
   "   LINE_RATE_US = 7" DSL_CPE_CRLF
   "   LINE_RATE_DS = 8" DSL_CPE_CRLF
   "   NET_RATE_US = 9" DSL_CPE_CRLF
   "   NET_RATE_DS = 10" DSL_CPE_CRLF
   "   TOTAL_DATA_RATE_US = 11" DSL_CPE_CRLF
   "   TOTAL_DATA_RATE_DS = 12" DSL_CPE_CRLF
   "   AGGREGATE_DATA_RATE_US = 13" DSL_CPE_CRLF
   "   AGGREGATE_DATA_RATE_DS = 14" DSL_CPE_CRLF
   "   MIN_INP_IP_LP0_DS = 15" DSL_CPE_CRLF
   "   MIN_INP_FP_LP1_DS = 16" DSL_CPE_CRLF
   "   ATT_AGGREGATE_DATA_RATE_US = 17" DSL_CPE_CRLF
   "   ATT_AGGREGATE_DATA_RATE_DS = 18" DSL_CPE_CRLF
   "   DSL_MLS_ATT_LINE_DATA_RATE_US = 19" DSL_CPE_CRLF
   "   DSL_MLS_ATT_LINE_DATA_RATE_DS = 20" DSL_CPE_CRLF
   "   DSL_MLS_ATT_TOTAL_DATA_RATE_US = 21" DSL_CPE_CRLF
   "   DSL_MLS_ATT_TOTAL_DATA_RATE_DS = 22" DSL_CPE_CRLF
   "   DSL_MLS_MASK_ANNEX_M_J_US = 23" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int32_t nStatusValue" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_MiscLineStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_MiscLineStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_MiscLineStatus_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.data.nStatusSelector);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_MISC_LINE_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {

      DSL_CPE_FPrintf (out, DSL_CPE_RET"nStatusValue=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nStatusValue);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

static const DSL_char_t g_sRaCs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_G997_RA_MODE_t RA_MODE" DSL_CPE_CRLF
   "   at init     = 2" DSL_CPE_CRLF
   "   dynamic     = 3" DSL_CPE_CRLF
   "   dynamic SOS = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_RateAdaptationConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_RateAdaptationConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_RateAdaptationConfig_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u", &pData.nDslMode,&pData.nDirection,
      &pData.data.RA_MODE);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_RATE_ADAPTATION_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDslMode=%u nDirection=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDslMode,
         pData.nDirection);
   }

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sRaCg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DslModeSelection_t nDslMode" DSL_CPE_CRLF
   "   mode adsl = 0" DSL_CPE_CRLF
#if defined(INCLUDE_DSL_CPE_API_VRX)
   "   mode vdsl = 1" DSL_CPE_CRLF
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_G997_RA_MODE_t RA_MODE" DSL_CPE_CRLF
   "   at init     = 2" DSL_CPE_CRLF
   "   dynamic     = 3" DSL_CPE_CRLF
   "   dynamic SOS = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_RateAdaptationConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_RateAdaptationConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_RateAdaptationConfig_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDslMode, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_RATE_ADAPTATION_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDslMode=%u nDirection=%hu RA_MODE=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDslMode,
         pData.nDirection, pData.data.RA_MODE);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_G997_ALARM
#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sG997amdpfcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_G997_BF_DataPathFailures_t nDataPathFailures (hex)" DSL_CPE_CRLF
   "   ncd = 0x00000001" DSL_CPE_CRLF
   "   lcd = 0x00000002" DSL_CPE_CRLF
   "   oos = 0x00000004" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_AlarmMaskDataPathFailuresConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_DataPathFailures_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_DataPathFailures_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nDataPathFailures=0x%x" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.data.nDataPathFailures);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
static const DSL_char_t g_sG997amdpfcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_G997_BF_DataPathFailures_t nDataPathFailures (hex)" DSL_CPE_CRLF
   "   ncd = 0x00000001" DSL_CPE_CRLF
   "   lcd = 0x00000002" DSL_CPE_CRLF
   "   oos = 0x00000004" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_AlarmMaskDataPathFailuresConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_DataPathFailures_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_DataPathFailures_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %xu", &pData.nChannel, &pData.nDirection,
      &pData.data.nDataPathFailures);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_ALARM_MASK_DATA_PATH_FAILURES_CONFIG_SET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sG997amlfcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_G997_BF_LineFailures_t nLineFailures (hex)" DSL_CPE_CRLF
   "   lpr = 0x00000001" DSL_CPE_CRLF
   "   lof = 0x00000002" DSL_CPE_CRLF
   "   los = 0x00000004" DSL_CPE_CRLF
   "   lom = 0x00000008" DSL_CPE_CRLF
   "   lol = 0x00000010" DSL_CPE_CRLF
   "   ese = 0x00000020" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_AlarmMaskLineFailuresConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineFailures_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineFailures_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nLineFailures=0x%x" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.nLineFailures);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
static const DSL_char_t g_sG997amlfcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_G997_BF_LineFailures_t nLineFailures (hex)" DSL_CPE_CRLF
   "   lpr = 0x00000001" DSL_CPE_CRLF
   "   lof = 0x00000002" DSL_CPE_CRLF
   "   los = 0x00000004" DSL_CPE_CRLF
   "   lom = 0x00000008" DSL_CPE_CRLF
   "   lol = 0x00000010" DSL_CPE_CRLF
   "   ese = 0x00000020" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_AlarmMaskLineFailuresConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineFailures_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineFailures_t));

   DSL_CPE_sscanf (pCommands, "%u %xu", &pData.nDirection, &pData.data.nLineFailures);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_ALARM_MASK_LINE_FAILURES_CONFIG_SET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_PER_TONE
static const DSL_char_t g_sG997bang[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint8_t nBit (hex)"DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_BitAllocationNscGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0, nSkipped = 0;
   DSL_uint16_t nUsedNumData = 0;
   unsigned char *pResult, *pParse, Num;
   DSL_G997_BitAllocationNsc_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_BitAllocationNsc_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, "nReturn=%d" DSL_CPE_CRLF, DSL_ERR_MEMORY);
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_BitAllocationNsc_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData->nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_BIT_ALLOCATION_NSC_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      /* Get the number of actually used tones*/
      for (i = 0; (i < pData->data.bitAllocationNsc.nNumData) && (i < DSL_MAX_NSC); i++)
      {
         if (pData->data.bitAllocationNsc.nNSCData[i])
         {
            nUsedNumData++;
         }
      }

      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection,
         nUsedNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nBit(hex)) nData=\"");

      /* alloc size of value-pairs, number of line-breaks and security */
      pResult = malloc(
         (pData->data.bitAllocationNsc.nNumData)*10 +
         (pData->data.bitAllocationNsc.nNumData)*2/10 + 10);

      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; (i < pData->data.bitAllocationNsc.nNumData) && (i < DSL_MAX_NSC); i++)
         {
            /* skip the printout when 0 */
            if (pData->data.bitAllocationNsc.nNSCData[i] == 0)
            {
               /* do a separator only when starting to skip */
               if (!nSkipped)
               {
                  nSkipped = 1;
                  *pParse++ = '\r';
                  *pParse++ = '\n';
               }
               continue;
            }

            nSkipped = 0;

            /* do own encoding for performance reasons */
            if (i%10==0)
            {
               *pParse++ = '\r';
               *pParse++ = '\n';
            }
            *pParse++ = '(';
            *pParse++ = (unsigned char)('0' + i/1000);
            *pParse++ = (unsigned char)('0' + (i%1000)/100);
            *pParse++ = (unsigned char)('0' + (i%100)/10);
            *pParse++ = (unsigned char)('0' + (i%10));
            *pParse++ = ',';
            Num = ((pData->data.bitAllocationNsc.nNSCData[i] & 0xf0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.bitAllocationNsc.nNSCData[i] & 0x0f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ')';
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s\"" DSL_CPE_CRLF, pResult );
         free(pResult);
      }
      else
      {
         /* not enough memory */
         DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
      }
   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997bansg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_uint8_t nBit (hex)"DSL_CPE_CRLF
   "- ... nParamNr[1] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_BitAllocationNscShortGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, i = 0;
   DSL_G997_BitAllocationNsc_t *pData;
   unsigned char *pResult, *pParse, Num;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_BitAllocationNsc_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF , DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_BitAllocationNsc_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData->nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_BIT_ALLOCATION_NSC_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection,
         pData->data.bitAllocationNsc.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=nBit(hex) nData=\"" DSL_CPE_CRLF);

      /* use own encoding for performance reasons */
      pResult = malloc((pData->data.bitAllocationNsc.nNumData)*3+10);
      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; (i < pData->data.bitAllocationNsc.nNumData) && (i < DSL_MAX_NSC); i++)
         {
            Num = ((pData->data.bitAllocationNsc.nNSCData[i] & 0xf0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.bitAllocationNsc.nNSCData[i] & 0x0f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s\"" DSL_CPE_CRLF,pResult );
         free(pResult);
      }
   }
   free(pData);
   return 0;
}
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_CONFIG_GET
#ifdef INCLUDE_DSL_G997_ALARM
static const DSL_char_t g_sG997cdrtcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nDataRateThresholdUpshift" DSL_CPE_CRLF
   "- DSL_uint32_t nDataRateThresholdDownshift" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_ChannelDataRateThresholdConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_ChannelDataRateThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_ChannelDataRateThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nDataRateThresholdUpshift=%u "
         "nDataRateThresholdDownshift=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.data.nDataRateThresholdUpshift,
         (DSL_uint32_t)pData.data.nDataRateThresholdDownshift);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/
#endif /* INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_G997_ALARM
static const DSL_char_t g_sG997cdrtcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nDataRateThresholdUpshift" DSL_CPE_CRLF
   "- DSL_uint32_t nDataRateThresholdDownshift" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_ChannelDataRateThresholdConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_ChannelDataRateThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_ChannelDataRateThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u", &pData.nChannel, &pData.nDirection,
      &pData.data.nDataRateThresholdUpshift,
      &pData.data.nDataRateThresholdDownshift);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_CHANNEL_DATA_RATE_THRESHOLD_CONFIG_SET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

static const DSL_char_t g_sG997csg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint32_t ActualDataRate" DSL_CPE_CRLF
   "- DSL_uint32_t PreviousDataRate" DSL_CPE_CRLF
   "- DSL_uint32_t ActualInterleaveDelay" DSL_CPE_CRLF
   "- DSL_uint16_t ActualImpulseNoiseProtection" DSL_CPE_CRLF
   "- DSL_uint32_t ActualNetDataRate" DSL_CPE_CRLF
   "- DSL_uint16_t ActualImpulseNoiseProtectionRein" DSL_CPE_CRLF
   "- DSL_uint16_t ActualImpulseNoiseProtectionNoErasure" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_ChannelStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_ChannelStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_ChannelStatus_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_CHANNEL_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u ActualDataRate=%u "
         "PreviousDataRate=%u ActualInterleaveDelay=%u "
         "ActualImpulseNoiseProtection=%hu ActualNetDataRate=%u "
         "ActualImpulseNoiseProtectionRein=%hu "
         "ActualImpulseNoiseProtectionNoErasure=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.data.ActualDataRate,(DSL_uint32_t) pData.data.PreviousDataRate,
         (DSL_uint32_t)pData.data.ActualInterleaveDelay,
         (DSL_uint32_t)pData.data.ActualImpulseNoiseProtection,
         (DSL_uint32_t)pData.data.ActualNetDataRate,
         (DSL_uint32_t)pData.data.ActualImpulseNoiseProtectionRein,
         (DSL_uint32_t)pData.data.ActualImpulseNoiseProtectionNoErasure);
   }

   return 0;
}

static const DSL_char_t g_sG997dpfsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_G997_BF_DataPathFailures_t nDataPathFailures (hex)" DSL_CPE_CRLF
   "   datapathfailure cleaned = 0x00000000" DSL_CPE_CRLF
   "   datapathfailure ncd = 0x00000001" DSL_CPE_CRLF
   "   datapathfailure lcd = 0x00000002" DSL_CPE_CRLF
   "   datapathfailure oos = 0x00000004" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DataPathFailuresStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_DataPathFailures_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_DataPathFailures_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_DATA_PATH_FAILURES_STATUS_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nChannel=%hu nDirection=%u nDataPathFailures=%x" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nDataPathFailures);
   }

   return 0;
}

#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
static const DSL_char_t g_sG997fpsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNFEC" DSL_CPE_CRLF
   "- DSL_uint16_t nRFEC" DSL_CPE_CRLF
   "- DSL_uint16_t nLSYMB" DSL_CPE_CRLF
   "- DSL_uint16_t nINTLVDEPTH" DSL_CPE_CRLF
   "- DSL_uint16_t nINTLVBLOCK" DSL_CPE_CRLF
   "- DSL_uint8_t nLPATH" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_FramingParameterStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_FramingParameterStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_FramingParameterStatus_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_FRAMING_PARAMETER_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nNFEC=%hu nRFEC=%hu nLSYMB=%hu "
         "nINTLVDEPTH=%hu nINTLVBLOCK=%hu nLPATH=%hu"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nNFEC, pData.data.nRFEC, pData.data.nLSYMB,
         pData.data.nINTLVDEPTH, pData.data.nINTLVBLOCK, pData.data.nLPATH);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/

#ifdef INCLUDE_DSL_G997_PER_TONE
static const DSL_char_t g_sG997gang[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nGain (hex, linear, represented as multiple of 1/512: 20*log[gain/512])"DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_GainAllocationNscGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0, nSkipped = 0, nUsedNumData=0;
   DSL_G997_GainAllocationNsc_t *pData;
   unsigned char *pResult, *pParse, Num;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_GainAllocationNsc_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_GainAllocationNsc_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData->nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      /* Get the number of actually used tones*/
      for (i = 0; i < pData->data.gainAllocationNsc.nNumData; i++)
      {
         if (pData->data.gainAllocationNsc.nNSCData[i])
         {
            nUsedNumData++;
         }
      }

      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn),pData->nDirection,
         nUsedNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nGain(hex)) nData=\"");

      /* alloc size of value-pairs, number of line-breaks and security */
      pResult = malloc(
         (pData->data.gainAllocationNsc.nNumData)*12 +
         (pData->data.gainAllocationNsc.nNumData)*2/10 + 10);

      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; (i < pData->data.gainAllocationNsc.nNumData) && (i < DSL_MAX_NSC); i++)
         {
            /* skip the printout when 0 */
            if (pData->data.gainAllocationNsc.nNSCData[i] == 0)
            {
               /* do a separator only when starting to skip */
               if (!nSkipped)
               {
                  nSkipped = 1;
                  *pParse++ = '\r';
                  *pParse++ = '\n';
               }
               continue;
            }

            nSkipped = 0;

            /* do own encoding for performance reasons */
            if (i%10==0)
            {
               *pParse++ = '\r';
               *pParse++ = '\n';
            }
            *pParse++ = '(';
            *pParse++ = (unsigned char)('0' + i/1000);
            *pParse++ = (unsigned char)('0' + (i%1000)/100);
            *pParse++ = (unsigned char)('0' + (i%100)/10);
            *pParse++ = (unsigned char)('0' + (i%10));
            *pParse++ = ',';
            Num = ((pData->data.gainAllocationNsc.nNSCData[i] & 0xf000)>>12);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = ((pData->data.gainAllocationNsc.nNSCData[i] & 0x0f00)>>8);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = ((pData->data.gainAllocationNsc.nNSCData[i] & 0x00f0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.gainAllocationNsc.nNSCData[i] & 0x000f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ')';
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s\"" DSL_CPE_CRLF, pResult);
         free(pResult);
      }
      else
      {
         /* not enough memory */
         DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
      }
   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997gansg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_uint16_t nGain (hex, linear, represented as multiple of 1/512: 20*log[gain/512])"DSL_CPE_CRLF
   "- ... nParamNr[1] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_GainAllocationNscShortGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   DSL_G997_GainAllocationNsc_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_GainAllocationNsc_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_GainAllocationNsc_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData->nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_GAIN_ALLOCATION_NSC_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection,
         pData->data.gainAllocationNsc.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=nGain(hex) nData=\"" DSL_CPE_CRLF);

      for (i = 0; (i < pData->data.gainAllocationNsc.nNumData) && (i < DSL_MAX_NSC); i++)
      {
         DSL_CPE_FPrintf(out, "%04X ", (DSL_int_t)pData->data.gainAllocationNsc.nNSCData[i]);
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);
   }
   free(pData);
   return 0;
}
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_G997_STATUS
static const DSL_char_t g_sG997lstg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nLastStateTransmitted" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LastStateTransmittedGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LastStateTransmitted_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LastStateTransmitted_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LAST_STATE_TRANSMITTED_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nLastStateTransmitted=%hd" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nLastStateTransmitted);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sG997lacg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_LDSF_t nLDSF" DSL_CPE_CRLF
   "   inhibit ldsf = 0" DSL_CPE_CRLF
   "   force ldsf = 1" DSL_CPE_CRLF
   "   auto ldsf = 2" DSL_CPE_CRLF
   "   force filter detection = 3" DSL_CPE_CRLF
   "   auto filter detection = 4" DSL_CPE_CRLF
   "- DSL_G997_ACSF_t nACSF" DSL_CPE_CRLF
   "   inhibit acsf = 0" DSL_CPE_CRLF
   "   force acsf = 1" DSL_CPE_CRLF
   "- DSL_G997_StartupMode_t nStartupMode" DSL_CPE_CRLF
   "   normal startup = 0" DSL_CPE_CRLF
   "   force short startup = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineActivateConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineActivate_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineActivate_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_ACTIVATE_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nLDSF=%d nACSF=%d nStartupMode=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nLDSF, pData.data.nACSF,
         pData.data.nStartupMode);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

static const DSL_char_t g_sG997lacs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_LDSF_t nLDSF" DSL_CPE_CRLF
   "   inhibit ldsf = 0" DSL_CPE_CRLF
   "   force ldsf = 1" DSL_CPE_CRLF
   "   auto ldsf = 2" DSL_CPE_CRLF
   "   force filter detection = 3" DSL_CPE_CRLF
   "   auto filter detection = 4" DSL_CPE_CRLF
   "- DSL_G997_ACSF_t nACSF" DSL_CPE_CRLF
   "   inhibit acsf = 0" DSL_CPE_CRLF
   "   force acsf = 1" DSL_CPE_CRLF
   "- DSL_G997_StartupMode_t nStartupMode" DSL_CPE_CRLF
   "   normal startup = 0" DSL_CPE_CRLF
   "   force short startup = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineActivateConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineActivate_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineActivate_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u", &(pData.data.nLDSF),
      &(pData.data.nACSF), &(pData.data.nStartupMode));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_ACTIVATE_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sG997lfsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_G997_BF_LineFailures_t nLineFailures (hex)" DSL_CPE_CRLF
   "   linefailure cleaned = 0x00000000" DSL_CPE_CRLF
   "   linefailure lpr = 0x00000001" DSL_CPE_CRLF
   "   linefailure lof = 0x00000002" DSL_CPE_CRLF
   "   linefailure los = 0x00000004" DSL_CPE_CRLF
   "   linefailure lom = 0x00000008" DSL_CPE_CRLF
   "   linefailure lol = 0x00000010" DSL_CPE_CRLF
   "   linefailure ese = 0x00000020" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineFailureStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineFailures_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineFailures_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_FAILURES_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nLineFailures=0x%x" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.nLineFailures);
   }

   return 0;
}

#ifdef INCLUDE_DSL_G997_STATUS
static const DSL_char_t g_sG997lisg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_LineInit_t nLineInitStatus" DSL_CPE_CRLF
   "   successful = 0" DSL_CPE_CRLF
   "   config error = 1" DSL_CPE_CRLF
   "   config not feasible = 2" DSL_CPE_CRLF
   "   communication problem = 3" DSL_CPE_CRLF
   "   no peer xtu = 4" DSL_CPE_CRLF
   "   unknown = 5" DSL_CPE_CRLF
   "- DSL_G997_LineInitSubStatus_t nLineInitSubStatus" DSL_CPE_CRLF
   "   LINIT_SUB_NONE = 0" DSL_CPE_CRLF
   "   LINIT_SUB_UNKNOWN = 1" DSL_CPE_CRLF
   "   LINIT_SUB_NO_COMMON_MODE = 2" DSL_CPE_CRLF
   "   LINIT_SUB_OPP_MODE = 3" DSL_CPE_CRLF
   "   LINIT_SUB_FW_RETRY = 4" DSL_CPE_CRLF
   "   LINIT_SUB_FW_HYBRID = 5" DSL_CPE_CRLF
   "   LINIT_SUB_PORT_MODE = 6" DSL_CPE_CRLF
   "   LINIT_SUB_S_PP_DRIVER = 7" DSL_CPE_CRLF
   "   LINIT_SUB_S_INTENDED_LOCAL_SHUTDOWN = 8" DSL_CPE_CRLF
   "   LINIT_SUB_TIMEOUT = 9" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineInitStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineInitStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineInitStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_INIT_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nLineInitStatus=%d nLineInitSubStatus=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nLineInitStatus,
         pData.data.nLineInitSubStatus );
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
static const DSL_char_t g_sG997lig[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint8_t G994VendorID[" _MKSTR(DSL_G997_LI_MAXLEN_VENDOR_ID) "]" DSL_CPE_CRLF
   "- DSL_uint8_t SystemVendorID[" _MKSTR(DSL_G997_LI_MAXLEN_VENDOR_ID) "]" DSL_CPE_CRLF
   "- DSL_uint8_t VersionNumber[" _MKSTR(DSL_G997_LI_MAXLEN_VERSION) "]" DSL_CPE_CRLF
   "- DSL_uint8_t SerialNumber[" _MKSTR(DSL_G997_LI_MAXLEN_SERIAL) "]" DSL_CPE_CRLF
   "- DSL_uint32_t SelfTestResult" DSL_CPE_CRLF
   "- DSL_uint8_t XTSECapabilities[" _MKSTR(DSL_G997_NUM_XTSE_OCTETS) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineInventoryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineInventory_t pData;
   DSL_char_t buf[256];

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineInventory_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_INVENTORY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u ",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);

      DSL_CPE_ArraySPrintF(buf, pData.data.G994VendorID,
         sizeof(pData.data.G994VendorID), sizeof(pData.data.G994VendorID[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf(out, "G994VendorID=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.data.SystemVendorID,
         sizeof(pData.data.SystemVendorID), sizeof(pData.data.SystemVendorID[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf(out, "SystemVendorID=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.data.VersionNumber,
         sizeof(pData.data.VersionNumber), sizeof(pData.data.VersionNumber[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf(out, "VersionNumber=%s ", buf);

      DSL_CPE_ArraySPrintF(buf, pData.data.SerialNumber,
         sizeof(pData.data.SerialNumber), sizeof(pData.data.SerialNumber[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf(out, "SerialNumber=%s ", buf);

      DSL_CPE_FPrintf(out, "SelfTestResult=%u ", (DSL_uint32_t)pData.data.SelfTestResult);

      DSL_CPE_ArraySPrintF(buf, pData.data.XTSECapabilities,
         sizeof(pData.data.XTSECapabilities), sizeof(pData.data.XTSECapabilities[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf(out, "XTSECapabilities=%s" DSL_CPE_CRLF, buf);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
static const DSL_char_t g_sG997listrg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_char_t G994VendorID[" _MKSTR(DSL_G997_LI_MAXLEN_VENDOR_ID) "]" DSL_CPE_CRLF
   "- DSL_char_t SystemVendorID[" _MKSTR(DSL_G997_LI_MAXLEN_VENDOR_ID) "]" DSL_CPE_CRLF
   "- DSL_char_t VersionNumber[" _MKSTR(DSL_G997_LI_MAXLEN_VERSION) "]" DSL_CPE_CRLF
   "- DSL_char_t SerialNumber[" _MKSTR(DSL_G997_LI_MAXLEN_SERIAL) "]" DSL_CPE_CRLF
   "- DSL_uint32_t SelfTestResult" DSL_CPE_CRLF
   "- DSL_char_t XTSECapabilities[" _MKSTR(DSL_G997_NUM_XTSE_OCTETS) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineInventoryStringGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineInventory_t pData;
   DSL_char_t buf[256];

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineInventory_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_INVENTORY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u ",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);

      DSL_CPE_ArraySPrintF(buf, pData.data.G994VendorID,
         sizeof(pData.data.G994VendorID), sizeof(pData.data.G994VendorID[0]),
         DSL_ARRAY_FORMAT_STRING);
      DSL_CPE_FPrintf(out, "G994VendorID=%s ", buf);
      buf[0] = '\0';

      DSL_CPE_ArraySPrintF(buf, pData.data.SystemVendorID,
         sizeof(pData.data.SystemVendorID), sizeof(pData.data.SystemVendorID[0]),
         DSL_ARRAY_FORMAT_STRING);
      DSL_CPE_FPrintf(out, "SystemVendorID=%s ", buf);
      buf[0] = '\0';

      DSL_CPE_ArraySPrintF(buf, pData.data.VersionNumber,
         sizeof(pData.data.VersionNumber), sizeof(pData.data.VersionNumber[0]),
         DSL_ARRAY_FORMAT_STRING);
      DSL_CPE_FPrintf(out, "VersionNumber=%s ", buf);
      buf[0] = '\0';

      DSL_CPE_ArraySPrintF(buf, pData.data.SerialNumber,
         sizeof(pData.data.SerialNumber), sizeof(pData.data.SerialNumber[0]),
         DSL_ARRAY_FORMAT_STRING);
      DSL_CPE_FPrintf(out, "SerialNumber=%s ", buf);
      buf[0] = '\0';

      DSL_CPE_FPrintf(out, "SelfTestResult=%u ", (DSL_uint32_t)pData.data.SelfTestResult);

      DSL_CPE_ArraySPrintF(buf, pData.data.XTSECapabilities,
         sizeof(pData.data.XTSECapabilities), sizeof(pData.data.XTSECapabilities[0]),
         DSL_ARRAY_FORMAT_HEX);
      DSL_CPE_FPrintf(out, "XTSECapabilities=%s" DSL_CPE_CRLF, buf);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   #define DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS 4
#else
   #define DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS 3
#endif

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
static const DSL_char_t g_sG997lis[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t SystemVendorID[" _MKSTR(DSL_G997_LI_MAXLEN_VENDOR_ID) "]" DSL_CPE_CRLF
   "- DSL_uint8_t VersionNumber[" _MKSTR(DSL_G997_LI_MAXLEN_VERSION)   "]" DSL_CPE_CRLF
   "- DSL_uint8_t SerialNumber[" _MKSTR(DSL_G997_LI_MAXLEN_SERIAL)    "]" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint8_t Auxiliary[0-" _MKSTR(DSL_G993_LI_MAXLEN_AUX)   "]" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineInventorySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineInventoryNe_t pData;
   DSL_char_t string[256] = { 0 };
   DSL_char_t seps[] = " ";
   DSL_char_t *token;
   DSL_uint16_t i = 0, nIdx = 0;
   DSL_int_t nParams = 0;
   DSL_int_t nParamList[DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS] = { 0 };

   for (i = 0; i < DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS; i++)
   {
      switch (i)
      {
      case 0:
         nParamList[i] = DSL_G997_LI_MAXLEN_VENDOR_ID;
         break;
      case 1:
         nParamList[i] = nParamList[i - 1] + DSL_G997_LI_MAXLEN_VERSION;
         break;
      case 2:
         nParamList[i] = nParamList[i - 1] + DSL_G997_LI_MAXLEN_SERIAL;
         break;
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      case 3:
         nParamList[i] = nParamList[i - 1] + DSL_G993_LI_MAXLEN_AUX;
         break;
#endif
      default:
         DSL_CPE_FPrintf(out, "nReturn=-1 (interanl error)" DSL_CPE_CRLF);
         return 0;
      }
   }

   nParams = nParamList[DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS - 1];

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if (DSL_CPE_CLI_CheckParamNumber(
      pCommands,
      nParamList[DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS - 2], DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, nParams, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }
#else
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, nParams, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }
#endif

   memset (&pData, 0, sizeof(DSL_G997_LineInventoryNe_t));

   strncpy (string, pCommands, sizeof(string)-1);
   string[sizeof(string)-1]=0;

   /* Get first token */
   token = strtok (string, seps);
   if (token != DSL_NULL)
   {
      for (i = 0; i < nParams; i++)
      {
         if (i < nParamList[0])
         {
            nIdx = i;
            DSL_CPE_sscanf (token, "%bx", (unsigned char *)&(pData.data.SystemVendorID[nIdx]));
         }
         else if (i < nParamList[1])
         {
            nIdx = (DSL_uint16_t)(i - nParamList[0]);
            DSL_CPE_sscanf (token, "%bx", (unsigned char *)&(pData.data.VersionNumber[nIdx]));
         }
         else if (i < nParamList[2])
         {
            nIdx = (DSL_uint16_t)(i - nParamList[1]);
            DSL_CPE_sscanf (token, "%bx", (unsigned char *)&(pData.data.SerialNumber[nIdx]));
         }
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
         else if (i < nParamList[3])
         {
            nIdx = (DSL_uint16_t)(i - nParamList[2]);
            DSL_CPE_sscanf (token, "%bx", (unsigned char *)&(pData.data.Auxiliary.pData[nIdx]));
         }
#endif
         else
         {
            DSL_CPE_FPrintf(out, "nReturn=-1 (interanl error)" DSL_CPE_CRLF);
            return 0;
         }

         /* Get next token */
         token = strtok(DSL_NULL, seps);

         /* Exit scanning if no further information is included */
         if (token == DSL_NULL)
         {
            break;
         }
      }
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   pData.data.Auxiliary.nLength =
      (DSL_uint32_t)(i + 1 - nParamList[DSL_CPE_MAX_NE_INVENTORY_PARAM_LISTS - 2]);
#endif

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_INVENTORY_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

static const DSL_char_t g_sG997lsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_int16_t LATN" DSL_CPE_CRLF
   "- DSL_int16_t SATN" DSL_CPE_CRLF
   "- DSL_int16_t SNR" DSL_CPE_CRLF
   "- DSL_uint32_t ATTNDR" DSL_CPE_CRLF
   "- DSL_int16_t ACTPS" DSL_CPE_CRLF
   "- DSL_int16_t ACTATP" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineStatus_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u LATN=%hd SATN=%hd SNR=%hd "
         "ATTNDR=%u ACTPS=%hd ACTATP=%hd" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.nDeltDataType, pData.data.LATN,
         (DSL_int32_t)pData.data.SATN, (DSL_int32_t)pData.data.SNR, (DSL_uint32_t)pData.data.ATTNDR, (DSL_int32_t)pData.data.ACTPS,
         (DSL_int32_t)pData.data.ACTATP);
   }

   return 0;
}

#ifdef INCLUDE_DSL_G997_STATUS
static const DSL_char_t g_sG997ltsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_LineTransmission_t nLineTransmissionStatus" DSL_CPE_CRLF
   "   available = 0" DSL_CPE_CRLF
   "   not available = 1" DSL_CPE_CRLF
   "   not supported = 2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineTransmissionStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineTransmissionStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LineTransmissionStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_TRANSMISSION_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nLineTransmissionStatus=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nLineTransmissionStatus);
   }

   return 0;
}
#endif /* INCLUDE_DSL_G997_STATUS*/

static const DSL_char_t g_sG997pmsft[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_PowerManagementStateForce_t nPowerManagementState" DSL_CPE_CRLF
   "   pmsf l3 to l0 = 0" DSL_CPE_CRLF
   "   pmsf l0 to l2 = 2" DSL_CPE_CRLF
   "   pmsf lx to l3 = 3" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_PowerManagementStateForcedTrigger(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_PowerManagementStateForcedTrigger_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_PowerManagementStateForcedTrigger_t));

   DSL_CPE_sscanf (pCommands, "%u", &(pData.data.nPowerManagementState));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_POWER_MANAGEMENT_STATE_FORCED_TRIGGER,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   }

   return 0;
}

static const DSL_char_t g_sG997pmsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_PowerManagementStatus_t nPowerManagementStatus" DSL_CPE_CRLF
   "   pms na = -1" DSL_CPE_CRLF
   "   pms l0 = 0" DSL_CPE_CRLF
   "   pms l1 = 1" DSL_CPE_CRLF
   "   pms l2 = 2" DSL_CPE_CRLF
   "   pms l3 = 3" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_PowerManagementStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_PowerManagementStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_PowerManagementStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_POWER_MANAGEMENT_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nPowerManagementStatus=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(ret), pData.data.nPowerManagementStatus);
   }

   return 0;
}

#ifdef INCLUDE_DSL_CEOC
static const DSL_char_t g_sG997sms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nMessageData[1-" _MKSTR(DSL_G997_SNMP_MESSAGE_LENGTH) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_SnmpMessageSend(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint32_t nVal = 0;
   DSL_int_t i = 0;
   DSL_char_t *pChar = DSL_NULL, *pTmp = DSL_NULL;
   DSL_G997_Snmp_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, DSL_G997_SNMP_MESSAGE_LENGTH,
      DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0, sizeof(DSL_G997_Snmp_t));

   pTmp = pCommands;
   pChar = pCommands;

   for (i = 0; i < DSL_G997_SNMP_MESSAGE_LENGTH; i++)
   {
      if ( pChar == DSL_NULL )
      {
         /* no more entries, leave the loop */
         break;
      }

      ret = DSL_CPE_sscanf(pChar, "%x", &nVal);

      if ( ret != 1 )
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "invalid SNMP data!" DSL_CPE_CRLF);
         DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERROR));
         return 0;
      }

      pData.data.nMessageData[i] = (DSL_uint8_t)nVal;

      ret = DSL_CPE_MoveCharPtr(pTmp, 1, " ", &pChar);
      if ( (pTmp == pChar) || (ret != DSL_SUCCESS) )
      {
         i++;
         break;
      }
      else
      {
         pTmp = pChar;
      }
   }

   /* Set current length of SNMP message */
   pData.data.nMessageLength = (DSL_uint16_t)i;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_SNMP_MESSAGE_SEND, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CEOC */

#ifdef INCLUDE_DSL_CEOC
static const DSL_char_t g_sG997smr[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nMessageLength" DSL_CPE_CRLF
   "- DSL_uint8_t nMessageData[" _MKSTR(DSL_G997_SNMP_MESSAGE_LENGTH) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_SnmpMessageReceive(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   DSL_G997_Snmp_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0, sizeof(DSL_G997_Snmp_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_SNMP_MESSAGE_RECEIVE, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nMessageLength=%hu nData(hex)=\"" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nMessageLength);

      /* Print only the number of values that are returned */
      for (i = 0; (i < pData.data.nMessageLength) && (i < DSL_G997_SNMP_MESSAGE_LENGTH); i++)
      {
         DSL_CPE_FPrintf(out, "0x%02X ", (DSL_int_t)(pData.data.nMessageData[i]));
      }

      DSL_CPE_FPrintf (out, "\"" DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CEOC */

#ifdef INCLUDE_DSL_G997_PER_TONE
static const DSL_char_t g_sG997sang[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint8_t nSnr ( represented as -32 + SNR(nToneIndex)/2, nSnr(nToneIndex)=255 indicates that "
   "no measurement could be done for the subcarrier)" DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_SnrAllocationNscGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0, nSkipped = 0, nUsedNumData = 0;
   unsigned char *pResult, *pParse, Num;
   DSL_G997_SnrAllocationNsc_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_SnrAllocationNsc_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_SnrAllocationNsc_t));


   DSL_CPE_sscanf (pCommands, "%u", &pData->nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_SNR_ALLOCATION_NSC_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      if (pData->data.snrAllocationNsc.nNumData > DSL_MAX_NSC)
      {
         pData->data.snrAllocationNsc.nNumData = DSL_MAX_NSC;
      }

      /* Get the number of actually used tones*/
      for (i = 0; i < pData->data.snrAllocationNsc.nNumData; i++)
      {
         if (pData->data.snrAllocationNsc.nNSCData[i] != 0xFF)
         {
            nUsedNumData++;
         }
      }

      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection,
         nUsedNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nSnr(hex)) nData=\"");

      /* alloc size of value-pairs, number of line-breaks and security */
      pResult = malloc(
         (pData->data.snrAllocationNsc.nNumData)*10 +
         (pData->data.snrAllocationNsc.nNumData)*2/10 + 10);

      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; i < pData->data.snrAllocationNsc.nNumData; i++)
         {
            /* skip the printout when 0xff */
            if (pData->data.snrAllocationNsc.nNSCData[i] == 0xFF)
            {
               /* do a separator only when starting to skip */
               if (!nSkipped)
               {
                  nSkipped = 1;
                  *pParse++ = '\r';
                  *pParse++ = '\n';
               }
               continue;
            }

            nSkipped = 0;

            /* do own encoding for performance reasons */
            if (i%10==0)
            {
               *pParse++ = '\n';
               *pParse++ = '\r';
            }
            *pParse++ = '(';
            *pParse++ = (unsigned char)('0' + i/1000);
            *pParse++ = (unsigned char)('0' + (i%1000)/100);
            *pParse++ = (unsigned char)('0' + (i%100)/10);
            *pParse++ = (unsigned char)('0' + (i%10));
            *pParse++ = ',';
            Num = ((pData->data.snrAllocationNsc.nNSCData[i] & 0xf0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.snrAllocationNsc.nNSCData[i] & 0x0f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ')';
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s\"" DSL_CPE_CRLF,pResult );
         free(pResult);
      }
      else
      {
         /* not enough memory */
         DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
      }
   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997sansg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_uint8_t nSnr (hex, represented as -32 + SNR(nToneIndex)/2, nSnr(nToneIndex)=255 indicates that "
   "no measurement could be done for the subcarrier)" DSL_CPE_CRLF
   "- ... nParamNr[1] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_SnrAllocationNscShortGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   unsigned char *pResult, *pParse, Num;
   DSL_G997_SnrAllocationNsc_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_SnrAllocationNsc_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_SnrAllocationNsc_t));


   DSL_CPE_sscanf (pCommands, "%u", &pData->nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_SNR_ALLOCATION_NSC_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nNumData=%hu ",
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection,
         pData->data.snrAllocationNsc.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=nSnr(hex) nData=\"" DSL_CPE_CRLF);

      /* use own encoding for performance reasons */
      pResult = malloc(pData->data.snrAllocationNsc.nNumData*3+10);
      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; (i < pData->data.snrAllocationNsc.nNumData) && (i < DSL_MAX_NSC); i++)
         {
            Num = ((pData->data.snrAllocationNsc.nNSCData[i] & 0xf0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.snrAllocationNsc.nNSCData[i] & 0x0f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s" DSL_CPE_CRLF, pResult);
         free(pResult);
      }
   }
   free(pData);
   return 0;
}
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_DELT
static const DSL_char_t g_sg997dfr[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DeltFreeResources(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_DeltFreeResources_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_DeltFreeResources_t));

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_G997_DELT_FREE_RESOURCES, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sG997dhling[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nMeasurementTime" DSL_CPE_CRLF
   "- DSL_uint8_t nGroupSize (currently unused!)" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nHlin_real (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nHlin_imag (hex)" DSL_CPE_CRLF
   "- ... nParamNr[3] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DeltHLINGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   DSL_G997_DeltHlin_t *pData;
   unsigned char *pResult, *pParse, Num;


   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_DeltHlin_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_DeltHlin_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData->nDirection, &pData->nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_DELT_HLIN_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u"
         " nMeasurementTime=%hu nGroupSize=%hu nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection,
         pData->nDeltDataType, pData->data.nMeasurementTime,
         pData->data.nGroupSize, pData->data.deltHlin.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nHlin_real(hex),nHlin_imag(hex)) nData=\"");

      /* alloc size of value-pairs, number of line-breaks and security */
      pResult = malloc(
         (pData->data.deltHlin.nNumData)*24 +
         (pData->data.deltHlin.nNumData)*2/10 + 10);

      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; (i < pData->data.deltHlin.nNumData) && (i < DSL_MAX_NSC); i++)
         {
            /* do own encoding for performance reasons */
            if (i%10==0)
            {
               *pParse++ = '\r';
               *pParse++ = '\n';
            }
            *pParse++ = '(';
            *pParse++ = (unsigned char)('0' + i/1000);
            *pParse++ = (unsigned char)('0' + (i%1000)/100);
            *pParse++ = (unsigned char)('0' + (i%100)/10);
            *pParse++ = (unsigned char)('0' + (i%10));
            *pParse++ = ',';
            Num = ((pData->data.deltHlin.nNSCComplexData[i].nReal & 0xf000)>>12);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = ((pData->data.deltHlin.nNSCComplexData[i].nReal & 0x0f00)>>8);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = ((pData->data.deltHlin.nNSCComplexData[i].nReal & 0x00f0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.deltHlin.nNSCComplexData[i].nReal & 0x000f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ',';
            Num = ((pData->data.deltHlin.nNSCComplexData[i].nImag & 0xf000)>>12);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = ((pData->data.deltHlin.nNSCComplexData[i].nImag & 0x0f00)>>8);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = ((pData->data.deltHlin.nNSCComplexData[i].nImag & 0x00f0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.deltHlin.nNSCComplexData[i].nImag & 0x000f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ')';
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s\"" DSL_CPE_CRLF, pResult);
         free(pResult);
      }
      else
      {
         /* not enough memory */
         DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
      }
   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997dhlinsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nDeltHlinScale" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DeltHLINScaleGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_DeltHlinScale_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_DeltHlinScale_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_DeltHlinScale_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData->nDirection, &pData->nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_DELT_HLIN_SCALE_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf( out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u "
         "nDeltHlinScale=%hu"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection, pData->nDeltDataType,
         pData->data.nDeltHlinScale );
   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997dhlogg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nMeasurementTime" DSL_CPE_CRLF
   "- DSL_uint8_t nGroupSize (currently unused!)" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nHlog" DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DeltHLOGGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
#if 0
   DSL_char_t *pResult, *pParse, Num;
#endif
   DSL_G997_DeltHlog_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_DeltHlog_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_DeltHlog_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData->nDirection, &pData->nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_DELT_HLOG_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u "
         "nMeasurementTime=%hu nGroupSize=%hu nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn),
         pData->nDirection, pData->nDeltDataType,
         pData->data.nMeasurementTime, pData->data.nGroupSize,
         pData->data.deltHlog.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nHlog(dec)) nData=\"");

      for (i = 0; i < pData->data.deltHlog.nNumData; i++)
      {
         if (i >= DSL_MAX_NSC)
         {
            break;
         }

         if (i%10 == 0) DSL_CPE_FPrintf(out, "" DSL_CPE_CRLF );
         DSL_CPE_FPrintf(out, "(%04d,%04d) ", i, pData->data.deltHlog.nNSCData[i] );
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);

#if 0
      DSL_CPE_FPrintf(out, "nFormat=(tone(dec),hlog(hex)) nData=\"");

      /* alloc size of value-pairs, number of line-breaks and security */
      pResult = malloc(
         (pData->data.deltHlog.nNumData)*10 +
         (pData->data.deltHlog.nNumData)*2/10 + 10);

      if (pResult != DSL_NULL)
      {
         pParse = pResult;
         for (i = 0; (i < pData->data.deltHlog.nNumData) && (i < DSL_MAX_NSC); i++)
         {
            /* do own encoding for performance reasons */
            if (i%10==0)
            {
               *pParse++ = '\n';
               *pParse++ = '\r';
            }
            *pParse++ = '(';
            *pParse++ = '0' + i/1000;
            *pParse++ = '0' + (i%1000)/100;
            *pParse++ = '0' + (i%100)/10;
            *pParse++ = '0' + (i%10);
            *pParse++ = ',';
            Num = ((pData->data.deltHlog.nNSCData[i] & 0xf0)>>4);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            Num = (pData->data.deltHlog.nNSCData[i] & 0x0f);
            *pParse++ = (Num > 9) ? (('A'-10)+Num) : ('0'+Num) ;
            *pParse++ = ')';
            *pParse++ = ' ';
         }
         *pParse = '\0';
         DSL_CPE_FPrintf(out, "%s\"" DSL_CPE_CRLF,pResult );
         free(pResult);
      }
      else
      {
         /* not enough memory */
         DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
      }
#endif
   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997dqlng[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nMeasurementTime" DSL_CPE_CRLF
   "- DSL_uint8_t nGroupSize (currently unused!)" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nQln" DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DeltQLNGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   DSL_G997_DeltQln_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_DeltQln_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_DeltQln_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData->nDirection, &pData->nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_DELT_QLN_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u "
         "nMeasurementTime=%hu nGroupSize=%hu nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn), pData->nDirection, pData->nDeltDataType,
         pData->data.nMeasurementTime, pData->data.nGroupSize,
         pData->data.deltQln.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nQln(dec)) nData=\"");
      for (i = 0; (i < pData->data.deltQln.nNumData) && (i < DSL_MAX_NSC); i++)
      {
         if (i%10 == 0) DSL_CPE_FPrintf(out, "" DSL_CPE_CRLF );
         DSL_CPE_FPrintf(out, "(%04d,%03d) ", i, pData->data.deltQln.nNSCData[i] );
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);

   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sG997dsnrg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nMeasurementTime" DSL_CPE_CRLF
   "- DSL_uint8_t nGroupSize (currently unused!)" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nSnr" DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_DeltSNRGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   DSL_G997_DeltSnr_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_DeltSnr_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_DeltSnr_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData->nDirection, &pData->nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_DELT_SNR_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u "
         "nMeasurementTime=%hu nGroupSize=%hu nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn),
         pData->nDirection, pData->nDeltDataType,
         pData->data.nMeasurementTime, pData->data.nGroupSize,
         pData->data.deltSnr.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nSnr(dec)) nData=\"");
      for (i = 0; (i < pData->data.deltSnr.nNumData) && (i < DSL_MAX_NSC); i++)
      {
         if (i%10 == 0) DSL_CPE_FPrintf(out, "" DSL_CPE_CRLF );
         DSL_CPE_FPrintf(out, "(%04d,%03d) ", i, pData->data.deltSnr.nNSCData[i] );
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);

   }
   free(pData);
   return 0;
}
#endif /* INCLUDE_DSL_DELT*/

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sG997xtusecg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t XTSE1 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE2 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE3 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE4 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE5 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE6 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE7 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE8 (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_XTUSystemEnablingConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t i = 0;
   DSL_int_t ret = 0;
   DSL_G997_XTUSystemEnabling_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_XTUSystemEnabling_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));

      for (i = 0; i < DSL_G997_NUM_XTSE_OCTETS; i++)
      {
         DSL_CPE_FPrintf(out, "XTSE%d=0x%x ", i + 1, (DSL_int_t)(pData.data.XTSE[i]) );
      }
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF );
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

static const DSL_char_t g_sG997xtusecs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE1 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE2 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE3 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE4 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE5 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE6 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE7 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE8 (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_XTUSystemEnablingConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_int_t i = 0;
   DSL_G997_XTUSystemEnabling_t pData;
   DSL_uint32_t nParam[DSL_G997_NUM_XTSE_OCTETS] = { 0 };

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 8, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_XTUSystemEnabling_t));

   DSL_CPE_sscanf ( pCommands, "%x %x %x %x %x %x %x %x",
      &(nParam[0]), &(nParam[1]), &(nParam[2]), &(nParam[3]), &(nParam[4]),
      &(nParam[5]), &(nParam[6]), &(nParam[7]));

   for (i = 0; i < DSL_G997_NUM_XTSE_OCTETS; i++)
   {
      pData.data.XTSE[i] = (DSL_uint8_t) nParam[i];
   }

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_XTU_SYSTEM_ENABLING_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sG997xtusesg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t XTSE1 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE2 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE3 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE4 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE5 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE6 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE7 (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t XTSE8 (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_XTUSystemEnablingStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_XTUSystemEnabling_t pData;
   DSL_int_t i = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_XTUSystemEnabling_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));

      for (i = 0; i < DSL_G997_NUM_XTSE_OCTETS; i++)
      {
         DSL_CPE_FPrintf(out, "XTSE%d=0x%x ", i + 1, (DSL_int_t)(pData.data.XTSE[i]) );
      }
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF );
   }

   return 0;
}

static const DSL_char_t g_sLocs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_LineOptionsSelector_t nConfigSelector (dec)" DSL_CPE_CRLF
   "   DSL_OPT_NOISE_MARGIN_DELTA_DS = 0" DSL_CPE_CRLF
   "   DSL_ERASURE_DECODING_TYPE_DS = 1" DSL_CPE_CRLF
   "   DSL_TRUST_ME_BIT = 2" DSL_CPE_CRLF
   "   DSL_INBAND_SPECTRAL_SHAPING_US = 3" DSL_CPE_CRLF
   "   DSL_ENHANCED_FRAMING_US = 4" DSL_CPE_CRLF
   "- DSL_int32_t nConfigValue (dec)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LineOptionsConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LineOptionsConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LineOptionsConfig_t));

   DSL_CPE_sscanf (pCommands, "%u %d",
      &pData.data.nConfigSelector, &pData.data.nConfigValue);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LINE_OPTIONS_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sLocg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_LineOptionsSelector_t nConfigSelector (dec)" DSL_CPE_CRLF
   "   DSL_OPT_NOISE_MARGIN_DELTA_DS = 0" DSL_CPE_CRLF
   "   DSL_ERASURE_DECODING_TYPE_DS = 1" DSL_CPE_CRLF
   "   DSL_TRUST_ME_BIT = 2" DSL_CPE_CRLF
   "   DSL_INBAND_SPECTRAL_SHAPING_US = 3" DSL_CPE_CRLF
   "   DSL_ENHANCED_FRAMING_US = 4" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_int32_t nConfigValue (dec)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LineOptionsConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LineOptionsConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LineOptionsConfig_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.data.nConfigSelector);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LINE_OPTIONS_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nConfigValue=%d" DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nConfigValue);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CONFIG
static const DSL_char_t g_sPMcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bNePollingOff" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bFePollingOff" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_uint8_t nBasicUpdateCycle" DSL_CPE_CRLF
   "- DSL_uint8_t nFeUpdateCycleFactor" DSL_CPE_CRLF
   "- DSL_uint8_t nFeUpdateCycleFactorL2" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_Config_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 5, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_Config_t));

   DSL_CPE_sscanf (pCommands, "%u %u %bu %bu %bu", &pData.data.bNePollingOff,
      &pData.data.bFePollingOff, &pData.data.nBasicUpdateCycle,
      &pData.data.nFeUpdateCycleFactor, &pData.data.nFeUpdateCycleFactorL2);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_CONFIG*/
#endif /* INCLUDE_DSL_PM*/

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CONFIG
#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sPMcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bNePollingOff" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_boolean_t bFePollingOff" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_uint8_t nBasicUpdateCycle" DSL_CPE_CRLF
   "- DSL_uint8_t nFeUpdateCycleFactor" DSL_CPE_CRLF
   "- DSL_uint8_t nFeUpdateCycleFactorL2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_Config_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_Config_t));

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"bNePollingOff=%u bFePollingOff=%u "
      "nBasicUpdateCycle=%u nFeUpdateCycleFactor=%u nFeUpdateCycleFactorL2=%u" DSL_CPE_CRLF,
      DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.bNePollingOff, pData.data.bFePollingOff,
      pData.data.nBasicUpdateCycle, pData.data.nFeUpdateCycleFactor, pData.data.nFeUpdateCycleFactorL2);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_CPE_PM_CONFIG*/
#endif /* INCLUDE_DSL_PM*/

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPM15meet[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bOneDayElapsed" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_15MinElapsedExtTrigger(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ElapsedExtTrigger_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ElapsedExtTrigger_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.data.bOneDayElapsed);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_15MIN_ELAPSED_EXT_TRIGGER, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMbms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bActivate" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPmTick" DSL_CPE_CRLF
   "- DSL_uint32_t nPm15Min" DSL_CPE_CRLF
   "- DSL_uint32_t nPm15MinPerDay" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_BurninModeSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_BurninMode_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_BurninMode_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u", &pData.data.bActivate,
      &pData.data.nMode.nPmTick, &pData.data.nMode.nPm15Min,
      &pData.data.nMode.nPm15MinPerDay);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_BURNIN_MODE_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMetr[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ElapsedTimeReset(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ElapsedTimeReset_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ElapsedTimeReset_t));

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_ELAPSED_TIME_RESET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
static const DSL_char_t g_sPMr[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_PM_ResetTypes_t nResetType" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1" DSL_CPE_CRLF
   "   total = 2" DSL_CPE_CRLF
   "   all = 3" DSL_CPE_CRLF
   "   current showtime = 4" DSL_CPE_CRLF
   "   history showtime = 5" DSL_CPE_CRLF
   "- DSL_boolean_t bUseDetailedReset (optional, used only in conjunction with the next parameter)" DSL_CPE_CRLF
   "   do not use detailed reset = 0" DSL_CPE_CRLF
   "   use detailed reset = 1" DSL_CPE_CRLF
   "- DSL_PM_BF_ResetMask_t nResetMask (hex, optional, used only if detailed reset was specified)" DSL_CPE_CRLF
   "   cleaned = 0x00000000" DSL_CPE_CRLF
   "   line init counters = 0x00000001" DSL_CPE_CRLF
   "   line sec counters = 0x00000002" DSL_CPE_CRLF
   "   line failure counters = 0x00000004" DSL_CPE_CRLF
   "   line event showtime counters = 0x00000008" DSL_CPE_CRLF
   "   channel counters = 0x00000100" DSL_CPE_CRLF
   "   data path counters = 0x00001000" DSL_CPE_CRLF
   "   data path failure counters = 0x00002000" DSL_CPE_CRLF
   "   retx counters = 0x00010000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_Reset(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_Reset_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_PM_Reset_t));

   DSL_CPE_sscanf (pCommands, "%u %u %x",
   &pData.data.nResetType, &pData.data.bUseDetailedReset, &pData.data.nResetMask);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RESET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_PM */



#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMsms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_PM_SyncModeType_t nMode" DSL_CPE_CRLF
   "   free = 0" DSL_CPE_CRLF
   "   sys time = 1" DSL_CPE_CRLF
   "   external = 2" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_SyncModeSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_SyncMode_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_SyncMode_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.data.nMode);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_SYNC_MODE_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sPMsmg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_PM_SyncModeType_t nMode" DSL_CPE_CRLF
   "   free = 0" DSL_CPE_CRLF
   "   sys time = 1" DSL_CPE_CRLF
   "   external = 2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_SyncModeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_SyncMode_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_SyncMode_t));

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_SYNC_MODE_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nMode=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nMode);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CONFIG_GET*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMcc15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_CHANNEL_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_CHANNEL_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nCodeViolations=%u nFEC=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.nHistoryInterval, pData.interval.nElapsedTime,
         pData.interval.bValid, pData.data.nCodeViolations,
         pData.data.nFEC);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMcc1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_CHANNEL_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_CHANNEL_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nCodeViolations=%u nFEC=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.nHistoryInterval, pData.interval.nElapsedTime,
         pData.interval.bValid, pData.data.nCodeViolations,
         pData.data.nFEC);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMchs15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_HISTORY_STATS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMchs1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_HISTORY_STATS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMcctg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelCountersTotal_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nElapsedTime=%u bValid=%u nCodeViolations=%u "
         "nFEC=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.total.nElapsedTime, pData.total.bValid, pData.data.nCodeViolations,
         pData.data.nFEC);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMccsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nCodeViolations=%u nFEC=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.nHistoryInterval, pData.interval.nElapsedTime,
         pData.interval.bValid, pData.data.nCodeViolations,
         pData.data.nFEC);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMct15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelThresholds15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_THRESHOLDS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nCodeViolations=%u nFEC=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.data.nCodeViolations, pData.data.nFEC);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMct15ms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelThresholds15MinSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u", &pData.nChannel, &pData.nDirection,
      &pData.data.nCodeViolations, &pData.data.nFEC);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_THRESHOLDS_15MIN_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMct1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelThresholds1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_THRESHOLDS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nCodeViolations=%u nFEC=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nCodeViolations, pData.data.nFEC);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMct1ds[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nCodeViolations" DSL_CPE_CRLF
   "- DSL_uint32_t nFEC" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ChannelThresholds1DaySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ChannelThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ChannelThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u", &pData.nChannel, &pData.nDirection,
      &pData.data.nCodeViolations, &pData.data.nFEC);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_CHANNEL_THRESHOLDS_1DAY_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMdpc15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nHEC=%u nTotalCells=%u nUserTotalCells=%u "
         "nIBE=%u nTxUserTotalCells=%u nTxIBE=%u",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nHEC, (DSL_uint32_t)pData.data.nTotalCells,
         (DSL_uint32_t)pData.data.nUserTotalCells, (DSL_uint32_t)pData.data.nIBE,
         (DSL_uint32_t)pData.data.nTxUserTotalCells, (DSL_uint32_t)pData.data.nTxIBE);
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      DSL_CPE_FPrintf (out,
         " nCRC_P=%u nCRCP_P=%u nCV_P=%u nCVP_P=%u",
         (DSL_uint32_t)pData.data.nCRC_P, (DSL_uint32_t)pData.data.nCRCP_P, (DSL_uint32_t)pData.data.nCV_P,
         (DSL_uint32_t)pData.data.nCVP_P);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMdpc1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nHEC=%u nTotalCells=%u nUserTotalCells=%u "
         "nIBE=%u nTxUserTotalCells=%u nTxIBE=%u",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nHEC, (DSL_uint32_t)pData.data.nTotalCells,
         (DSL_uint32_t)pData.data.nUserTotalCells, (DSL_uint32_t)pData.data.nIBE,
         (DSL_uint32_t)pData.data.nTxUserTotalCells, (DSL_uint32_t)pData.data.nTxIBE);
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      DSL_CPE_FPrintf (out,
         " nCRC_P=%u nCRCP_P=%u nCV_P=%u nCVP_P=%u",
         (DSL_uint32_t)pData.data.nCRC_P, (DSL_uint32_t)pData.data.nCRCP_P, (DSL_uint32_t)pData.data.nCV_P,
         (DSL_uint32_t)pData.data.nCVP_P);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdphs15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_HISTORY_STATS_15MIN_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdphs1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_HISTORY_STATS_1DAY_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMdpctg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathCountersTotal_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nElapsedTime=%u bValid=%u nHEC=%u "
         "nTotalCells=%u nUserTotalCells=%u nIBE=%u nTxUserTotalCells=%u nTxIBE=%u",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.total.nElapsedTime, pData.total.bValid, pData.data.nHEC, pData.data.nTotalCells,
         pData.data.nUserTotalCells, pData.data.nIBE,
         pData.data.nTxUserTotalCells, pData.data.nTxIBE);
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      DSL_CPE_FPrintf (out,
         " nCRC_P=%u nCRCP_P=%u nCV_P=%u nCVP_P=%u",
         pData.data.nCRC_P, pData.data.nCRCP_P, pData.data.nCV_P,
         pData.data.nCVP_P);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMdpcsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nHEC=%u nTotalCells=%u nUserTotalCells=%u "
         "nIBE=%u nTxUserTotalCells=%u nTxIBE=%u",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nHEC, (DSL_uint32_t)pData.data.nTotalCells,
         (DSL_uint32_t)pData.data.nUserTotalCells, (DSL_uint32_t)pData.data.nIBE,
         (DSL_uint32_t)pData.data.nTxUserTotalCells, (DSL_uint32_t)pData.data.nTxIBE);
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      DSL_CPE_FPrintf (out,
         " nCRC_P=%u nCRCP_P=%u nCV_P=%u nCVP_P=%u",
         (DSL_uint32_t)pData.data.nCRC_P, (DSL_uint32_t)pData.data.nCRCP_P, (DSL_uint32_t)pData.data.nCV_P,
         (DSL_uint32_t)pData.data.nCVP_P);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdpt15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathThresholds15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_THRESHOLDS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHEC=%u nTotalCells=%u "
         "nUserTotalCells=%u nIBE=%u nTxUserTotalCells=%u nTxIBE=%u",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.data.nHEC, pData.data.nTotalCells, pData.data.nUserTotalCells,
         pData.data.nIBE, pData.data.nTxUserTotalCells, pData.data.nTxIBE);
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      DSL_CPE_FPrintf (out,
         " nCRC_P=%u nCRCP_P=%u nCV_P=%u nCVP_P=%u",
         pData.data.nCRC_P, pData.data.nCRCP_P, pData.data.nCV_P,
         pData.data.nCVP_P);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdpt15ms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathThresholds15MinSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathThreshold_t pData;

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 12, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u %u %u %u %u %u %u %u %u", &pData.nChannel,
      &pData.nDirection, &pData.data.nHEC, &pData.data.nTotalCells,
      &pData.data.nUserTotalCells, &pData.data.nIBE,
      &pData.data.nTxUserTotalCells, &pData.data.nTxIBE, &pData.data.nCRC_P,
      &pData.data.nCRCP_P, &pData.data.nCV_P, &pData.data.nCVP_P);
#else
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 8, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u %u %u %u %u", &pData.nChannel,
      &pData.nDirection, &pData.data.nHEC, &pData.data.nTotalCells,
      &pData.data.nUserTotalCells, &pData.data.nIBE, &pData.data.nTxUserTotalCells,
      &pData.data.nTxIBE);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_THRESHOLDS_15MIN_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdpt1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathThresholds1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_THRESHOLDS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHEC=%u nTotalCells=%u "
         "nUserTotalCells=%u nIBE=%u nTxUserTotalCells=%u nTxIBE=%u",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.data.nHEC, pData.data.nTotalCells, pData.data.nUserTotalCells,
         pData.data.nIBE, pData.data.nTxUserTotalCells, pData.data.nTxIBE);
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
      DSL_CPE_FPrintf (out,
         " nCRC_P=%u nCRCP_P=%u nCV_P=%u nCVP_P=%u",
         pData.data.nCRC_P, pData.data.nCRCP_P, pData.data.nCV_P,
         pData.data.nCVP_P);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdpt1ds[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHEC" DSL_CPE_CRLF
   "- DSL_uint32_t nTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nIBE" DSL_CPE_CRLF
   "- DSL_uint32_t nTxUserTotalCells" DSL_CPE_CRLF
   "- DSL_uint32_t nTxIBE" DSL_CPE_CRLF
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   "- DSL_uint32_t nCRC_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCRCP_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCV_P" DSL_CPE_CRLF
   "- DSL_uint32_t nCVP_P" DSL_CPE_CRLF
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathThresholds1DaySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathThreshold_t pData;

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 12, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u %u %u %u %u %u %u %u %u", &pData.nChannel,
      &pData.nDirection, &pData.data.nHEC, &pData.data.nTotalCells,
      &pData.data.nUserTotalCells, &pData.data.nIBE,
      &pData.data.nTxUserTotalCells, &pData.data.nTxIBE,
      &pData.data.nCRC_P, &pData.data.nCRCP_P, &pData.data.nCV_P, &pData.data.nCVP_P);
#else
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 8, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathThreshold_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u %u %u %u %u %u", &pData.nChannel,
      &pData.nDirection, &pData.data.nHEC, &pData.data.nTotalCells,
      &pData.data.nUserTotalCells, &pData.data.nIBE,
      &pData.data.nTxUserTotalCells, &pData.data.nTxIBE);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_THRESHOLDS_1DAY_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMrtc15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_RETX_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_RETX_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u "
         "nEftrMin=%u nErrorFreeBits=%u nLeftr=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nEftrMin, (DSL_uint32_t)pData.data.nErrorFreeBits,
         (DSL_uint32_t)pData.data.nLeftr);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMrtc1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_RETX_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_RETX_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u "
         "nEftrMin=%u nErrorFreeBits=%u nLeftr=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nEftrMin, (DSL_uint32_t)pData.data.nErrorFreeBits,
         (DSL_uint32_t)pData.data.nLeftr);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMrths15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_HISTORY_STATS_15MIN_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMrths1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_HISTORY_STATS_1DAY_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMrtctg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxCountersTotal_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nElapsedTime=%u bValid=%u "
         "nEftrMin=%u nErrorFreeBits=%u nLeftr=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.total.nElapsedTime, (DSL_uint32_t)pData.total.bValid,
         (DSL_uint32_t)pData.data.nEftrMin, (DSL_uint32_t)pData.data.nErrorFreeBits,
         (DSL_uint32_t)pData.data.nLeftr);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMrtcsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_RETX_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_RETX_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u "
         "nEftrMin=%u nErrorFreeBits=%u nLeftr=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nEftrMin, (DSL_uint32_t)pData.data.nErrorFreeBits,
         (DSL_uint32_t)pData.data.nLeftr);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMrtt15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxThresholds15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_THRESHOLDS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u "
         "nEftrMin=%u nErrorFreeBits=%u nLeftr=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nEftrMin, pData.data.nErrorFreeBits,
         pData.data.nLeftr);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMrtt15ms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxThresholds15MinSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u",
      &pData.nDirection,
      &pData.data.nEftrMin, &pData.data.nErrorFreeBits, &pData.data.nLeftr);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_THRESHOLDS_15MIN_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMrtt1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxThresholds1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_THRESHOLDS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u "
         "nEftrMin=%u nErrorFreeBits=%u nLeftr=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nEftrMin, pData.data.nErrorFreeBits, pData.data.nLeftr);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMrtt1ds[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nEftrMin" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorFreeBits" DSL_CPE_CRLF
   "- DSL_uint32_t nLeftr" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_ReTxThresholds1DaySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_ReTxThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_ReTxThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u",
      &pData.nDirection,
      &pData.data.nEftrMin, &pData.data.nErrorFreeBits, &pData.data.nLeftr);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_RETX_THRESHOLDS_1DAY_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMdpfc15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nNCD" DSL_CPE_CRLF
   "- DSL_uint32_t nLCD" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathFailureCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathFailureCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathFailureCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nNCD=%u nLCD=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nNCD, (DSL_uint32_t)pData.data.nLCD);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMdpfc1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nNCD" DSL_CPE_CRLF
   "- DSL_uint32_t nLCD" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathFailureCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathFailureCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathFailureCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nNCD=%u nLCD=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nNCD,
         (DSL_uint32_t)pData.data.nLCD);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdpfhs15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathFailureHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_FAILURE_HISTORY_STATS_15MIN_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMdpfhs1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathFailureHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_FAILURE_HISTORY_STATS_1DAY_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nChannel, pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMdpfctg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nNCD" DSL_CPE_CRLF
   "- DSL_uint32_t nLCD" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathFailureCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathFailureCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathFailureCountersTotal_t));

   DSL_CPE_sscanf (pCommands, "%bu %u", &pData.nChannel, &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nElapsedTime=%u bValid=%u nNCD=%u nLCD=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         pData.total.nElapsedTime, pData.total.bValid, pData.data.nNCD, pData.data.nLCD);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMdpfcsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nChannel" DSL_CPE_CRLF
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nNCD" DSL_CPE_CRLF
   "- DSL_uint32_t nLCD" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_DataPathFailureCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_DataPathFailureCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_DataPathFailureCounters_t));

   DSL_CPE_sscanf (pCommands, "%bu %u %u", &pData.nChannel, &pData.nDirection,
      &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nChannel=%hu nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nNCD=%u nLCD=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nChannel, pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nNCD,
         (DSL_uint32_t)pData.data.nLCD);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMlic15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitCounters_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nHistoryInterval=%u nElapsedTime=%u bValid=%u nFullInits=%u "
         "nFailedFullInits=%u nShortInits=%u nFailedShortInits=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime, (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nFullInits, (DSL_uint32_t)pData.data.nFailedFullInits,
         (DSL_uint32_t)pData.data.nShortInits, (DSL_uint32_t)pData.data.nFailedShortInits);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMlic1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitCounters_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nHistoryInterval=%u nElapsedTime=%u bValid=%u nFullInits=%u "
         "nFailedFullInits=%u nShortInits=%u nFailedShortInits=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime, (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nFullInits, (DSL_uint32_t)pData.data.nFailedFullInits,
         (DSL_uint32_t)pData.data.nShortInits, (DSL_uint32_t)pData.data.nFailedShortInits);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlihs15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStats_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStats_t));

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_HISTORY_STATS_15MIN_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlihs1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStats_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStats_t));

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_HISTORY_STATS_1DAY_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMlictg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitCountersTotal_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nElapsedTime=%u bValid=%u nFullInits=%u nFailedFullInits=%u "
         "nShortInits=%u nFailedShortInits=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.total.nElapsedTime, pData.total.bValid,
         pData.data.nFullInits, pData.data.nFailedFullInits,
         pData.data.nShortInits, pData.data.nFailedShortInits);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMlicsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_LINE_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_LINE_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitCounters_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nHistoryInterval=%u nElapsedTime=%u bValid=%u nFullInits=%u "
         "nFailedFullInits=%u nShortInits=%u nFailedShortInits=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime, (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nFullInits, (DSL_uint32_t)pData.data.nFailedFullInits,
         (DSL_uint32_t)pData.data.nShortInits, (DSL_uint32_t)pData.data.nFailedShortInits);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlit15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitThresholds15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitThreshold_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_THRESHOLDS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nFullInits=%u nFailedFullInits=%u nShortInits=%u "
         "nFailedShortInits=%u" DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nFullInits, pData.data.nFailedFullInits,
         pData.data.nShortInits, pData.data.nFailedShortInits);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlit15ms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitThresholds15MinSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u", &pData.data.nFullInits,
      &pData.data.nFailedFullInits, &pData.data.nShortInits,
      &pData.data.nFailedShortInits);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_THRESHOLDS_15MIN_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlit1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitThresholds1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitThreshold_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_THRESHOLDS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nFullInits=%u nFailedFullInits=%u nShortInits=%u "
         "nFailedShortInits=%u" DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nFullInits, pData.data.nFailedFullInits,
         pData.data.nShortInits, pData.data.nFailedShortInits);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlit1ds[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedFullInits" DSL_CPE_CRLF
   "- DSL_uint32_t nShortInits" DSL_CPE_CRLF
   "- DSL_uint32_t nFailedShortInits" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineInitThresholds1DaySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineInitThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineInitThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u", &pData.data.nFullInits,
      &pData.data.nFailedFullInits, &pData.data.nShortInits,
      &pData.data.nFailedShortInits);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_INIT_THRESHOLDS_1DAY_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMlsc15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u nElapsedTime=%u bValid=%u "
         "nES=%u nSES=%u nLOSS=%u nUAS=%u nLOFS=%u nFECS=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, (DSL_uint32_t)pData.nHistoryInterval,
         (DSL_uint32_t)pData.interval.nElapsedTime, (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nES, (DSL_uint32_t)pData.data.nSES,
         (DSL_uint32_t)pData.data.nLOSS, (DSL_uint32_t)pData.data.nUAS,
         (DSL_uint32_t)pData.data.nLOFS, (DSL_uint32_t)pData.data.nFECS);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMlsc1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u nElapsedTime=%u bValid=%u "
         "nES=%u nSES=%u nLOSS=%u nUAS=%u nLOFS=%u nFECS=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, (DSL_uint32_t)pData.nHistoryInterval,
         (DSL_uint32_t)pData.interval.nElapsedTime, (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nES, (DSL_uint32_t)pData.data.nSES,
         (DSL_uint32_t)pData.data.nLOSS, (DSL_uint32_t)pData.data.nUAS,
         (DSL_uint32_t)pData.data.nLOFS, (DSL_uint32_t)pData.data.nFECS);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlshs15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsDir_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_HISTORY_STATS_15MIN_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection,pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlshs1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsDir_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_HISTORY_STATS_1DAY_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection,pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMlsctg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
    DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecCountersTotal_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nElapsedTime=%u bValid=%u nES=%u nSES=%u nLOSS=%u "
         "nUAS=%u nLOFS=%u nFECS=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.total.nElapsedTime, pData.total.bValid,
         pData.data.nES, pData.data.nSES, pData.data.nLOSS, pData.data.nUAS,
         pData.data.nLOFS, pData.data.nFECS);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMlscsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_LINE_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_LINE_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u nElapsedTime=%u bValid=%u "
         "nES=%u nSES=%u nLOSS=%u nUAS=%u nLOFS=%u nFECS=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, (DSL_uint32_t)pData.nHistoryInterval,
         (DSL_uint32_t)pData.interval.nElapsedTime, (DSL_uint32_t)pData.interval.bValid,
         (DSL_uint32_t)pData.data.nES, (DSL_uint32_t)pData.data.nSES,
         (DSL_uint32_t)pData.data.nLOSS, (DSL_uint32_t)pData.data.nUAS,
         (DSL_uint32_t)pData.data.nLOFS, (DSL_uint32_t)pData.data.nFECS);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlst15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecThresholds15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_THRESHOLDS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u nES=%u nSES=%u nLOSS=%u"
         "nUAS=%u nLOFS=%u nFECS=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection, pData.data.nES,
         pData.data.nSES, pData.data.nLOSS, pData.data.nUAS, pData.data.nLOFS,
         pData.data.nFECS);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlst15ms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecThresholds15MinSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 6, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u %u %u %u", &pData.nDirection, &pData.data.nES,
      &pData.data.nSES, &pData.data.nLOSS, &pData.data.nUAS, &pData.data.nLOFS,
      &pData.data.nFECS);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_THRESHOLDS_15MIN_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlst1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecThresholds1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_THRESHOLDS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nDirection=%u nES=%u nSES=%u nLOSS=%u"
         "nUAS=%u nLOFS=%u nFECS=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection, pData.data.nES, pData.data.nSES, pData.data.nLOSS,
         pData.data.nUAS, pData.data.nLOFS, pData.data.nFECS);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMlst1ds[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nES" DSL_CPE_CRLF
   "- DSL_uint32_t nSES" DSL_CPE_CRLF
   "- DSL_uint32_t nLOSS" DSL_CPE_CRLF
   "- DSL_uint32_t nUAS" DSL_CPE_CRLF
   "- DSL_uint32_t nLOFS" DSL_CPE_CRLF
   "- DSL_uint32_t nFECS" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineSecThresholds1DaySet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineSecThreshold_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 6, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineSecThreshold_t));

   DSL_CPE_sscanf (pCommands, "%u %u %u %u %u %u %u", &pData.nDirection, &pData.data.nES,
      &pData.data.nSES, &pData.data.nLOSS, &pData.data.nUAS, &pData.data.nLOFS,
      &pData.data.nFECS);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_SEC_THRESHOLDS_1DAY_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMlesc15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nLOF" DSL_CPE_CRLF
   "- DSL_uint32_t nLOS" DSL_CPE_CRLF
   "- DSL_uint32_t nLPR" DSL_CPE_CRLF
   "- DSL_uint32_t nLOM" DSL_CPE_CRLF
   "- DSL_uint32_t nSosSuccess" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineEventShowtimeCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineEventShowtimeCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineEventShowtimeCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_15MIN_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nLOF=%u nLOS=%u nLPR=%u nLOM=%u nSosSuccess=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nLOF, (DSL_uint32_t)pData.data.nLOS,
         (DSL_uint32_t)pData.data.nLPR, (DSL_uint32_t)pData.data.nLOM,
         (DSL_uint32_t)pData.data.nSosSuccess);
   }

   return 0;
}

#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
static const DSL_char_t g_sPMlesc1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   current = 0" DSL_CPE_CRLF
   "   history = 1.." _MKSTR(DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nLOF" DSL_CPE_CRLF
   "- DSL_uint32_t nLOS" DSL_CPE_CRLF
   "- DSL_uint32_t nLPR" DSL_CPE_CRLF
   "- DSL_uint32_t nLOM" DSL_CPE_CRLF
   "- DSL_uint32_t nSosSuccess" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineEventShowtimeCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineEventShowtimeCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineEventShowtimeCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_1DAY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nLOF=%u nLOS=%u nLPR=%u nLOM=%u nSosSuccess=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nLOF,
         (DSL_uint32_t)pData.data.nLOS, (DSL_uint32_t)pData.data.nLPR,
         (DSL_uint32_t)pData.data.nLOM, (DSL_uint32_t)pData.data.nSosSuccess);
   }

   return 0;
}

#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMleshs15mg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineEventShowtimeHistoryStats15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_EVENT_SHOWTIME_HISTORY_STATS_15MIN_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}


#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
static const DSL_char_t g_sPMleshs1dg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevIvs" DSL_CPE_CRLF
   "- DSL_uint32_t nPrevInvalidIvs" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineEventShowtimeHistoryStats1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_HistoryStatsChDir_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_HistoryStatsChDir_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_EVENT_SHOWTIME_HISTORY_STATS_1DAY_GET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nPrevIvs=%u nPrevInvalidIvs=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nPrevIvs, pData.data.nPrevInvalidIvs);
   }

   return 0;
}

#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_DATA_LINE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
static const DSL_char_t g_sPMlesctg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nLOF" DSL_CPE_CRLF
   "- DSL_uint32_t nLOS" DSL_CPE_CRLF
   "- DSL_uint32_t nLPR" DSL_CPE_CRLF
   "- DSL_uint32_t nLOM" DSL_CPE_CRLF
   "- DSL_uint32_t nSosSuccess" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineEventShowtimeCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineEventShowtimeCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineEventShowtimeCountersTotal_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_TOTAL_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nElapsedTime=%u bValid=%u nLOF=%u nLOS=%u nLPR=%u nLOM=%u "
         "nSosSuccess=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.total.nElapsedTime, pData.total.bValid, pData.data.nLOF, pData.data.nLOS, pData.data.nLPR,
         pData.data.nLOM, pData.data.nSosSuccess);
   }

   return 0;
}

#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
static const DSL_char_t g_sPMlescsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nHistoryInterval" DSL_CPE_CRLF
   "   most recent showtime = 0" DSL_CPE_CRLF
   "   former most recent showtime = 1.." _MKSTR(DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM) "" DSL_CPE_CRLF
   "- DSL_uint32_t nElapsedTime" DSL_CPE_CRLF
   "- DSL_boolean_t bValid" DSL_CPE_CRLF
   "- DSL_uint32_t nLOF" DSL_CPE_CRLF
   "- DSL_uint32_t nLOS" DSL_CPE_CRLF
   "- DSL_uint32_t nLPR" DSL_CPE_CRLF
   "- DSL_uint32_t nLOM" DSL_CPE_CRLF
   "- DSL_uint32_t nSosSuccess" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineEventShowtimeCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineEventShowtimeCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PM_LineEventShowtimeCounters_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nHistoryInterval=%u "
         "nElapsedTime=%u bValid=%u nLOF=%u nLOS=%u nLPR=%u nLOM=%u nSosSuccess=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         (DSL_uint32_t)pData.nHistoryInterval, (DSL_uint32_t)pData.interval.nElapsedTime,
         (DSL_uint32_t)pData.interval.bValid, (DSL_uint32_t)pData.data.nLOF,
         (DSL_uint32_t)pData.data.nLOS, (DSL_uint32_t)pData.data.nLPR,
         (DSL_uint32_t)pData.data.nLOM, (DSL_uint32_t)pData.data.nSosSuccess);
   }

   return 0;
}

#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef DSL_DEBUG_TOOL_INTERFACE
static const DSL_char_t g_sTcpStart[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_char_t sTcpMsgServerIp" DSL_CPE_CRLF
   "- DSL_uint16_t  nTcpMsgServerPort (optional, default: 2000)" DSL_CPE_CRLF
   "- DSL_boolean_t bEnableTcpCli (optional, default: enabled)" DSL_CPE_CRLF
   "   enable = 1" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_TCPMessageInterfaceSTART(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = DSL_SUCCESS;
   DSL_boolean_t bEnableTcpCli = DSL_FALSE;
   DSL_uint16_t tcpIpPort;
   DSL_char_t   tcpIpAddr[32];

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf ( pCommands, "%s %hu %hu",
                    tcpIpAddr, &tcpIpPort, &bEnableTcpCli);

   if (DSL_CPE_TcpDebugMessageIntfStart(DSL_CPE_GetGlobalContext(),
                                                   tcpIpPort, tcpIpAddr) < 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
        "ERROR - TCP debug messages interface start failed!" DSL_CPE_CRLF));

      ret = DSL_ERROR;
   }
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (DSL_CPE_TcpDebugCliIntfStart(DSL_CPE_GetGlobalContext(), bEnableTcpCli) < 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
        "ERROR - TCP debug CLI interface start failed!" DSL_CPE_CRLF));

      ret = DSL_ERROR;
   }
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

   DSL_CPE_FPrintf (out, "nReturn=%d", ret);

   return 0;
}

static const DSL_char_t g_sTcpStop[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_TCPMessageInterfaceSTOP(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = DSL_SUCCESS;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (DSL_CPE_TcpDebugCliIntfStop(DSL_CPE_GetGlobalContext()) < 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
        "ERROR - TCP debug CLI interface stop failed!" DSL_CPE_CRLF));

      ret = DSL_ERROR;
   }
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
   if (DSL_CPE_TcpDebugMessageIntfStop(DSL_CPE_GetGlobalContext()) < 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - TCP debug messages interface stop failed!" DSL_CPE_CRLF));

      ret = DSL_ERROR;
   }

   DSL_CPE_FPrintf (out, "nReturn=%d", ret);

   return 0;
}
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
static const DSL_char_t g_sDtiStart[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint16_t numOfPhyDevices" DSL_CPE_CRLF
   "- DSL_uint16_t numOfLinesPerPhyDevice" DSL_CPE_CRLF
   "- DSL_char_t sDtiServerIp" DSL_CPE_CRLF
   "- DSL_uint16_t  DTI Server Port" DSL_CPE_CRLF
   "- DSL_boolean_t bEnableCliAutoMsg" DSL_CPE_CRLF
   "   enable = 1" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   "- DSL_boolean_t bEnableDevAutoMsg" DSL_CPE_CRLF
   "   enable = 1" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   "- DSL_boolean_t bEnableSingleThreadMode" DSL_CPE_CRLF
   "   enable = 1" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DebugTraceInterfaceSTART(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_boolean_t bEnableCliAutoMsg = DSL_FALSE, bEnableDevAutoMsg = DSL_FALSE, bEnableSingleThrMode = DSL_FALSE;
   DSL_uint16_t numOfDevs, portsPerDev, dtiIpPort;
   DSL_char_t   dtiIpAddr[32];

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 7, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf ( pCommands, "%hu %hu %s %hu %hu %hu %hu",
                    &numOfDevs, &portsPerDev, dtiIpAddr, &dtiIpPort,
                    &bEnableCliAutoMsg, &bEnableDevAutoMsg, &bEnableSingleThrMode);

   /* start DTI agent */
   ret = DSL_CPE_Dti_Start(
                  DSL_CPE_GetGlobalContext(), numOfDevs, portsPerDev, dtiIpPort, dtiIpAddr,
                  (bEnableCliAutoMsg) ? DSL_TRUE : DSL_FALSE,
                  (bEnableDevAutoMsg) ? DSL_TRUE : DSL_FALSE,
                  (bEnableSingleThrMode) ? DSL_TRUE : DSL_FALSE);

   DSL_CPE_FPrintf (out, "nReturn=%d", ret);

   return 0;
}

static const DSL_char_t g_sDtiStop[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DebugTraceInterfaceSTOP(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   /* stop DTI agent */
   ret = DSL_CPE_Dti_Stop(DSL_CPE_GetGlobalContext());

   DSL_CPE_FPrintf (out, "nReturn=%d", ret);

   return 0;
}
#endif /* #if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/

#ifdef INCLUDE_PILOT_TONES_STATUS
static const DSL_char_t g_sPtsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_uint16_t nPilotTone" DSL_CPE_CRLF
   "- ... nParamNr[1] nCount[" _MKSTR(MAX_NUMBER_OF_PILOT_TONES) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PilotTonesStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint8_t nPilot;
   DSL_PilotTonesStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_PilotTonesStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PILOT_TONES_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {

      DSL_CPE_FPrintf (out, DSL_CPE_RET"nNumData=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nNumData);
      DSL_CPE_FPrintf (out,
      "nFormat=(nPilotNum(dec),nPilotIndex(dec)) nData=\"" DSL_CPE_CRLF);

      for (nPilot = 0; nPilot < pData.data.nNumData; nPilot++)
      {
         DSL_CPE_FPrintf (out, "(%02d,%05d)"DSL_CPE_CRLF, nPilot, pData.data.nPilotTone[nPilot]);
      }

      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
   }

   return 0;
}
#endif /* #ifdef INCLUDE_PILOT_TONES_STATUS*/

static const DSL_char_t g_sRccs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_BF_RebootCriteriaConfigData_t nRebootCriteria (hex)" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_CLEANED = 0x00000000" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LOM = 0x00000001" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LOF = 0x00000002" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LOS = 0x00000004" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_ESE = 0x00000008" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_ES90 = 0x00000010" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_SES30 = 0x00000020" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_NEGATIVE_MARGIN = 0x00000040" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_OOS_BC0 = 0x00000080" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_OOS_BC1 = 0x00000100" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_NCD_BC0 = 0x00000200" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_NCD_BC1 = 0x00000400" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LCD_BC0 = 0x00000800" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LCD_BC1 = 0x00001000" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RebootCriteriaConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RebootCriteriaConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_RebootCriteriaConfig_t));

   DSL_CPE_sscanf (pCommands, "%xu", &pData.data.nRebootCriteria);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_REBOOT_CRITERIA_CONFIG_SET,
      (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sRccg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_BF_RebootCriteriaConfigData_t nRebootCriteria (hex)" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_CLEANED = 0x00000000" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LOM = 0x00000001" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LOF = 0x00000002" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LOS = 0x00000004" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_ESE = 0x00000008" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_ES90 = 0x00000010" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_SES30 = 0x00000020" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_NEGATIVE_MARGIN = 0x00000040" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_OOS_BC0 = 0x00000080" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_OOS_BC1 = 0x00000100" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_NCD_BC0 = 0x00000200" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_NCD_BC1 = 0x00000400" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LCD_BC0 = 0x00000800" DSL_CPE_CRLF
   "   DSL_REBOOT_CRITERIA_LCD_BC1 = 0x00001000" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RebootCriteriaConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RebootCriteriaConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_RebootCriteriaConfig_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_REBOOT_CRITERIA_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nRebootCriteria=0x%08X" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nRebootCriteria);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
static const DSL_char_t g_sLecg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorCode1 (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t nSubErrorCode1 (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t nFwErrorCode1 (dec)" DSL_CPE_CRLF
   "- DSL_uint32_t nErrorCode2 (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t nSubErrorCode2 (hex)" DSL_CPE_CRLF
   "- DSL_uint32_t nFwErrorCode2 (dec)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LastExceptionCodesGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LastExceptionCodes_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LastExceptionCodes_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LAST_EXCEPTION_CODES_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET
         "nErrorCode1=0x%02X nSubErrorCode1=0x%02X nFwErrorCode1=%u "
         "nErrorCode2=0x%02X nSubErrorCode2=0x%02X nFwErrorCode2=%u"DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nErrorCode1, pData.data.nSubErrorCode1, pData.data.nFwErrorCode1,
         pData.data.nErrorCode2, pData.data.nSubErrorCode2, pData.data.nFwErrorCode2);
   }

   return 0;
}
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/

#ifdef INCLUDE_REAL_TIME_TRACE
static const DSL_char_t g_sRtti[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "- DSL_char_t IP address and port (xxx.xxx.xxx.xxx:xxx)" DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RTT_Init(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RTT_Init_t pData;
   DSL_int_t ip1, ip2, ip3, ip4, port;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_RTT_Init_t));

   sscanf( pCommands, "%d.%d.%d.%d:%d", &ip1, &ip2, &ip3, &ip4, &port);

   if (ip1 < 0 || ip1 >  255 ||
       ip2 < 0 || ip2 >  255 ||
       ip3 < 0 || ip3 >  255 ||
       ip4 < 0 || ip4 >  255 ||
       port < 0 || port > 65535)
   {
      return -1;
   }

   pData.data.nIpAddress[0] = ip1;
   pData.data.nIpAddress[1] = ip2;
   pData.data.nIpAddress[2] = ip3;
   pData.data.nIpAddress[3] = ip4;
   pData.data.nPort = port;

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_RTT_INIT, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sRttcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nStartIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nSize" DSL_CPE_CRLF
   "- DSL_RTT_ModeControlSet_t nRttMode" DSL_CPE_CRLF
   "   DSL_RTT_MODE_SHOWTIME = 0" DSL_CPE_CRLF
   "- DSL_uint16_t nStopCriteria" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RTT_ConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RTT_Config_t pData;
   DSL_int_t nStartIndex, nSize, nRttMode, nStopCriteria;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 4, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_RTT_Config_t));

   sscanf(pCommands,"%u %u %u %u",&nStartIndex,
                                  &nSize,
                                  &nRttMode,
                                  &nStopCriteria);

   pData.data.nStartIndex = nStartIndex;
   pData.data.nSize = nSize;
   pData.data.nRttMode = nRttMode;
   pData.data.nStopCriteria = nStopCriteria;

   ret = ioctl(fd, DSL_FIO_RTT_CONFIG_SET, &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sRttcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nStartIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nSize" DSL_CPE_CRLF
   "- DSL_RTT_ModeControlSet_t nRttMode" DSL_CPE_CRLF
   "   DSL_RTT_MODE_SHOWTIME = 0" DSL_CPE_CRLF
   "- DSL_uint16_t nStopCriteria" DSL_CPE_CRLF
   DSL_CPE_CRLF;
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RTT_ConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RTT_Config_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_RTT_Config_t));

   ret = ioctl(fd, DSL_FIO_RTT_CONFIG_GET, &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nStartIndex=%u nSize=%u nRttMode=%u nStopcriteria=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nStartIndex, pData.data.nSize,
         pData.data.nRttMode, pData.data.nStopCriteria);
   }

   return 0;
}

static const DSL_char_t g_sRttsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_RTT_FwStatus_t nFwStatus" DSL_CPE_CRLF
   "   DSL_RTT_FW_STATUS_IDLE = 0" DSL_CPE_CRLF
   "   DSL_RTT_FW_STATUS_IN_PROGRESS = 1" DSL_CPE_CRLF
   "   DSL_RTT_FW_STATUS_COMPLETED = 2" DSL_CPE_CRLF
   "   DSL_RTT_FW_STATUS_ABORTED = 3" DSL_CPE_CRLF
   "- DSL_RTT_FwFailreason_t nFwFailReason" DSL_CPE_CRLF
   "   DSL_RTT_FW_FAIL_REASON_NONE = 0" DSL_CPE_CRLF
   "   DSL_RTT_FW_FAIL_REASON_INVALID_CONFIG = 1" DSL_CPE_CRLF
   "   DSL_RTT_FW_FAIL_REASON_NOT_SUPPORTED = 2" DSL_CPE_CRLF
   "- DSL_Error_t nSwErrorCode" DSL_CPE_CRLF
   DSL_CPE_CRLF;
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RTT_StatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RTT_Status_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_RTT_Status_t));

   ret = ioctl(fd, DSL_FIO_RTT_STATUS_GET, &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nFwStatus=%u nFwFailReason=%u nSwErrorCode=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nFwStatus, pData.data.nFwFailReason,
         pData.data.nSwErrorCode);
   }

   return 0;
}

static const DSL_char_t g_sRttctrls[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bRttEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RTT_ControlSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RTT_Control_t pData;
   DSL_uint32_t nRttEnable;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_RTT_Control_t));

   sscanf(pCommands,"%u",&nRttEnable);

   pData.data.nRttEnable = (nRttEnable == 1) ? 1 : 0;

   ret = ioctl(fd, DSL_FIO_RTT_CONTROL_SET, &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sRttstatg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t nFwPacketsSend" DSL_CPE_CRLF
   "- DSL_uint32_t nFwPacketsDropped" DSL_CPE_CRLF
   "- DSL_uint32_t nSwPacketsSend" DSL_CPE_CRLF
   "- DSL_uint32_t nSwPacketsDropped" DSL_CPE_CRLF
   DSL_CPE_CRLF;
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_RTT_StatisticsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_RTT_Statistics_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x00, sizeof(DSL_RTT_Statistics_t));

   ret = ioctl(fd, DSL_FIO_RTT_STATISTICS_GET, &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nFwPacketsSend=%u nFwPacketsDropped=%u nSwPacketsSend=%u nSwPacketsDropped=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nFwPacketsSend, pData.data.nFwPacketsDropped,
         pData.data.nSwPacketsSend, pData.data.nSwPacketsDropped);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_REAL_TIME_TRACE */

static const DSL_char_t g_sOsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   DSL_UPSTREAM = 0" DSL_CPE_CRLF
   "   DSL_DOWNSTREAM = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   DSL_UPSTREAM = 0" DSL_CPE_CRLF
   "   DSL_DOWNSTREAM = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nBitswapRequested" DSL_CPE_CRLF
   "- DSL_uint16_t nExtBitswapRequested" DSL_CPE_CRLF
   "- DSL_uint16_t nBitswapExecuted" DSL_CPE_CRLF
   "- DSL_uint16_t nBitswapRejected" DSL_CPE_CRLF
   "- DSL_uint16_t nBitswapTimeout" DSL_CPE_CRLF
   "- DSL_uint16_t nSraRequested" DSL_CPE_CRLF
   "- DSL_uint16_t nSraExecuted" DSL_CPE_CRLF
   "- DSL_uint16_t nSraRejected" DSL_CPE_CRLF
   "- DSL_uint16_t nSraTimeout" DSL_CPE_CRLF
   "- DSL_uint16_t nSosRequested" DSL_CPE_CRLF
   "- DSL_uint16_t nSosExecuted" DSL_CPE_CRLF
   "- DSL_uint16_t nSosRejected" DSL_CPE_CRLF
   "- DSL_uint16_t nSosTimeout" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_OlrStatisticsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_OlrStatistics_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_OlrStatistics_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_OLR_STATISTICS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nBitswapRequested=%hu "
         "nExtBitswapRequested=%hu nBitswapExecuted=%hu nBitswapRejected=%hu "
         "nBitswapTimeout=%hu nSraRequested=%hu nSraExecuted=%hu "
         "nSraRejected=%hu nSraTimeout=%hu nSosRequested=%hu "
         "nSosExecuted=%hu nSosRejected=%hu nSosTimeout=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.nDirection,pData.data.nBitswapRequested,
         pData.data.nExtBitswapRequested,pData.data.nBitswapExecuted,
         pData.data.nBitswapRejected,pData.data.nBitswapTimeout,
         pData.data.nSraRequested,pData.data.nSraExecuted,
         pData.data.nSraRejected,pData.data.nSraTimeout,
         pData.data.nSosRequested,pData.data.nSosExecuted,
         pData.data.nSosRejected,pData.data.nSosTimeout);
   }

   return 0;
}

static const DSL_char_t g_sLlsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_UnitSelector_t nUnit" DSL_CPE_CRLF
   "   feet = 0" DSL_CPE_CRLF
   "   meter = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_UnitSelector_t nUnit" DSL_CPE_CRLF
   "   feet = 0" DSL_CPE_CRLF
   "   meter = 1" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint32_t nLength_Awg26" DSL_CPE_CRLF
   "- DSL_uint32_t nLength_Awg24" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LoopLengthStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LoopLengthStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LoopLengthStatus_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nUnit);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LOOP_LENGTH_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, "nReturn=%d"DSL_CPE_CRLF, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d nUnit=%u nLength_Awg26=%u nLength_Awg24=%u"
         DSL_CPE_CRLF, pData.accessCtl.nReturn, pData.nUnit,
         pData.data.nLength_Awg26,
         pData.data.nLength_Awg24);
   }

   return 0;
}

#ifdef INCLUDE_DSL_FILTER_DETECTION
static const DSL_char_t g_sFddg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint8_t nNoiseLevel" DSL_CPE_CRLF
   "- DSL_BridgeTapLength_t nBridgeTapFlag" DSL_CPE_CRLF
   "   DSL_BRIDGE_TAP_NONE = 0" DSL_CPE_CRLF
   "   DSL_BRIDGE_TAP_SHORT = 1" DSL_CPE_CRLF
   "   DSL_BRIDGE_TAP_1KFT = 2" DSL_CPE_CRLF
   "   DSL_BRIDGE_TAP_LONG = 3" DSL_CPE_CRLF
   "- DSL_boolean_t bNoConfidence" DSL_CPE_CRLF
   "- DSL_int16_t nNonLinearEchoMetric1" DSL_CPE_CRLF
   "- DSL_int16_t nNonLinearEchoMetric2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_FilterDetectionDataGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_FilterDetection_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_FilterDetection_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_FILTER_DETECTION_DATA_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d nNoiseLevel=%hu nBridgeTapFlag=%u "
         "bNoConfidence=%u nNonLinearEchoMetric1=%hd nNonLinearEchoMetric2=%hd"
         DSL_CPE_CRLF, pData.accessCtl.nReturn,
         pData.data.nNoiseLevel,
         pData.data.nBridgeTapFlag,
         pData.data.bNoConfidence,
         pData.data.nNonLinearEchoMetric1,
         pData.data.nNonLinearEchoMetric2);
   }

   return 0;
}
#endif /* #ifdef INCLUDE_DSL_FILTER_DETECTION*/

static const DSL_char_t g_sHsdg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_int16_t nHybridIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nHybridMetric" DSL_CPE_CRLF
   "- ... 3 times previous 2 params (act seleted, second best, FD act selected, "
   "FD second best)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_HybridSelectionDataGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_HybridSelection_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_HybridSelection_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_HYBRID_SELECTION_DATA_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, pData.accessCtl.nReturn);
   }
   else
   {
      DSL_CPE_FPrintf (out, "nReturn=%d " DSL_CPE_CRLF, pData.accessCtl.nReturn);
      DSL_CPE_FPrintf (out, "nFormat=(nHybridIndex(dec),nHybridMetric(hex)) nData=\"" DSL_CPE_CRLF);
      DSL_CPE_FPrintf (out, "(%hd,0x%04X) (%hd,0x%04X) (%hd,0x%04X) (%hd,0x%04X)" DSL_CPE_CRLF,
         pData.data.actualSelection.nHybridIndex,
         pData.data.actualSelection.nHybridMetric,
         pData.data.secondBestSelection.nHybridIndex,
         pData.data.secondBestSelection.nHybridMetric,
         pData.data.fdActualSelection.nHybridIndex,
         pData.data.fdActualSelection.nHybridMetric,
         pData.data.fdSecondBestSelection.nHybridIndex,
         pData.data.fdSecondBestSelection.nHybridMetric);
      DSL_CPE_FPrintf (out, "\"" DSL_CPE_CRLF);
   }

   return 0;
}

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
static const DSL_char_t g_sRtsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   "- DSL_uint32_t nRxCorruptedTotal" DSL_CPE_CRLF
   "- DSL_uint32_t nRxUncorrectedProtected" DSL_CPE_CRLF
   "- DSL_uint32_t nRxRetransmitted" DSL_CPE_CRLF
   "- DSL_uint32_t nRxCorrected" DSL_CPE_CRLF
   "- DSL_uint32_t nTxRetransmitted" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_ReTxStatisticsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_ReTxStatistics_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_ReTxStatistics_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_RETX_STATISTICS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nDirection=%u nRxCorruptedTotal=%u "
                    "nRxUncorrectedProtected=%u nRxRetransmitted=%u nRxCorrected=%u "
                    "nTxRetransmitted=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection,
         pData.data.nRxCorruptedTotal, pData.data.nRxUncorrectedProtected,
         pData.data.nRxRetransmitted, pData.data.nRxCorrected, pData.data.nTxRetransmitted);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */

static const DSL_char_t g_sDsnrg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   "- DSL_DeltDataType_t nDeltDataType" DSL_CPE_CRLF
   "   diagnostic = 0" DSL_CPE_CRLF
   "   showtime = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nMeasurementTime" DSL_CPE_CRLF
   "- DSL_uint8_t nGroupSize (currently unused!)" DSL_CPE_CRLF
   "- DSL_uint16_t nNumData" DSL_CPE_CRLF
   "- DSL_int_t nToneIndex" DSL_CPE_CRLF
   "- DSL_uint16_t nSnr" DSL_CPE_CRLF
   "- ... nParamNr[2] nCount[" _MKSTR(DSL_MAX_NSC) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DeltSNRGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint16_t i = 0;
   DSL_G997_DeltSnr_t *pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = malloc(sizeof(DSL_G997_DeltSnr_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }
   memset(pData, 0, sizeof(DSL_G997_DeltSnr_t));

   DSL_CPE_sscanf (pCommands, "%u %u", &pData->nDirection, &pData->nDeltDataType);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_DELT_SNR_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u nDeltDataType=%u "
         "nMeasurementTime=%hu nGroupSize=%hu nNumData=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData->accessCtl.nReturn),
         pData->nDirection, pData->nDeltDataType,
         pData->data.nMeasurementTime, pData->data.nGroupSize,
         pData->data.deltSnr.nNumData );

      DSL_CPE_FPrintf(out, "nFormat=(nToneIndex(dec),nSnr(dec)) nData=\"");
      for (i = 0; (i < pData->data.deltSnr.nNumData) && (i < DSL_MAX_NSC); i++)
      {
         if (i%10 == 0) DSL_CPE_FPrintf(out, "" DSL_CPE_CRLF );
         DSL_CPE_FPrintf(out, "(%04d,%03d) ", i, pData->data.deltSnr.nNSCData[i] );
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);

   }
   free(pData);
   return 0;
}

static const DSL_char_t g_sFdsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_FirmwareDownloadStatusType_t nStatus" DSL_CPE_CRLF
   "   DSL_FW_DWNLD_STATUS_NA      = 0" DSL_CPE_CRLF
   "   DSL_FW_DWNLD_STATUS_PENDING = 1" DSL_CPE_CRLF
   "   DSL_FW_DWNLD_STATUS_READY   = 2" DSL_CPE_CRLF
   "   DSL_FW_DWNLD_STATUS_FAILED  = 3" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_FirmwareDownloadStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_FirmwareDownloadStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_FirmwareDownloadStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_FIRMWARE_DOWNLOAD_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nStatus=%d" DSL_CPE_CRLF,
      DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nStatus);
   }

   return 0;
}

/**
   Register the CLI commands.

   \param pContext Pointer to dsl library context structure, [I]
   \param command optional parameters [I]
*/
DSL_void_t DSL_CPE_CLI_AccessCommandsRegister(DSL_void_t)
{
   /* Debug functionalities */
#ifndef DSL_CPE_DEBUG_DISABLE
   DSL_CPE_CLI_CMD_ADD_COMM ("ccadbgmlg", "CCA_DBG_ModuleLevelGet", DSL_CPE_CLI_CCA_DBG_ModuleLevelGet, g_sCcaDbgmlg);
   DSL_CPE_CLI_CMD_ADD_COMM ("ccadbgmls", "CCA_DBG_ModuleLevelSet", DSL_CPE_CLI_CCA_DBG_ModuleLevelSet, g_sCcaDbgmls);
#endif
   DSL_CPE_CLI_CMD_ADD_COMM ("dbgmlg", "DBG_ModuleLevelGet", DSL_CPE_CLI_DBG_ModuleLevelGet, g_sDBGmlg);
   DSL_CPE_CLI_CMD_ADD_COMM ("dbgmls", "DBG_ModuleLevelSet", DSL_CPE_CLI_DBG_ModuleLevelSet, g_sDBGmls);

   /* Common functionalities */
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
#ifdef INCLUDE_DSL_CONFIG_GET
#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
   DSL_CPE_CLI_CMD_ADD_COMM ("asecg", "AutobootScriptExecuteConfigGet", DSL_CPE_CLI_AutobootScriptExecuteConfigGet, g_sAsecg);
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/
   DSL_CPE_CLI_CMD_ADD_COMM ("acog", "AutobootConfigOptionGet", DSL_CPE_CLI_AutobootConfigOptionGet, g_sAcfgg);
#endif /* INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
   DSL_CPE_CLI_CMD_ADD_COMM ("se", "ScriptExecute", DSL_CPE_CLI_ScriptExecute, g_sSe);
   DSL_CPE_CLI_CMD_ADD_COMM ("asecs", "AutobootScriptExecuteConfigSet", DSL_CPE_CLI_AutobootScriptExecuteConfigSet, g_sAsecs);
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/
   DSL_CPE_CLI_CMD_ADD_COMM ("acos", "AutobootConfigOptionSet", DSL_CPE_CLI_AutobootConfigOptionSet, g_sAcfgs);
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT*/

#ifdef INCLUDE_SCRIPT_NOTIFICATION
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("nsecg", "NotificationScriptExecuteConfigGet", DSL_CPE_CLI_NotificationScriptExecuteConfigGet, g_sNsecg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("nsecs", "NotificationScriptExecuteConfigSet", DSL_CPE_CLI_NotificationScriptExecuteConfigSet, g_sNsecs);
#endif /* INCLUDE_SCRIPT_NOTIFICATION*/

   DSL_CPE_CLI_CMD_ADD_COMM ("acs", "AutobootControlSet", DSL_CPE_CLI_AutobootControlSet, g_sAcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("asg", "AutobootStatusGet", DSL_CPE_CLI_AutobootStatusGet, g_sAsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("lsg", "LineStateGet", DSL_CPE_CLI_LineStateGet, g_sLsg);
#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
   DSL_CPE_CLI_CMD_ADD_COMM ("rusg", "ResourceUsageStatisticsGet", DSL_CPE_CLI_ResourceUsageStatisticsGet, g_sRusg);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("esmcg", "EventStatusMaskConfigGet", DSL_CPE_CLI_EventStatusMaskConfigGet, g_sEsmcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("esmcs", "EventStatusMaskConfigSet", DSL_CPE_CLI_EventStatusMaskConfigSet, g_sEsmcs);
#ifdef INCLUDE_DSL_FRAMING_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("fpsg", "FramingParameterStatusGet", DSL_CPE_CLI_FramingParameterStatusGet, g_sFpsg);
#endif /* INCLUDE_DSL_FRAMING_PARAMETERS*/
   DSL_CPE_CLI_CMD_ADD_COMM ("ics", "InstanceControlSet", DSL_CPE_CLI_InstanceControlSet, g_sIcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("isg", "InstanceStatusGet", DSL_CPE_CLI_InstanceStatusGet, g_sIsg);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("lfcg", "LineFeatureConfigGet", DSL_CPE_CLI_LineFeatureConfigGet, g_sLfcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("lfcs", "LineFeatureConfigSet", DSL_CPE_CLI_LineFeatureConfigSet, g_sLfcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("lfsg", "LineFeatureStatusGet", DSL_CPE_CLI_LineFeatureStatusGet, g_sLfsg);
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("sicg", "SystemInterfaceConfigGet", DSL_CPE_CLI_SystemInterfaceConfigGet, g_sSicg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("sics", "SystemInterfaceConfigSet", DSL_CPE_CLI_SystemInterfaceConfigSet, g_sSics);
   DSL_CPE_CLI_CMD_ADD_COMM ("sisg", "SystemInterfaceStatusGet", DSL_CPE_CLI_SystemInterfaceStatusGet, g_sSisg);
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/
   DSL_CPE_CLI_CMD_ADD_COMM ("tmcs", "TestModeControlSet", DSL_CPE_CLI_TestModeControlSet, g_sTmcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("tmsg", "TestModeStatusGet", DSL_CPE_CLI_TestModeStatusGet, g_sTmsg);
#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
   DSL_CPE_CLI_CMD_ADD_COMM ("bbsg", "BandBorderStatusGet", DSL_CPE_CLI_BandBorderStatusGet, g_sBbsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("mlsg", "MiscLineStatusGet", DSL_CPE_CLI_MiscLineStatusGet, g_sMlsg);
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/
   /* G.997.1 related functionlities */
   DSL_CPE_CLI_CMD_ADD_COMM ("g997racs", "G997_RateAdaptationConfigSet", DSL_CPE_CLI_G997_RateAdaptationConfigSet, g_sRaCs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997racg", "G997_RateAdaptationConfigGet", DSL_CPE_CLI_G997_RateAdaptationConfigGet, g_sRaCg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
#ifdef INCLUDE_DSL_G997_ALARM
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997amdpfcg", "G997_AlarmMaskDataPathFailuresConfigGet", DSL_CPE_CLI_G997_AlarmMaskDataPathFailuresConfigGet, g_sG997amdpfcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997amdpfcs", "G997_AlarmMaskDataPathFailuresConfigSet", DSL_CPE_CLI_G997_AlarmMaskDataPathFailuresConfigSet, g_sG997amdpfcs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997amlfcg", "G997_AlarmMaskLineFailuresConfigGet", DSL_CPE_CLI_G997_AlarmMaskLineFailuresConfigGet, g_sG997amlfcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997amlfcs", "G997_AlarmMaskLineFailuresConfigSet", DSL_CPE_CLI_G997_AlarmMaskLineFailuresConfigSet, g_sG997amlfcs);
#endif /* INCLUDE_DSL_G997_ALARM*/
#ifdef INCLUDE_DSL_G997_PER_TONE
   DSL_CPE_CLI_CMD_ADD_COMM ("g997bang", "G997_BitAllocationNscGet", DSL_CPE_CLI_G997_BitAllocationNscGet, g_sG997bang);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997bansg", "G997_BitAllocationNscShortGet", DSL_CPE_CLI_G997_BitAllocationNscShortGet, g_sG997bansg);
#endif /* INCLUDE_DSL_G997_PER_TONE*/
#ifdef INCLUDE_DSL_G997_ALARM
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997cdrtcg", "G997_ChannelDataRateThresholdConfigGet", DSL_CPE_CLI_G997_ChannelDataRateThresholdConfigGet, g_sG997cdrtcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997cdrtcs", "G997_ChannelDataRateThresholdConfigSet", DSL_CPE_CLI_G997_ChannelDataRateThresholdConfigSet, g_sG997cdrtcs);
#endif /* INCLUDE_DSL_G997_ALARM*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997csg", "G997_ChannelStatusGet", DSL_CPE_CLI_G997_ChannelStatusGet, g_sG997csg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dpfsg", "G997_DataPathFailuresStatusGet", DSL_CPE_CLI_G997_DataPathFailuresStatusGet, g_sG997dpfsg);
#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("g997fpsg", "G997_FramingParameterStatusGet", DSL_CPE_CLI_G997_FramingParameterStatusGet, g_sG997fpsg);
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/
#ifdef INCLUDE_DSL_G997_PER_TONE
   DSL_CPE_CLI_CMD_ADD_COMM ("g997gang", "G997_GainAllocationNscGet", DSL_CPE_CLI_G997_GainAllocationNscGet, g_sG997gang);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997gansg", "G997_GainAllocationNscShortGet", DSL_CPE_CLI_G997_GainAllocationNscShortGet, g_sG997gansg);
#endif /* INCLUDE_DSL_G997_PER_TONE*/
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lacg", "G997_LineActivateConfigGet", DSL_CPE_CLI_G997_LineActivateConfigGet, g_sG997lacg);
#endif /*  INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lacs", "G997_LineActivateConfigSet", DSL_CPE_CLI_G997_LineActivateConfigSet, g_sG997lacs);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lfsg", "G997_LineFailureStatusGet", DSL_CPE_CLI_G997_LineFailureStatusGet, g_sG997lfsg);
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lig", "G997_LineInventoryGet", DSL_CPE_CLI_G997_LineInventoryGet, g_sG997lig);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lis", "G997_LineInventorySet", DSL_CPE_CLI_G997_LineInventorySet, g_sG997lis);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997listrg", "G997_LineInventorySTRingGet", DSL_CPE_CLI_G997_LineInventoryStringGet, g_sG997listrg);
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lsg", "G997_LineStatusGet", DSL_CPE_CLI_G997_LineStatusGet, g_sG997lsg);
#ifdef INCLUDE_DSL_G997_STATUS
   DSL_CPE_CLI_CMD_ADD_COMM ("g997ltsg", "G997_LineTransmissionStatusGet", DSL_CPE_CLI_G997_LineTransmissionStatusGet, g_sG997ltsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lisg", "G997_LineInitStatusGet", DSL_CPE_CLI_G997_LineInitStatusGet, g_sG997lisg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lstg", "G997_LastStateTransmittedGet", DSL_CPE_CLI_G997_LastStateTransmittedGet, g_sG997lstg);
#endif /* INCLUDE_DSL_G997_STATUS*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997pmsft", "G997_PowerManagementStateForcedTrigger", DSL_CPE_CLI_G997_PowerManagementStateForcedTrigger, g_sG997pmsft);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997pmsg", "G997_PowerManagementStatusGet", DSL_CPE_CLI_G997_PowerManagementStatusGet, g_sG997pmsg);
#ifdef INCLUDE_DSL_CEOC
   DSL_CPE_CLI_CMD_ADD_COMM ("g997sms", "G997_SnmpMessageSend", DSL_CPE_CLI_G997_SnmpMessageSend, g_sG997sms);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997smr", "G997_SnmpMessageReceive", DSL_CPE_CLI_G997_SnmpMessageReceive, g_sG997smr);
#endif /* INCLUDE_DSL_CEOC */
#ifdef INCLUDE_DSL_G997_PER_TONE
   DSL_CPE_CLI_CMD_ADD_COMM ("g997sang", "G997_SnrAllocationNscGet", DSL_CPE_CLI_G997_SnrAllocationNscGet, g_sG997sang);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997sansg", "G997_SnrAllocationNscShortGet", DSL_CPE_CLI_G997_SnrAllocationNscShortGet, g_sG997sansg);
#endif /* INCLUDE_DSL_G997_PER_TONE*/
#ifdef INCLUDE_DSL_DELT
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dfr", "G997_DeltFreeResources", DSL_CPE_CLI_G997_DeltFreeResources, g_sg997dfr);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dhling", "G997_DeltHLINGet", DSL_CPE_CLI_G997_DeltHLINGet, g_sG997dhling);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dhlinsg", "G997_DeltHLINScaleGet", DSL_CPE_CLI_G997_DeltHLINScaleGet, g_sG997dhlinsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dhlogg", "G997_DeltHLOGGet", DSL_CPE_CLI_G997_DeltHLOGGet,g_sG997dhlogg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dqlng", "G997_DeltQLNGet", DSL_CPE_CLI_G997_DeltQLNGet,g_sG997dqlng);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dsnrg", "G997_DeltSNRGet", DSL_CPE_CLI_G997_DeltSNRGet,g_sG997dsnrg);
#endif /* INCLUDE_DSL_DELT*/
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997xtusecg", "G997_XTUSystemEnablingConfigGet", DSL_CPE_CLI_G997_XTUSystemEnablingConfigGet, g_sG997xtusecg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997xtusecs", "G997_XTUSystemEnablingConfigSet", DSL_CPE_CLI_G997_XTUSystemEnablingConfigSet, g_sG997xtusecs);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997xtusesg", "G997_XTUSystemEnablingStatusGet", DSL_CPE_CLI_G997_XTUSystemEnablingStatusGet, g_sG997xtusesg);

   DSL_CPE_CLI_CMD_ADD_COMM ("locs", "LineOptionsConfigSet", DSL_CPE_CLI_LineOptionsConfigSet, g_sLocs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("locg", "LineOptionsConfigGet", DSL_CPE_CLI_LineOptionsConfigGet, g_sLocg);
#endif /* INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_CONFIG
   DSL_CPE_CLI_CMD_ADD_COMM ("pmcs", "PM_ConfigSet", DSL_CPE_CLI_PM_ConfigSet, g_sPMcs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("pmcg", "PM_ConfigGet", DSL_CPE_CLI_PM_ConfigGet, g_sPMcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_CPE_PM_CONFIG*/
   /* PM related functionalities */
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pm15meet", "PM_15MinElapsedExtTrigger", DSL_CPE_CLI_PM_15MinElapsedExtTrigger, g_sPM15meet);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmbms", "PM_BurninModeSet", DSL_CPE_CLI_PM_BurninModeSet, g_sPMbms);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmetr", "PM_ElapsedTimeReset", DSL_CPE_CLI_PM_ElapsedTimeReset, g_sPMetr);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmsms", "PM_SyncModeSet", DSL_CPE_CLI_PM_SyncModeSet, g_sPMsms);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("pmsmg", "PM_SyncModeGet", DSL_CPE_CLI_PM_SyncModeGet, g_sPMsmg);
#endif /* #ifdef INCLUDE_DSL_CONFIG_GET*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   DSL_CPE_CLI_CMD_ADD_COMM ("pmr", "PM_Reset", DSL_CPE_CLI_PM_Reset, g_sPMr);

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   /* Channel related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmcc15mg", "PM_ChannelCounters15MinGet", DSL_CPE_CLI_PM_ChannelCounters15MinGet, g_sPMcc15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmcc1dg", "PM_ChannelCounters1DayGet", DSL_CPE_CLI_PM_ChannelCounters1DayGet, g_sPMcc1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmchs15mg", "PM_ChannelHistoryStats15MinGet", DSL_CPE_CLI_PM_ChannelHistoryStats15MinGet, g_sPMchs15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmchs1dg", "PM_ChannelHistoryStats1DayGet", DSL_CPE_CLI_PM_ChannelHistoryStats1DayGet, g_sPMchs1dg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmcctg", "PM_ChannelCountersTotalGet", DSL_CPE_CLI_PM_ChannelCountersTotalGet, g_sPMcctg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmccsg", "PM_ChannelCountersShowtimeGet", DSL_CPE_CLI_PM_ChannelCountersShowtimeGet, g_sPMccsg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmct15mg", "PM_ChannelThresholds15MinGet", DSL_CPE_CLI_PM_ChannelThresholds15MinGet, g_sPMct15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmct15ms", "PM_ChannelThresholds15MinSet", DSL_CPE_CLI_PM_ChannelThresholds15MinSet, g_sPMct15ms);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmct1dg", "PM_ChannelThresholds1DayGet", DSL_CPE_CLI_PM_ChannelThresholds1DayGet, g_sPMct1dg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmct1ds", "PM_ChannelThresholds1DaySet", DSL_CPE_CLI_PM_ChannelThresholds1DaySet, g_sPMct1ds);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   /* Data path related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpc15mg", "PM_DataPathCounters15MinGet", DSL_CPE_CLI_PM_DataPathCounters15MinGet, g_sPMdpc15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpc1dg", "PM_DataPathCounters1DayGet", DSL_CPE_CLI_PM_DataPathCounters1DayGet, g_sPMdpc1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdphs15mg", "PM_DataPathHistoryStats15MinGet", DSL_CPE_CLI_PM_DataPathHistoryStats15MinGet, g_sPMdphs15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdphs1dg", "PM_DataPathHistoryStats1DayGet", DSL_CPE_CLI_PM_DataPathHistoryStats1DayGet, g_sPMdphs1dg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpctg", "PM_DataPathCountersTotalGet", DSL_CPE_CLI_PM_DataPathCountersTotalGet, g_sPMdpctg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpcsg", "PM_DataPathCountersShowtimeGet", DSL_CPE_CLI_PM_DataPathCountersShowtimeGet, g_sPMdpcsg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpt15mg", "PM_DataPathThresholds15MinGet", DSL_CPE_CLI_PM_DataPathThresholds15MinGet, g_sPMdpt15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpt15ms", "PM_DataPathThresholds15MinSet", DSL_CPE_CLI_PM_DataPathThresholds15MinSet, g_sPMdpt15ms);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpt1dg", "PM_DataPathThresholds1DayGet", DSL_CPE_CLI_PM_DataPathThresholds1DayGet, g_sPMdpt1dg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpt1ds", "PM_DataPathThresholds1DaySet", DSL_CPE_CLI_PM_DataPathThresholds1DaySet, g_sPMdpt1ds);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   /* ReTx related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtc15mg", "PM_ReTxCounters15MinGet", DSL_CPE_CLI_PM_ReTxCounters15MinGet, g_sPMrtc15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtc1dg", "PM_ReTxCounters1DayGet", DSL_CPE_CLI_PM_ReTxCounters1DayGet, g_sPMrtc1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrths15mg", "PM_ReTxHistoryStats15MinGet", DSL_CPE_CLI_PM_ReTxHistoryStats15MinGet, g_sPMrths15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrths1dg", "PM_ReTxHistoryStats1DayGet", DSL_CPE_CLI_PM_ReTxHistoryStats1DayGet, g_sPMrths1dg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtctg", "PM_ReTxCountersTotalGet", DSL_CPE_CLI_PM_ReTxCountersTotalGet, g_sPMrtctg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtcsg", "PM_ReTxCountersShowtimeGet", DSL_CPE_CLI_PM_ReTxCountersShowtimeGet, g_sPMrtcsg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtt15mg", "PM_ReTxThresholds15MinGet", DSL_CPE_CLI_PM_ReTxThresholds15MinGet, g_sPMrtt15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtt15ms", "PM_ReTxThresholds15MinSet", DSL_CPE_CLI_PM_ReTxThresholds15MinSet, g_sPMrtt15ms);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtt1dg", "PM_ReTxThresholds1DayGet", DSL_CPE_CLI_PM_ReTxThresholds1DayGet, g_sPMrtt1dg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmrtt1ds", "PM_ReTxThresholds1DaySet", DSL_CPE_CLI_PM_ReTxThresholds1DaySet, g_sPMrtt1ds);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   /* Data path related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpfc15mg", "PM_DataPathFailureCounters15MinGet", DSL_CPE_CLI_PM_DataPathFailureCounters15MinGet, g_sPMdpfc15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpfc1dg", "PM_DataPathFailureCounters1DayGet", DSL_CPE_CLI_PM_DataPathFailureCounters1DayGet, g_sPMdpfc1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpfhs15mg", "PM_DataPathFailureHistoryStats15MinGet", DSL_CPE_CLI_PM_DataPathFailureHistoryStats15MinGet, g_sPMdpfhs15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpfhs1dg", "PM_DataPathFailureHistoryStats1DayGet", DSL_CPE_CLI_PM_DataPathFailureHistoryStats1DayGet, g_sPMdpfhs1dg);
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpfctg", "PM_DataPathFailureCountersTotalGet", DSL_CPE_CLI_PM_DataPathFailureCountersTotalGet, g_sPMdpfctg);
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmdpfcsg", "PM_DataPathFailureCountersShowtimeGet", DSL_CPE_CLI_PM_DataPathFailureCountersShowtimeGet, g_sPMdpfcsg);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   /* Data path related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlesc15mg", "PM_LineEventShowtimeCounters15MinGet", DSL_CPE_CLI_PM_LineEventShowtimeCounters15MinGet, g_sPMlesc15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlesc1dg", "PM_LineEventShowtimeCounters1DayGet", DSL_CPE_CLI_PM_LineEventShowtimeCounters1DayGet, g_sPMlesc1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmleshs15mg", "PM_LineEventShowtimeHistoryStats15MinGet", DSL_CPE_CLI_PM_LineEventShowtimeHistoryStats15MinGet, g_sPMleshs15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmleshs1dg", "PM_LineEventShowtimeHistoryStats1DayGet", DSL_CPE_CLI_PM_LineEventShowtimeHistoryStats1DayGet, g_sPMleshs1dg);
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlesctg", "PM_LineEventShowtimeCountersTotalGet", DSL_CPE_CLI_PM_LineEventShowtimeCountersTotalGet, g_sPMlesctg);
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlescsg", "PM_LineEventShowtimeCountersShowtimeGet", DSL_CPE_CLI_PM_LineEventShowtimeCountersShowtimeGet, g_sPMlescsg);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   /* Line init related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlic15mg", "PM_LineInitCounters15MinGet", DSL_CPE_CLI_PM_LineInitCounters15MinGet, g_sPMlic15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlic1dg", "PM_LineInitCounters1DayGet", DSL_CPE_CLI_PM_LineInitCounters1DayGet, g_sPMlic1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlihs15mg", "PM_LineInitHistoryStats15MinGet", DSL_CPE_CLI_PM_LineInitHistoryStats15MinGet, g_sPMlihs15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlihs1dg", "PM_LineInitHistoryStats1DayGet", DSL_CPE_CLI_PM_LineInitHistoryStats1DayGet, g_sPMlihs1dg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlictg", "PM_LineInitCountersTotalGet", DSL_CPE_CLI_PM_LineInitCountersTotalGet, g_sPMlictg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlicsg", "PM_LineInitCountersShowtimeGet", DSL_CPE_CLI_PM_LineInitCountersShowtimeGet, g_sPMlicsg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlit15mg", "PM_LineInitThresholds15MinGet", DSL_CPE_CLI_PM_LineInitThresholds15MinGet, g_sPMlit15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlit15ms", "PM_LineInitThresholds15MinSet", DSL_CPE_CLI_PM_LineInitThresholds15MinSet, g_sPMlit15ms);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlit1dg", "PM_LineInitThresholds1DayGet", DSL_CPE_CLI_PM_LineInitThresholds1DayGet, g_sPMlit1dg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlit1ds", "PM_LineInitThresholds1DaySet", DSL_CPE_CLI_PM_LineInitThresholds1DaySet, g_sPMlit1ds);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */

   /* Line seconds related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlsc15mg", "PM_LineSecCounters15MinGet", DSL_CPE_CLI_PM_LineSecCounters15MinGet, g_sPMlsc15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlsc1dg", "PM_LineSecCounters1DayGet", DSL_CPE_CLI_PM_LineSecCounters1DayGet, g_sPMlsc1dg);
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlshs15mg", "PM_LineSecHistoryStats15MinGet", DSL_CPE_CLI_PM_LineSecHistoryStats15MinGet, g_sPMlshs15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlshs1dg", "PM_LineSecHistoryStats1DayGet", DSL_CPE_CLI_PM_LineSecHistoryStats1DayGet, g_sPMlshs1dg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlsctg", "PM_LineSecCountersTotalGet", DSL_CPE_CLI_PM_LineSecCountersTotalGet, g_sPMlsctg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlscsg", "PM_LineSecCountersShowtimeGet", DSL_CPE_CLI_PM_LineSecCountersShowtimeGet, g_sPMlscsg);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlst15mg", "PM_LineSecThresholds15MinGet", DSL_CPE_CLI_PM_LineSecThresholds15MinGet, g_sPMlst15mg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlst15ms", "PM_LineSecThresholds15MinSet", DSL_CPE_CLI_PM_LineSecThresholds15MinSet, g_sPMlst15ms);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlst1dg", "PM_LineSecThresholds1DayGet", DSL_CPE_CLI_PM_LineSecThresholds1DayGet, g_sPMlst1dg);
   DSL_CPE_CLI_CMD_ADD_COMM ("pmlst1ds", "PM_LineSecThresholds1DaySet", DSL_CPE_CLI_PM_LineSecThresholds1DaySet, g_sPMlst1ds);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS */

#endif /* #ifdef INCLUDE_DSL_PM */

#ifdef DSL_DEBUG_TOOL_INTERFACE
   DSL_CPE_CLI_CMD_ADD_COMM ("tcpmistart", "TCPMessageInterfaceSTART", DSL_CPE_CLI_TCPMessageInterfaceSTART, g_sTcpStart);
   DSL_CPE_CLI_CMD_ADD_COMM ("tcpmistop", "TCPMessageInterfaceSTOP", DSL_CPE_CLI_TCPMessageInterfaceSTOP, g_sTcpStop);
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
   DSL_CPE_CLI_CMD_ADD_COMM ("dtistart", "DebugTraceInterfaceSTART", DSL_CPE_CLI_DebugTraceInterfaceSTART, g_sDtiStart);
   DSL_CPE_CLI_CMD_ADD_COMM ("dtistop", "DebugTraceInterfaceSTOP", DSL_CPE_CLI_DebugTraceInterfaceSTOP, g_sDtiStop);
#endif /* #if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/

#ifdef INCLUDE_PILOT_TONES_STATUS
   DSL_CPE_CLI_CMD_ADD_COMM ("ptsg", "PilotTonesStatusGet", DSL_CPE_CLI_PilotTonesStatusGet, g_sPtsg);
#endif /* #ifdef INCLUDE_PILOT_TONES_STATUS*/

   DSL_CPE_CLI_CMD_ADD_COMM ("rccs", "RebootCriteriaConfigSet", DSL_CPE_CLI_RebootCriteriaConfigSet, g_sRccs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("rccg", "RebootCriteriaConfigGet", DSL_CPE_CLI_RebootCriteriaConfigGet, g_sRccg);
#endif /* INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
   DSL_CPE_CLI_CMD_ADD_COMM ("lecg", "LastExceptionCodesGet", DSL_CPE_CLI_LastExceptionCodesGet, g_sLecg);
#endif /* #ifdef INCLUDE_DEVICE_EXCEPTION_CODES*/

#ifdef INCLUDE_REAL_TIME_TRACE
   DSL_CPE_CLI_CMD_ADD_COMM ("rtti", "RTT_Init", DSL_CPE_CLI_RTT_Init, g_sRtti);
   DSL_CPE_CLI_CMD_ADD_COMM ("rttcs", "RTT_ConfigSet", DSL_CPE_CLI_RTT_ConfigSet, g_sRttcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("rttcg", "RTT_ConfigGet", DSL_CPE_CLI_RTT_ConfigGet, g_sRttcg);
   DSL_CPE_CLI_CMD_ADD_COMM ("rttsg", "RTT_StatusGet", DSL_CPE_CLI_RTT_StatusGet, g_sRttsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("rttctrls", "RTT_ConTRoLSet", DSL_CPE_CLI_RTT_ControlSet, g_sRttctrls);
   DSL_CPE_CLI_CMD_ADD_COMM ("rttstatg", "RTT_STATisticsGet", DSL_CPE_CLI_RTT_StatisticsGet, g_sRttstatg);
#endif /*#ifdef INCLUDE_REAL_TIME_TRACE*/

   DSL_CPE_CLI_CMD_ADD_COMM ("osg", "OlrStatisticsGet", DSL_CPE_CLI_OlrStatisticsGet, g_sOsg);

   DSL_CPE_CLI_CMD_ADD_COMM ("llsg", "LoopLengthStatusGet", DSL_CPE_CLI_LoopLengthStatusGet, g_sLlsg);

#ifdef INCLUDE_DSL_FILTER_DETECTION
   DSL_CPE_CLI_CMD_ADD_COMM ("fddg", "FilterDetectionDataGet", DSL_CPE_CLI_FilterDetectionDataGet, g_sFddg);
#endif /* #ifdef INCLUDE_DSL_FILTER_DETECTION*/
   DSL_CPE_CLI_CMD_ADD_COMM ("hsdg", "HybridSelectionDataGet", DSL_CPE_CLI_HybridSelectionDataGet, g_sHsdg);

   DSL_CPE_CLI_DeviceCommandsRegister();

#ifdef INCLUDE_DSL_ADSL_MIB
   DSL_CPE_CLI_MibCommandsRegister();
#endif /* INCLUDE_DSL_ADSL_MIB*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
   DSL_CPE_CLI_CMD_ADD_COMM ("rtsg", "ReTxStatisticsGet", DSL_CPE_CLI_ReTxStatisticsGet, g_sRtsg);
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS */

   DSL_CPE_CLI_CMD_ADD_COMM ("dsnrg", "DeltSNRGet", DSL_CPE_CLI_DeltSNRGet,g_sDsnrg);
   DSL_CPE_CLI_CMD_ADD_COMM ("fdsg", "FirmwareDownloadStatusGet", DSL_CPE_CLI_FirmwareDownloadStatusGet, g_sFdsg);
}

#endif /* defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && !defined(INCLUDE_DSL_CPE_CLI_AUTOGEN_SUPPORT) */
