/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
Includes
*/
#include "dsl_cpe_control.h"

#if defined(INCLUDE_DSL_CPE_API_VRX)

#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_api_ioctl.h"

#ifdef DSL_DEBUG_TOOL_INTERFACE

/** TCP messages debug stuff */
#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_TCPMSG

extern DSL_char_t *g_sFirmwareName2;

/* =============================================== */
/* Functions to swap between H&N byteorder         */
/* =============================================== */

/* swap an CMV 32 bit payload from net to host order */
#define N2H_SWAP_CMV_PAYL_32v16(val_32bit) \
             ( (ntohs(((DSL_uint16_t)(((val_32bit) & 0xFFFF0000) >> 16)))) \
            | ((ntohs((DSL_uint16_t)((val_32bit) & 0x0000FFFF))) << 16) )

/* swap an CMV 32 bit payload from host to net order */
#define H2N_SWAP_CMV_PAYL_32v16(val_32bit) \
             ( (htons(((DSL_uint16_t)(((val_32bit) & 0xFFFF0000) >> 16)))) \
            | ((htons((DSL_uint16_t)((val_32bit) & 0x0000FFFF))) << 16) )


/* swap an CMV 32 bit payload vs 2 16 bit CMV fields */
#define SWAP_CMV_PAYL_32_VS_16BIT(x)  ( (((x)&0xFFFF0000)>>16) \
                                      | (((x)&0x0000FFFF)<<16) )


#if ((__BYTE_ORDER == __BIG_ENDIAN )    && (WINHOST_REMOTE_SW_BYTE_ORDER == __LITTLE_ENDIAN)) || \
    ((__BYTE_ORDER == __LITTLE_ENDIAN ) && (WINHOST_REMOTE_SW_BYTE_ORDER == __BIG_ENDIAN))

/**
   Swap WORD fields (payload + header) within a CMV message from net to host order.
   This function will be used to swap CMV message fields incoming form the net.
*/
DSL_void_t N2H_SwapCmvBlockWord(CMV_MESSAGE_ALL_T *pCmvMsg, DSL_int_t startIdx, DSL_int_t count_16bit)
{
   DSL_int_t i;

   for (i=startIdx; i < (startIdx + count_16bit); i++)
   {
      pCmvMsg->rawMsg[i] = ntohs ( pCmvMsg->rawMsg[i] );
   }
   return;
}

/**
   Swap WORD fields (payload + header) within a CMV message from host to net order.
   This function will be used to swap CMV message fields outgoing to the net.
*/
DSL_void_t H2N_SwapCmvBlockWord(CMV_MESSAGE_ALL_T *pCmvMsg, DSL_int_t startIdx, DSL_int_t count_16bit)
{
   DSL_int_t i;

   for (i=startIdx; i < (startIdx + count_16bit); i++)
   {
      pCmvMsg->rawMsg[i] = htons( pCmvMsg->rawMsg[i] );
   }
   return;
}

/**
   Swap DWORD fields (payload) within a CMV message from net to host order.
   This function will be used to swap CMV message fields incoming form the net.

\remark
   The incoming 32 bit payload is a 16 bit aligned CMV message.
   --> Swap HI / LO 16 bit word from net to host order
   --> Swap HI / LO word

*/
DSL_void_t N2H_SwapCmvPaylDWord(CMV_MESSAGE_ALL_T *pCmvMsg, DSL_int_t startIdx, DSL_int_t count_32bit)
{
   DSL_int_t i;

   for (i=startIdx; i < (startIdx + count_32bit); i++)
   {
      pCmvMsg->cmv.payload.params_32Bit[i] =
                  N2H_SWAP_CMV_PAYL_32v16(pCmvMsg->cmv.payload.params_32Bit[i]);
   }
   return;
}

/**
   Swap DWORD fields (payload) within a CMV message from host to net order.
   This function will be used to swap CMV message fields outgoing to the net.
*/
DSL_void_t H2N_SwapCmvPaylDWord(CMV_MESSAGE_ALL_T *pCmvMsg, DSL_int_t startIdx, DSL_int_t count_32bit)
{
   DSL_int_t i;
   for (i=startIdx; i < (startIdx + count_32bit); i++)
   {
      pCmvMsg->cmv.payload.params_32Bit[i] =
                  H2N_SWAP_CMV_PAYL_32v16(pCmvMsg->cmv.payload.params_32Bit[i]);
   }
   return;
}
#endif

/**
   Swap DWORD fields (payload) within a CMV message from host to target order.
   This function will be used to swap 32 bit CMV message fields outgoing to the
   MEI interface.

\remark
   The CMV 32 bit payload into 2 a 16 bit aligned CMV message fields.
   --> Swap HI / LO word

*/
DSL_void_t SwapCmvPayl_32vs16(CMV_MESSAGE_ALL_T *pCmvMsg, DSL_int_t startIdx, DSL_int_t count_32bit)
{
   DSL_int_t i;
   for (i=startIdx; i < (startIdx + count_32bit); i++)
   {
      pCmvMsg->cmv.payload.params_32Bit[i] =
                  SWAP_CMV_PAYL_32_VS_16BIT(pCmvMsg->cmv.payload.params_32Bit[i]);
   }
   return;
}

/* =============================================== */
/* VRX device specific functions                 */
/* =============================================== */

DSL_int_t DSL_CPE_VRX_LoadFirmwareFromFile(
   DSL_char_t *psFirmwareName,
   DSL_uint8_t **pFirmware,
   DSL_uint32_t *pnFirmwareSize)
{
   DSL_int_t nRet = 0;
   DSL_CPE_File_t *fd_image = DSL_NULL;
   DSL_CPE_stat_t file_stat;

   *pnFirmwareSize = 0;
   if (*pFirmware != DSL_NULL)
   {
      DSL_CPE_Free(*pFirmware);
      *pFirmware = DSL_NULL;
   }

   for (;;)
   {
      fd_image = DSL_CPE_FOpen (psFirmwareName, "rb");
      if (fd_image == DSL_NULL)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "open %s fail!"DSL_CPE_CRLF, psFirmwareName));

         nRet = -1;
         break;
      }

      DSL_CPE_FStat (psFirmwareName, &file_stat);
      *pnFirmwareSize = file_stat.st_size;
      *pFirmware = DSL_CPE_Malloc (*pnFirmwareSize);

      if (*pFirmware == DSL_NULL)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            DSL_CPE_CRLF" firmware cannot be allocated: out of memory"DSL_CPE_CRLF));

         nRet = -1;
         break;
      }

      if (DSL_CPE_FRead (*pFirmware, sizeof (DSL_uint8_t), *pnFirmwareSize, fd_image) <= 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            DSL_CPE_CRLF" firmware_image not present"DSL_CPE_CRLF));

         nRet = -1;
         break;
      }

      break;
   }

   if (fd_image != DSL_NULL)
   {
      DSL_CPE_FClose (fd_image);
      fd_image = DSL_NULL;
   }

   return nRet;
}

