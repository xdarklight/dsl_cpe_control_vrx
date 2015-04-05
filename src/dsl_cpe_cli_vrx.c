/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   VRX specific DSL CLI, access function implementation
*/

#include "dsl_cpe_control.h"

#if defined(INCLUDE_DSL_CPE_API_VRX)

#include "dsl_cpe_cli.h"
#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_ioctl.h"

/* MEI CPE driver specific headers*/
#include "drv_mei_cpe_interface.h"

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT

#ifdef INCLUDE_DSL_BONDING
#include "dsl_cpe_bnd_vrx.h"
#endif /* #ifdef INCLUDE_DSL_BONDING */

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_CLI

extern const char *sFailureReturn;

/* for debugging: */
#ifdef DSL_CLI_LOCAL
#undef DSL_CLI_LOCAL
#endif
#if 0
#define DSL_CLI_LOCAL
#else
#define DSL_CLI_LOCAL static
#endif


static const DSL_char_t g_sDms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nMsgID (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nIndex (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nLength (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nPayload[0-256] (hex, for 32-bit messages payload contains maximum 128 32-bit values, leading 0's can be skipped)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nMsgID (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nIndex (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nLength (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nPayload[256] (hex, for 32-bit messages payload contains maximum 128 32-bit values)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_DeviceMessageSend(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   /* maximum message size includinf ID, Index and Length field */
   #define VRX_MSG_LEN  262
   /**
      This bit of the Message ID identifies 32 bit aligned TC messages */
   #define VDSL2_MBX_MSG_ID_IFX_MASK 0x0010

   DSL_int_t ret = 0;
   DSL_DeviceMessage_t DevMsg;
   DSL_uint8_t msg[VRX_MSG_LEN];
   DSL_uint16_t *pMsg16 = (DSL_uint16_t*)&msg[0];
   DSL_uint16_t const *pMsgId = (DSL_uint16_t*)&msg[0];
   DSL_uint32_t *pMsg32 = (DSL_uint32_t*)&msg[6];
   DSL_char_t *pChar;
   DSL_uint32_t nParam = 0;
   DSL_uint32_t i = 0;
   DSL_boolean_t  bXtcMsg = DSL_FALSE;


   if (pCommands == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF ,
         DSL_CPE_RET_VAL(DSL_ERROR));
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   pChar = pCommands;

   /* get MsgId, Index and Length parameter */
   while (pChar != DSL_NULL)
   {
      /* Extract next parameter */
      if (DSL_CPE_sscanf(pChar, "%x", &nParam) > 0)
      {
         i += 2;
         *pMsg16 = (DSL_uint16_t)nParam;
         /* Find next parameter */
         pChar = strpbrk( pChar, " ");
         if (pChar != DSL_NULL)
         {
            pChar++;
            pMsg16++;
         }
         else
         {
            break;
         }
      }
      else
      {
         break;
      }

      if (i >= 6) break;
   }

   /* decide on the MsgId wether to treat 16 or 32bit params */
   if (*pMsgId & VDSL2_MBX_MSG_ID_IFX_MASK)
      bXtcMsg = DSL_TRUE;


   while (pChar != DSL_NULL)
   {
      /* Extract next parameter */
      if (DSL_CPE_sscanf(pChar, "%x", &nParam) > 0)
      {
         if (bXtcMsg == DSL_TRUE)
         {
            i += 4;
            *pMsg32 = (DSL_uint32_t)nParam;
         }
         else
         {
            i += 2;
            *pMsg16 = (DSL_uint16_t)nParam;
         }
         /* Find next parameter */
         pChar = strpbrk( pChar, " ");
         if (pChar != DSL_NULL)
         {
            pChar++;
            if (bXtcMsg == DSL_TRUE)
               pMsg32++;
            else
               pMsg16++;
         }
         else
         {
            break;
         }
      }
      else
      {
         break;
      }

      if (i >= VRX_MSG_LEN) break;
   }
   if ( (i > 0) && (i < VRX_MSG_LEN) )
   {
      /* Firmware message must consist of MsgID and two parameters minimum (6 bytes)*/
      if (i < 6)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_RET"(firmware message has to be at least 3 "
            "hex parameters long - '0x' does not have to be written)" DSL_CPE_CRLF ,
            DSL_CPE_RET_VAL(DSL_ERROR));
         return -1;
      }
      else
      {
         memset(&DevMsg, 0x0, sizeof(DSL_DeviceMessage_t));

         DevMsg.data.pMsg    = (DSL_uint8_t*)&msg;
         DevMsg.data.nSizeTx = (DSL_uint16_t)i;
         DevMsg.data.nSizeRx = (DSL_uint16_t)VRX_MSG_LEN;

         ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int) &DevMsg);

         if ((ret < 0) && (DevMsg.accessCtl.nReturn < DSL_SUCCESS))
         {
            DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(ret));
            return ret;
         }
         else
         {
            DSL_CPE_FPrintf(out, DSL_CPE_RET"nData=\"", DSL_CPE_RET_VAL(ret));
            pMsg16 = (DSL_uint16_t*)&msg[0];
            pMsg32 = (DSL_uint32_t*)&msg[6];
            /* print Msg ID, Index and Length*/
            DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
            pMsg16++;
            DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
            pMsg16++;
            DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
            pMsg16++;
            i = 6;
            if (bXtcMsg == DSL_TRUE)
            {
               while (i < DevMsg.data.nSizeRx)
               {
                  DSL_CPE_FPrintf(out, "%08X ", *pMsg32 );
                  pMsg32++;
                  i += 4;
               }
            }
            else
            {
               while (i < DevMsg.data.nSizeRx)
               {
                  DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
                  pMsg16++;
                  i += 2;
               }
            }

            DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);
            return 0;
         }
      }

   }
   else
   {
      DSL_CPE_FPrintf(out, "DSL: error during processing of message data." DSL_CPE_CRLF );
      ret = -1;
   }

   return ret;
}

