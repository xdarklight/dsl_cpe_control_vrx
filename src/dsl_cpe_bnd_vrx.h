/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

#ifndef _DSL_CPE_BND_VRX_H_
#define _DSL_CPE_BND_VRX_H_

#ifdef __cplusplus
extern "C" {
#endif

/** \file
   Bonding implementation/interface for CPE Control Application
*/

#define DSL_CPE_BND_TX_RATE_RATIO_UNITY   (0x0100)

typedef enum
{
   DSL_BND_MASTER = 0,
   DSL_BND_SLAVE  = 1
} DSL_CPE_BND_MsStatus_t;

/*
   LineMonitorStateMachine
*/
typedef struct {
   DSL_uint16_t Port;
   DSL_uint16_t PafAvailable;
   DSL_uint16_t RemotePafAvailable;
   DSL_uint16_t PafAggregate;
   DSL_uint16_t PafEnable;
   DSL_uint32_t TxDataRate;
   DSL_CPE_BND_MsStatus_t MsStatus;
} DSL_CPE_BND_LineMonitorStateMachine_t;

/**
   Bonding Context
*/
typedef struct
{
   /**
   Line Monitor SM data*/
   DSL_CPE_BND_LineMonitorStateMachine_t
      lineMonitorStateMachine[DSL_CPE_MAX_DSL_ENTITIES];
   /**
   Remote discovery code*/
   DSL_uint8_t remoteDiscoveryCode[6];
   /**
   Aggregate data state*/
   DSL_uint32_t aggregateReg;
   /**
   CPE API Control Context*/
   DSL_CPE_Control_Context_t *pCtrlCtx;
   /**
   TBD*/
   DSL_PortMode_t nPortMode;
   /**
   TBD*/
   DSL_boolean_t bFwRequested[DSL_CPE_MAX_DSL_ENTITIES];
   /**
   TBD*/
   DSL_boolean_t bInitialFwRequestHandled;
   /**
   Paf line num firts reach FULL_INIT*/
   DSL_int8_t nPafLineHandled;
   /**
   Line num was disabled by absent Paf*/
   DSL_int8_t nPafLineDisabled;
} DSL_CPE_BND_Context_t;

/*
   Function to start Bonding handling
*/
DSL_Error_t DSL_CPE_BND_Start(
   DSL_CPE_Control_Context_t *pCtrlCtx,
   DSL_int_t fd);

/*
   Function to stop Bonding handling
*/
DSL_void_t DSL_CPE_BND_Stop(
   DSL_CPE_BND_Context_t *pBndContext);

/*
   Function to check the remote PAF status
*/
DSL_Error_t DSL_CPE_BND_RemotePafAvailableCheck(
                     DSL_int_t fd,
                     DSL_uint16_t *pRemotePafAvailable);
/*
   Bonding handling for the Autoboot Restart Wait state
*/
DSL_Error_t DSL_CPE_BND_AutobootStatusRestartWait(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_uint_t nDevice);

/*
   Firmware Download for bonding
*/
DSL_Error_t DSL_CPE_BND_SyncDownloadFirmware(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t nDevice,
               DSL_FirmwareRequestType_t nFwReqType,
               DSL_PortMode_t nPortMode);

/*
   Bonding handling for the Line State change
*/
DSL_Error_t DSL_CPE_BND_LineStateHandle(
   DSL_CPE_BND_Context_t *pBndCtx,
   DSL_int_t fd, DSL_uint_t nDevice,
   DSL_LineStateValue_t nLineState,
   DSL_LineStateValue_t nPrevLineState);

/*
   Bonding handling for the System Interface status notification
*/
DSL_Error_t DSL_CPE_BND_SystemInterfaceStatusHandle(
               DSL_CPE_BND_Context_t *pBndCtx,
               DSL_int_t fd,
               DSL_uint_t nDevice);

#ifdef __cplusplus
}
#endif

#endif /* _DSL_CPE_BND_VRX_H_ */