static DSL_int_t DSL_CPE_VRX_DeviceStateCheck(
   DSL_int_t fd)
{
   IOCTL_VRX_reqCfg_t  Vdsl2_requCfg;

   memset(&Vdsl2_requCfg, 0x0, sizeof(IOCTL_VRX_reqCfg_t));

   if (ioctl(fd, FIO_VRX_REQ_CONFIG, (DSL_int_t)&Vdsl2_requCfg) != 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ioct(FIO_VRX_REQ_CONFIG): ERROR - cannot request config, retCode = %d!"DSL_CPE_CRLF,
         Vdsl2_requCfg.ictl.retCode));
      return -1;
   }

   if (Vdsl2_requCfg.currDrvState != 6)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ioct(FIO_VRX_REQ_CONFIG): ERROR - device not available, state = %d!"DSL_CPE_CRLF,
         Vdsl2_requCfg.currDrvState));
      return -1;
   }

   return 0;
}

/**
   Builds the response message
*/
static DSL_int_t DSL_CPE_VRX_ResponseMessage(
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   memset(pCmvMsg, 0x0, CMV_MESSAGE_SIZE);
   CMV_MSGHDR_CODE_RESERVED_SET((pCmvMsg->cmv), 3);
   CMV_MSGHDR_CODE_MBOX_CODE_SET((pCmvMsg->cmv), CMV_MBOX_CODE_ME_MSG);
   CMV_MSGHDR_BIT_SIZE_SET((pCmvMsg->cmv), 1);
   return 0;
}


/**
   Send a message to the mailbox.

\remark:
   Because for Winhost the "raw" interface is used it is in responsibility of
   the caller that the payload is already correct aligned.

*/
static DSL_int_t DSL_CPE_VRX_MailboxSend(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t  msgSize_16bit, ret;
   IOCTL_VRX_mboxSend_t DSL_Ioctl_DeviceArg;
   CMV_MESSAGE_ALL_T CmvMsg_Ack;

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Start - send to mailbox"DSL_CPE_CRLF));

   msgSize_16bit = CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg->cmv) + CMV_HEADER_16BIT_SIZE;

   /* swap cmv payload (16 bit fields) from host order to target */
   switch (CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
   {
      case CMV_MSG_BIT_SIZE_8BIT:
      case CMV_MSG_BIT_SIZE_16BIT:
         /* nothing to do */
         break;
      case CMV_MSG_BIT_SIZE_32BIT:
         SWAP_CMV_PAYL_32_VS_16(pCmvMsg, 0, ((CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg->cmv)) >> 1));
         break;
      default:
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Swap Payload CMD Raw - invalid bit size 0x%X (32 bit)"DSL_CPE_CRLF,
            CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv)));
         break;
   }

   DSL_Ioctl_DeviceArg.write_msg.pData_16 = (unsigned short *)pCmvMsg;
   DSL_Ioctl_DeviceArg.write_msg.count_16bit = (unsigned int)msgSize_16bit;

   DSL_Ioctl_DeviceArg.ack_msg.pData_16 = (unsigned short *)(&CmvMsg_Ack);
   DSL_Ioctl_DeviceArg.ack_msg.count_16bit = CMV_MESSAGE_SIZE / 2;

   ret = ioctl(fd, FIO_VRX_MBOX_MSG_RAW_SEND, (DSL_int_t)&DSL_Ioctl_DeviceArg);
   if (ret < 0)
   {

      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - send to mailbox failed !! - retCode = %d!"DSL_CPE_CRLF,
         DSL_Ioctl_DeviceArg.ictl.retCode));

      DSL_CPE_VRX_ResponseMessage(pCmvMsg);
      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
      CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_TIMEOUT);
   }
   else
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Send to mailbox successful"DSL_CPE_CRLF));

      memcpy(pCmvMsg, &CmvMsg_Ack, CMV_MESSAGE_SIZE);
   }


   /* swap cmv payload (16 bit fields) from target to host order */
   switch (CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
   {
      case CMV_MSG_BIT_SIZE_8BIT:
      case CMV_MSG_BIT_SIZE_16BIT:
         /* nothing to do */
         break;
      case CMV_MSG_BIT_SIZE_32BIT:
         SWAP_CMV_PAYL_32_VS_16(pCmvMsg, 0, ((CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg->cmv)) >> 1));
         break;
      default:
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Swap Payload ACK Raw - invalid bit size 0x%X (32 bit)"DSL_CPE_CRLF,
            CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv)));
         break;
   }

   return 0;
}

DSL_int_t DSL_CPE_VRX_Reboot(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t nRet = 0;
   DSL_char_t *pcFw = DSL_NULL, *pcFw2 = DSL_NULL;
   DSL_AutobootLoadFirmware_t ldFw;
#if (DSL_CPE_LINES_PER_DEVICE > 1)
   DSL_AutobootControl_t autobootCtrl;
#endif
#ifdef INCLUDE_SMS00976338
   DSL_AutobootConfig_t pAcfg;
#endif /* #ifdef INCLUDE_SMS00976338*/
   DSL_CPE_Control_Context_t *pContext = DSL_NULL;
   DSL_int_t nDevice = 0;

   nDevice = CMV_MSGHDR_CODE_PORT_NUMBER_GET(pCmvMsg->cmv);

   if (nDevice >= DSL_CPE_MAX_DSL_ENTITIES)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL_CPE_VRX_MsgHandle : unsupported device number (%d)"DSL_CPE_CRLF,
         nDevice));
      return -1;
   }

   memset(&ldFw, 0, sizeof(DSL_AutobootLoadFirmware_t));

   DSL_CPE_VRX_ResponseMessage(pCmvMsg);

   if (strlen(g_sFirmwareName1) > 0)
   {
      pcFw = g_sFirmwareName1;
   }

   if (strlen(g_sFirmwareName2) > 0)
   {
      pcFw2 = g_sFirmwareName2;
   }

   if (pcFw != DSL_NULL)
   {
      nRet = DSL_CPE_VRX_LoadFirmwareFromFile(
                pcFw, &ldFw.data.pFirmware, &ldFw.data.nFirmwareSize);
   }

   if (pcFw2 != DSL_NULL)
   {
      nRet = DSL_CPE_VRX_LoadFirmwareFromFile(
                pcFw2, &ldFw.data.pFirmware2, &ldFw.data.nFirmwareSize2);
   }

   while(1)
   {
      pContext = DSL_CPE_GetGlobalContext();
      if (pContext == DSL_NULL)
      {
         nRet = -1;
         break;
      }
#ifdef INCLUDE_SMS00976338
      /* Set Special Autoboot Configuration Options*/
      g_bWaitBeforeConfigWrite[pContext->nDevNum]    = DSL_TRUE;
      g_bWaitBeforeLinkActivation[pContext->nDevNum] = DSL_FALSE;
      g_bWaitBeforeRestart[pContext->nDevNum]        = DSL_FALSE;

      g_bAutoContinueWaitBeforeConfigWrite[pContext->nDevNum]    = DSL_FALSE;
      g_bAutoContinueWaitBeforeLinkActivation[pContext->nDevNum] = DSL_TRUE;
      g_bAutoContinueWaitBeforeRestart[pContext->nDevNum]        = DSL_TRUE;

      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "Autoboot Configuration options changed: "
         "bWaitBeforeConfigWrite=%u bWaitBeforeLinkActivation=%u bWaitBeforeRestart=%u "
         "bAutoContinueWaitBeforeConfigWrite=%u bAutoContinueWaitBeforeLinkActivation=%u "
         "bAutoContinueWaitBeforeRestart=%u" DSL_CPE_CRLF,
         g_bWaitBeforeConfigWrite[pContext->nDevNum], g_bWaitBeforeLinkActivation[pContext->nDevNum],
         g_bWaitBeforeRestart[pContext->nDevNum], g_bAutoContinueWaitBeforeConfigWrite[pContext->nDevNum],
         g_bAutoContinueWaitBeforeLinkActivation[pContext->nDevNum],
         g_bAutoContinueWaitBeforeRestart[pContext->nDevNum]));

      pAcfg.data.nStateMachineOptions.bWaitBeforeConfigWrite =
         g_bWaitBeforeConfigWrite[pContext->nDevNum];

      pAcfg.data.nStateMachineOptions.bWaitBeforeLinkActivation =
         g_bWaitBeforeLinkActivation[pContext->nDevNum];

      pAcfg.data.nStateMachineOptions.bWaitBeforeRestart =
         g_bWaitBeforeRestart[pContext->nDevNum];

      nRet = DSL_CPE_Ioctl (pContext->fd[nDevice], DSL_FIO_AUTOBOOT_CONFIG_SET, (int) &pAcfg);

      if (nRet < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "IOCTL (DSL_FIO_AUTOBOOT_CONFIG_SET) failed, nRet = %d!"
            DSL_CPE_CRLF, pAcfg.accessCtl.nReturn)));

         nRet = -1;
         break;
      }
