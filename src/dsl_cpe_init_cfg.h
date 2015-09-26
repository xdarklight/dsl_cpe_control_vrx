/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DSL_CPE_INIT_CFG_H
#define _DSL_CPE_INIT_CFG_H

#ifdef __cplusplus
   extern "C" {
#endif

#define DSL_CPE_FW1_SET(pFw1, szFw1) pFirmware: pFw1, nFirmwareSize: szFw1

#define DSL_CPE_FW_FEATURE_SET(nFwPId, nFwFsMode, nFwFsAdsl, nFwFsVdsl) \
   {nPlatformId: nFwPId, nFirmwareXdslModes: nFwFsMode, \
    nFirmwareAdslFeatures: nFwFsAdsl, nFirmwareVdslFeatures: nFwFsVdsl}

#define DSL_CPE_FW2_SET(pFw2, szFw2) pFirmware2: pFw2, nFirmwareSize2: szFw2

#define DSL_CPE_XTU_SET(oct0, oct1, oct2, oct3, oct4, oct5, oct6, oct7) \
   {{oct0, oct1, oct2, oct3, oct4, oct5, oct6, oct7}}

#define DSL_CPE_LINE_INV_NE_SET(pLi) pInventory: pLi

#define DSL_CPE_AUTOBOOT_CTRL_SET(opt) nAutobootStartupMode: opt

#define DSL_CPE_AUTOBOOT_CFG_SET(opt1, opt2, opt3) \
   {{bWaitBeforeConfigWrite: opt1, bWaitBeforeLinkActivation: opt2, bWaitBeforeRestart: opt3}}

#define DSL_CPE_TEST_MODE_CTRL_SET(opt) nTestModeControl: opt

#define DSL_CPE_LINE_ACTIVATE_CTRL_SET(LDSF, ACSF, STARTUP_MODE) \
   {nLDSF: LDSF, nACSF: ACSF, nStartupMode: STARTUP_MODE}

#if defined(INCLUDE_DSL_CPE_API_DANUBE)
#define DSL_CPE_LL_CFG_SET(HYBRID) {nHybrid: HYBRID}

#define DSL_CPE_SIC_SET(TCLAYER, EFM_TC_CFG_US, EFM_TC_CFG_DS, SICS) \
   {{nTcLayer: TCLAYER, nEfmTcConfigUs: EFM_TC_CFG_US, \
    nEfmTcConfigDs: EFM_TC_CFG_DS, nSystemIf: SICS}}
#endif /* #if defined(INCLUDE_DSL_CPE_API_DANUBE)*/

#if defined(INCLUDE_DSL_CPE_API_VRX)
#define DSL_CPE_LL_CFG_SET(FILTER, HS_TONE_GROUP_MODE, HS_TONE_GROUP_A, \
   HS_TONE_GROUP_V, HS_TONE_GROUP_AV, BASEADDR, IRQNUM, NTR_ENABLE) \
   {nFilter: FILTER, nHsToneGroupMode: HS_TONE_GROUP_MODE, \
    nHsToneGroup_A: HS_TONE_GROUP_A, nHsToneGroup_V: HS_TONE_GROUP_V, \
    nHsToneGroup_AV: HS_TONE_GROUP_AV, nBaseAddr: BASEADDR, nIrqNum: IRQNUM, \
    bNtrEnable: NTR_ENABLE}

#define DSL_CPE_SIC_SET(TCLAYER_A, EFM_TC_CFG_US_A, EFM_TC_CFG_DS_A, SICS_A, \
                        TCLAYER_V, EFM_TC_CFG_US_V, EFM_TC_CFG_DS_V, SICS_V) \
   {{nTcLayer: TCLAYER_A, nEfmTcConfigUs: EFM_TC_CFG_US_A, \
     nEfmTcConfigDs:EFM_TC_CFG_DS_A, nSystemIf: SICS_A}, \
    {nTcLayer: TCLAYER_V, nEfmTcConfigUs: EFM_TC_CFG_US_V, \
     nEfmTcConfigDs: EFM_TC_CFG_DS_V, nSystemIf: SICS_V}}
#endif /* #ifdef INCLUDE_DSL_CPE_API_VRX*/

#define DSL_CPE_MAC_CFG_SET(SPEED, DUPLEX, FLOW, ANEGOT, MAC_OCT_0, MAC_OCT_1, MAC_OCT_2, \
   MAC_OCT_3, MAC_OCT_4, MAC_OCT_5, MAX_FRAME_SIZE, EXT_ETH_OAM) \
   {SPEED, DUPLEX, FLOW, ANEGOT, {MAC_OCT_0, MAC_OCT_1, MAC_OCT_2, MAC_OCT_3, MAC_OCT_4, MAC_OCT_5}, \
   MAX_FRAME_SIZE, EXT_ETH_OAM}

extern DSL_InitData_t gInitCfgData;

#if defined(INCLUDE_DSL_CPE_API_VRX)
/**
   This function returns the Initial Low Level Configuration from the give file
   or the fixed default settings.

   \param pName      file name of the given LowLevelConfig
   \param ppRetLLCfg Pointer to return the settings.

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - The return pointer is set to the given configuration or ULL

*/
DSL_Error_t DSL_CPE_GetInitialLowLevelConfig( DSL_char_t const *pName,
                                          DSL_DeviceLowLevelConfig_t *pRetLLCfg );
#endif /* #ifdef INCLUDE_DSL_CPE_API_VRX*/


#ifdef __cplusplus
}
#endif

#endif /* _DSL_CPE_INIT_CFG_H */
