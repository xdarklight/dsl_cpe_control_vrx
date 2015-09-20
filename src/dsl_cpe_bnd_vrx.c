/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "dsl_cpe_control.h"

#if defined(INCLUDE_DSL_BONDING) && defined(INCLUDE_DSL_CPE_API_VRX)

#include "dsl_cpe_bnd_vrx.h"

#include "drv_dsl_cpe_api_ioctl.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_APP

static DSL_Error_t DSL_CPE_BND_BondingCheck(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd,
               DSL_uint16_t nPort)
{
   DSL_int_t ret = 0, i, j;
   DSL_BND_HsStatusGet_t status;
   DSL_BND_HsContinue_t  HsContinue;

   /* read bonding status */
   memset (&status, 0, sizeof(DSL_BND_HsStatusGet_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_HS_STATUS_GET, (DSL_int_t)&status);

   if ((ret < 0) || (status.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HS_STATUS_GET ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   /* Set RemotePafAvailable */
   pBndCtx->lineMonitorStateMachine[nPort].RemotePafAvailable =
      (DSL_uint16_t)status.data.nRemotePafSupported;

   /* clear if same */
   if ((status.data.nActivationMode & DSL_BND_DISCOVERY_CLEAR_IF_SAME))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Clear If Same Command Received" DSL_CPE_CRLF));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Remote Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         pBndCtx->remoteDiscoveryCode[0], pBndCtx->remoteDiscoveryCode[1],
         pBndCtx->remoteDiscoveryCode[2], pBndCtx->remoteDiscoveryCode[3],
         pBndCtx->remoteDiscoveryCode[4], pBndCtx->remoteDiscoveryCode[5]));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         status.data.nDiscoveryCode[0], status.data.nDiscoveryCode[1],
         status.data.nDiscoveryCode[2], status.data.nDiscoveryCode[3],
         status.data.nDiscoveryCode[4], status.data.nDiscoveryCode[5]));

      /* if received discovery code = current Remote Discovery Register,
         then clear the Remote Discovery and Aggregate Register */
      for (i = 0; i < 6; i++)
      {
         if (status.data.nDiscoveryCode[i] != pBndCtx->remoteDiscoveryCode[i])
            break;
      }

      if (i == 6)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - Update Remote Discovery code" DSL_CPE_CRLF));

         memset(pBndCtx->remoteDiscoveryCode, 0x0, 6);

         pBndCtx->aggregateReg = 0;
         for (j = 0; j < DSL_CPE_MAX_DSL_ENTITIES; j++)
         {
            pBndCtx->lineMonitorStateMachine[j].PafAggregate = 0;
         }

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - Remote Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
            pBndCtx->remoteDiscoveryCode[0], pBndCtx->remoteDiscoveryCode[1],
            pBndCtx->remoteDiscoveryCode[2], pBndCtx->remoteDiscoveryCode[3],
            pBndCtx->remoteDiscoveryCode[4], pBndCtx->remoteDiscoveryCode[5]));

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - Aggregation Register: 0x%X" DSL_CPE_CRLF,
            pBndCtx->aggregateReg));
      }
   }

   /* set if clear */
   if ((status.data.nActivationMode & DSL_BND_DISCOVERY_SET_IF_CLEAR))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Set If Clear Command Received" DSL_CPE_CRLF));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Remote Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         pBndCtx->remoteDiscoveryCode[0], pBndCtx->remoteDiscoveryCode[1],
         pBndCtx->remoteDiscoveryCode[2], pBndCtx->remoteDiscoveryCode[3],
         pBndCtx->remoteDiscoveryCode[4], pBndCtx->remoteDiscoveryCode[5]));
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
         status.data.nDiscoveryCode[0], status.data.nDiscoveryCode[1],
         status.data.nDiscoveryCode[2], status.data.nDiscoveryCode[3],
         status.data.nDiscoveryCode[4], status.data.nDiscoveryCode[5]));

      /*  if current Remote Discovery Register is 0,
          then set the Remote Discovery Register to received discovery code */
      for (i = 0; i < 6; i++)
      {
         if (pBndCtx->remoteDiscoveryCode[i] != 0)
            break;
      }

      if (i == 6)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - Update Remote Discovery code" DSL_CPE_CRLF));

         memcpy(pBndCtx->remoteDiscoveryCode, status.data.nDiscoveryCode, 6);

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "BND - Remote Discovery code: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" DSL_CPE_CRLF,
            pBndCtx->remoteDiscoveryCode[0], pBndCtx->remoteDiscoveryCode[1],
            pBndCtx->remoteDiscoveryCode[2], pBndCtx->remoteDiscoveryCode[3],
            pBndCtx->remoteDiscoveryCode[4], pBndCtx->remoteDiscoveryCode[5]));
      }
   }

   /* aggregate set */
   if ((status.data.nActivationMode & DSL_BND_AGGREGATE_SET))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - AggregateSet Command Received" DSL_CPE_CRLF));

      pBndCtx->aggregateReg |= (1 << nPort);
      pBndCtx->lineMonitorStateMachine[nPort].PafAggregate = 1;

      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Aggregation Register: 0x%X" DSL_CPE_CRLF,
         pBndCtx->aggregateReg));
   }

   /* aggregate clear */
   if ((status.data.nActivationMode & DSL_BND_AGGREGATE_CLR) != 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - AggregateClr Command Received" DSL_CPE_CRLF));

      pBndCtx->aggregateReg &= (~(1 << nPort));
      pBndCtx->lineMonitorStateMachine[nPort].PafAggregate = 0;

      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Aggregation Register: 0x%X" DSL_CPE_CRLF,
         pBndCtx->aggregateReg));
   }

   /* set continue flag after bonding registers processed */

   memset (&HsContinue, 0, sizeof(DSL_BND_HsContinue_t));

   memcpy(HsContinue.data.nDiscoveryCode, pBndCtx->remoteDiscoveryCode, 6);
   HsContinue.data.nAggregateData = pBndCtx->aggregateReg;

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_HS_CONTINUE, (DSL_int_t)&HsContinue);

   if ((ret < 0) || (HsContinue.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HS_CONTINUE ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_BND_RemotePafAvailableCheck(
                     DSL_int_t fd,
                     DSL_uint16_t *pRemotePafAvailable)
{
   DSL_int_t ret = 0;
   DSL_BND_HsStatusGet_t status;

   /* read bonding status */
   memset (&status, 0, sizeof(DSL_BND_HsStatusGet_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_HS_STATUS_GET, (DSL_int_t)&status);

   if ((ret < 0) || (status.accessCtl.nReturn < DSL_SUCCESS))
   {
      if (status.accessCtl.nReturn != DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - DSL_FIO_BND_HS_STATUS_GET ioctl call failed!" DSL_CPE_CRLF));

         return DSL_ERROR;
      }
   }

   /* Set RemotePafAvailable */
   *pRemotePafAvailable = (DSL_uint16_t)status.data.nRemotePafSupported;

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_LocalPafAvailableCheck(
                     DSL_int_t fd,
                     DSL_uint16_t *pLocalPafAvailable)
{
   DSL_int_t ret = 0;
   DSL_BND_ConfigGet_t bndConfig;

   /* read bonding status */
   memset (&bndConfig, 0, sizeof(DSL_BND_ConfigGet_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_BND_CONFIG_GET, (DSL_int_t)&bndConfig);

   if ((ret < 0) || (bndConfig.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_CONFIG_GET ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   *pLocalPafAvailable = (DSL_uint16_t)bndConfig.data.bPafEnable;

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_BND_AutobootStatusRestartWait(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_uint_t nDevice)
{
   return DSL_SUCCESS;
}

#if (DSL_CPE_LINES_PER_DEVICE == 2)
DSL_Error_t DSL_CPE_BND_DeviceFirmwareDownload(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_FirmwareRequestType_t nFwReqType,
               DSL_PortMode_t nPortMode,
               DSL_boolean_t bMaster,
               DSL_boolean_t bSlave)
{
   DSL_Error_t nErrorCode = DSL_SUCCESS;
   DSL_CPE_Control_Context_t *pContext = pBndCtx->pCtrlCtx;

   if (bMaster)
   {
      /* Download MASTER*/
      nErrorCode = DSL_CPE_DownloadFirmware(pContext->fd[0], nFwReqType,
                                            nPortMode, DSL_NULL, DSL_NULL);
      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Master line FW download failed!" DSL_CPE_CRLF));
         return nErrorCode;
      }
   }

   if (bSlave)
   {
      /* Download MASTER*/
      nErrorCode = DSL_CPE_DownloadFirmware(pContext->fd[1], nFwReqType,
                                            nPortMode, DSL_NULL, DSL_NULL);
      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Slave line FW download failed!" DSL_CPE_CRLF));
         return nErrorCode;
      }
   }

   return nErrorCode;
}

#define DSL_CPE_SLAVE_READY_WAIT_CNT_MAX  10
DSL_Error_t DSL_CPE_BND_SyncDownloadFirmware(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t nDevice,
               DSL_FirmwareRequestType_t nFwReqType,
               DSL_PortMode_t nPortMode)
{
   DSL_Error_t nErrorCode = DSL_SUCCESS;
   DSL_CPE_Control_Context_t *pContext = pBndCtx->pCtrlCtx;
   DSL_AutobootControl_t pAcs;
   DSL_boolean_t bSlavePort;
   DSL_uint32_t nMasterDevNum, nSlaveDevNum, wait_cnt;
   DSL_int_t ret;
   DSL_AutobootStatus_t nAutobootStatus;
   DSL_BND_PortModeSync_t PortModeSync;

   bSlavePort = nDevice % DSL_CPE_LINES_PER_DEVICE ? DSL_TRUE : DSL_FALSE;

   nMasterDevNum = bSlavePort ? nDevice - 1 : nDevice;
   nSlaveDevNum = bSlavePort ? nDevice : nDevice + 1;

   if (nPortMode == DSL_PORT_MODE_DUAL)
   {
      if (!pBndCtx->bInitialFwRequestHandled)
      {
         pBndCtx->bFwRequested[nDevice] = DSL_TRUE;

         if (pBndCtx->bFwRequested[nMasterDevNum] && pBndCtx->bFwRequested[nSlaveDevNum])
         {
            /* Download firmware for MASTER and SLAVE*/
            nErrorCode = DSL_CPE_BND_DeviceFirmwareDownload(
                           pBndCtx, nFwReqType, nPortMode, DSL_TRUE, DSL_TRUE);
            if (nErrorCode != DSL_SUCCESS)
            {
               return nErrorCode;
            }

            pBndCtx->bInitialFwRequestHandled = DSL_TRUE;
            return nErrorCode;
         }
         else
         {
            return nErrorCode;
         }
      }

      if (bSlavePort)
      {
         /* restart MASTER*/
         pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART_FULL;
         DSL_CPE_Ioctl(pContext->fd[nMasterDevNum],
                              DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);
      }
      else
      {
         memset (&nAutobootStatus, 0, sizeof(DSL_AutobootStatus_t));
         ret = DSL_CPE_Ioctl(pContext->fd[nSlaveDevNum], DSL_FIO_AUTOBOOT_STATUS_GET, (DSL_int_t)&nAutobootStatus);
         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "BND - line %d Autoboot Status get failed!" DSL_CPE_CRLF, nSlaveDevNum));
            return DSL_ERROR;
         }

         /* check SLAVE status*/
         if (nAutobootStatus.data.nStatus != DSL_AUTOBOOT_STATUS_FW_WAIT)
         {
            /* restart SLAVE*/
            pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART_FULL;
            DSL_CPE_Ioctl(pContext->fd[nSlaveDevNum],
                                 DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);
            return DSL_SUCCESS;
         }

         /* Download firmware for MASTER and SLAVE*/
         nErrorCode = DSL_CPE_BND_DeviceFirmwareDownload(
                        pBndCtx, nFwReqType, nPortMode, DSL_TRUE, DSL_TRUE);
         if (nErrorCode != DSL_SUCCESS)
         {
            return nErrorCode;
         }
      }
   }
   else
   {
      pBndCtx->bFwRequested[nDevice] = DSL_TRUE;

      /* Disable SLAVE*/
      pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_DISABLE;
      DSL_CPE_Ioctl(pContext->fd[nSlaveDevNum], DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);

      /* wait for the disabled SLAVE*/
      for (wait_cnt = 0; wait_cnt < DSL_CPE_SLAVE_READY_WAIT_CNT_MAX; wait_cnt++)
      {
         memset (&nAutobootStatus, 0, sizeof(DSL_AutobootStatus_t));

         ret = DSL_CPE_Ioctl(pContext->fd[nSlaveDevNum], DSL_FIO_AUTOBOOT_STATUS_GET, (DSL_int_t)&nAutobootStatus);
         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "BND - line %d Autoboot Status get failed!" DSL_CPE_CRLF, nSlaveDevNum));
            return DSL_ERROR;
         }

         if (nAutobootStatus.data.nStatus == DSL_AUTOBOOT_STATUS_STOPPED ||
             nAutobootStatus.data.nStatus == DSL_AUTOBOOT_STATUS_DISABLED ||
             nAutobootStatus.data.nStatus == DSL_AUTOBOOT_STATUS_FW_WAIT)
         {
            break;
         }

         DSL_CPE_Sleep(1);
      }

      if (wait_cnt >= DSL_CPE_SLAVE_READY_WAIT_CNT_MAX)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - line %d disable failed!" DSL_CPE_CRLF, nSlaveDevNum));
         return DSL_ERROR;
      }

      if (!bSlavePort)
      {
         /* Download firmware for MASTER*/
         nErrorCode = DSL_CPE_BND_DeviceFirmwareDownload(
                        pBndCtx, nFwReqType, nPortMode, DSL_TRUE, DSL_FALSE);
         if (nErrorCode != DSL_SUCCESS)
         {
            return nErrorCode;
         }
      }
      else
      {
         /* Set MASTER Port Mode sync*/
         memset(&PortModeSync, 0, sizeof(PortModeSync));
         PortModeSync.data.nPortMode = DSL_PORT_MODE_SINGLE;
         ret = DSL_CPE_Ioctl(pContext->fd[nMasterDevNum], DSL_FIO_BND_PORT_MODE_SYNC_SET, (DSL_int_t)&PortModeSync);
         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "BND - line %d Port Mode sync set failed!" DSL_CPE_CRLF, nMasterDevNum));
            return DSL_ERROR;
         }

         /* restart MASTER*/
         pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART_FULL;
         ret = DSL_CPE_Ioctl(pContext->fd[nMasterDevNum],
                              DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);
         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "BND - line %d full restart failed failed!" DSL_CPE_CRLF, nMasterDevNum));
            return DSL_ERROR;
         }
      }
   }

   return nErrorCode;
}
#endif