static const DSL_char_t g_sDmms[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nMsgID (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nIndex (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nLength (hex, N)" DSL_CPE_CRLF
   "- DSL_uint16_t nPayload[0-256] (hex, <data_1> <mask_1> ... <data_N> <mask_N>)" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint16_t nMsgID (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nIndex (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nLength (hex)" DSL_CPE_CRLF
   "- DSL_uint16_t nPayload[256] (hex, for 32-bit messages payload contains maximum 128 32-bit values)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DBG_DeviceMessageModifySend(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   /* maximum mask size */
   #define VRX_MASK_LEN  (VRX_MSG_LEN - 6 /*message header size = 6*/)

   DSL_int_t ret = 0;
   DSL_DeviceMessageModify_t DevMsg;
   DSL_uint8_t msg[VRX_MSG_LEN];
   DSL_uint16_t *pMsg16 = (DSL_uint16_t*)&msg[0];
   DSL_uint16_t const *pMsgId = (DSL_uint16_t*)&msg[0];
   DSL_uint32_t *pMsg32 = (DSL_uint32_t*)&msg[6];

   DSL_uint8_t msk[VRX_MASK_LEN];
   DSL_uint16_t *pMask16 = (DSL_uint16_t*)&msk[0];
   DSL_uint32_t *pMask32 = (DSL_uint32_t*)&msk[0];

   DSL_char_t *pChar;
   DSL_uint32_t nParam = 0;
   DSL_uint32_t i = 0;
   DSL_boolean_t  bXtcMsg = DSL_FALSE;
   DSL_boolean_t  bProcMask = DSL_FALSE;

   if (pCommands == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF ,
         DSL_CPE_RET_VAL(DSL_ERROR));
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 3, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   pChar = pCommands;

   memset(msg,0,VRX_MSG_LEN);
   memset(msk,0,VRX_MASK_LEN);

   /* get MsgId, Index and Length parameter */
   while (pChar != DSL_NULL)
   {
      /* Extract next parameter */
      if (DSL_CPE_sscanf(pChar, "%x", &nParam) > 0)
      {
         i += 2;
         *pMsg16 = (DSL_uint16_t)nParam;
         /* Find next parameter */
         pChar = strpbrk( pChar, " ");
         if (pChar != DSL_NULL)
         {
            pChar++;
            pMsg16++;
         }
         else
         {
            break;
         }
      }
      else
      {
         break;
      }

      if (i >= 6) break;
   }

   /* decide on the MsgId wether to treat 16 or 32bit params */
   if (*pMsgId & VDSL2_MBX_MSG_ID_IFX_MASK)
      bXtcMsg = DSL_TRUE;


   while (pChar != DSL_NULL)
   {
      /* Extract next parameter */
      if (DSL_CPE_sscanf(pChar, "%x", &nParam) > 0)
      {
         if (bProcMask == DSL_TRUE)
         {
            if (bXtcMsg == DSL_TRUE)
            {
               *pMask32 = (DSL_uint32_t)nParam;
               pMask32++;
            }
            else
            {
               *pMask16 = (DSL_uint16_t)nParam;
               pMask16++;
            }

            bProcMask = DSL_FALSE;
         } else
         {
            if (bXtcMsg == DSL_TRUE)
            {
               i += 4;
               *pMsg32 = (DSL_uint32_t)nParam;
               pMsg32++;
            }
            else
            {
               i += 2;
               *pMsg16 = (DSL_uint16_t)nParam;
               pMsg16++;
            }

            bProcMask = DSL_TRUE;
         }

         /* Find next parameter */
         pChar = strpbrk( pChar, " ");
         if (pChar != DSL_NULL)
         {
            pChar++;
         }
         else
         {
            break;
         }
      } else
      {
         break;
      }

      if (i >= VRX_MSG_LEN) break;
   }
   if ( (i > 0) && (i < VRX_MSG_LEN) )
   {
      /* Firmware message must consist of MsgID and two parameters minimum (6 bytes)*/
      if (i < 6)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_RET"(firmware message has to be at least 3 "
            "hex parameters long - '0x' does not have to be written)" DSL_CPE_CRLF ,
            DSL_CPE_RET_VAL(DSL_ERROR));
         return -1;
      }
      else
      {
         memset(&DevMsg, 0x0, sizeof(DSL_DeviceMessageModify_t));

         DevMsg.data.pMsg    = (DSL_uint8_t*)&msg;
         DevMsg.data.pMsgMask= (DSL_uint8_t*)&msk;
         DevMsg.data.nSizeTx = (DSL_uint16_t)i;
         DevMsg.data.nSizeRx = (DSL_uint16_t)VRX_MSG_LEN;

         ret = DSL_CPE_Ioctl (fd, DSL_FIO_DBG_DEVICE_MESSAGE_MODIFY_SEND, (int) &DevMsg);

         if ((ret < 0) && (DevMsg.accessCtl.nReturn < DSL_SUCCESS))
         {
            DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(ret));
            return ret;
         }
         else
         {
            /*Check that all data values passed with mask*/
            if (bProcMask == DSL_TRUE)
            {
               DSL_CPE_FPrintf(out,"Last data value passed without mask."DSL_CPE_CRLF);
               ret = DSL_WRN_CONFIG_PARAM_IGNORED;
            } else
            {
               ret = 0;
            }

            DSL_CPE_FPrintf(out, DSL_CPE_RET"nData=\"", DSL_CPE_RET_VAL(ret));
            pMsg16 = (DSL_uint16_t*)&msg[0];
            pMsg32 = (DSL_uint32_t*)&msg[6];
            /* print Msg ID, Index and Length*/
            DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
            pMsg16++;
            DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
            pMsg16++;
            DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
            pMsg16++;
            i = 6;
            if (bXtcMsg == DSL_TRUE)
            {
               while (i < DevMsg.data.nSizeRx)
               {
                  DSL_CPE_FPrintf(out, "%08X ", *pMsg32 );
                  pMsg32++;
                  i += 4;
               }
            }
            else
            {
               while (i < DevMsg.data.nSizeRx)
               {
                  DSL_CPE_FPrintf(out, "%04X ", *pMsg16 );
                  pMsg16++;
                  i += 2;
               }
            }

            DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF);
            return 0;
         }
      }

   }
   else
   {
      DSL_CPE_FPrintf(out, "DSL: error during processing of message data." DSL_CPE_CRLF );
      ret = -1;
   }

   return ret;
}

static const DSL_char_t g_sAufg[] =
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
   "- DSL_char_t xdsl_firmware[1-256]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootUsedFirmwareGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_CPE_Control_Context_t *pCtrlCtx = DSL_NULL;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pCtrlCtx = DSL_CPE_GetGlobalContext();
   if(pCtrlCtx == DSL_NULL)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
   }

#ifdef DSL_CPE_SOAP_FW_UPDATE
   if ((pCtrlCtx->firmware.pData != DSL_NULL) && (pCtrlCtx->firmware.nSize > 0))
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"xdsl_firmware=<%s>" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(ret),
         (DSL_char_t *)DSL_CPE_SOAP_FWNAME_1);
   }
   else
   {
#endif /* DSL_CPE_SOAP_FW_UPDATE */
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"xdsl_firmware=%s" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(ret),
         g_sFirmwareName1 != DSL_NULL ? g_sFirmwareName1 : "n/a");
#ifdef DSL_CPE_SOAP_FW_UPDATE
   }
#endif /* DSL_CPE_SOAP_FW_UPDATE */

   return 0;
}

