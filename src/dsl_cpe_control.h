/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DSL_CPE_CONTROL_H
#define _DSL_CPE_CONTROL_H

/** \defgroup DSL_CPE_CONTROL Lantiq DSL CPE API Control Application
    Lists the entire modules to the DSL CPE_API Control Application.
  @{ */

/**
   \defgroup DSL_CPE_CLI Implementation of Command Line Interface access commands
   This module implements the Command Line Interface.
*/

/** @} */

#ifdef HAVE_CONFIG_H
#include "dsl_cpe_config.h"
#endif

/* To disable patch version handling set up definition below.*/
/* #define DISABLE_DSL_PATCH_VERSION*/

#ifndef DSL_DOC_GENERATION_EXCLUDE_UNWANTED

#include "dsl_cpe_os.h"

#ifdef WIN32
#include "dsl_cpe_win32.h"
#endif /* WIN32*/

/* Maximum number of FW reload retries in case of a failed FW download*/
#define DSL_CPE_MAX_FW_RELOAD_RETRY_COUNT   (5)

#if defined(INCLUDE_DSL_ADSL_MIB) && defined (INCLUDE_DSL_CPE_API_VRX)
   #error "MIB not supported for the VRX devices yet!!!"
#endif

extern DSL_char_t *g_sFirmwareName1;
extern DSL_FirmwareFeatures_t g_nFwFeatures1;

extern DSL_char_t *g_sFirmwareName2;
extern DSL_FirmwareFeatures_t g_nFwFeatures2;

/*
#if defined(DSL_DEBUG_TOOL_INTERFACE) && (!defined(INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT))
#error "DSL Debug Interface can be used with Linux and file \
system support enabled only"
#endif
*/

#if defined(DSL_DEBUG_TOOL_INTERFACE) && !defined(INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT)
   #error "Debug Tool Interface needs --enable-cmv-scripts option enabled!"
#endif

/* \todo [RTT] Temporary exclude RTT because of important bugfix is needed first! */
#if defined(INCLUDE_REAL_TIME_TRACE)
#error Real Time Trace functionality is currently not supported.
#endif

#if defined(DSL_DEBUG_TOOL_INTERFACE) || defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
#ifdef LINUX
   #define DSL_DEBUG_TOOL_INTERFACE_DEFAULT_IFACE "eth0"
#elif VXWORKS
   #define DSL_DEBUG_TOOL_INTERFACE_DEFAULT_IFACE "ei0"
#else
   #error "OS type undefined!"
#endif
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #define DSL_CPE_DSL_LOW_DEV "/dev/ifx_mei"
#elif defined (INCLUDE_DSL_CPE_API_VRX)
   #define DSL_CPE_DSL_LOW_DEV "/dev/mei_cpe"
#else
   #error "Device undefined!"
#endif

/**
   This define specifies the maximum number of device instances
*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
#ifdef DSL_CPE_MAX_DEVICE_NUMBER
   #if (DSL_CPE_MAX_DEVICE_NUMBER > 1) || (DSL_CPE_MAX_DEVICE_NUMBER == 0)
   #error "DSL_CPE_MAX_DEVICE_NUMBER should be 1!!!"
   #endif
#else
   /* Danube/Amazon-Se/ARX100 low level driver support only single instance */
   #define DSL_CPE_MAX_DEVICE_NUMBER 1
#endif /* DSL_CPE_MAX_DEVICE_NUMBER*/
#endif /* defined (INCLUDE_DSL_CPE_API_VRX)*/

#if defined(RTEMS)
   #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
      #error "No support for DSL_CPE_MAX_DEVICE_NUMBER > 1 under RTEMS!!!"
   #endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* defined(RTEMS)*/

#if !defined (DSL_CPE_LINES_PER_DEVICE)
#  define DSL_CPE_LINES_PER_DEVICE 1
#else
#  if (DSL_CPE_LINES_PER_DEVICE == 0)
#     error "DSL_CPE_LINES_PER_DEVICE should not be 0!!!"
#  endif
#endif

#if defined (INCLUDE_DSL_CPE_API_VRX)
#  if (DSL_CPE_LINES_PER_DEVICE > 2)
#     error "DSL_CPE_LINES_PER_DEVICE incorrect, please fix!!!"
#  endif
#elif defined (INCLUDE_DSL_CPE_API_DANUBE)
#  if (DSL_CPE_LINES_PER_DEVICE > 1)
#     error "DSL_CPE_LINES_PER_DEVICE incorrect, please fix!!!"
#  endif
#endif