DSL_Error_t DSL_CPE_BND_SystemInterfaceStatusHandle(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd,
               DSL_uint_t nDevice)
{
   DSL_Error_t nErrorCode = DSL_SUCCESS;
   DSL_uint16_t nRemotePafAvailable = 0;
   DSL_uint32_t nCurrentLine, nOppositeLine, nLine;
   DSL_AutobootControl_t nAcs;

   nCurrentLine = nDevice % DSL_CPE_MAX_DSL_ENTITIES;
   nOppositeLine = (nDevice + 1) % DSL_CPE_MAX_DSL_ENTITIES;

   nErrorCode = DSL_CPE_BND_RemotePafAvailableCheck(fd, &nRemotePafAvailable);
   if (nErrorCode != DSL_SUCCESS)
   {
      return nErrorCode;
   }

#ifdef INCLUDE_SCRIPT_NOTIFICATION
   nErrorCode = DSL_CPE_SetEnv("DSL_BONDING_STATUS", nRemotePafAvailable ?
                                                   "ACTIVE" : "INACTIVE");
   if (nErrorCode != DSL_SUCCESS)
   {
      return nErrorCode;
   }
#endif

   if (!nRemotePafAvailable)
   {
      /* If paf-not-available line first reach FULL_INIT => disable opposite line */
      /* If paf-available line first reach FULL_INIT => disable current line */
      nLine = (pBndCtx->nPafLineHandled == nOppositeLine) ? nCurrentLine :
                                                            nOppositeLine;

      memset(&nAcs, 0x0, sizeof(DSL_AutobootControl_t));
      nAcs.data.nCommand = DSL_AUTOBOOT_CTRL_DISABLE;

      nErrorCode = DSL_CPE_Ioctl(pBndCtx->pCtrlCtx->fd[nLine],
                    DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &nAcs);
      if ((nErrorCode < 0) || (nAcs.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "DSL_FIO_AUTOBOOT_CONTROL_SET ioctl call failed!" DSL_CPE_CRLF));

         return DSL_ERROR;
      }

      /* Mark disabled line num */
      pBndCtx->nPafLineDisabled = nLine;

      /* Clear paf-available line num */
      pBndCtx->nPafLineHandled = -1;
   }
   else
   {
      /* Save paf-available line num reach FULL_INIT */
      if (pBndCtx->nPafLineHandled != nCurrentLine)
      {
         pBndCtx->nPafLineHandled = nCurrentLine;
      }
   }

   return nErrorCode;
}