static const DSL_char_t g_sAlf[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_char_t xdsl_firmware[1-256] (Attention: Use absolute firmware path only"DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AutobootLoadFirmware(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_char_t sFirmwareName1[256] = { 0 };
   DSL_char_t *pcFw = DSL_NULL;
#ifdef INCLUDE_FW_REQUEST_SUPPORT
   DSL_AutobootControl_t autobootCtrl;
#if (DSL_CPE_LINES_PER_DEVICE < 2)
   DSL_AutobootStatus_t  AutobootStatus;
#endif /* #if (DSL_CPE_LINES_PER_DEVICE < 2) */
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%s", sFirmwareName1);

   /* Get Firmware binary 1 */
   if (strlen(sFirmwareName1) > 0)
   {
      pcFw = &sFirmwareName1[0];
   }

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

#ifdef INCLUDE_FW_REQUEST_SUPPORT
#if (DSL_CPE_LINES_PER_DEVICE < 2)
   memset(&autobootCtrl, 0x00, sizeof(DSL_AutobootControl_t));
   memset(&AutobootStatus, 0x00, sizeof(DSL_AutobootStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_STATUS_GET, (int) &AutobootStatus);

   if ((ret < 0) && (AutobootStatus.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(autobootCtrl.accessCtl.nReturn));
      return 0;
   }

   if (AutobootStatus.data.nStatus == DSL_AUTOBOOT_STATUS_FW_WAIT)
   {
      ret = DSL_CPE_DownloadFirmware(fd, AutobootStatus.data.nFirmwareRequestType, pcFw, DSL_NULL);

      DSL_CPE_FPrintf (out, DSL_CPE_RET, DSL_CPE_RET_VAL(ret));
      if (ret < DSL_SUCCESS)
      {
         DSL_CPE_FPrintf(out, " (Have you used absolute firmware path?)" DSL_CPE_CRLF);
      }
   }
   else
#endif
   {
      /* Trigger Autoboot restart sequence. FW binary will be requested
         from the DSL CPE API driver later*/
      autobootCtrl.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART_FULL;

      ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_CONTROL_SET, (int) &autobootCtrl);

      if ((ret < 0) && (autobootCtrl.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(autobootCtrl.accessCtl.nReturn));
      }
      else
      {
         DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
      }
   }
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

static const DSL_char_t g_sAlig[] =
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
   "- DSL_AuxLineInventory_t nData[" _MKSTR(DSL_G993_LI_MAXLEN_AUX)"]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_AuxLineInventoryGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_uint32_t i = 0;
   DSL_AuxLineInventory_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   memset(&pData, 0x0, sizeof(DSL_AuxLineInventory_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUX_LINE_INVENTORY_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nLength=%u nData=\"" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         (unsigned int)pData.data.nLength);

      for (i = 0; i < pData.data.nLength; i++)
      {
         if (i%10 == 0) DSL_CPE_FPrintf ( out, DSL_CPE_CRLF);
         DSL_CPE_FPrintf ( out, "%02x ", pData.data.pData[i]);
      }
      DSL_CPE_FPrintf ( out, "\""DSL_CPE_CRLF);
   }

   return 0;
}

#define DSL_CPE_CLI_MAX_PROFILES_PER_BANDS   2030

static const DSL_char_t g_sBpsg[] =
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
   "- DSL_uint32_t nBandPlan" DSL_CPE_CRLF
   "- DSL_uint32_t nProfile" DSL_CPE_CRLF
   "- DSL_boolean_t bSupported" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   "- ... nParamNr[3] nCount[" _MKSTR(DSL_CPE_CLI_MAX_PROFILES_PER_BANDS) "]" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BandPlanSupportGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BandPlanSupport_t *pData = DSL_NULL;
   DSL_int_t nBandPlan = 0, nProfile = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   pData = (DSL_BandPlanSupport_t*)DSL_CPE_Malloc(sizeof(DSL_BandPlanSupport_t));
   if (pData == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERR_MEMORY));
      return 0;
   }

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BAND_PLAN_SUPPORT_GET, (int) pData);

   if ((ret < 0) && (pData->accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData->accessCtl.nReturn));
      DSL_CPE_FPrintf (out, "nFormat=(nBandPlan(dec),nProfile(dec),bSupported(dec)) nData=\"" DSL_CPE_CRLF);

      for (nBandPlan = 0; nBandPlan < DSL_BANDPLAN_LAST; nBandPlan++)
      {
         for (nProfile = 0; nProfile < DSL_PROFILE_LAST; nProfile++)
         {
            DSL_CPE_FPrintf (out, "(%02d,%02d,%d) ", nBandPlan, nProfile,
               pData->data.bSupported[nBandPlan][nProfile]);
         }
         DSL_CPE_FPrintf (out, DSL_CPE_CRLF );
      }
      DSL_CPE_FPrintf(out, "\"" DSL_CPE_CRLF );
   }

   DSL_CPE_Free(pData);

   return 0;
}

static const DSL_char_t g_sBpstg[] =
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
   "- DSL_uint32_t nBandPlan (deprecated)" DSL_CPE_CRLF
   "   DSL_BANDPLAN_NA = -1" DSL_CPE_CRLF
   "- DSL_uint32_t nProfile" DSL_CPE_CRLF
   "   DSL_PROFILE_8A = 0" DSL_CPE_CRLF
   "   DSL_PROFILE_8B = 1" DSL_CPE_CRLF
   "   DSL_PROFILE_8C = 2" DSL_CPE_CRLF
   "   DSL_PROFILE_8D = 3" DSL_CPE_CRLF
   "   DSL_PROFILE_12A = 4" DSL_CPE_CRLF
   "   DSL_PROFILE_12B = 5" DSL_CPE_CRLF
   "   DSL_PROFILE_17A = 6" DSL_CPE_CRLF
   "   DSL_PROFILE_30A = 7" DSL_CPE_CRLF
   "   DSL_PROFILE_17B = 8" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BandPlanStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BandPlanStatus_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_BandPlanStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BAND_PLAN_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"nBandPlan=%d nProfile=%d" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nBandPlan, pData.data.nProfile);
   }

   return 0;
}

static const DSL_char_t g_sVig[] =
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
   "- DSL_char_t DSL_DriverVersionApi[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_ChipSetFWVersion[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_ChipSetHWVersion[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_ChipSetType[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_char_t DSL_DriverVersionMeiBsp[" _MKSTR(MAX_INFO_STRING_LEN)"]"DSL_CPE_CRLF
   "- DSL_HybridType_t nHybridType"DSL_CPE_CRLF
   "   unknown = -1"DSL_CPE_CRLF
   "   na = 0"DSL_CPE_CRLF
   "   pots = 1"DSL_CPE_CRLF
   "   isdn = 2"DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_VersionInformationGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_VersionInformation_t pData;
   DSL_CPE_Control_Context_t *pCtrlCtx = DSL_NULL;
   DSL_char_t pAppPatch[8];

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_VersionInformation_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_VERSION_INFORMATION_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(ret));
   }
   else
   {
      pCtrlCtx = DSL_CPE_GetGlobalContext();
      if(pCtrlCtx == DSL_NULL)
      {
         DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
         return -1;
      }

      if ((pCtrlCtx->driverVer.nMajor != pCtrlCtx->applicationVer.nMajor) ||
          (pCtrlCtx->driverVer.nMinor != pCtrlCtx->applicationVer.nMinor))
      {
         pData.accessCtl.nReturn = DSL_WRN_VERSION_INCOMPATIBLE;
      }

      pAppPatch[0] = 0;

#ifndef DISABLE_DSL_PATCH_VERSION
      if (pCtrlCtx->applicationVer.nPatch != 0)
      {
         snprintf (pAppPatch, sizeof(pAppPatch), "%spa%d", (pCtrlCtx->driverVer.nPatch == 0) ? "-" : "",
            pCtrlCtx->applicationVer.nPatch);
      }
#endif /*#ifndef DISABLE_DSL_PATCH_VERSION*/

      DSL_CPE_FPrintf (out,
         DSL_CPE_RET"DSL_DriverVersionApi=%s%s DSL_ChipSetFWVersion=%s "
         "DSL_ChipSetHWVersion=%s DSL_ChipSetType=%s DSL_DriverVersionMeiBsp=%s "
         "nHybrid=%d"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.DSL_DriverVersionApi, pAppPatch,
         pData.data.DSL_ChipSetFWVersion, pData.data.DSL_ChipSetHWVersion,
         pData.data.DSL_ChipSetType, pData.data.DSL_DriverVersionMeiBsp,
         pData.data.nHybridType);
   }

   return 0;
}

#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
static const DSL_char_t g_sDscs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DeviceSelectionConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_uint32_t nDevice = 0;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &nDevice);

   if (nDevice >= DSL_CPE_MAX_DSL_ENTITIES)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
   }
   else
   {
      if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
      {
         pCtx->nDevNum = nDevice;
         DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(0));
      }
      else
      {
         DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
      }
   }


   return 0;
}