#endif /* #ifdef INCLUDE_SMS00976338*/
      if ( ((ldFw.data.pFirmware  != DSL_NULL) && (ldFw.data.nFirmwareSize)) ||
           ((ldFw.data.pFirmware2 != DSL_NULL) && (ldFw.data.nFirmwareSize2)) )
      {
#if (DSL_CPE_LINES_PER_DEVICE < 2)
         ldFw.data.bLastChunk = DSL_TRUE;

         nRet = (DSL_Error_t) DSL_CPE_Ioctl(pContext->fd[nDevice],
            DSL_FIO_AUTOBOOT_LOAD_FIRMWARE, (DSL_int_t) &ldFw);
#else
         /* Trigger Autoboot restart sequence. FW binary will be requested
            from the DSL CPE API driver later*/
         autobootCtrl.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART_FULL;

         nRet = (DSL_Error_t) DSL_CPE_Ioctl (pContext->fd[nDevice],
            DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &autobootCtrl);
#endif

         if (nRet < 0)
         {
            DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Reboot failed!"DSL_CPE_CRLF));
            break;
         }
         else
         {
            CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
            CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
            CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_SUCCESS);
            break;
         }
      }
      else
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Both FW binaries are not specified!"DSL_CPE_CRLF));
         nRet = -1;
         break;
      }
   }

   if (ldFw.data.pFirmware)
   {
      DSL_CPE_Free(ldFw.data.pFirmware);
   }

   if (ldFw.data.pFirmware2)
   {
      DSL_CPE_Free(ldFw.data.pFirmware2);
   }

   return nRet;
}

DSL_int_t DSL_CPE_VRX_Halt(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_CPE_VRX_ResponseMessage(pCmvMsg);

   CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
   CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
   CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_SUCCESS);

   return 0;
}

DSL_int_t DSL_CPE_VRX_Run(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_CPE_VRX_ResponseMessage(pCmvMsg);

   CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
   CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
   CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_SUCCESS);

   return 0;
}

/**
   Request information on the Mailbox size
*/
DSL_int_t DSL_CPE_VRX_MailboxSize(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t ret;
   DSL_uint32_t tmp;
   IOCTL_VRX_reqCfg_t reqCfg;

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "Mailbox size query"DSL_CPE_CRLF));

   DSL_CPE_VRX_ResponseMessage(pCmvMsg);

   memset(&reqCfg, 0x00, sizeof(IOCTL_VRX_reqCfg_t));

   ret = ioctl(fd, FIO_VRX_REQ_CONFIG, (DSL_int_t)&reqCfg);

   if (ret != 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - Get cfg (payload size) failed - retCode = %d!"DSL_CPE_CRLF,
         reqCfg.ictl.retCode));

      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
      CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_TIMEOUT);
   }
   else
   {
      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_MBOX_PAYLOAD_SIZE_REPLY);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 2);

      /* get the mimimal size between the driver's and the chip's mailbox size */
      /* the size is sent in 16bits */
      tmp = (reqCfg.drvArc2MeMbSize > reqCfg.Arc2MeOnlineMbSize) ? \
               reqCfg.Arc2MeOnlineMbSize : reqCfg.drvArc2MeMbSize;
      (pCmvMsg->cmv).payload.params_16Bit[0] = (unsigned short)(tmp / 2);

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "ARC-to-ME Mailbox Payload Size = %d (16bit)"DSL_CPE_CRLF, tmp));

      tmp = (reqCfg.drvMe2ArcMbSize > reqCfg.Me2ArcOnlineMbSize) ? \
               reqCfg.Me2ArcOnlineMbSize : reqCfg.drvMe2ArcMbSize;
      (pCmvMsg->cmv).payload.params_16Bit[1] = (unsigned short)(tmp / 2);

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "ME-to-ARC Mailbox Payload Size = %d (16bit)"DSL_CPE_CRLF, tmp));
   }

   return 0;
}

/**
   Get directly a 32bits value from the MEI register.
   --> VOS_Ioctl_Device(..., FIO_VRX_REG_GET, ...)
*/
DSL_int_t DSL_CPE_VRX_GetReg(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t ret;
   IOCTL_VRX_regInOut_t Vdsl2_RegIo;

   Vdsl2_RegIo.addr  = (DSL_uint32_t)P_CMV_MSGHDR_MEI_ADDR_GET(pCmvMsg);
   Vdsl2_RegIo.value = (DSL_uint32_t)0;
   DSL_CPE_VRX_ResponseMessage(pCmvMsg);

   DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "ioct(FIO_VRX_REG_GET): MEI[0x%X]"DSL_CPE_CRLF, Vdsl2_RegIo.addr));

   ret = ioctl(fd, FIO_VRX_REG_GET, (DSL_int_t)(&Vdsl2_RegIo));
   if (ret != 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - ioct(.., FIO_VRX_REG_GET, ..) - retCode = %d!"DSL_CPE_CRLF, Vdsl2_RegIo.ictl.retCode));

      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
      CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_MEI_REG_RD_ERROR);
   }
   else
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Get Reg --> MEI[0x%X] = 0x%X"DSL_CPE_CRLF,
         Vdsl2_RegIo.addr, Vdsl2_RegIo.value));

      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_DEBUG_READ_MEI_REPLY);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 1);
      P_CMV_MSGHDR_MEI_ADDR_SET(pCmvMsg, ((unsigned short)Vdsl2_RegIo.addr) );
      CMV_MSGHDR_LENGTH_SET((pCmvMsg->cmv), 1);
      (pCmvMsg->cmv).payload.params_16Bit[0] = (unsigned short)Vdsl2_RegIo.value;
   }

   return 0;
}

