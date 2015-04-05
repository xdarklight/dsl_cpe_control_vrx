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
#include "drv_dsl_cpe_api.h"
#include "dsl_cpe_init_cfg.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_APP

#ifndef _lint
DSL_InitData_t gInitCfgData =
{
   DSL_CPE_FW1_SET(DSL_NULL, 0x0),
   DSL_CPE_FW_FEATURE_SET(0, DSL_FW_XDSLMODE_CLEANED, DSL_FW_XDSLFEATURE_CLEANED, \
      DSL_FW_XDSLFEATURE_CLEANED),
   DSL_CPE_FW2_SET(DSL_NULL, 0x0),
   DSL_CPE_XTU_SET(0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7),
   DSL_CPE_LINE_INV_NE_SET(DSL_NULL),
   DSL_CPE_AUTOBOOT_CTRL_SET(DSL_AUTOBOOT_CTRL_START),
   DSL_CPE_AUTOBOOT_CFG_SET(DSL_FALSE, DSL_FALSE, DSL_FALSE),
   DSL_CPE_TEST_MODE_CTRL_SET(DSL_TESTMODE_DISABLE),
   DSL_CPE_LINE_ACTIVATE_CTRL_SET(DSL_G997_INHIBIT_LDSF, DSL_G997_INHIBIT_ACSF, DSL_G997_NORMAL_STARTUP),
   /** Device specific configuration parameters*/
#if defined(INCLUDE_DSL_CPE_API_VRX)
   {
   DSL_CPE_LL_CFG_SET(0, 1, \
      DSL_DEV_HS_TONE_GROUP_ADSL2_A43C | DSL_DEV_HS_TONE_GROUP_VDSL2_A43, \
      DSL_DEV_HS_TONE_GROUP_ADSL2_A43C | DSL_DEV_HS_TONE_GROUP_VDSL2_A43, \
      DSL_DEV_HS_TONE_GROUP_ADSL2_A43C | DSL_DEV_HS_TONE_GROUP_VDSL2_A43, \
      0x1E116000, 0x37, 0),
   DSL_CPE_SIC_SET(DSL_TC_ATM, DSL_EMF_TC_CLEANED, DSL_EMF_TC_CLEANED, DSL_SYSTEMIF_MII, \
                   DSL_TC_EFM, DSL_EMF_TC_CLEANED, DSL_EMF_TC_CLEANED, DSL_SYSTEMIF_MII),
   DSL_CPE_MAC_CFG_SET(DSL_EFM_SPEED_100, DSL_EFM_DUPLEX_FULL, DSL_EFM_FLOWCTRL_ON, DSL_EFM_AUTONEG_OFF, \
      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 1536, DSL_FALSE)
   }
#endif
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   {
   DSL_CPE_LL_CFG_SET(DSL_DEV_HYBRID_NA),
   DSL_CPE_SIC_SET(DSL_TC_ATM, DSL_EMF_TC_NORMAL, DSL_EMF_TC_NORMAL, DSL_SYSTEMIF_MII)
   }
#endif
};
#endif /* _lint*/