static const DSL_char_t g_sDscg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
   "- DSL_uint32_t nDevice" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DeviceSelectionConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
   {
      DSL_CPE_FPrintf (out, "nReturn=%d nDevice=%d" DSL_CPE_CRLF,0, pCtx->nDevNum);
   }
   else
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(-1));
   }

   return 0;
}
#endif /* (DSL_CPE_MAX_DSL_ENTITIES > 1)*/

static const DSL_char_t g_sLlcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_DEV_Filter_t nFilter" DSL_CPE_CRLF
   "   na      = -1" DSL_CPE_CRLF
   "   off     = 0" DSL_CPE_CRLF
   "   isdn    = 1" DSL_CPE_CRLF
   "   pots    = 2" DSL_CPE_CRLF
   "   pots_2  = 3" DSL_CPE_CRLF
   "   pots_3  = 4" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroupMode_t nHsToneGroupMode" DSL_CPE_CRLF
   "   na      = -1" DSL_CPE_CRLF
   "   auto    = 0" DSL_CPE_CRLF
   "   manual  = 1" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroup_t nHsToneGroup_A (hex)" DSL_CPE_CRLF
   "   na            = -1" DSL_CPE_CRLF
   "   VDSL2 B43     = 0001" DSL_CPE_CRLF
   "   VDSL2 A43     = 0002" DSL_CPE_CRLF
   "   VDSL2 V43     = 0004" DSL_CPE_CRLF
   "   VDSL1 V43POTS = 0008" DSL_CPE_CRLF
   "   VDSL1 V43ISDN = 0010" DSL_CPE_CRLF
   "   ADSL1 C43     = 0020" DSL_CPE_CRLF
   "   ADSL2 J43     = 0040" DSL_CPE_CRLF
   "   ADSL2 B43C    = 0080" DSL_CPE_CRLF
   "   ADSL2 A43C    = 0100" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroup_t nHsToneGroup_V (hex)" DSL_CPE_CRLF
   "   na            = -1" DSL_CPE_CRLF
   "   VDSL2 B43     = 0001" DSL_CPE_CRLF
   "   VDSL2 A43     = 0002" DSL_CPE_CRLF
   "   VDSL2 V43     = 0004" DSL_CPE_CRLF
   "   VDSL1 V43POTS = 0008" DSL_CPE_CRLF
   "   VDSL1 V43ISDN = 0010" DSL_CPE_CRLF
   "   ADSL1 C43     = 0020" DSL_CPE_CRLF
   "   ADSL2 J43     = 0040" DSL_CPE_CRLF
   "   ADSL2 B43C    = 0080" DSL_CPE_CRLF
   "   ADSL2 A43C    = 0100" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroup_t nHsToneGroup_AV (hex)" DSL_CPE_CRLF
   "   not used anymore (set it to zero)" DSL_CPE_CRLF
   "- DSL_uint32_t nBaseAddr (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t nIrqNum" DSL_CPE_CRLF
   "- DSL_boolean_t bNtrEnable" DSL_CPE_CRLF
   "   na          = -1" DSL_CPE_CRLF
   "   enable      = 1" DSL_CPE_CRLF
   "   disable     = 0" DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LowLevelConfigurationSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_Error_t nReturn = DSL_SUCCESS;
   DSL_LowLevelConfiguration_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_MIN) == DSL_FALSE)
   {
      return -1;
   }

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 8, DSL_CLI_MAX) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_LowLevelConfiguration_t));
   memset(&(pData.data), 0xFF, sizeof(DSL_DeviceLowLevelConfig_t));

   DSL_CPE_sscanf (pCommands, "%d %d %x %x %x %x %bu %d",
      &pData.data.nFilter,
      &pData.data.nHsToneGroupMode,
      &pData.data.nHsToneGroup_A,
      &pData.data.nHsToneGroup_V,
      &pData.data.nHsToneGroup_AV,
      &pData.data.nBaseAddr,
      &pData.data.nIrqNum,
      &pData.data.bNtrEnable);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LOW_LEVEL_CONFIGURATION_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      /* Check Low Level configuration*/
      nReturn = DSL_CPE_LowLevelConfigurationCheck(fd);
      if (nReturn != DSL_SUCCESS)
      {
         pData.accessCtl.nReturn = nReturn;
      }

      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

static const DSL_char_t g_sLlcg[] =
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
   "- DSL_DEV_Filter_t nFilter" DSL_CPE_CRLF
   "   na      = -1" DSL_CPE_CRLF
   "   off     = 0" DSL_CPE_CRLF
   "   isdn    = 1" DSL_CPE_CRLF
   "   pots    = 2" DSL_CPE_CRLF
   "   pots_2  = 3" DSL_CPE_CRLF
   "   pots_3  = 4" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroupMode_t nHsToneGroupMode" DSL_CPE_CRLF
   "   na      = -1" DSL_CPE_CRLF
   "   auto    = 0" DSL_CPE_CRLF
   "   manual  = 1" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroup_t nHsToneGroup_A (hex)" DSL_CPE_CRLF
   "   na            = -1" DSL_CPE_CRLF
   "   VDSL2 B43     = 0001" DSL_CPE_CRLF
   "   VDSL2 A43     = 0002" DSL_CPE_CRLF
   "   VDSL2 V43     = 0004" DSL_CPE_CRLF
   "   VDSL1 V43POTS = 0008" DSL_CPE_CRLF
   "   VDSL1 V43ISDN = 0010" DSL_CPE_CRLF
   "   ADSL1 C43     = 0020" DSL_CPE_CRLF
   "   ADSL2 J43     = 0040" DSL_CPE_CRLF
   "   ADSL2 B43C    = 0080" DSL_CPE_CRLF
   "   ADSL2 A43C    = 0100" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroup_t nHsToneGroup_V (hex)" DSL_CPE_CRLF
   "   na            = -1" DSL_CPE_CRLF
   "   VDSL2 B43     = 0001" DSL_CPE_CRLF
   "   VDSL2 A43     = 0002" DSL_CPE_CRLF
   "   VDSL2 V43     = 0004" DSL_CPE_CRLF
   "   VDSL1 V43POTS = 0008" DSL_CPE_CRLF
   "   VDSL1 V43ISDN = 0010" DSL_CPE_CRLF
   "   ADSL1 C43     = 0020" DSL_CPE_CRLF
   "   ADSL2 J43     = 0040" DSL_CPE_CRLF
   "   ADSL2 B43C    = 0080" DSL_CPE_CRLF
   "   ADSL2 A43C    = 0100" DSL_CPE_CRLF
   "- DSL_DEV_HsToneGroup_t nHsToneGroup_AV (hex)" DSL_CPE_CRLF
   "   not used anymore" DSL_CPE_CRLF
   "- DSL_uint32_t nBaseAddr (hex)" DSL_CPE_CRLF
   "- DSL_uint8_t nIrqNum" DSL_CPE_CRLF
   "- DSL_boolean_t bNtrEnable" DSL_CPE_CRLF
   "   na          = -1" DSL_CPE_CRLF
   "   enable      = 1" DSL_CPE_CRLF
   "   disable     = 0" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_LowLevelConfigurationGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_LowLevelConfiguration_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0, sizeof(DSL_LowLevelConfiguration_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LOW_LEVEL_CONFIGURATION_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out,
         DSL_CPE_RET
         "nFilter=%d "
         "nHsToneGroupMode=%d "
         "nHsToneGroup_A=%x "
         "nHsToneGroup_V=%x "
         "nHsToneGroup_AV=%x "
         "nBaseAddr=%x "
         "nIrqNum=%u "
         "bNtrEnable=%d",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nFilter,
         pData.data.nHsToneGroupMode,
         pData.data.nHsToneGroup_A,
         pData.data.nHsToneGroup_V,
         pData.data.nHsToneGroup_AV,
         pData.data.nBaseAddr,
         pData.data.nIrqNum,
         pData.data.bNtrEnable);
   }

   return 0;
}