/**
   Set directly a 32bit value to the MEI register.
   --> VOS_Ioctl_Device(..., FIO_VRX_REG_SET, ...)
*/
static DSL_int_t DSL_CPE_VRX_SetReg(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t ret;
   IOCTL_VRX_regInOut_t Vdsl2_RegIo;

   Vdsl2_RegIo.addr  = (DSL_uint32_t)P_CMV_MSGHDR_MEI_ADDR_GET(pCmvMsg);
   Vdsl2_RegIo.value = (DSL_uint32_t)(pCmvMsg->cmv).payload.params_16Bit[0];
   DSL_CPE_VRX_ResponseMessage(pCmvMsg);

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "ioct(FIO_VRX_REG_SET): MEI[0x%X] = 0x%X"DSL_CPE_CRLF,
      Vdsl2_RegIo.addr, Vdsl2_RegIo.value));

   ret = ioctl(fd, FIO_VRX_REG_SET, (DSL_int_t)&Vdsl2_RegIo);
   if (ret != 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - ioct(.., FIO_VRX_REG_SET, ..) - retCode = %d!"DSL_CPE_CRLF, Vdsl2_RegIo.ictl.retCode));

      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
      CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE_MEI_REG_WR_ERROR);
   }

   else
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Set Reg --> MEI[0x%X] = 0x%08X"DSL_CPE_CRLF,
         Vdsl2_RegIo.addr, Vdsl2_RegIo.value));

      CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_DEBUG_WRITE_MEI_REPLY);
      CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
      P_CMV_MSGHDR_MEI_ADDR_SET(pCmvMsg, ((unsigned short)Vdsl2_RegIo.addr) );
      CMV_MSGHDR_LENGTH_SET((pCmvMsg->cmv), 1);
   }

   return 0;
}

/**
   check the bitsize to the corresponding address.
*/
static DSL_int_t DSL_CPE_VRX_BitSizeCheck(
   DSL_char_t *pStr,
   CMV_MESSAGE_ALL_T *pCmvMsg,
   DSL_uint32_t destAddr)
{
   /* prepare the CMV payload for return */
   switch (CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
   {
      case CMV_MSG_BIT_SIZE_8BIT:
         break;
      case CMV_MSG_BIT_SIZE_16BIT:
         if(destAddr & 0x00000001)
         {
            DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "ERROR %s - addr 0x%08X missaligned (16 bit)"DSL_CPE_CRLF, pStr, destAddr));

            return -1;
         }
         break;
      case CMV_MSG_BIT_SIZE_32BIT:
         if(destAddr & 0x00000003)
         {
            DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "ERROR %s - addr 0x%08X missaligned (32 bit)"DSL_CPE_CRLF, pStr, destAddr));

            return -1;
         }
         break;
      default:
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR %s - invalid bit size 0x%X (32 bit)"DSL_CPE_CRLF, pStr, CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv)));

         return -1;
   }

   return 0;
}

/**
   Read the designed address using the Debug IF
*/
static DSL_int_t DSL_CPE_VRX_ReadDebug(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t remainCnt_byte, tmpCnt;
   DSL_uint32_t i, blkSize_byte;
   DSL_int_t ret = DSL_SUCCESS;
   DSL_uint32_t destAddr;
   /* fix unsigned int tmpData; */
   DSL_uint32_t tmpData = 0;
   DSL_char_t *pDest, *pTmp;
   /* unsigned short *pDest16; */

   IOCTL_VRX_dbgAccess_t vdsl2_dbg_access;
   CMV_DEBUG_ACCESS_BUFFER_T buffer;

   /* swap ? */
   destAddr =  P_CMV_MSGHDR_ADDR_LSW_GET(pCmvMsg)  |
                ( (P_CMV_MSGHDR_ADDR_MSW_GET(pCmvMsg) << 16) & 0xFFFF0000);

   if ( (DSL_CPE_VRX_BitSizeCheck("Debug Read", pCmvMsg, destAddr) ) != 0)
      goto RESPONSE_ERROR;

   blkSize_byte = CMV_MSGHDR_LENGTH_GET(pCmvMsg->cmv) << CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv);
   if (blkSize_byte > CMV_USED_PAYLOAD_8BIT_SIZE)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "ERROR Debug Read - size %d / max %d"DSL_CPE_CRLF,
         blkSize_byte, CMV_USED_PAYLOAD_8BIT_SIZE));

      goto RESPONSE_ERROR;
   }

   remainCnt_byte = blkSize_byte;
   pDest = (DSL_char_t*)(pCmvMsg->cmv.payload.params_8Bit);

   /*
      the requested data block is not 32 bit aligned
   */
   if(destAddr & 0x00000003)
   {
      tmpCnt = sizeof(DSL_uint32_t) - (destAddr & 0x00000003);
      tmpCnt = (tmpCnt > remainCnt_byte) ? remainCnt_byte : tmpCnt;

      vdsl2_dbg_access.count     = 1;
      vdsl2_dbg_access.pData_32  = (unsigned int*)&tmpData;
      vdsl2_dbg_access.dbgAddr   = destAddr & ~0x00000003;
      ret = ioctl(fd, FIO_VRX_DBG_LS_READ, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "ERROR Debug Read (unasigned cat) - retCode = %d"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_ERROR;
      }

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Debug Read - cat (addr 0x%X, tmpCnt %d, remain %d) = 0x%08lX"DSL_CPE_CRLF,
         destAddr, tmpCnt, remainCnt_byte, (unsigned long)tmpData));

      pTmp = ((DSL_char_t *)&tmpData);

      /* receive data as 32 bit value --> swap to the corresponding target */
      switch(CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
      {
         case CMV_MSG_BIT_SIZE_8BIT:
            /* tmpData = WH_SWAP_HOST32_TARGET8(tmpData); */

            memcpy(pDest, &pTmp[destAddr & 0x00000003], (DSL_uint32_t)tmpCnt);
            break;
         case CMV_MSG_BIT_SIZE_16BIT:
            /*
            tmpData = WH_SWAP_HOST32_TARGET16(tmpData);
            */
            /* address has to be 16 bit aligned --> only 0x00 or 0x02 */
            memcpy(pDest, &pTmp[destAddr & 0x00000003], (DSL_uint32_t)tmpCnt);
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            /* no swap necessary */
            break;
         default:
            goto RESPONSE_ERROR;
      }

      pDest += tmpCnt;
      remainCnt_byte -= tmpCnt;
      destAddr &= ~0x00000003;
      destAddr += sizeof(DSL_uint32_t);
   }

   /*
      get the next block 32 bit aligend
   */
   if (remainCnt_byte > 3)
   {
      /* fix tmpCnt = (remainCnt_byte & 0x00000003); */
      tmpCnt = (remainCnt_byte & ~0x00000003);

      vdsl2_dbg_access.count     = ((DSL_uint32_t)tmpCnt) >> 2;
      vdsl2_dbg_access.pData_32  = (unsigned int *) &buffer;
      vdsl2_dbg_access.dbgAddr   = destAddr;
      ret = ioctl(fd, FIO_VRX_DBG_LS_READ, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Debug Read - retCode = %d!"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_ERROR;
      }

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Debug Read - 32 bit aligned (addr 0x%X, tmpCnt %d, remain %d) = 0x%08X"DSL_CPE_CRLF,
         destAddr, tmpCnt, remainCnt_byte, buffer.params_32Bit[0]));

      /* receive data as 32 bit value --> swap to the corresponding target */
      switch(CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
      {
         case CMV_MSG_BIT_SIZE_8BIT:
            #if 0
            for (i=0; i < (tmpCnt>>2); i++)
            {
               buffer.params_32Bit[i] = TCP_DEBUG_SWAP_HOST32_TARGET8(buffer.params_32Bit[i]);
            }
            #endif
            break;
         case CMV_MSG_BIT_SIZE_16BIT:
            #if 0
            for (i=0; i < (tmpCnt>>2); i++)
            {
               buffer.params_32Bit[i] = TCP_DEBUG_SWAP_HOST32_TARGET16(buffer.params_32Bit[i]);
            }
            #endif
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            for (i=0; i < (((DSL_uint32_t)tmpCnt)>>2); i++)
            {
               buffer.params_32Bit[i] = TCP_DEBUG_SWAP_HOST32_TARGET32(buffer.params_32Bit[i]);
            }
            break;
         default:
            goto RESPONSE_ERROR;
      }

      /* copy received data */
      memcpy(pDest, buffer.params_8Bit, (DSL_uint32_t)(tmpCnt));
      pDest += tmpCnt;
      destAddr += ((DSL_uint32_t)tmpCnt);
      remainCnt_byte -= tmpCnt;
   }

   /*
      get tail - the requested data block is not 32 bit aligned
   */
   if ((remainCnt_byte > 0) && (remainCnt_byte < 4) )
   {
      vdsl2_dbg_access.count     = 1;
      vdsl2_dbg_access.pData_32  = (unsigned int *) &tmpData;
      vdsl2_dbg_access.dbgAddr   = destAddr;
      ret = ioctl(fd, FIO_VRX_DBG_LS_READ, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Debug Read (unasigned tail) - retCode = %d"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_ERROR;
      }

      /* copy received data */
      pTmp = ((DSL_char_t *)&tmpData);

      /* receive data as 32 bit value --> swap to the corresponding target */
      switch(CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
      {
         case CMV_MSG_BIT_SIZE_8BIT:
            {
               memcpy(pDest, &pTmp[0], (DSL_uint32_t)remainCnt_byte);
            }
            break;
         case CMV_MSG_BIT_SIZE_16BIT:
            /* the high word will be cached by the "CAT" section */
            memcpy(pDest, &pTmp[0], (DSL_uint32_t)remainCnt_byte);
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            /* no swap necessary */
            tmpData = TCP_DEBUG_SWAP_HOST32_TARGET32(tmpData);
            memcpy(pDest, &pTmp[0], (DSL_uint32_t)remainCnt_byte);
            break;
         default:
            goto RESPONSE_ERROR;
      }

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Debug Read - tail (addr 0x%X, remain %d) = 0x%08lX"DSL_CPE_CRLF,
         destAddr, remainCnt_byte, (unsigned long)tmpData));

      pDest += remainCnt_byte;
      remainCnt_byte = 0;
   }

   if (remainCnt_byte != 0)
   {
      /* ERROR */
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR Debug Read (remain %d)"DSL_CPE_CRLF,
         remainCnt_byte));

      goto RESPONSE_ERROR;
   }

   if (CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv) == CMV_MSG_BIT_SIZE_8BIT)
   {
      for (i = 0; i < ((blkSize_byte>1) ? blkSize_byte/sizeof(unsigned short) : 1); i++)
      {
         pCmvMsg->cmv.payload.params_16Bit[i] =
            TCP_DEBUG_SWAP_HOST16_TARGET16(pCmvMsg->cmv.payload.params_16Bit[i]);
      }
   }

   /* number of 16bit CMV units + one extra field for single bytes */
   tmpCnt = blkSize_byte/sizeof(unsigned short) + (blkSize_byte & 0x1);

   /* build a success response message */
   CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_DEBUG_READDEBUG_ACK);
   CMV_MSGHDR_PAYLOAD_SIZE_SET( pCmvMsg->cmv, (DSL_uint32_t)tmpCnt);

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "Debug read successful"DSL_CPE_CRLF));

   return 0;

