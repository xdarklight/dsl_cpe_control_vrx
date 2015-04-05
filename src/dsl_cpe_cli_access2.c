/******************************************************************************

                               Copyright (c) 2011
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL CLI, access function implementation
*/

#include "dsl_cpe_control.h"
#if defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && defined(INCLUDE_DSL_CPE_CLI_AUTOGEN_SUPPORT)

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

DSL_void_t DSL_CPE_CLI_AutogenCommandsRegister (DSL_void_t);

DSL_void_t DSL_CPE_CLI_DeviceCommandsRegister ( DSL_void_t );
#ifdef INCLUDE_DSL_ADSL_MIB
DSL_void_t DSL_CPE_CLI_MibCommandsRegister ( DSL_void_t );
#endif /* INCLUDE_DSL_ADSL_MIB*/

#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_CCA_debugModules_t nCcaDbgModule (dec)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_APP = 1" DSL_CPE_CRLF
   "   DSL_CCA_DBG_OS = 2" DSL_CPE_CRLF
   "   DSL_CCA_DBG_CLI = 3" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PIPE = 4" DSL_CPE_CRLF
   "   DSL_CCA_DBG_SOAP = 5" DSL_CPE_CRLF
   "   DSL_CCA_DBG_CONSOLE = 6" DSL_CPE_CRLF
   "   DSL_CCA_DBG_TDPMSG = 7" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_debugLevels_t nDbgLevel (hex)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_NONE = 0" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PRN = 1" DSL_CPE_CRLF
   "   DSL_CCA_DBG_ERR = 2" DSL_CPE_CRLF
   "   DSL_CCA_DBG_WRN = 40" DSL_CPE_CRLF
   "   DSL_CCA_DBG_MSG = 80" DSL_CPE_CRLF
   "   DSL_CCA_DBG_LOCAL = FF" DSL_CPE_CRLF
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
   "- DSL_CCA_debugLevels_t nDbgLevel (hex)" DSL_CPE_CRLF
   "   DSL_CCA_DBG_NONE = 0" DSL_CPE_CRLF
   "   DSL_CCA_DBG_PRN = 1" DSL_CPE_CRLF
   "   DSL_CCA_DBG_ERR = 2" DSL_CPE_CRLF
   "   DSL_CCA_DBG_WRN = 40" DSL_CPE_CRLF
   "   DSL_CCA_DBG_MSG = 80" DSL_CPE_CRLF
   "   DSL_CCA_DBG_LOCAL = FF" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

#ifdef INCLUDE_SCRIPT_NOTIFICATION
static const DSL_char_t g_sNsecs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t notify_script[1-256] (Attention: Use absolute firmware path only!)"
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sAcfgs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   DSL_uint32_t nDevice = 0;
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
   DSL_int_t nDeviceNum = -1;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 6, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
      else
      {
         for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
         {
            if(pCtx->fd[nDevice] == fd)
            {
               nDeviceNum = nDevice;
               break;
            }
         }
      }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   DSL_uint32_t nDevice = 0;
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
   DSL_int_t nDeviceNum = -1;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
         else
         {
            for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
            {
               if(pCtx->fd[nDevice] == fd)
               {
                  nDeviceNum = nDevice;
                  break;
               }
            }
         }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
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

static const DSL_char_t g_sAlf[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
#if defined(INCLUDE_DSL_CPE_API_DANUBE)
   "- DSL_char_t adsl_firmware[1-256] (Attention: Use absolute firmware path only,"
#elif defined(INCLUDE_DSL_CPE_API_VINAX)
   "- DSL_char_t vdsl_firmware[1-256] (Attention: Use absolute firmware path only,"
#else
   "- DSL_char_t firmware1[1-256] (Attention: Use absolute firmware path only,"
#endif
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   " enter '.' to let empty, to specify adsl_firmware only)" DSL_CPE_CRLF
   "- DSL_char_t adsl_firmware[1-256] (Attention: Use absolute firmware path only!)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootLoadFirmware(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_char_t sFirmwareName1[256] = { 0 };
   DSL_char_t *pcFw = DSL_NULL;
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   DSL_char_t *pcFw2 = DSL_NULL;
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) || !defined(INCLUDE_FW_REQUEST_SUPPORT)*/
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   DSL_char_t sFirmwareName2[256] = { 0 };
#endif
#ifdef INCLUDE_FW_REQUEST_SUPPORT
#if defined(INCLUDE_DSL_CPE_API_VINAX)
   DSL_AutobootControl_t autobootCtrl;
   DSL_AutobootStatus_t  AutobootStatus;