#define DSL_CPE_MAX_DSL_ENTITIES (DSL_CPE_MAX_DEVICE_NUMBER * DSL_CPE_LINES_PER_DEVICE)

#if defined(INCLUDE_DSL_BONDING) && defined (INCLUDE_DSL_CPE_API_VRX)
#  ifndef INCLUDE_FW_REQUEST_SUPPORT
#     error "Bonding is only supported with the FW request feature!!!"
#  endif
#  ifndef INCLUDE_SCRIPT_NOTIFICATION
#     error "Bonding is only supported with the Script Notification feature!!!"
#  endif
#endif

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_types.h"
#include "drv_dsl_cpe_api_error.h"

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   #include "dsl_cpe_cli.h"
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#define DSL_DEV_TIMEOUT_SELECT   1000

#define DSL_CPE_FW_CHUNK_SIZE    (64 * 1024)

/*#define INCLUDE_DSL_API_CLI_LEGACY*/

/*
   Security checks for configure options
*/
#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   #ifndef INCLUDE_DSL_CPE_CLI_SUPPORT
      #error "SOAP support requires CLI please define 'INCLUDE_DSL_CPE_CLI_SUPPORT'"
   #endif
#endif /* INCLUDE_DSL_CPE_SOAP_SUPPORT */

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT) && defined(INCLUDE_DSL_CPE_API_DANUBE)
   #ifndef INCLUDE_DSL_CPE_CLI_SUPPORT
      #error "DTI support for Danube requires CLI please define 'INCLUDE_DSL_CPE_CLI_SUPPORT'"
   #endif
#endif /* INCLUDE_DSL_CPE_DTI_SUPPORT */

#if !defined (INCLUDE_DSL_CPE_SOAP_SUPPORT) && !defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
#define DSL_CPE_REMOVE_PIPE_SUPPORT
#endif


#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
#define DSL_CPE_SOAP_PORT           8080
   /* Enable firmware update functionality in case of SOAP support by default */
   #define DSL_CPE_SOAP_FW_UPDATE
#endif /* INCLUDE_DSL_CPE_SOAP_SUPPORT */

extern DSL_boolean_t g_bWaitBeforeLinkActivation[DSL_CPE_MAX_DSL_ENTITIES];
extern DSL_boolean_t g_bWaitBeforeConfigWrite[DSL_CPE_MAX_DSL_ENTITIES];
extern DSL_boolean_t g_bWaitBeforeRestart[DSL_CPE_MAX_DSL_ENTITIES];

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
extern DSL_char_t *g_sAdslScript;
extern DSL_char_t *g_sVdslScript;
extern DSL_boolean_t g_bAutoContinueWaitBeforeLinkActivation[DSL_CPE_MAX_DSL_ENTITIES];
extern DSL_boolean_t g_bAutoContinueWaitBeforeConfigWrite[DSL_CPE_MAX_DSL_ENTITIES];
extern DSL_boolean_t g_bAutoContinueWaitBeforeRestart[DSL_CPE_MAX_DSL_ENTITIES];
#endif

#ifdef INCLUDE_SCRIPT_NOTIFICATION
extern DSL_char_t *g_sRcScript;
DSL_void_t DSL_CPE_ScriptRun(DSL_void_t);
#endif /* INCLUDE_SCRIPT_NOTIFICATION*/

#define MAX_WHAT_STRING_LEN   64
#define WHAT_STRING_CHUNK_LEN  (64*1024)

#define DSL_CPE_DEVICE_NAME         "/dev/dsl_cpe_api"
#define DSL_CPE_PREFIX              "DSL_CPE: "
#define DSL_CPE_SCRIPT_PREFIX       "DSL_CPE_SCRIPT: "

#define DSL_SCRIPT_PRINTF (void) DSL_CPE_FPrintf

#define DSL_SCRIPT_DEBUG(echo, body) \
{ \
   if (echo) \
   { \
      DSL_SCRIPT_PRINTF body; \
   } \
}

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
   #define DSL_CPE_SHOWTIME_EVENT_LOGGING_FILENAME "/tmp/post_mortem_data.txt"