static const DSL_char_t g_sG997lspbg[] =
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
   "- DSL_int16_t[" _MKSTR(DSL_G997_MAX_NUMBER_OF_BANDS) "] LATN" DSL_CPE_CRLF
   "- DSL_int16_t[" _MKSTR(DSL_G997_MAX_NUMBER_OF_BANDS) "] SATN" DSL_CPE_CRLF
   "- DSL_int16_t[" _MKSTR(DSL_G997_MAX_NUMBER_OF_BANDS) "] SNR" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LineStatusPerBandGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LineStatusPerBand_t pData;
   DSL_int_t i = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &pData.nDirection);

   memset(&pData, 0x0, sizeof(DSL_G997_LineStatusPerBand_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LINE_STATUS_PER_BAND_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nDirection=%u ",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.nDirection );

      for (i = 0; i < DSL_G997_MAX_NUMBER_OF_BANDS; i++)
      {
         DSL_CPE_FPrintf(out, "LATN[%d]=%hd ", i, pData.data.LATN[i]);
      }
      for (i = 0; i < DSL_G997_MAX_NUMBER_OF_BANDS; i++)
      {
         DSL_CPE_FPrintf(out, "SATN[%d]=%hd ", i, pData.data.SATN[i]);
      }
      for (i = 0; i < DSL_G997_MAX_NUMBER_OF_BANDS; i++)
      {
         DSL_CPE_FPrintf(out, "SNR[%d]=%hd ", i, pData.data.SNR[i]);
      }
      DSL_CPE_FPrintf(out, DSL_CPE_CRLF );
   }

   return 0;
}

static const DSL_char_t g_sG997upbosg[] =
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
   "- DSL_G997_AeleMode_t nAeleMode" DSL_CPE_CRLF
   "   AELE_MODE_0 = 0" DSL_CPE_CRLF
   "   AELE_MODE_1 = 1" DSL_CPE_CRLF
   "   AELE_MODE_2 = 2" DSL_CPE_CRLF
   "   AELE_MODE_3 = 3" DSL_CPE_CRLF
   "- DSL_int16_t[" _MKSTR(DSL_G997_MAX_NUMBER_OF_BANDS) "] nKlePb" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_UsPowerBackOffStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_UsPowerBackOffStatus_t pData;
   DSL_int_t i = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0, sizeof(DSL_G997_UsPowerBackOffStatus_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_US_POWER_BACK_OFF_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf(out, DSL_CPE_RET"nAeleMode=%u ",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.nAeleMode);

      for (i = 0; i < DSL_G997_MAX_NUMBER_OF_BANDS; i++)
      {
         DSL_CPE_FPrintf(out, "nKlePb[%d]=%hu ", i, pData.data.nKl0EstimOPb[i]);
      }

      DSL_CPE_FPrintf(out, DSL_CPE_CRLF );
   }

   return 0;
}

static const DSL_char_t g_sLpmcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_G997_PMMode_t PMMode" DSL_CPE_CRLF
   "   DSL_G997_PMMODE_BIT_L3_STATE = 1" DSL_CPE_CRLF
   "   DSL_G997_PMMODE_BIT_L2_STATE = 2" DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LowPowerModeConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LowPowerModeConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LowPowerModeConfig_t));

   DSL_CPE_sscanf (pCommands, "%u", &(pData.data.PMMode));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LOW_POWER_MODE_CONFIG_SET, (int) &pData);

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
static const DSL_char_t g_sLpmcg[] =
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
   "- DSL_G997_PMMode_t PMMode" DSL_CPE_CRLF
   "   DSL_G997_PMMODE_BIT_L3_STATE = 1" DSL_CPE_CRLF
   "   DSL_G997_PMMODE_BIT_L2_STATE = 2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_G997_LowPowerModeConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_G997_LowPowerModeConfig_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_G997_LowPowerModeConfig_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_G997_LOW_POWER_MODE_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"PMMode=%d" DSL_CPE_CRLF,
      DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
      pData.data.PMMode);
   }

   return 0;
}
#endif /*INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sDsmcg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- IOCTL_MEI_VectorControl_t eVectorControl" DSL_CPE_CRLF
   "   e_MEI_VECTOR_CTRL_OFF = 0" DSL_CPE_CRLF
   "   e_MEI_VECTOR_CTRL_ON = 1" DSL_CPE_CRLF
   "   e_MEI_VECTOR_FRIENDLY_CTRL_ON = 2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DsmConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, mei_fd;
   IOCTL_MEI_dsmConfig_t pData;
   DSL_uint32_t nDevice = 0;
   DSL_char_t dev_name[32] = {0};

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(IOCTL_MEI_dsmConfig_t));

   DSL_CPE_Fd2DevNum(fd, &nDevice);

   sprintf(dev_name, "/dev/mei_cpe/%u", nDevice);

   mei_fd = DSL_CPE_Open(dev_name);
   if (mei_fd <= 0) {
      DSL_CPE_Close(mei_fd);
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(mei_fd));
      return 0;
   }

   ret = ioctl(mei_fd, FIO_MEI_DSM_CONFIG_GET, (DSL_int_t)&pData);
   if (ret != 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.ictl.retCode));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"eVectorControl=%u "
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret), pData.eVectorControl);
   }

   DSL_CPE_Close(mei_fd);

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET */

static const DSL_char_t g_sDsmcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- IOCTL_MEI_VectorControl_t eVectorControl" DSL_CPE_CRLF
   "   e_MEI_VECTOR_CTRL_OFF = 0" DSL_CPE_CRLF
   "   e_MEI_VECTOR_CTRL_ON = 1" DSL_CPE_CRLF
   "   e_MEI_VECTOR_FRIENDLY_CTRL_ON = 2" DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DsmConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, mei_fd;
   IOCTL_MEI_dsmConfig_t pData;
   DSL_uint32_t nDevice = 0;
   DSL_char_t dev_name[32] = {0};

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(IOCTL_MEI_dsmConfig_t));

   DSL_CPE_sscanf (pCommands, "%u", &pData.eVectorControl);

   if ((pData.eVectorControl < 0) || (pData.eVectorControl > 2))
   {
      DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "invalid parameters!" DSL_CPE_CRLF);
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERROR));

      return 0;
   }

   DSL_CPE_Fd2DevNum(fd, &nDevice);

   sprintf(dev_name, "/dev/mei_cpe/%u", nDevice);

   mei_fd = DSL_CPE_Open(dev_name);
   if (mei_fd <= 0) {
      DSL_CPE_Close(mei_fd);
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(mei_fd));
      return 0;
   }

   ret = ioctl(mei_fd, FIO_MEI_DSM_CONFIG_SET, (DSL_int_t)&pData);
   if (ret != 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.ictl.retCode));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   }

   DSL_CPE_Close(mei_fd);

   return 0;
}