#endif /* defined(DSL_FIRMWARE_MEMORY_FREE_ENABLED) || defined(INCLUDE_DSL_CPE_API_VINAX)*/
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }
#else
   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }
#endif

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   DSL_CPE_sscanf (pCommands, "%s %s", sFirmwareName1, sFirmwareName2);
#else
   DSL_CPE_sscanf (pCommands, "%s", sFirmwareName1);
#endif

   /* Get Firmware binary 1 */
   if ((strlen(sFirmwareName1) > 0) && (strcmp(sFirmwareName1, ".") != 0))
   {
      pcFw = &sFirmwareName1[0];
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   /* Get Firmware binary 2 */
   if (strlen(sFirmwareName2) > 0)
   {
      pcFw2 = &sFirmwareName2[0];
   }
#endif

   /* Update links to the FW binaries */
   if (pcFw)
   {
      if (g_sFirmwareName1 != DSL_NULL)
      {
         DSL_CPE_Free(g_sFirmwareName1);
      }

      g_sFirmwareName1 = DSL_CPE_Malloc (strlen (pcFw) + 1);
      if (g_sFirmwareName1)
      {
         strcpy (g_sFirmwareName1, pcFw);
      }
      DSL_CPE_FwFeaturesGet(g_sFirmwareName1, &g_nFwFeatures1);
   }

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   if (pcFw2)
   {
      if (g_sFirmwareName2 != DSL_NULL)
      {
         DSL_CPE_Free(g_sFirmwareName2);
      }

      g_sFirmwareName2 = DSL_CPE_Malloc (strlen (pcFw2) + 1);
      if (g_sFirmwareName2)
      {
         strcpy (g_sFirmwareName2, pcFw2);
      }
      DSL_CPE_FwFeaturesGet(g_sFirmwareName2, &g_nFwFeatures2);
   }
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)*/

#ifdef INCLUDE_FW_REQUEST_SUPPORT

#if defined(INCLUDE_DSL_CPE_API_DANUBE)
   ret = DSL_CPE_DownloadFirmware(fd, DSL_FW_REQUEST_NA, pcFw, DSL_NULL);

   DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(ret));
   if (ret < DSL_SUCCESS)
   {
      DSL_CPE_FPrintf(out, " (Have you used absolute firmware path?)" DSL_CPE_CRLF);
   }
#else

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_STATUS_GET, (int) &AutobootStatus);

   if ((ret < 0) && (AutobootStatus.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(autobootCtrl.accessCtl.nReturn));
      return 0;
   }

   if (AutobootStatus.data.nStatus == DSL_AUTOBOOT_STATUS_FW_WAIT)
   {
      ret = DSL_CPE_DownloadFirmware(fd, AutobootStatus.data.nFirmwareRequestType, pcFw, pcFw2);

      DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(ret));
      if (ret < DSL_SUCCESS)
      {
         DSL_CPE_FPrintf(out, " (Have you used absolute firmware path?)" DSL_CPE_CRLF);
      }
   }
   else
   {
      /* Trigger Autoboot restart sequence. FW binary will be requested
         from the DSL CPE API driver later*/
      autobootCtrl.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART;

      ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_CONTROL_SET, (int) &autobootCtrl);

      if ((ret < 0) && (autobootCtrl.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CPE_FPrintf (out, sFailureReturn, autobootCtrl.accessCtl.nReturn);
      }
      else
      {
         DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
      }
   }