#endif


#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif


typedef unsigned int DSL_size_t;

typedef void (DSL_CliFunction_t) (DSL_int32_t fd,
                                  DSL_char_t *pCommands,
                                  DSL_CPE_File_t *out);

typedef struct
{
   char *sShortCommand;
   char *sLongCommand;
   DSL_CliFunction_t *pFunction;
} DSL_CliCommand_t;


DSL_int32_t DSL_CPE_sscanf (DSL_char_t * buf, DSL_char_t const *fmt, ...);

#ifdef INCLUDE_DSL_API_CLI_LEGACY

#define GET_ADSL_LINE_STATUS        0
#define DANUBE_MEI_CMV_WINHOST      0
#define LINE_RATE_DATA_RATEDS_FLAG  1
#define LINE_RATE_DATA_RATEUS_FLAG  2

typedef struct
{
   DSL_uint32_t dummy;
} adslLineStatusInfo;

typedef struct
{
   DSL_uint32_t flags;
   DSL_uint32_t adslDataRateus;
   DSL_uint32_t adslDataRateds;
} adslLineRateInfo;

#endif /* INCLUDE_DSL_API_CLI_LEGACY */

#if defined(INCLUDE_DSL_CPE_API_VRX)
/* Structure to keep the decoded Firmware Verion information
   contained in the 32bit Version number */
typedef struct
{
   /* Revision 1, 2, ... */
   DSL_uint8_t nPlatform;
   /* */
   DSL_uint8_t nFeatureSet;
   /* */
   DSL_uint8_t nMajor;
   /* */
   DSL_uint8_t nMinor;
   /* Pre, Verification, Development */
   DSL_uint8_t nReleaseStatus;
   /* VDSL1/2, ADSL, ... */
   DSL_uint8_t nApplication;
} DSL_VRX_FwVersion_t;
#endif /* defined(INCLUDE_DSL_CPE_API_VRX)*/

#ifdef INCLUDE_DSL_CPE_API_DANUBE
/* Structure to keep the decoded Firmware Verion information
   contained in the 32bit Version number */
typedef struct
{
   /* Major version*/
   DSL_uint8_t nMajor;
   /* Minor version*/
   DSL_uint8_t nMinor;
   /* Release State */
   DSL_uint8_t nRelState;
   /* Annex A, Annex B*/
   DSL_uint8_t nApplication;
   /* External Version*/
   DSL_uint8_t nExtVersion;
   /* Internal Version */
   DSL_uint8_t nIntVersion;
} DSL_DANUBE_FwVersion_t;
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/

/*
   Structure for the SW (driver, application) version*/
typedef struct
{
   /* Major version*/
   DSL_int_t nMajor;
   /* Minor version*/
   DSL_int_t nMinor;
   /* Development version*/
   DSL_int_t nDevelopment;
   /* Maintenance version*/
   DSL_int_t nMaintenance;
#ifndef DISABLE_DSL_PATCH_VERSION
   /* Patch number*/
   DSL_int_t nPatch;
#endif /*#ifndef DISABLE_DSL_PATCH_VERSION*/
} DSL_SwVersion_t;

/*
   This structure is used to get resource usage statistics
   data
*/
typedef struct
{
   /*
   Total memory allocated statically (bytes) */
   DSL_uint32_t staticMemUsage;
   /*
   Total memory allocated dynamically (bytes) */
   DSL_uint32_t dynamicMemUsage;
} DSL_CPE_ResourceUsageStatisticsData_t;

typedef struct
{
   /*
   File name is only used in case of using firmware binary that is located
   within file system. If firmware that is used which is already stored
   within context of DSL CPE Control Application (for example in case of
   using SOAP firmware download function) this pointer shall be DSL_NULL. */
   DSL_char_t *pFileName;
   /*
   Pointer to firmware binary in case of using data stored within context of
   DSL CPE Control Application. In case of using reference to file instead
   (pFileName does not equal DSL_NULL) this pointer shall be DSL_NULL. */
   DSL_uint8_t *pData;
   /*
   Size of firmware binary in case of using data stored within context of
   DSL CPE Control Application. In case of using reference to file instead
   (pFileName does not equal DSL_NULL) this parameter shall be zero. */
   DSL_uint32_t nSize;
   /*
   Includes information about xDSL mode related firmware features */
   DSL_FirmwareFeatures_t fwFeatures;
} DSL_CPE_Firmware_t;