static const DSL_char_t g_sDsmsg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- IOCTL_MEI_VectorStatus_t eVectorStatus" DSL_CPE_CRLF
   "   e_MEI_VECTOR_STAT_OFF = 0" DSL_CPE_CRLF
   "   e_MEI_VECTOR_STAT_ON_DS = 1" DSL_CPE_CRLF
   "   e_MEI_VECTOR_STAT_ON_DS_US = 2" DSL_CPE_CRLF
   "- IOCTL_MEI_VectorFriendlyStatus_t eVectorFriendlyStatus" DSL_CPE_CRLF
   "   e_MEI_VECTOR_FRIENDLY_STAT_OFF = 0" DSL_CPE_CRLF
   "   e_MEI_VECTOR_FRIENDLY_STAT_ON_DS = 1" DSL_CPE_CRLF
   "   e_MEI_VECTOR_FRIENDLY_STAT_ON_DS_US = 2" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DsmStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, mei_fd;
   IOCTL_MEI_dsmStatus_t pData;
   DSL_uint32_t nDevice = 0;
   DSL_char_t dev_name[32] = {0};

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(IOCTL_MEI_dsmStatus_t));

   DSL_CPE_Fd2DevNum(fd, &nDevice);

   sprintf(dev_name, "/dev/mei_cpe/%u", nDevice);

   mei_fd = DSL_CPE_Open(dev_name);
   if (mei_fd <= 0) {
      DSL_CPE_Close(mei_fd);
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(mei_fd));
      return 0;
   }

   ret = ioctl(mei_fd, FIO_MEI_DSM_STATUS_GET, (DSL_int_t)&pData);
   if (ret != 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.ictl.retCode));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"eVectorStatus=%u eVectorFriendlyStatus=%u "
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret),
         pData.eVectorStatus, pData.eVectorFriendlyStatus);
   }

   DSL_CPE_Close(mei_fd);

   return 0;
}

static const DSL_char_t g_sDsmstatg[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "Output Parameter" DSL_CPE_CRLF
   "- DSL_Error_t nReturn" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint32_t n_processed" DSL_CPE_CRLF
   "- DSL_uint32_t n_fw_dropped_size" DSL_CPE_CRLF
   "- DSL_uint32_t n_mei_dropped_size" DSL_CPE_CRLF
   "- DSL_uint32_t n_mei_dropped_no_pp_cb" DSL_CPE_CRLF
   "- DSL_uint32_t n_pp_dropped" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DsmStatisticsGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, mei_fd;
   IOCTL_MEI_dsmStatistics_t pData;
   DSL_uint32_t nDevice = 0;
   DSL_char_t dev_name[32] = {0};

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(IOCTL_MEI_dsmStatistics_t));

   DSL_CPE_Fd2DevNum(fd, &nDevice);

   sprintf(dev_name, "/dev/mei_cpe/%u", nDevice);

   mei_fd = DSL_CPE_Open(dev_name);
   if (mei_fd <= 0) {
      DSL_CPE_Close(mei_fd);
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(mei_fd));
      return 0;
   }

   ret = ioctl(mei_fd, FIO_MEI_DSM_STATISTICS_GET, (DSL_int_t)&pData);
   if (ret != 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.ictl.retCode));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"n_processed=%u n_fw_dropped_size=%u "
         "n_mei_dropped_size=%u n_mei_dropped_no_pp_cb=%u n_pp_dropped=%u"
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret), pData.n_processed, pData.n_fw_dropped_size,
         pData.n_mei_dropped_size, pData.n_mei_dropped_no_pp_cb, pData.n_pp_dropped);
   }

   DSL_CPE_Close(mei_fd);

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sDsmmcg[] =
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
   "- DSL_uint8_t[6] nMacAddress (used hex format XX:XX:XX:XX:XX:XX)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DsmMacConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, mei_fd;
   IOCTL_MEI_MacConfig_t pData;
   DSL_uint32_t nDevice = 0;
   DSL_char_t dev_name[32] = {0};

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(IOCTL_MEI_MacConfig_t));

   DSL_CPE_Fd2DevNum(fd, &nDevice);

   sprintf(dev_name, "/dev/mei_cpe/%u", nDevice);

   mei_fd = DSL_CPE_Open(dev_name);
   if (mei_fd <= 0) {
      DSL_CPE_Close(mei_fd);
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(mei_fd));
      return 0;
   }

   ret = ioctl(mei_fd, FIO_MEI_MAC_CONFIG_GET, (DSL_int_t)&pData);
   if (ret != 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.ictl.retCode));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nMacAddress=%02X:%02X:%02X:%02X:%02X:%02X "
         DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret),
         pData.nMacAddress[0], pData.nMacAddress[1],
         pData.nMacAddress[2], pData.nMacAddress[3],
         pData.nMacAddress[4], pData.nMacAddress[5]);
   }

   DSL_CPE_Close(mei_fd);

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET */

static const DSL_char_t g_sDsmmcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_uint8_t[6] nMacAddress (used hex format XX:XX:XX:XX:XX:XX)" DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_DsmMacConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, mei_fd;
   IOCTL_MEI_MacConfig_t pData;
   DSL_uint32_t nDevice = 0;
   DSL_char_t dev_name[32] = {0};
   DSL_char_t sMacAdr[18];
   DSL_CPE_MacAddress_t macAdr;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(IOCTL_MEI_MacConfig_t));

   ret = DSL_CPE_sscanf (pCommands, "%s", sMacAdr);

   ret = DSL_CPE_GetMacAdrFromString(sMacAdr, &macAdr);
   if (ret < DSL_SUCCESS)
   {
      DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "inconsistent MAC address!" DSL_CPE_CRLF);
      DSL_CPE_FPrintf(out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(DSL_ERROR));
      return 0;
   }
   else
   {
      memcpy (pData.nMacAddress, &macAdr, DSL_MAC_ADDRESS_OCTETS);
   }

   DSL_CPE_Fd2DevNum(fd, &nDevice);

   sprintf(dev_name, "/dev/mei_cpe/%u", nDevice);

   mei_fd = DSL_CPE_Open(dev_name);
   if (mei_fd <= 0) {
      DSL_CPE_Close(mei_fd);
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(mei_fd));
      return 0;
   }

   ret = ioctl(mei_fd, FIO_MEI_MAC_CONFIG_SET, (DSL_int_t)&pData);

   if (ret != 0)
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.ictl.retCode));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(ret));
   }

   DSL_CPE_Close(mei_fd);

   return 0;
}

#ifdef INCLUDE_DSL_BONDING