#endif /* defined(INCLUDE_DSL_CPE_API_DANUBE) && !defined(DSL_FIRMWARE_MEMORY_FREE_ENABLED)*/

#else
   ret = DSL_CPE_DownloadFirmware(fd, DSL_FW_REQUEST_NA, pcFw, pcFw2);

   DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(ret));
   if (ret < DSL_SUCCESS)
   {
      DSL_CPE_FPrintf(out, " (Have you used absolute firmware path?)" DSL_CPE_CRLF);
   }
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/

   return 0;
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
static const DSL_char_t g_sRusg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
            DSL_CPE_FPrintf (out, sFailureReturn, ret);
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

static const DSL_char_t g_sIcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
            if (!pCtx->bEvtRun)
            {
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

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
static const DSL_char_t g_sSe[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t sFileName[1-" _MKSTR(DSL_MAX_COMMAND_LINE_LENGTH) "] (full path to the script)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
static const DSL_char_t g_sBbsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

#ifdef INCLUDE_DSL_G997_PER_TONE
static const DSL_char_t g_sG997bang[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
      for (i = 0; i < pData->data.bitAllocationNsc.nNumData; i++)
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
         for (i = 0; i < pData->data.bitAllocationNsc.nNumData; i++)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
      DSL_CPE_FPrintf (out, sFailureReturn, pData->accessCtl.nReturn);
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
         for (i = 0; i < pData->data.bitAllocationNsc.nNumData; i++)
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

#ifdef INCLUDE_DSL_G997_PER_TONE
static const DSL_char_t g_sG997gang[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
         for (i = 0; i < pData->data.gainAllocationNsc.nNumData; i++)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

      for (i = 0; i < pData->data.gainAllocationNsc.nNumData; i++)
      {
         DSL_CPE_FPrintf(out, "%04X ", (DSL_int_t)pData->data.gainAllocationNsc.nNSCData[i]);
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);
   }
   free(pData);
   return 0;
}
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
static const DSL_char_t g_sG997lig[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_XTUDir_t nDirection" DSL_CPE_CRLF
   "   near end = 0" DSL_CPE_CRLF
   "   far end = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

#ifdef INCLUDE_DSL_CEOC
static const DSL_char_t g_sG997sms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t nMessageData[1-" _MKSTR(DSL_G997_SNMP_MESSAGE_LENGTH) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
      DSL_CPE_FPrintf (out, sFailureReturn, pData.accessCtl.nReturn);
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
      for (i = 0; i < pData.data.nMessageLength; i++)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_AccessDir_t nDirection" DSL_CPE_CRLF
   "   upstream = 0" DSL_CPE_CRLF
   "   downstream = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
         for (i = 0; i < pData->data.snrAllocationNsc.nNumData; i++)
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
static const DSL_char_t g_sG997dhling[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
         for (i = 0; i < pData->data.deltHlin.nNumData; i++)
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

static const DSL_char_t g_sG997dhlogg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
         for (i = 0; i < pData->data.deltHlog.nNumData; i++)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
      for (i = 0; i < pData->data.deltQln.nNumData; i++)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
      for (i = 0; i < pData->data.deltSnr.nNumData; i++)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, pData.accessCtl.nReturn);
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_LineOptionsSelector_t nConfigSelector (dec)" DSL_CPE_CRLF
   "   DSL_OPT_NOISE_MARGIN_DELTA_DS = 0" DSL_CPE_CRLF
   "   DSL_ERASURE_DECODING_TYPE_DS = 1" DSL_CPE_CRLF
   "   DSL_TRUST_ME_BIT = 2" DSL_CPE_CRLF
   "   DSL_INBAND_SPECTRAL_SHAPING_US = 3" DSL_CPE_CRLF
   "- DSL_int32_t nConfigValue (dec)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   DSL_CPE_CRLF "";
#else
   "";
#endif

#ifdef INCLUDE_DSL_PM
static const DSL_char_t g_sPMr[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineFailureCounters15MinGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineFailureCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_FAILURE_COUNTERS_15MIN_GET, (int) &pData);

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
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineFailureCounters1DayGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineFailureCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_FAILURE_COUNTERS_1DAY_GET, (int) &pData);

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
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineFailureHistoryStats15MinGet(
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

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_FAILURE_HISTORY_STATS_15MIN_GET,
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
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineFailureHistoryStats1DayGet(
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

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret =  DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_FAILURE_HISTORY_STATS_1DAY_GET,
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
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#endif /* INCLUDE_DSL_CPE_PM_DATA_LINE_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineFailureCountersTotalGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineFailureCountersTotal_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_FAILURE_COUNTERS_TOTAL_GET, (int) &pData);

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
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_PM_LineFailureCountersShowtimeGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_PM_LineFailureCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 2, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u %u", &pData.nDirection, &pData.nHistoryInterval);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_PM_LINE_FAILURE_COUNTERS_SHOWTIME_GET, (int) &pData);

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
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_PM */

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
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
   DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
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

   DSL_CPE_CLI_CMD_ADD_COMM ("alf", "AutobootLoadFirmware", DSL_CPE_CLI_AutobootLoadFirmware, g_sAlf);
#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
   DSL_CPE_CLI_CMD_ADD_COMM ("rusg", "ResourceUsageStatisticsGet", DSL_CPE_CLI_ResourceUsageStatisticsGet, g_sRusg);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

   DSL_CPE_CLI_CMD_ADD_COMM ("ics", "InstanceControlSet", DSL_CPE_CLI_InstanceControlSet, g_sIcs);
#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
   DSL_CPE_CLI_CMD_ADD_COMM ("bbsg", "BandBorderStatusGet", DSL_CPE_CLI_BandBorderStatusGet, g_sBbsg);
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/
   /* G.997.1 related functionlities */

#ifdef INCLUDE_DSL_G997_PER_TONE
   DSL_CPE_CLI_CMD_ADD_COMM ("g997bang", "G997_BitAllocationNscGet", DSL_CPE_CLI_G997_BitAllocationNscGet, g_sG997bang);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997bansg", "G997_BitAllocationNscShortGet", DSL_CPE_CLI_G997_BitAllocationNscShortGet, g_sG997bansg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997gang", "G997_GainAllocationNscGet", DSL_CPE_CLI_G997_GainAllocationNscGet, g_sG997gang);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997gansg", "G997_GainAllocationNscShortGet", DSL_CPE_CLI_G997_GainAllocationNscShortGet, g_sG997gansg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997sang", "G997_SnrAllocationNscGet", DSL_CPE_CLI_G997_SnrAllocationNscGet, g_sG997sang);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997sansg", "G997_SnrAllocationNscShortGet", DSL_CPE_CLI_G997_SnrAllocationNscShortGet, g_sG997sansg);
#endif /* INCLUDE_DSL_G997_PER_TONE*/

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lig", "G997_LineInventoryGet", DSL_CPE_CLI_G997_LineInventoryGet, g_sG997lig);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lis", "G997_LineInventorySet", DSL_CPE_CLI_G997_LineInventorySet, g_sG997lis);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997listrg", "G997_LineInventorySTRingGet", DSL_CPE_CLI_G997_LineInventoryStringGet, g_sG997listrg);
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_CEOC
   DSL_CPE_CLI_CMD_ADD_COMM ("g997sms", "G997_SnmpMessageSend", DSL_CPE_CLI_G997_SnmpMessageSend, g_sG997sms);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997smr", "G997_SnmpMessageReceive", DSL_CPE_CLI_G997_SnmpMessageReceive, g_sG997smr);
#endif /* INCLUDE_DSL_CEOC */
#ifdef INCLUDE_DSL_DELT
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dhling", "G997_DeltHLINGet", DSL_CPE_CLI_G997_DeltHLINGet, g_sG997dhling);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dhlogg", "G997_DeltHLOGGet", DSL_CPE_CLI_G997_DeltHLOGGet,g_sG997dhlogg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dqlng", "G997_DeltQLNGet", DSL_CPE_CLI_G997_DeltQLNGet,g_sG997dqlng);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997dsnrg", "G997_DeltSNRGet", DSL_CPE_CLI_G997_DeltSNRGet,g_sG997dsnrg);
#endif /* INCLUDE_DSL_DELT*/
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997xtusecg", "G997_XTUSystemEnablingConfigGet", DSL_CPE_CLI_G997_XTUSystemEnablingConfigGet, g_sG997xtusecg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("g997xtusecs", "G997_XTUSystemEnablingConfigSet", DSL_CPE_CLI_G997_XTUSystemEnablingConfigSet, g_sG997xtusecs);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997xtusesg", "G997_XTUSystemEnablingStatusGet", DSL_CPE_CLI_G997_XTUSystemEnablingStatusGet, g_sG997xtusesg);