RESPONSE_ERROR:
   DSL_CPE_VRX_ResponseMessage(pCmvMsg);
   CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
   CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
   CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv),
                              (errno != ETIMEDOUT) ? DCE2H_RESPONSE_HOST_DBG_PORT_RD_ERROR : DCE2H_RESPONSE_TIMEOUT);
   return -1;
}

static DSL_int_t DSL_CPE_VRX_WriteDebug(
  DSL_int_t fd,
  CMV_MESSAGE_ALL_T *pCmvMsg)
{
   DSL_int_t ret = DSL_SUCCESS;
   DSL_uint32_t destAddr, tmpCnt, remainCnt_byte, blkSize_byte, msgPaylSize_byte;
   DSL_uint32_t tmpData = 0, tmpSrc = 0;
   DSL_char_t *pSrc, *pTmp, *pTmpSrc;

   IOCTL_VRX_dbgAccess_t vdsl2_dbg_access;
   CMV_DEBUG_ACCESS_BUFFER_T buffer;

   destAddr = (P_CMV_MSGHDR_ADDR_LSW_GET(pCmvMsg)) |
              (( (P_CMV_MSGHDR_ADDR_MSW_GET(pCmvMsg)) << 16) & 0xFFFF0000);

   if ( (DSL_CPE_VRX_BitSizeCheck("Debug Write", pCmvMsg, destAddr) ) != 0 )
      goto RESPONSE_WR_ERROR;

   msgPaylSize_byte = CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg->cmv) << 1;  /* always 16 bit */

   blkSize_byte = CMV_MSGHDR_LENGTH_GET(pCmvMsg->cmv) << CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv);

   if (blkSize_byte > msgPaylSize_byte)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR Debug Write - msg size %d / length size %d"DSL_CPE_CRLF,
         msgPaylSize_byte, blkSize_byte));

      goto RESPONSE_WR_ERROR;
   }

   remainCnt_byte = blkSize_byte;
   pSrc = (DSL_char_t*)(pCmvMsg->cmv.payload.params_8Bit);

   /*
      the destination address is not 32 bit aligned
   */
   if(destAddr & 0x00000003)
   {
      tmpCnt = sizeof(DSL_uint32_t) - (destAddr & 0x00000003);
      tmpCnt = (tmpCnt > remainCnt_byte) ? remainCnt_byte : tmpCnt;

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Debug Write - cat (addr 0x%X, tmpCnt %d, remain %d)"DSL_CPE_CRLF,
         destAddr, tmpCnt, remainCnt_byte));

      /* read/modify/write - get current 32 bit value */
      vdsl2_dbg_access.count     = 1;
      vdsl2_dbg_access.pData_32  = (unsigned int *)&tmpData;
      vdsl2_dbg_access.dbgAddr   = destAddr & ~0x00000003;
      ret = ioctl(fd, FIO_VRX_DBG_LS_READ, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Debug Write (rd/mod/wr) - ret = %d"DSL_CPE_CRLF,
            ret));

         goto RESPONSE_WR_ERROR;
      }


      /* get cat source data --> modify write data */
      pTmp = ((DSL_char_t *)&tmpData);
      pTmpSrc = ((DSL_char_t *)&tmpSrc);
      memcpy(pTmpSrc, pSrc, (DSL_uint32_t)tmpCnt);

      /* receive msg is 16 bit target aligened --> data as 32 bit host value */
      switch(CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
      {
         case CMV_MSG_BIT_SIZE_8BIT:
            tmpSrc = TCP_DEBUG_SWAP_TARGET8_HOST32(tmpSrc);
            memcpy(&pTmp[destAddr & 0x00000003], pTmpSrc, (DSL_uint32_t)tmpCnt);

            break;
         case CMV_MSG_BIT_SIZE_16BIT:
            /*
            tmpSrc = WH_SWAP_TARGET16_HOST32(tmpSrc);
            */
            memcpy(&pTmp[destAddr & 0x00000003], pTmpSrc, (DSL_uint32_t)tmpCnt);
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            /* no swap necessary */
            break;
         default:
            goto RESPONSE_WR_ERROR;
      }

      /* write back */
      ret = ioctl(fd, FIO_VRX_DBG_LS_WRITE, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Debug Write (rd/mod/wr) - retCode = %d!"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_WR_ERROR;
      }

      pSrc += tmpCnt;
      remainCnt_byte -= tmpCnt;
      destAddr &= ~0x00000003;
      destAddr += sizeof(DSL_uint32_t);
   }

   /*
      write the next block (dest Addr is now 32 bit aligend)
   */
   memset(&buffer, 0x0, sizeof(CMV_DEBUG_ACCESS_BUFFER_T));

   if (remainCnt_byte > 3)
   {
      tmpCnt = remainCnt_byte & ~0x00000003;

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Debug Write - 32 bit aligned (addr 0x%X, tmpCnt %d, remain %d)"DSL_CPE_CRLF,
         destAddr, tmpCnt, remainCnt_byte));

      memcpy(&buffer, pSrc, (DSL_uint32_t)tmpCnt);

      /* receive data as 32 bit value --> swap to the corresponding target */
      switch(CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
      {
         DSL_uint32_t i;
         case CMV_MSG_BIT_SIZE_8BIT:
            for (i=0; i < (tmpCnt>>2); i++)
            {
               buffer.params_32Bit[i] = TCP_DEBUG_SWAP_TARGET8_HOST32(buffer.params_32Bit[i]);
            }
            break;
         case CMV_MSG_BIT_SIZE_16BIT:
            for (i=0; i < (tmpCnt>>2); i++)
            {
               buffer.params_32Bit[i] = TCP_DEBUG_SWAP_TARGET16_HOST32(buffer.params_32Bit[i]);
            }
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            for (i=0; i < (tmpCnt>>2); i++)
            {
               buffer.params_32Bit[i] = TCP_DEBUG_SWAP_TARGET32_HOST32(buffer.params_32Bit[i]);
            }
            break;
         default:
            goto RESPONSE_WR_ERROR;
      }

      vdsl2_dbg_access.count     = tmpCnt >> 2;  /* 32 bit */
      vdsl2_dbg_access.pData_32  = (unsigned int *) &buffer;
      vdsl2_dbg_access.dbgAddr   = destAddr;
      ret = ioctl(fd, FIO_VRX_DBG_LS_WRITE, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Debug Write - retCode = %d!"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_WR_ERROR;
      }

      pSrc += tmpCnt;
      destAddr += tmpCnt;
      remainCnt_byte -= tmpCnt;
   }

   /*
      write tail - the requested data block is not 32 bit aligned
   */
   if ((remainCnt_byte > 0) && (remainCnt_byte < 4) )
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "Debug Write - tail (addr 0x%X, remain %d)"DSL_CPE_CRLF,
         destAddr, remainCnt_byte));

      /* read/modify/write - get current 32 bit value */
      vdsl2_dbg_access.count     = 1;
      vdsl2_dbg_access.pData_32  = (unsigned int *) &tmpData;
      vdsl2_dbg_access.dbgAddr   = destAddr;
      ret = ioctl(fd, FIO_VRX_DBG_LS_READ, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Debug Write (rd/mod/wr) rd tail - retCode =%d!"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_WR_ERROR;
      }


      /* get cat source data --> modify write data */
      pTmp = ((DSL_char_t *)&tmpData);
      pTmpSrc = ((DSL_char_t *)&tmpSrc);
      memcpy(pTmpSrc, pSrc, remainCnt_byte);

      /* receive msg is 16 bit target aligened --> data as 32 bit host value */
      switch(CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
      {
         case CMV_MSG_BIT_SIZE_8BIT:
            break;
         case CMV_MSG_BIT_SIZE_16BIT:
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            tmpSrc = TCP_DEBUG_SWAP_TARGET32_HOST32(tmpSrc);
            break;
         default:
            goto RESPONSE_WR_ERROR;
      }

      memcpy(pTmp, pTmpSrc, remainCnt_byte);

      vdsl2_dbg_access.count     = 1;
      vdsl2_dbg_access.pData_32  = (unsigned int *) &tmpData;
      vdsl2_dbg_access.dbgAddr   = destAddr;
      ret = ioctl(fd, FIO_VRX_DBG_LS_WRITE, (DSL_int_t)&vdsl2_dbg_access);
      if (ret < 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            DSL_CPE_PREFIX "ERROR Debug Write wr tail - retCode=%d!"DSL_CPE_CRLF,
            vdsl2_dbg_access.ictl.retCode));

         goto RESPONSE_WR_ERROR;
      }
      remainCnt_byte = 0;
   }

   if (remainCnt_byte != 0)
   {
      /* ERROR */
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR Debug Write (remain %d)!"DSL_CPE_CRLF,
         remainCnt_byte));


      goto RESPONSE_WR_ERROR;
   }

   /* build a success response message */
   CMV_MSGHDR_CODE_RESERVED_SET((pCmvMsg->cmv), 3);
   CMV_MSGHDR_CODE_MBOX_CODE_SET((pCmvMsg->cmv), CMV_MBOX_CODE_ME_MSG);
   CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_DEBUG_WRITEDEBUG_ACK);
   CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "Debug write successful"DSL_CPE_CRLF));

   return 0;