#ifndef DSL_CPE_DEBUG_DISABLE
static const DSL_char_t g_sBNDDBGst[] =
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
   "- DSL_uint32_t nAggregateReg" DSL_CPE_CRLF
   "- DSL_uint8_t nRemoteDiscoveryCode[6] (hex)" DSL_CPE_CRLF
   "- DSL_CPE_BND_MsStatus_t nMsStatus" DSL_CPE_CRLF
   "   DSL_BND_MASTER = 0" DSL_CPE_CRLF
   "   DSL_BND_SLAVE  = 1" DSL_CPE_CRLF
   "- DSL_uint16_t nPafAvailable" DSL_CPE_CRLF
   "- DSL_uint16_t nRemotePafAvailable" DSL_CPE_CRLF
   "- DSL_uint16_t nPafAggregate" DSL_CPE_CRLF
   "- DSL_uint16_t nPafEnable" DSL_CPE_CRLF
   "- DSL_uint32_t nTxDataRate" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_DBG_StatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t i = 0;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;
   DSL_CPE_BND_Context_t *pBnd = DSL_NULL;
   DSL_uint32_t nDevice = 0;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   if ((pCtx = DSL_CPE_GetGlobalContext()) != DSL_NULL)
   {
      if ((pBnd = (DSL_CPE_BND_Context_t*)pCtx->pBnd) == DSL_NULL)
      {
         DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
         return 0;
      }
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
      return 0;
   }

   if ((DSL_CPE_Fd2DevNum(fd, &nDevice)) < DSL_SUCCESS)
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF, DSL_CPE_RET_VAL(-1));
      return 0;
   }

   DSL_CPE_FPrintf (out, DSL_CPE_RET"nAggregateReg=%u ",
      DSL_CPE_RET_VAL(DSL_SUCCESS), pBnd->aggregateReg);

   DSL_CPE_FPrintf (out, "nRemoteDiscoveryCode=\"");
   for (i = 0; i < 6; i++)
   {
      DSL_CPE_FPrintf (out, "0x%02x ", pBnd->remoteDiscoveryCode[i]);
   }
   DSL_CPE_FPrintf (out, "\"");

   DSL_CPE_FPrintf (out, "nMsStatus=%u nPafAvailable=%u nRemotePafAvailable=%u "
      "nPafAggregate=%u nPafEnable=%u nTxDataRate=%u"DSL_CPE_CRLF,
      pBnd->lineMonitorStateMachine[nDevice].MsStatus,
      pBnd->lineMonitorStateMachine[nDevice].PafAvailable,
      pBnd->lineMonitorStateMachine[nDevice].RemotePafAvailable,
      pBnd->lineMonitorStateMachine[nDevice].PafAggregate,
      pBnd->lineMonitorStateMachine[nDevice].PafEnable,
      pBnd->lineMonitorStateMachine[nDevice].TxDataRate);

   return 0;
}
#endif /* #ifndef DSL_CPE_DEBUG_DISABLE */

static const DSL_char_t g_sBNDcs[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_boolean_t bPafEnable" DSL_CPE_CRLF
   "   disable = 0" DSL_CPE_CRLF
   "   enable = 1" DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_ConfigSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BND_ConfigSet_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &pData.data.bPafEnable);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_CONFIG_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

#ifdef INCLUDE_DSL_CONFIG_GET
static const DSL_char_t g_sBNDcg[] =
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
   "- DSL_boolean_t bPafEnable" DSL_CPE_CRLF
   "   false = 0" DSL_CPE_CRLF
   "   true = 1" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_ConfigGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BND_ConfigGet_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_BND_ConfigGet_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_CONFIG_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"bPafEnable=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn), pData.data.bPafEnable);
   }

   return 0;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

static const DSL_char_t g_sBNDhsg[] =
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
   "- DSL_BND_ActivationMode_t nActivationMode (hex)" DSL_CPE_CRLF
   "   DSL_BND_NO_COMMAND = 0x00000000" DSL_CPE_CRLF
   "   DSL_BND_DISCOVERY_SET_IF_CLEAR = 0x00000001" DSL_CPE_CRLF
   "   DSL_BND_DISCOVERY_CLEAR_IF_SAME = 0x00000002" DSL_CPE_CRLF
   "   DSL_BND_AGGREGATE_SET = 0x00000004" DSL_CPE_CRLF
   "   DSL_BND_AGGREGATE_CLR = 0x00000008" DSL_CPE_CRLF
   "- DSL_uint32_t nAggregateData" DSL_CPE_CRLF
   "- DSL_BND_RemotePafSupported_t nRemotePafSupported" DSL_CPE_CRLF
   "   DSL_BND_ENABLE_NOT_INITIALIZED = -1" DSL_CPE_CRLF
   "   DSL_BND_ENABLE_OFF = 0" DSL_CPE_CRLF
   "   DSL_BND_ENABLE_ON = 1" DSL_CPE_CRLF
   "- DSL_uint8_t nDiscoveryCode[6] (hex)" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_HsStatusGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0, i;
   DSL_BND_HsStatusGet_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_BND_HsStatusGet_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_HS_STATUS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nActivationMode=0x%08x nAggregateData=%d "
         "nRemotePafSupported=%d ",
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nActivationMode, pData.data.nAggregateData, pData.data.nRemotePafSupported);

      DSL_CPE_FPrintf (out, "nDiscoveryCode=\"");

      for (i = 0; i < 6; i++)
      {
         DSL_CPE_FPrintf (out, "0x%02x ", pData.data.nDiscoveryCode[i]);
      }
      DSL_CPE_FPrintf (out, "\"" DSL_CPE_CRLF);
   }

   return 0;
}

static const DSL_char_t g_sBNDETHDBGcg[] =
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
   "- DSL_uint16_t nControlRegister" DSL_CPE_CRLF
   "- DSL_uint16_t nRxFragmentsDrop" DSL_CPE_CRLF
   "- DSL_uint16_t nRxFragmentsLink0" DSL_CPE_CRLF
   "- DSL_uint16_t nRxFragmentsLink1" DSL_CPE_CRLF
   "- DSL_uint32_t nRxGoodFragments" DSL_CPE_CRLF
   "- DSL_uint16_t nRxPackets" DSL_CPE_CRLF
   "- DSL_uint16_t nTxFragmentsLink0" DSL_CPE_CRLF
   "- DSL_uint16_t nTxFragmentsLink1" DSL_CPE_CRLF
   "- DSL_uint16_t nTxPackets" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_ETH_DBG_CountersGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BND_EthDbgCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_BND_EthDbgCounters_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_ETH_DBG_COUNTERS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nControlRegister=%hu nRxFragmentsDrop=%hu "
         "nRxFragmentsLink0=%hu nRxFragmentsLink1=%hu nRxGoodFragments=%u "
         "nRxPackets=%hu nTxFragmentsLink0=%hu nTxFragmentsLink1=%hu nTxPackets=%hu" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nControlRegister, pData.data.nRxFragmentsDrop,
         pData.data.nRxFragmentsLink0, pData.data.nRxFragmentsLink1,
         pData.data.nRxGoodFragments, pData.data.nRxPackets,
         pData.data.nTxFragmentsLink0, pData.data.nTxFragmentsLink1, pData.data.nTxPackets);
   }

   return 0;
}

static const DSL_char_t g_sBNDETHcg[] =
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
   "- DSL_uint16_t nRxBadFragments" DSL_CPE_CRLF
   "- DSL_uint32_t nRxErroredFragments" DSL_CPE_CRLF
   "- DSL_uint32_t nRxLargeFragments" DSL_CPE_CRLF
   "- DSL_uint16_t nRxLostEnds" DSL_CPE_CRLF
   "- DSL_uint16_t nRxLostFragments" DSL_CPE_CRLF
   "- DSL_uint16_t nRxLostStarts" DSL_CPE_CRLF
   "- DSL_uint32_t nRxOverlfows" DSL_CPE_CRLF
   "- DSL_uint32_t nRxSmallFragments" DSL_CPE_CRLF
   DSL_CPE_CRLF "";
#else
   "";
#endif

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_ETH_CountersGet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BND_EthCounters_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 0, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   memset(&pData, 0x0, sizeof(DSL_BND_EthCounters_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_ETH_COUNTERS_GET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET"nRxBadFragments=%hu nRxErroredFragments=%u "
         "nRxLargeFragments=%u nRxLostEnds=%hu nRxLostFragments=%hu nRxLostStarts=%hu "
         "nRxOverlfows=%u nRxSmallFragments=%u" DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn),
         pData.data.nRxBadFragments, pData.data.nRxErroredFragments,
         pData.data.nRxLargeFragments, pData.data.nRxLostEnds, pData.data.nRxLostFragments,
         pData.data.nRxLostStarts, pData.data.nRxOverlfows, pData.data.nRxSmallFragments);
   }

   return 0;
}