#ifdef INCLUDE_DSL_PM
   /* PM related functionalities */
   DSL_CPE_CLI_CMD_ADD_COMM ("pmr", "PM_Reset", DSL_CPE_CLI_PM_Reset, g_sPMr);

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   /* Data path related counters */
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
   DSL_CPE_CLI_CMD_ADD_DEPR ("pmlfc15mg", "PM_LineFailureCounters15MinGet", DSL_CPE_CLI_PM_LineFailureCounters15MinGet, g_sPMlesc15mg);
   DSL_CPE_CLI_CMD_ADD_DEPR ("pmlfc1dg", "PM_LineFailureCounters1DayGet", DSL_CPE_CLI_PM_LineFailureCounters1DayGet, g_sPMlesc1dg);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/

#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
   DSL_CPE_CLI_CMD_ADD_DEPR ("pmlfhs15mg", "PM_LineFailureHistoryStats15MinGet", DSL_CPE_CLI_PM_LineFailureHistoryStats15MinGet, g_sPMleshs15mg);
   DSL_CPE_CLI_CMD_ADD_DEPR ("pmlfhs1dg",  "PM_LineFailureHistoryStats1DayGet", DSL_CPE_CLI_PM_LineFailureHistoryStats1DayGet, g_sPMleshs1dg);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY */
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
   DSL_CPE_CLI_CMD_ADD_DEPR ("pmlfctg", "PM_LineFailureCountersTotalGet", DSL_CPE_CLI_PM_LineFailureCountersTotalGet, g_sPMlesctg);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS */
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
   DSL_CPE_CLI_CMD_ADD_DEPR ("pmlfcsg", "PM_LineFailureCountersShowtimeGet", DSL_CPE_CLI_PM_LineFailureCountersShowtimeGet, g_sPMlescsg);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED*/
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */

#endif /* #ifdef INCLUDE_DSL_PM */

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
   DSL_CPE_CLI_CMD_ADD_COMM ("dtistart", "DebugTraceInterfaceSTART", DSL_CPE_CLI_DebugTraceInterfaceSTART, g_sDtiStart);
   DSL_CPE_CLI_CMD_ADD_COMM ("dtistop", "DebugTraceInterfaceSTOP", DSL_CPE_CLI_DebugTraceInterfaceSTOP, g_sDtiStop);
#endif /* #if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/

#ifdef INCLUDE_PILOT_TONES_STATUS
   DSL_CPE_CLI_CMD_ADD_COMM ("ptsg", "PilotTonesStatusGet", DSL_CPE_CLI_PilotTonesStatusGet, g_sPtsg);
#endif /* #ifdef INCLUDE_PILOT_TONES_STATUS*/

   DSL_CPE_CLI_AutogenCommandsRegister();

   DSL_CPE_CLI_DeviceCommandsRegister();

#ifdef INCLUDE_DSL_ADSL_MIB
   DSL_CPE_CLI_MibCommandsRegister();
#endif /* INCLUDE_DSL_ADSL_MIB*/
}

#endif /* #if defined(INCLUDE_DSL_CPE_CLI_SUPPORT) && defined(INCLUDE_DSL_CPE_CLI_AUTOGEN_SUPPORT) */