RESPONSE_WR_ERROR:
   DSL_CPE_VRX_ResponseMessage(pCmvMsg);
   CMV_MSGHDR_FCT_OPCODE_SET((pCmvMsg->cmv), DCE2H_RESPONSE);
   CMV_MSGHDR_PAYLOAD_SIZE_SET((pCmvMsg->cmv), 0);
   CMV_MSGHDR_RESULT_CODE_SET((pCmvMsg->cmv),
                              ( (errno != ETIMEDOUT) ? DCE2H_RESPONSE_HOST_DBG_PORT_RD_ERROR :
                                                       DCE2H_RESPONSE_TIMEOUT ));

   return -1;

}

static DSL_int_t DSL_CPE_VRX_TcpMsgSend(
   DSL_int_t fd,
   CMV_MESSAGE_ALL_T   *pCmvMsg)
{
   DSL_int_t tmp;
   DSL_uint32_t CMVSizeByte;

   CMVSizeByte = ( CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg->cmv) + CMV_HEADER_16BIT_SIZE ) * 2;

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "DSL_CPE_VRX_MsgSend - %d bytes"DSL_CPE_CRLF, CMVSizeByte));

   /* prepare the CMV payload for return
      swap payload at first (still need header information in host order) */
   switch (CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv))
   {
      case CMV_MSG_BIT_SIZE_8BIT:
      case CMV_MSG_BIT_SIZE_16BIT:
         /*H2N_SWAP_CMV_BLOCK_WORD(pCmvMsg, CMV_HEADER_16BIT_SIZE, (CMVSizeByte >> 1));*/
         H2N_SWAP_CMV_BLOCK_WORD(pCmvMsg, CMV_HEADER_16BIT_SIZE, (CMVSizeByte >> 1)-CMV_HEADER_16BIT_SIZE);
         break;
      case CMV_MSG_BIT_SIZE_32BIT:
         H2N_SWAP_CMV_PAYL_DWORD(pCmvMsg, 0, (CMVSizeByte >> 2));
         break;
      default:
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR Swap Payload Replay - invalid bit size 0x%X (32 bit)"DSL_CPE_CRLF,
            CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg->cmv)));

         break;
   }

   /* swap header */
   H2N_SWAP_CMV_BLOCK_WORD(pCmvMsg, 0, CMV_HEADER_16BIT_SIZE);

   /* send the CMV message */
   /* Bytes of message sent has to be 32 bytes */
   tmp = DSL_CPE_SocketSend(fd, (DSL_char_t *)pCmvMsg, CMVSizeByte);

   if ( tmp != ((DSL_int_t)CMVSizeByte))
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - DSL_CPE_VRX_MsgSend failed !!! (sent = %d, exp = %d)"DSL_CPE_CRLF,
         tmp, CMVSizeByte));

      return -1;
   }

   return 0;
}