DSL_Error_t DSL_CPE_BND_LineStateHandle(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd, DSL_uint_t nDevice,
               DSL_LineStateValue_t nLineState,
               DSL_LineStateValue_t nPrevLineState)
{
   DSL_Error_t nErrorCode = DSL_SUCCESS;
   DSL_uint32_t nCurrentLine, nOppositeLine;
   DSL_AutobootControl_t nAcs;

   DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
      "BND - Device=%d nLineState=0x%x nPrevLineState=0x%x" DSL_CPE_CRLF,
      nDevice, nLineState,nPrevLineState));

   if (nLineState == DSL_LINESTATE_SILENT)
   {
      nCurrentLine = nDevice % DSL_CPE_MAX_DSL_ENTITIES;
      nOppositeLine = (nDevice + 1) % DSL_CPE_MAX_DSL_ENTITIES;

      /* Try to enable opposite line (if it was previously disabled) */
      if (pBndCtx->nPafLineDisabled == nOppositeLine)
      {
         memset(&nAcs, 0x0, sizeof(DSL_AutobootControl_t));
         nAcs.data.nCommand = DSL_AUTOBOOT_CTRL_ENABLE;

         nErrorCode = DSL_CPE_Ioctl(pBndCtx->pCtrlCtx->fd[nOppositeLine],
                        DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &nAcs);
         if ((nErrorCode < 0) || (nAcs.accessCtl.nReturn < DSL_SUCCESS))
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "DSL_FIO_AUTOBOOT_CONTROL_SET ioctl call failed!" DSL_CPE_CRLF));

            return DSL_ERROR;
         }

         /* Clear disabled line num */
         pBndCtx->nPafLineDisabled = -1;

         /* Clear current paf-available line num */
         if (pBndCtx->nPafLineHandled == nCurrentLine)
         {
            pBndCtx->nPafLineHandled = -1;
         }
      }
   }

   /* BONDING CLR state*/
   if (nLineState == DSL_LINESTATE_BONDING_CLR)
   {
      nErrorCode = DSL_CPE_BND_BondingCheck(pBndCtx, fd, nDevice);

      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Bonding Check failed, Device=%d" DSL_CPE_CRLF, nDevice));
         return nErrorCode;
      }
   }

   /* Entry to SHOWTIME */
   if ( (nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
        (nPrevLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC) )
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Device=%d entering SHOWTIME_TC_SYNC" DSL_CPE_CRLF, nDevice));

      /* Check CL information for RemotePafAvailable */
      nErrorCode = DSL_CPE_BND_RemotePafAvailableCheck(
                     fd,
                     &(pBndCtx->lineMonitorStateMachine[nDevice].RemotePafAvailable));

      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Remote Paf Check failed, Device=%d" DSL_CPE_CRLF, nDevice));
         return nErrorCode;
      }

      /* Check CL information for PafAvailable */
      nErrorCode = DSL_CPE_BND_LocalPafAvailableCheck(
                     fd,
                     &(pBndCtx->lineMonitorStateMachine[nDevice].PafAvailable));

      if (nErrorCode != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "BND - Local Paf Check failed, Device=%d" DSL_CPE_CRLF, nDevice));
         return nErrorCode;
      }

      /* enable/disable bonding */
      if ((pBndCtx->lineMonitorStateMachine[nDevice].PafAvailable) &&
         (pBndCtx->lineMonitorStateMachine[nDevice].RemotePafAvailable == 1) &&
         (pBndCtx->lineMonitorStateMachine[nDevice].PafAggregate == 1) )
      {
         pBndCtx->lineMonitorStateMachine[nDevice].PafEnable = 1;
      }
      else
      {
         pBndCtx->lineMonitorStateMachine[nDevice].PafEnable = 0;
      }


      /* Check M/S status*/
      if (pBndCtx->lineMonitorStateMachine[1-nDevice].MsStatus == DSL_BND_SLAVE)
      {
         pBndCtx->lineMonitorStateMachine[nDevice].MsStatus = DSL_BND_MASTER;
      }
      else
      {
         if (pBndCtx->lineMonitorStateMachine[1-nDevice].PafEnable != 1)
         {
            return DSL_SUCCESS;
         }

         if (pBndCtx->lineMonitorStateMachine[nDevice].PafEnable != 1)
         {
            return DSL_SUCCESS;
         }
      }
   }
   else if ((nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
            (nPrevLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC) )
   {
      /* Exit from SHOWTIME */
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "BND - Device=%d leaving SHOWTIME_TC_SYNC" DSL_CPE_CRLF, nDevice));

      pBndCtx->lineMonitorStateMachine[nDevice].RemotePafAvailable = 0;

      if (pBndCtx->lineMonitorStateMachine[nDevice].MsStatus == DSL_BND_MASTER)
      {
         pBndCtx->lineMonitorStateMachine[nDevice].MsStatus = DSL_BND_SLAVE;

         if ((pBndCtx->lineMonitorStateMachine[nDevice].PafEnable == 1) &&
             (pBndCtx->lineMonitorStateMachine[1-nDevice].PafEnable == 1) )
         {
            /* Exchange line mastership*/
            pBndCtx->lineMonitorStateMachine[1-nDevice].MsStatus = DSL_BND_MASTER;
         }
      }
      else
      {
         if (pBndCtx->lineMonitorStateMachine[nDevice].PafEnable != 1)
         {
            return DSL_SUCCESS;
         }

         if (pBndCtx->lineMonitorStateMachine[1-nDevice].PafEnable != 1)
         {
            return DSL_SUCCESS;
         }
      }
   }

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_BND_HwInit(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd)
{
   DSL_int_t ret = 0;
   DSL_BND_HwInit_t bndHwInit;

   if (pBndCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - HW init: expecting non-zero bonding context pointer!" DSL_CPE_CRLF));
      return DSL_ERROR;
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "dsl_cpe_control - VDSL BONDING version!" DSL_CPE_CRLF));

   memset(&bndHwInit, 0x0, sizeof(DSL_BND_HwInit_t));

   ret = DSL_CPE_Ioctl (fd, DSL_FIO_BND_HW_INIT, (int) &bndHwInit);

   if ((ret < 0) || (bndHwInit.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - DSL_FIO_BND_HW_INIT ioctl call failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_BND_Start(
   DSL_CPE_Control_Context_t *pCtrlCtx,
   DSL_int_t fd)
{
   DSL_Error_t ret = DSL_SUCCESS;
   DSL_int_t i;
   DSL_CPE_BND_Context_t *pBndCtx = DSL_NULL;

   if (pCtrlCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - missing CPE context pointer pCtrlCtx!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   pBndCtx = DSL_CPE_Malloc(sizeof(DSL_CPE_BND_Context_t));

   if (pBndCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - Bonding context memory allocation failed!" DSL_CPE_CRLF));

      return DSL_ERROR;
   }

   pBndCtx->lineMonitorStateMachine[0].Port = 0;
   pBndCtx->lineMonitorStateMachine[1].Port = 1;

   for (i = 0; i < DSL_CPE_MAX_DSL_ENTITIES; i++)
   {
      pBndCtx->lineMonitorStateMachine[i].PafAvailable       = -1;
      pBndCtx->lineMonitorStateMachine[i].RemotePafAvailable = -1;
      pBndCtx->lineMonitorStateMachine[i].PafAggregate       = -1;
      pBndCtx->lineMonitorStateMachine[i].PafEnable          =  0;
      pBndCtx->lineMonitorStateMachine[i].TxDataRate         =  0;

      pBndCtx->lineMonitorStateMachine[i].MsStatus           = DSL_BND_SLAVE;
   }

   memset(&(pBndCtx->remoteDiscoveryCode), 0x0, 6);

   pBndCtx->aggregateReg = 0;
   pBndCtx->nPafLineHandled = -1;
   pBndCtx->nPafLineDisabled = -1;
   pBndCtx->pCtrlCtx = pCtrlCtx;

   /* Initialize Bonding HW*/
   ret = DSL_CPE_BND_HwInit(pBndCtx, fd);

   if (ret < DSL_SUCCESS)
   {
      DSL_CPE_Free(pBndCtx);
      pCtrlCtx->pBnd = DSL_NULL;
   }
   else
   {
      pCtrlCtx->pBnd = (DSL_void_t*)pBndCtx;
   }

   return ret;
}

DSL_void_t DSL_CPE_BND_Stop(
   DSL_CPE_BND_Context_t *pBndContext)
{
   if (pBndContext != DSL_NULL)
   {
      DSL_CPE_Free(pBndContext);
   }

   return;
}

#endif /* INCLUDE_DSL_BONDING*/