typedef struct
{
   DSL_int_t fd[DSL_CPE_MAX_DSL_ENTITIES];
   DSL_boolean_t bRun;
   DSL_CPE_ThreadCtrl_t EventControl;
   DSL_boolean_t bEvtRun;
   DSL_boolean_t bBackwardCompMode;
   DSL_int_t nDevNum;
#ifdef DSL_DEBUG_TOOL_INTERFACE
   DSL_CPE_ThreadCtrl_t nTcpMsgHandler;
   DSL_void_t *pDebugClientInfo;
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_CPE_ThreadCtrl_t nTcpCliHandler;
   DSL_void_t *pDebugCliClientInfo;
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
#endif /* DSL_DEBUG_TOOL_INTERFACE*/
#ifdef DSL_CPE_SOAP_FW_UPDATE
   #define DSL_CPE_SOAP_FWNAME_1    "SoapFwName1"
   #define DSL_CPE_SOAP_FWNAME_2    "SoapFwName2"
   /* Semaphore used in case of updating the firmware binar(y/ies) via SOAP */
   DSL_CPE_Lock_t semFwUpdate;
   /*
   Specifies an firmware that might be updated using SOAP */
   DSL_CPE_Firmware_t firmware;
   /*
   Specifies an firmware that might be updated using SOAP */
   DSL_CPE_Firmware_t firmware2;
#endif /* DSL_CPE_SOAP_FW_UPDATE */

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_CLI_Context_t *pCLI_List_head;
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   DSL_FirmwareStatusType_t nFwModeStatus;
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT*/
   /* DSL CPE API driver version*/
   DSL_SwVersion_t driverVer;
   /* DSL CPE API control application version*/
   DSL_SwVersion_t applicationVer;
#ifdef INCLUDE_DSL_BONDING
   DSL_void_t *pBnd;
#endif /* INCLUDE_DSL_BONDING*/
} DSL_CPE_Control_Context_t;

#include "dsl_cpe_debug.h"

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT

#define DSL_CPE_SCRIPT_WAIT_FOR_CONFIGURATION_TAG   "WaitForConfiguration"
#define DSL_CPE_SCRIPT_WAIT_FOR_LINK_ACTIVATE_TAG   "WaitForLinkActivate"
#define DSL_CPE_SCRIPT_WAIT_BEFORE_RESTART_TAG      "WaitForRestart"
#define DSL_CPE_SCRIPT_COMMON_TAG   "Common"

typedef enum
{
   /* Specifies common section.*/
   DSL_SCRIPT_SECTION_COMMON = 1,
   /* Specifies "WaitForConfiguration" section within
      Autoboot script*/
   DSL_SCRIPT_SECTION_WAIT_FOR_CONFIFURATION = 2,
   /* Specifies "WaitForLinkActivate" section within
      Autoboot script*/
   DSL_SCRIPT_SECTION_WAIT_FOR_LINK_ACTIVATE = 3,
   /* Specifies "WaitBeforeRestart" section within
      Autoboot script*/
   DSL_SCRIPT_SECTION_WAIT_BEFORE_RESTART = 4
} DSL_CPE_ScriptSection_t;
#endif /* #ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT*/

/**
   Structure for parsing startup arguments.
*/
typedef struct
{
   /** element value (to be returned) */
   DSL_int_t nValue;
   /** element base (for strtoul function,
       e.g. 10 for decimal or 16 for hex values) */
   DSL_int_t nBase;
} DSL_CPE_ArgElement_t;

/*
  A structure for event type<->string conversion tables.
*/
typedef struct
{
   /** event type */
   DSL_EventType_t eventType;
   /** event string */
   DSL_char_t const *string;
} DSL_CPE_EVT_CodeString_t;

DSL_CPE_Control_Context_t *DSL_CPE_GetGlobalContext(DSL_void_t);

#if defined(INCLUDE_DSL_CPE_API_VRX)
DSL_Error_t DSL_CPE_LowLevelConfigurationCheck(
   DSL_int_t fd);
#endif

DSL_Error_t DSL_CPE_LoadFirmwareFromFile
(
   DSL_char_t *psFirmwareName,
   DSL_uint8_t **pFirmware,
   DSL_uint32_t *pnFirmwareSize
);