static DSL_int_t DSL_CPE_VRX_MsgHandle(
   DSL_VRX_TcpDebugInfo_t *pTcpDebugInfo,
   CMV_MESSAGE_ALL_T *pCmvMsgRecv,
   DSL_int_t CMVSizeByte)
{
   DSL_int_t ret, mei_fd;
   DSL_uint32_t tmpChannel;
   DSL_uint32_t opcode;
   DSL_uint32_t mbxCode;
   CMV_MESSAGE_ALL_T *pCmvMsg = &(pTcpDebugInfo->CmvMsg);

   memcpy (pCmvMsg, pCmvMsgRecv, (DSL_uint32_t)CMVSizeByte);

   tmpChannel = CMV_MSGHDR_CODE_PORT_NUMBER_GET(pCmvMsg->cmv);
   /*tmpChannel = 0;*/

   if (tmpChannel >= WINHOST_VDSL2_DFE_CHANNELS_AVAILABLE)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL_CPE_VRX_MsgHandle : unsupported channel number (%d)"DSL_CPE_CRLF,
         tmpChannel));

      return -1;
   }

   if (pTcpDebugInfo->arrDeviceFd[tmpChannel] < 0)
   {
      pTcpDebugInfo->arrDeviceFd[tmpChannel] = DSL_CPE_DEV_DeviceOpen(DSL_CPE_IFX_LOW_DEV, tmpChannel);

      if (pTcpDebugInfo->arrDeviceFd[tmpChannel] <= 0)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Error: MEI BSP device file open failed!"DSL_CPE_CRLF));

         return -1;
      }
   }

   mei_fd = pTcpDebugInfo->arrDeviceFd[tmpChannel];

   opcode  = CMV_MSGHDR_FCT_OPCODE_GET((pCmvMsg->cmv));
   mbxCode = CMV_MSGHDR_CODE_MBOX_CODE_GET((pCmvMsg->cmv));

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "DSL_CPE_VRX_MsgHandle : mbxCode = 0x%x, opcode = 0x%x"DSL_CPE_CRLF,
      mbxCode, opcode));

   /* clear the channel number and the two MSBs in the mailbox code */
   (pCmvMsg->cmv).header.mbxCode = (unsigned short)mbxCode;

   /* handle the CMV message */
   if (mbxCode != CMV_MBOX_CODE_ME_MSG)
   {
      /*
         just forward the CMV through the mailbox
         these messages do not require ME intervention
      */
      ret = DSL_CPE_VRX_MailboxSend(mei_fd, pCmvMsg);

      /* set back channel number */
      return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;
   }
   else
   {
      /* interprete the other WinHost Commands */
      switch(opcode)
      {
         case H2DCE_DEBUG_HALT:
            ret = DSL_CPE_VRX_Halt(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_DEBUG_RUN:
            ret = DSL_CPE_VRX_Run(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_DEBUG_REBOOT:
         case H2DCE_DEBUG_DOWNLOAD:
            /* Reboot device*/
            ret = DSL_CPE_VRX_Reboot(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_MBOX_PAYLOAD_SIZE_QUERY:
            /* Request information on the Mailbox size*/
            ret = DSL_CPE_VRX_MailboxSize(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_DEBUG_READ_MEI:
            /* Read directly from and return 32-bit MEI register */

            /* !!!!
               The driver interface is 32 bit while the mei interface is 16 bit
               --> The driver uses only the lower word (16 mei register)
               --> swap ????
            */
            ret = DSL_CPE_VRX_GetReg(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_DEBUG_WRITE_MEI:
            /* Write directly to 32-bit MEI register             */

            /* !!!! */
            ret = DSL_CPE_VRX_SetReg(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_DEBUG_READDEBUG:
            /* Read the designed address using Debug IF */

            /* !!!!
               Here the driver interface and the value is 32 bit
               SWAP
            */
            ret = DSL_CPE_VRX_ReadDebug(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         case H2DCE_DEBUG_WRITEDEBUG:
            /* Write the designed address using Debug IF */

            /* !!!! */
            ret = DSL_CPE_VRX_WriteDebug(mei_fd, pCmvMsg);
            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || ret;

         default:
            DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "ERROR - DSL_CPE_VRX_MsgHandle: Unknown WH opcode 0x%x"DSL_CPE_CRLF,
               opcode));

            return DSL_CPE_VRX_TcpMsgSend(pTcpDebugInfo->socket_fd, pCmvMsg) || (-1);
      }
   }
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_DEV_TcpDebugMessageResourceUsageGet(
   DSL_CPE_TcpDebugClientInfo_t *clientInfo,
   DSL_uint32_t *pStaticMemUsage,
   DSL_uint32_t *pDynamicMemUsage)
{
   DSL_int_t c = 0;

   if (clientInfo == DSL_NULL ||
       pStaticMemUsage == DSL_NULL ||
       pDynamicMemUsage == DSL_NULL)
   {
      return DSL_ERROR;
   }

   *pStaticMemUsage = 0;
   *pDynamicMemUsage = 0;

   for (c = 0; c < DSL_FD_SETSIZE; c++)
   {
      if (clientInfo[c].pDevData != DSL_NULL)
      {
         *pDynamicMemUsage += sizeof(DSL_VRX_TcpDebugInfo_t);
      }
   }

   return DSL_SUCCESS;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

DSL_void_t DSL_CPE_DEV_DeviceDataFree(DSL_void_t *pDevData)
{
   DSL_int_t i = 0;

   if (pDevData)
   {
      for (i = 0; i < WINHOST_VDSL2_DFE_CHANNELS_AVAILABLE; i++)
      {
         if ( ((DSL_VRX_TcpDebugInfo_t*)pDevData)->arrDeviceFd[i] > 0)
         {
            /* Close low-level driver*/
            close(((DSL_VRX_TcpDebugInfo_t*)pDevData)->arrDeviceFd[i]);
         }
      }

      DSL_CPE_Free(pDevData);
   }

   return;
}

DSL_int_t DSL_CPE_DEV_DeviceOpen(DSL_char_t *pDevName, DSL_uint32_t dev_num)
{
   DSL_char_t text[30];
   DSL_int_t mei_fd = -1;

   if (pDevName == DSL_NULL)
   {
      return -1;
   }

   sprintf(text, "%s/%d", pDevName, dev_num);

   mei_fd = DSL_CPE_Open(text);
   /*mei_fd = open(text, O_RDWR);*/

   if (mei_fd < 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "xDSL device driver cannot be opened!"DSL_CPE_CRLF));
      return -1;
   }

   /* Check VRX device driver state*/
   if (DSL_CPE_VRX_DeviceStateCheck(mei_fd) < 0)
   {
      DSL_CPE_Close(mei_fd);
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "xDSL device driver not ready!"DSL_CPE_CRLF));
      return -1;
   }

   return mei_fd;
}

DSL_int_t DSL_CPE_DEV_TcpMessageHandle(
   DSL_CPE_TcpDebugClientInfo_t *pDevData)
{
   DSL_int_t i = 0, tmp, nReceived;
   DSL_uint32_t PayloadSize, CMVSizeByte;
   DSL_char_t *pPackage;
   CMV_MESSAGE_ALL_T *pRecvCmvMsg;
   DSL_VRX_TcpDebugInfo_t *pTcpDebugInfo;

   if (pDevData == DSL_NULL)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Pointer to the client info is NULL!"DSL_CPE_CRLF));
      return -1;
   }

   pTcpDebugInfo = (DSL_VRX_TcpDebugInfo_t*)(pDevData->pDevData);

   if (pTcpDebugInfo == DSL_NULL)
   {
      /* Create device specific data*/
      pTcpDebugInfo = DSL_CPE_Malloc(sizeof(DSL_VRX_TcpDebugInfo_t));
      if (pTcpDebugInfo == DSL_NULL)
      {
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Can't allocate memory for the xDSL device specific data!"DSL_CPE_CRLF));

         return -1;
      }
      pDevData->pDevData = pTcpDebugInfo;

      /* Get VRX device specific info*/
      pTcpDebugInfo->pEndReceive = pTcpDebugInfo->Buffer;
      pTcpDebugInfo->socket_fd   = pDevData->fd;

      for (i = 0 ; i< WINHOST_VDSL2_DFE_CHANNELS_AVAILABLE; i++)
      {
         pTcpDebugInfo->arrDeviceFd[i] = -1;
      }
   }

   pPackage = pTcpDebugInfo->Buffer;

   /* remaining free space in Buffer */
   tmp = ((DSL_int_t)sizeof(pTcpDebugInfo->Buffer)) -
          ((DSL_int_t)pTcpDebugInfo->pEndReceive - (DSL_int_t)pTcpDebugInfo->Buffer);

   /* Receive data from the socket*/
   nReceived = DSL_CPE_SocketRecv(pTcpDebugInfo->socket_fd, pTcpDebugInfo->pEndReceive, tmp);

   if (nReceived < 1)
   {
      /* the connection is reset, because the TcpDebug client has been closed */
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "connection terminated from host!"DSL_CPE_CRLF));

      return -ENODATA;
   }

   /*
      A WinHost CMV will be always in 16 bit units
      --> check the 16 bit assignment
      --> check length
      --> swap the whole msg here at once
   */
   if (nReceived & 0x00000001)
   {
      /* **it happens - skip or terminate ? */
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - unasigned message (size %d) terminate!"DSL_CPE_CRLF,
         nReceived));

      return -1;
   }

   /* check length - at least one CMV header */
   if (((DSL_uint32_t)nReceived) < CMV_HEADER_8BIT_SIZE)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - invalid message (size %d) terminate!"DSL_CPE_CRLF,
         nReceived));

      return -1;
   }

   DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Received %d bytes"DSL_CPE_CRLF, nReceived));

   /* update the index */
   pTcpDebugInfo->pEndReceive += nReceived;

   while (pTcpDebugInfo->pEndReceive >= pPackage + CMV_HEADER_8BIT_SIZE)
   {
      /* the received package is big enough to be analyzed */
      pRecvCmvMsg = (CMV_MESSAGE_ALL_T*)pPackage;

      /* swap cmv header (16 bit) from network to host order */
      N2H_SWAP_CMV_BLOCK_WORD(pRecvCmvMsg, 0, CMV_HEADER_16BIT_SIZE);

      /* get the payload size (16 bit count) and check length */
      PayloadSize = CMV_MSGHDR_PAYLOAD_SIZE_GET(pRecvCmvMsg->cmv);

      if ( ((DSL_uint32_t)nReceived) < (CMV_HEADER_8BIT_SIZE + (PayloadSize << 1)) )
      {
         /* error not all data received */
         DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR - msg incomplete (size %d, payl[16bit] %d) terminate!"DSL_CPE_CRLF,
            nReceived, PayloadSize));

         return -1;
      }

      /* swap cmv payload (16 bit fields) from network to host order */
      switch (CMV_MSGHDR_BIT_SIZE_GET(pRecvCmvMsg->cmv))
      {
         case CMV_MSG_BIT_SIZE_8BIT:
         case CMV_MSG_BIT_SIZE_16BIT:
            N2H_SWAP_CMV_BLOCK_WORD(pRecvCmvMsg, CMV_HEADER_16BIT_SIZE, PayloadSize);
            break;
         case CMV_MSG_BIT_SIZE_32BIT:
            N2H_SWAP_CMV_PAYL_DWORD(pRecvCmvMsg, 0, (PayloadSize >> 1));
            break;
         default:
            DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "ERROR Swap Payload Recv - invalid bit size 0x%X (32 bit)!"DSL_CPE_CRLF,
               CMV_MSGHDR_BIT_SIZE_GET(pRecvCmvMsg->cmv)));

            break;
      }

      /* calculate the size in bytes of the payload */
      CMVSizeByte = (PayloadSize << 1) + CMV_HEADER_8BIT_SIZE;

      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "CMV msg[0x%04X]: Payload: %d [byte], msg %d [byte]"DSL_CPE_CRLF,
         pRecvCmvMsg->cmv.header.MessageID, PayloadSize << 1, CMVSizeByte));

      if (pTcpDebugInfo->pEndReceive >= pPackage + CMVSizeByte)
      {
         /* handle the received message */
         DSL_CPE_VRX_MsgHandle(pTcpDebugInfo, pRecvCmvMsg, CMVSizeByte);
         /* go to the next message in the buffer */
         pPackage += CMVSizeByte;
      }
   }

   /* copy the parts of the new message to the beginning  of the buffer */
   tmp = (pTcpDebugInfo->pEndReceive - pPackage);
   if (tmp > 0)
   {
      memcpy (pTcpDebugInfo->Buffer, pPackage, (DSL_uint32_t)tmp);
   }

   pTcpDebugInfo->pEndReceive = pTcpDebugInfo->Buffer + tmp;

   return 0;
}

#endif

#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