static const DSL_char_t g_sBNDpmss[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF
   DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   "- DSL_uint32_t nDevice (optional, not used in the 'backward compatible' mode)" DSL_CPE_CRLF
#endif
   "- DSL_PortMode_t nPortMode" DSL_CPE_CRLF
   "   DSL_PORT_MODE_NA = 0" DSL_CPE_CRLF
   "   DSL_PORT_MODE_SINGLE = 1" DSL_CPE_CRLF
   "   DSL_PORT_MODE_DUAL = 2" DSL_CPE_CRLF
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

DSL_CLI_LOCAL DSL_int_t DSL_CPE_CLI_BND_PortModeSyncSet(
   DSL_int_t fd,
   DSL_char_t *pCommands,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_BND_PortModeSync_t pData;

   if (DSL_CPE_CLI_CheckParamNumber(pCommands, 1, DSL_CLI_EQUALS) == DSL_FALSE)
   {
      return -1;
   }

   DSL_CPE_sscanf (pCommands, "%u", &pData.data.nPortMode);

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_PORT_MODE_SYNC_SET, (int) &pData);

   if ((ret < 0) && (pData.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CPE_FPrintf (out, sFailureReturn, DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }
   else
   {
      DSL_CPE_FPrintf (out, DSL_CPE_RET DSL_CPE_CRLF,
         DSL_CPE_RET_VAL(pData.accessCtl.nReturn));
   }

   return 0;
}

#endif /* INCLUDE_DSL_BONDING*/

DSL_void_t DSL_CPE_CLI_DeviceCommandsRegister (DSL_void_t)
{
   /* Debug functionalities */
   DSL_CPE_CLI_CMD_ADD_COMM ("dms", "DeviceMessageSend", DSL_CPE_CLI_DBG_DeviceMessageSend, g_sDms);
   DSL_CPE_CLI_CMD_ADD_COMM ("dmms", "DeviceMessageModifySend", DSL_CPE_CLI_DBG_DeviceMessageModifySend, g_sDmms);

   /* Common functionalities */
   DSL_CPE_CLI_CMD_ADD_COMM ("aufg", "AutobootUsedFirmwareGet", DSL_CPE_CLI_AutobootUsedFirmwareGet, g_sAufg);
   DSL_CPE_CLI_CMD_ADD_COMM ("alf", "AutobootLoadFirmware", DSL_CPE_CLI_AutobootLoadFirmware, g_sAlf);
   DSL_CPE_CLI_CMD_ADD_COMM ("alig", "AuxLineInventoryGet", DSL_CPE_CLI_AuxLineInventoryGet, g_sAlig);
   DSL_CPE_CLI_CMD_ADD_COMM ("bpsg", "BandPlanSupportGet", DSL_CPE_CLI_BandPlanSupportGet, g_sBpsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("bpstg", "BandPlanSTatusGet", DSL_CPE_CLI_BandPlanStatusGet, g_sBpstg);
   DSL_CPE_CLI_CMD_ADD_COMM ("vig", "VersionInformationGet", DSL_CPE_CLI_VersionInformationGet, g_sVig);
#if (DSL_CPE_MAX_DSL_ENTITIES > 1)
   DSL_CPE_CLI_CMD_ADD_COMM ("dscs", "DeviceSelectionConfigSet", DSL_CPE_CLI_DeviceSelectionConfigSet, g_sDscs);
   DSL_CPE_CLI_CMD_ADD_COMM ("dscg", "DeviceSelectionConfigGet", DSL_CPE_CLI_DeviceSelectionConfigGet, g_sDscg);
#endif /* (DSL_CPE_MAX_DSL_ENTITIES > 1)*/

   DSL_CPE_CLI_CMD_ADD_COMM ("llcs", "LowLevelConfigurationSet", DSL_CPE_CLI_LowLevelConfigurationSet, g_sLlcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("llcg", "LowLevelConfigurationGet", DSL_CPE_CLI_LowLevelConfigurationGet, g_sLlcg);

   /* G.997.1 related functionlities */
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lspbg", "G997_LineStatusPerBandGet", DSL_CPE_CLI_G997_LineStatusPerBandGet, g_sG997lspbg);
   DSL_CPE_CLI_CMD_ADD_COMM ("g997upbosg", "G997_UsPowerBackOffStatusGet", DSL_CPE_CLI_G997_UsPowerBackOffStatusGet, g_sG997upbosg);

   DSL_CPE_CLI_CMD_ADD_COMM ("g997lpmcs", "G997_LowPowerModeConfigSet", DSL_CPE_CLI_G997_LowPowerModeConfigSet, g_sLpmcs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("g997lpmcg", "G997_LowPowerModeConfigGet", DSL_CPE_CLI_G997_LowPowerModeConfigGet, g_sLpmcg);
#endif /* #ifdef INCLUDE_DSL_CONFIG_GET*/

#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("dsmcg", "DSM_ConfigGet", DSL_CPE_CLI_DsmConfigGet, g_sDsmcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("dsmcs", "DSM_ConfigSet", DSL_CPE_CLI_DsmConfigSet, g_sDsmcs);
   DSL_CPE_CLI_CMD_ADD_COMM ("dsmsg", "DSM_StatusGet", DSL_CPE_CLI_DsmStatusGet, g_sDsmsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("dsmstatg", "DSM_STATisticsGet", DSL_CPE_CLI_DsmStatisticsGet, g_sDsmstatg);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("dsmmcg", "DSM_MacConfigGet", DSL_CPE_CLI_DsmMacConfigGet, g_sDsmmcg);
#endif /* INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("dsmmcs", "DSM_MacConfigSet", DSL_CPE_CLI_DsmMacConfigSet, g_sDsmmcs);

#ifdef INCLUDE_DSL_BONDING
   /* Debug functionalities */
#ifndef DSL_CPE_DEBUG_DISABLE
   DSL_CPE_CLI_CMD_ADD_COMM ("bnddbgsg", "BND_DBG_StatusGet", DSL_CPE_CLI_BND_DBG_StatusGet, g_sBNDDBGst);
#endif /* #ifndef DSL_CPE_DEBUG_DISABLE */
   DSL_CPE_CLI_CMD_ADD_COMM ("bndcs", "BND_ConfigSet", DSL_CPE_CLI_BND_ConfigSet, g_sBNDcs);
#ifdef INCLUDE_DSL_CONFIG_GET
   DSL_CPE_CLI_CMD_ADD_COMM ("bndcg", "BND_ConfigGet", DSL_CPE_CLI_BND_ConfigGet, g_sBNDcg);
#endif /* #ifdef INCLUDE_DSL_CONFIG_GET*/
   DSL_CPE_CLI_CMD_ADD_COMM ("bndhsg", "BND_HsStatusGet", DSL_CPE_CLI_BND_HsStatusGet, g_sBNDhsg);
   DSL_CPE_CLI_CMD_ADD_COMM ("bndethdbgcg", "BND_ETH_DBG_CountersGet", DSL_CPE_CLI_BND_ETH_DBG_CountersGet, g_sBNDETHDBGcg);
   DSL_CPE_CLI_CMD_ADD_COMM ("bndethcg", "BND_ETH_CountersGet", DSL_CPE_CLI_BND_ETH_CountersGet, g_sBNDETHcg);
   DSL_CPE_CLI_CMD_ADD_COMM ("bndpmss", "BND_PortModeSyncSet", DSL_CPE_CLI_BND_PortModeSyncSet, g_sBNDpmss);
#endif /* #ifdef INCLUDE_DSL_BONDING*/
}

#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#endif /* INCLUDE_DSL_CPE_API_VRX */