DSL_Error_t DSL_CPE_DownloadFirmware
(
   DSL_int_t fd,
   DSL_FirmwareRequestType_t nFwReqType,
   DSL_char_t *pcFw,
   DSL_char_t *pcFw2
);

DSL_Error_t DSL_CPE_FwFeaturesGet
(
   DSL_char_t *pcFw,
   DSL_FirmwareFeatures_t *pFwFeatures
);

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
typedef enum
{
   /* Format as hey values */
   DSL_ARRAY_FORMAT_HEX,
   /* Format as string */
   DSL_ARRAY_FORMAT_STRING
} DSL_CPE_ArrayPrintFormat_t;

DSL_void_t DSL_CPE_ArraySPrintF
(
   DSL_char_t *pDst,
   DSL_void_t *pSrc,
   DSL_uint16_t nSrcSize,
   DSL_uint16_t nSrcElementSize,
   DSL_CPE_ArrayPrintFormat_t nFormat
);

DSL_Error_t DSL_CPE_MoveCharPtr
(
   DSL_char_t *pCommands,
   DSL_int_t nParamNr,
   DSL_char_t *pSeps,
   DSL_char_t **pCmdOffset
);

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_char_t *DSL_CPE_Fd2DevStr(DSL_int_t fd);
DSL_Error_t DSL_CPE_Fd2DevNum(DSL_int_t fd, DSL_uint32_t *nDevice);

DSL_boolean_t DSL_CPE_IsFileExists(DSL_char_t *path);
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

DSL_int_t DSL_CPE_CliDeviceCommandExecute(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_char_t *cmd,
   DSL_char_t *arg,
   DSL_CPE_File_t *out);

#if defined(INCLUDE_DSL_CPE_API_VRX)
typedef struct
{
   DSL_uint8_t nAdr[DSL_MAC_ADDRESS_OCTETS];
} DSL_CPE_MacAddress_t;

DSL_Error_t DSL_CPE_GetMacAdrFromString
(
   DSL_char_t *pString,
   DSL_CPE_MacAddress_t *pMacAdr
);
#endif /* defined(INCLUDE_DSL_CPE_API_VRX) */
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

/*
   some declarations for modules without own header
*/
#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   DSL_int32_t DSL_CPE_SoapInit(DSL_CPE_Control_Context_t *pContext);
   DSL_int32_t DSL_CPE_SoapExit(DSL_void_t);

   #ifdef DSL_CPE_SOAP_FW_UPDATE
      DSL_Error_t DSL_CPE_SoapFirmwareUpdate
      (
         DSL_CPE_Firmware_t *pFirmware,
         DSL_CPE_Firmware_t *pFirmware2
      );

      DSL_Error_t DSL_CPE_SoapFirmwareStore
      (
         DSL_CPE_Control_Context_t *pContext,
         DSL_CPE_Firmware_t *pSrcFirmware,
         DSL_CPE_Firmware_t *pDestFirmware,
         DSL_char_t *pFwSoapName
      );
   #endif /* DSL_CPE_SOAP_FW_UPDATE */
#endif /* INCLUDE_DSL_CPE_SOAP_SUPPORT */

/*
   Start Event handler thread
*/
DSL_Error_t DSL_CPE_EventHandlerStart (
   DSL_CPE_Control_Context_t * pContext
);

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
DSL_void_t DSL_CPE_Echo
(
   DSL_char_t *buf
);

DSL_Error_t DSL_CPE_ScriptExecute
(
   DSL_CPE_Control_Context_t * pContext,
   DSL_int_t nDevice,
   DSL_CPE_File_t *pFile,
   DSL_CPE_ScriptSection_t searchSection
);

DSL_Error_t DSL_CPE_ScriptFileParse
(
   DSL_CPE_Control_Context_t * pContext,
   DSL_int_t nDevice,
   DSL_char_t *sFileName,
   DSL_CPE_ScriptSection_t searchSection
);
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_ResourceUsageStatisticsGet
(
   DSL_CPE_Control_Context_t *pContext,
   DSL_CPE_ResourceUsageStatisticsData_t *pData
);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

#endif /* #ifndef DSL_DOC_GENERATION_EXCLUDE_UNWANTED*/

#endif /* _DSL_CPE_CONTROL_H */