#if defined(INCLUDE_DSL_CPE_API_VRX)
static DSL_Error_t DSL_VRX_ProcessDevConfigLine(
   DSL_char_t *pLine,
   DSL_uint32_t cfgLine,
   DSL_DeviceLowLevelConfig_t *pRetLLCfg )
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_int_t nFilter = -1, nHsToneGroupMode = -1, nHsToneGroup_A = -1,
             nHsToneGroup_V = -1, nHsToneGroup_AV = -1,
             nBaseAddr = 0, nIrqNum = 0,
             bNtrEnable = -1;

   switch(cfgLine)
   {
   case 0:
      DSL_CPE_sscanf(pLine,
         "%d",
        &nFilter);

      pRetLLCfg->nFilter                   = (DSL_uint32_t)nFilter;
      break;
   case 1:
      DSL_CPE_sscanf(pLine,
         "%d %x %x %x",
        &nHsToneGroupMode,
        &nHsToneGroup_A,
        &nHsToneGroup_V,
        &nHsToneGroup_AV);

      pRetLLCfg->nHsToneGroupMode          = (DSL_uint32_t)nHsToneGroupMode;
      pRetLLCfg->nHsToneGroup_A            = (DSL_uint32_t)nHsToneGroup_A;
      pRetLLCfg->nHsToneGroup_V            = (DSL_uint32_t)nHsToneGroup_V;
      pRetLLCfg->nHsToneGroup_AV           = (DSL_uint32_t)nHsToneGroup_AV;
   case 2:
      DSL_CPE_sscanf(pLine,
         "%x %d",
        &nBaseAddr,
        &nIrqNum);

      pRetLLCfg->nBaseAddr                 = (DSL_uint32_t)nBaseAddr;
      pRetLLCfg->nIrqNum                   = (DSL_int8_t)nIrqNum;
      break;
   case 3:
      DSL_CPE_sscanf(pLine,
         "%d",
        &bNtrEnable);

      pRetLLCfg->bNtrEnable                = (DSL_boolean_t)bNtrEnable;
      break;
   default:
      break;
   }

   return(nRet);
}

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
                                          DSL_DeviceLowLevelConfig_t *pRetLLCfg )
{
   DSL_CPE_File_t *fdCfg = DSL_NULL;
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_uint32_t nCfgFileLine;
   DSL_char_t  line[256];

/*
# VRX Low Level Configuration File
#
# Parameters must be separated by tabs or spaces.
# Empty lines and comments will be ignored.
#

# nFilter
#
# NA     = -1
# OFF    = 0
# ISDN   = 1
# POTS   = 2
# POTS_2 = 3
# POTS_3 = 4
#
#  (dec)
    -1

# nHsToneGroupMode nHsToneGroup_A       nHsToneGroup_V    nHsToneGroup_AV
#
# NA     = -1      NA         = -1      see               see
# AUTO   = 0       VDSL2_B43  = 0x0001  nHsToneGroup_A    nHsToneGroup_A
# MANUAL = 1       VDSL2_A43  = 0x0002
#                  VDSL2_V43  = 0x0004
#                  VDSL1_V43P = 0x0008
#                  VDSL1_V43I = 0x0010
#                  ADSL1_C43  = 0x0020
#                  ADSL2_J43  = 0x0040
#                  ADSL2_B43C = 0x0080
#                  ADSL2_A43C = 0x0100
#
#  (dec)           (hex)                (hex)             (hex)
     0              0x1                  0x1               0x1

#   nBaseAddr     nIrqNum
#
#     (hex)        (dec)
    0x1e116000      55

# nUtopiaPhyAdr   nUtopiaBusWidth      nPosPhyParity
#                 default(16b) = 0     NA   = -1
#                 8-bit        = 1     ODD  = 0
#                 16-bit       = 2
#
#
#    (hex)            (dec)                (dec)
      0xFF              0                    0

# bNtrEnable
#
#  (dec)
    0
*/

   if (pName != DSL_NULL)
   {
      fdCfg = DSL_CPE_FOpen(pName, "r");
      if (fdCfg == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "DSL: Error opening low level configuration file %s " DSL_CPE_CRLF , pName));
         return DSL_ERROR;
      }

      nCfgFileLine = 0;
      while( (DSL_CPE_FGets(line, sizeof(line), fdCfg)) != DSL_NULL )
      {
         if( (line[0] == '\n') || (line[0] == '#') )
         {
            continue;
         }

         nRet = DSL_VRX_ProcessDevConfigLine(line, nCfgFileLine, pRetLLCfg);

         nCfgFileLine++;

         if( nRet != DSL_SUCCESS )
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "DSL: Error processing configuration file %s, line %d "DSL_CPE_CRLF, pName, nCfgFileLine));
            break;
         }
      }
      DSL_CPE_FClose (fdCfg);
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "DSL: Low Level Configuration file not specified, using default configuration"DSL_CPE_CRLF));
   }

   return nRet;
}
#endif /* #if defined(INCLUDE_DSL_CPE_API_VRX)*/
