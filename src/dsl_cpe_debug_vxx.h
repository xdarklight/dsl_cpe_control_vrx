/******************************************************************************

                               Copyright (c) 2011
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DSL_CPE_DEBUG_VXX_H
#define _DSL_CPE_DEBUG_VXX_H

#ifdef DSL_DEBUG_TOOL_INTERFACE


#include "cmv_message_format.h"

#if defined(INCLUDE_DSL_CPE_API_VINAX)
/* VINAX driver specific headers*/
#include "drv_vinax_interface.h"
#elif defined(INCLUDE_DSL_CPE_API_VRX)
/* MEI CPE driver specific headers*/
#include "drv_mei_cpe_interface.h"
#else
#error "Device undefined!"
#endif

#if defined(INCLUDE_DSL_CPE_API_VINAX)
   #define DSL_CPE_IFX_LOW_DEV "/dev/vinax"
#elif defined(INCLUDE_DSL_CPE_API_VRX)
   #define DSL_CPE_IFX_LOW_DEV "/dev/mei_cpe"
#endif


#if defined(INCLUDE_DSL_CPE_API_VINAX)
/* Vinax driver interface IOCTLs wrappers*/
#define FIO_VXX_REQ_CONFIG         FIO_VINAX_REQ_CONFIG
#define FIO_VXX_MBOX_MSG_RAW_SEND  FIO_VINAX_MBOX_MSG_RAW_SEND
#define FIO_VXX_REG_GET            FIO_VINAX_REG_GET
#define FIO_VXX_REG_SET            FIO_VINAX_REG_SET
#define FIO_VXX_DBG_LS_WRITE       FIO_VINAX_DBG_LS_WRITE
#define FIO_VXX_DBG_LS_READ        FIO_VINAX_DBG_LS_READ
/* Vinax driver interface data types wrappers*/
typedef IOCTL_VINAX_reqCfg_t    IOCTL_VXX_reqCfg_t;
typedef IOCTL_VINAX_mboxSend_t  IOCTL_VXX_mboxSend_t;
typedef IOCTL_VINAX_regInOut_t  IOCTL_VXX_regInOut_t;
typedef IOCTL_VINAX_dbgAccess_t IOCTL_VXX_dbgAccess_t;
#elif defined(INCLUDE_DSL_CPE_API_VRX)
/* MEI CPE driver interface IOCTLs wrappers*/
#define FIO_VXX_REQ_CONFIG         FIO_MEI_REQ_CONFIG
#define FIO_VXX_MBOX_MSG_RAW_SEND  FIO_MEI_MBOX_MSG_RAW_SEND
#define FIO_VXX_REG_GET            FIO_MEI_REG_GET
#define FIO_VXX_REG_SET            FIO_MEI_REG_SET
#define FIO_VXX_DBG_LS_WRITE       FIO_MEI_DBG_LS_WRITE
#define FIO_VXX_DBG_LS_READ        FIO_MEI_DBG_LS_READ
/* MEI CPE driver interface data types wrappers*/
typedef IOCTL_MEI_reqCfg_t    IOCTL_VXX_reqCfg_t;
typedef IOCTL_MEI_mboxSend_t  IOCTL_VXX_mboxSend_t;
typedef IOCTL_MEI_regInOut_t  IOCTL_VXX_regInOut_t;
typedef IOCTL_MEI_dbgAccess_t IOCTL_VXX_dbgAccess_t;
#endif

#define WINHOST_VDSL2_DFE_CHANNELS_AVAILABLE   2

typedef struct
{
   DSL_int_t socket_fd;
   /* device file descriptor */
   DSL_int_t   arrDeviceFd[WINHOST_VDSL2_DFE_CHANNELS_AVAILABLE];
   /* response message*/
   CMV_MESSAGE_ALL_T   CmvMsg;
   /* index of free space in Buffer */
   DSL_char_t   *pEndReceive;
   /* Buffer for the received data*/
   DSL_char_t   Buffer[3*CMV_MESSAGE_SIZE];
} DSL_VXX_TcpDebugInfo_t;


#ifndef __BYTE_ORDER
   #error __BYTE_ORDER has to be defined through including of drv_dsl_cpe_os.h
#endif

#define WINHOST_REMOTE_SW_BYTE_ORDER   __BIG_ENDIAN
/*#define WINHOST_REMOTE_SW_BYTE_ORDER   __LITTLE_ENDIAN*/

/* ============================================================================
   Macros & Definitions
   ========================================================================= */

#if ((__BYTE_ORDER == __BIG_ENDIAN )    && (WINHOST_REMOTE_SW_BYTE_ORDER == __LITTLE_ENDIAN)) || \
    ((__BYTE_ORDER == __LITTLE_ENDIAN ) && (WINHOST_REMOTE_SW_BYTE_ORDER == __BIG_ENDIAN))

   #define N2H_SWAP_CMV_BLOCK_WORD(cmvMsg, startIdx, count_16)\
                                 N2H_SwapCmvBlockWord(cmvMsg, startIdx, count_16)

   #define H2N_SWAP_CMV_BLOCK_WORD(cmvMsg, startIdx, count_16)\
                                 H2N_SwapCmvBlockWord(cmvMsg, startIdx, count_16)

   #define N2H_SWAP_CMV_PAYL_DWORD(cmvMsg, startIdx, count_32)\
                                 N2H_SwapCmvPaylDWord(cmvMsg, startIdx, count_32)

   #define H2N_SWAP_CMV_PAYL_DWORD(cmvMsg, startIdx, count_32)\
                                 H2N_SwapCmvPaylDWord(cmvMsg, startIdx, count_32)

#else
   #define N2H_SWAP_CMV_BLOCK_WORD(cmvMsg, startIdx, count_16)
   #define H2N_SWAP_CMV_BLOCK_WORD(cmvMsg, startIdx, count_16)
   #define N2H_SWAP_CMV_PAYL_DWORD(cmvMsg, startIdx, count_32)
   #define H2N_SWAP_CMV_PAYL_DWORD(cmvMsg, startIdx, count_32)

   /*#define SWAP_CMV_PAYL_32_VS_16(cmvMsg, startIdx, count_32) */
#endif

#define SWAP_CMV_PAYL_32_VS_16(cmvMsg, startIdx, count_32)\
                              SwapCmvPayl_32vs16(cmvMsg, startIdx, count_32)

#if 1
#define TCP_DEBUG_SWAP_HOST32_TARGET8(x)   ( (((x)&0xFFFF0000)>>16) \
                                            |(((x)&0x0000FFFF)<<16) )
#define TCP_DEBUG_SWAP_HOST32_TARGET16(x)  ( (((x)&0xFFFF0000)>>16) \
                                            |(((x)&0x0000FFFF)<<16) )

#define TCP_DEBUG_SWAP_HOST32_TARGET32(x)  ( (((x)&0xFFFF0000)>>16) \
                                            |(((x)&0x0000FFFF)<<16) )

#define TCP_DEBUG_SWAP_HOST16_TARGET16(x)  ( (((x)&0xFF00)>>8) \
                                            |(((x)&0x00FF)<<8) )

#else

#define TCP_DEBUG_SWAP_HOST32_TARGET8(x)   (x)
#define TCP_DEBUG_SWAP_HOST32_TARGET16(x)  (x)
#define TCP_DEBUG_SWAP_HOST32_TARGET32(x)  (x)
#define TCP_DEBUG_SWAP_HOST16_TARGET16(x)  (x)

#endif


#if 1

#define TCP_DEBUG_SWAP_TARGET8_HOST32(x)   ( (((x)&0xFFFF0000)>>16) \
                                     |(((x)&0x0000FFFF)<<16) )
#define TCP_DEBUG_SWAP_TARGET16_HOST32(x)  ( (((x)&0xFFFF0000)>>16) \
                                     |(((x)&0x0000FFFF)<<16) )
#define TCP_DEBUG_SWAP_TARGET32_HOST32(x)  ( (((x)&0xFFFF0000)>>16) \
                                     |(((x)&0x0000FFFF)<<16) )
#else

#define TCP_DEBUG_SWAP_TARGET8_HOST32(x)   (x)
#define TCP_DEBUG_SWAP_TARGET16_HOST32(x)  (x)
#define TCP_DEBUG_SWAP_TARGET32_HOST32(x)  (x)

#endif


/* ============================================================================
   Debug Read/Write Buffer
   ========================================================================= */
/* two 32bit elements margin, if request address is not 32bit aligned */
typedef union cmv_debug_access_buffer_s
{
   unsigned char  params_8Bit[CMV_USED_PAYLOAD_8BIT_SIZE + 8];
   unsigned short params_16Bit[CMV_USED_PAYLOAD_16BIT_SIZE + 4];
   unsigned int   params_32Bit[CMV_USED_PAYLOAD_32BIT_SIZE + 2];
} CMV_DEBUG_ACCESS_BUFFER_T;


/* ============================================================================
   CMV Message format: offset code [16 bit]
   ========================================================================= */
#define CMV_MSGHDR_CODE_MBOX_CODE_POS           (0)
#define CMV_MSGHDR_CODE_MBOX_CODE_MASK          (0x00FF << CMV_MSGHDR_CODE_MBOX_CODE_POS)

#define CMV_MSGHDR_CODE_PORT_NUMBER_POS         (8)
#define CMV_MSGHDR_CODE_PORT_NUMBER_MASK        (0x003F << CMV_MSGHDR_CODE_PORT_NUMBER_POS)

#define CMV_MSGHDR_CODE_RESERVED_POS            (14)
#define CMV_MSGHDR_CODE_RESERVED_MASK           (0x0003 << CMV_MSGHDR_CODE_RESERVED_POS)


/*
   Access Macros Offset code
*/
#define P_CMV_MSGHDR_CODE_MBOX_CODE_GET(pmsg) \
         ((pmsg->header.mbxCode & CMV_MSGHDR_CODE_MBOX_CODE_MASK) >> CMV_MSGHDR_CODE_MBOX_CODE_POS)

#define CMV_MSGHDR_CODE_MBOX_CODE_GET(msg) \
         ((msg.header.mbxCode & CMV_MSGHDR_CODE_MBOX_CODE_MASK) >> CMV_MSGHDR_CODE_MBOX_CODE_POS)

#define P_CMV_MSGHDR_CODE_MBOX_CODE_SET(pmsg, val) ( pmsg->header.mbxCode = \
         ( (pmsg->header.mbxCode & ~(CMV_MSGHDR_CODE_MBOX_CODE_MASK)) | \
           ((val << CMV_MSGHDR_CODE_MBOX_CODE_POS) & (CMV_MSGHDR_CODE_MBOX_CODE_MASK)) ) )

#define CMV_MSGHDR_CODE_MBOX_CODE_SET(msg, val) ( msg.header.mbxCode = \
         ( (msg.header.mbxCode & ~(CMV_MSGHDR_CODE_MBOX_CODE_MASK)) | \
           ((val << CMV_MSGHDR_CODE_MBOX_CODE_POS) & (CMV_MSGHDR_CODE_MBOX_CODE_MASK)) ) )

#define P_CMV_MSGHDR_CODE_PORT_NUMBER_GET(pmsg) \
         ((pmsg->header.mbxCode & (CMV_MSGHDR_CODE_PORT_NUMBER_MASK)) >> CMV_MSGHDR_CODE_PORT_NUMBER_POS)

#define CMV_MSGHDR_CODE_PORT_NUMBER_GET(msg) \
         ((msg.header.mbxCode & (CMV_MSGHDR_CODE_PORT_NUMBER_MASK)) >> CMV_MSGHDR_CODE_PORT_NUMBER_POS)

#define P_CMV_MSGHDR_CODE_PORT_NUMBER_SET(pmsg, val) ( pmsg->header.mbxCode = \
         ( (pmsg->header.mbxCode & ~(CMV_MSGHDR_CODE_PORT_NUMBER_MASK)) | \
           ((val << CMV_MSGHDR_CODE_PORT_NUMBER_POS) & (CMV_MSGHDR_CODE_PORT_NUMBER_MASK)) ) )

#define CMV_MSGHDR_CODE_PORT_NUMBER_SET(msg, val) ( msg.header.mbxCode = \
         ( (msg.header.mbxCode & ~(CMV_MSGHDR_CODE_PORT_NUMBER_MASK)) | \
           ((val << CMV_MSGHDR_CODE_PORT_NUMBER_POS) & (CMV_MSGHDR_CODE_PORT_NUMBER_MASK)) ) )


#define P_CMV_MSGHDR_CODE_RESERVED_GET(pmsg) \
         ((pmsg->header.mbxCode & CMV_MSGHDR_CODE_RESERVED_MASK) >> CMV_MSGHDR_CODE_RESERVED_POS)

#define CMV_MSGHDR_CODE_RESERVED_GET(msg) \
         ((msg.header.mbxCode & CMV_MSGHDR_CODE_RESERVED_MASK) >> CMV_MSGHDR_CODE_RESERVED_POS)

#define P_CMV_MSGHDR_CODE_RESERVED_SET(pmsg, val) ( pmsg->header.mbxCode = \
         ( (pmsg->header.mbxCode & ~(CMV_MSGHDR_CODE_RESERVED_MASK)) | \
           ((val << CMV_MSGHDR_CODE_RESERVED_POS) & (CMV_MSGHDR_CODE_RESERVED_MASK)) ) )

#define CMV_MSGHDR_CODE_RESERVED_SET(msg, val) ( msg.header.mbxCode = \
         ( (msg.header.mbxCode & ~(CMV_MSGHDR_CODE_RESERVED_MASK)) | \
           ((val << CMV_MSGHDR_CODE_RESERVED_POS) & (CMV_MSGHDR_CODE_RESERVED_MASK)) ) )


/* ============================================================================
   CMV Message format: offset 1 [16 bit]
   ========================================================================= */
#define CMV_RESPONSE_RESULT_CODE_POS      (0)
#define CMV_RESPONSE_RESULT_CODE_MASK     (0xFFFF << CMV_RESPONSE_RESULT_CODE_POS)

/*
   Access Macros Offset 1
*/
#define P_CMV_MSGHDR_RESULT_CODE_GET(pmsg) \
         ((pmsg->header.MessageID & CMV_RESPONSE_RESULT_CODE_MASK) >> CMV_RESPONSE_RESULT_CODE_POS)
#define CMV_MSGHDR_RESULT_CODE_GET(msg) \
         ((msg.header.MessageID & CMV_RESPONSE_RESULT_CODE_MASK) >> CMV_RESPONSE_RESULT_CODE_POS)
#define P_CMV_MSGHDR_RESULT_CODE_SET(pmsg, val) ( pmsg->header.MessageID = \
         (unsigned short)( (pmsg->header.MessageID & ~(CMV_RESPONSE_RESULT_CODE_MASK)) | \
           ((val << CMV_RESPONSE_RESULT_CODE_POS) & ~(CMV_RESPONSE_RESULT_CODE_MASK)) ) )
#define CMV_MSGHDR_RESULT_CODE_SET(msg, val) ( msg.header.MessageID = \
         (unsigned short)( (msg.header.MessageID & ~(CMV_RESPONSE_RESULT_CODE_MASK)) | \
           (((val) << CMV_RESPONSE_RESULT_CODE_POS) & ~(CMV_RESPONSE_RESULT_CODE_MASK)) ) )


/* ============================================================================
   CMV Message format: offset 2 [16 bit]
   ========================================================================= */
/* CMV header MSW debug address field */
#define P_CMV_MSGHDR_ADDR_MSW_GET(pmsg)         (pmsg->rawMsg[3])
#define CMV_MSGHDR_ADDR_MSW_GET(msg)            (msg.rawMsg[3])
#define P_CMV_MSGHDR_ADDR_MSW_SET(pmsg, val)    (pmsg->rawMsg[3] = val)
#define CMV_MSGHDR_ADDR_MSW_SET(msg, val)       (msg.rawMsg[3] = val)


/* ============================================================================
   CMV Message format: offset 3 [16 bit]
   ========================================================================= */
/* CMV header LSW debug address field */
#define P_CMV_MSGHDR_MEI_ADDR_GET(pmsg)         (pmsg->rawMsg[3])
#define CMV_MSGHDR_MEI_ADDR_GET(msg)            (msg.rawMsg[3])
#define P_CMV_MSGHDR_MEI_ADDR_SET(pmsg, val)    (pmsg->rawMsg[3] = val)
#define CMV_MSGHDR_MEI_ADDR_SET(msg, val)       (msg.rawMsg[3] = val)


/* ============================================================================
   CMV Message format: offset 3 [16 bit]
   ========================================================================= */
/* CMV header LSW debug address field */
#define P_CMV_MSGHDR_ADDR_LSW_GET(pmsg)         (pmsg->rawMsg[4])
#define CMV_MSGHDR_ADDR_LSW_GET(msg)            (msg.rawMsg[4])
#define P_CMV_MSGHDR_ADDR_LSW_SET(pmsg, val)    (pmsg->rawMsg[4] = val)
#define CMV_MSGHDR_ADDR_LSW_SET(msg, val)       (msg.rawMsg[4] = val)



/* ========================================================================
   Definition of the MESSAGE MAILBOX CODES.
   ===================================================================== */
/* Must be interpreted by the WinHost task */
#define CMV_MBOX_CODE_ME_MSG              0x80


/* ========================================================================
   Definition of the MESSAGE OPCODES.
   ===================================================================== */
/*
   NOTE:
      The MESSAGE OPCODE is a combination of thection fields:
      OPCODE, MSG MODE and DIRECTION
*/

/* Message Codes: WinHost-to-MEI */
#define H2DCE_DEBUG_REBOOT                      0x04
#define H2DCE_DEBUG_DOWNLOAD                    0x0A
#define H2DCE_DEBUG_RUN                         0x0C
#define H2DCE_DEBUG_HALT                        0x0E
#define H2DCE_DEBUG_READ_MEI                    0x06
#define H2DCE_DEBUG_WRITE_MEI                   0x08
#define H2DCE_DEBUG_READDEBUG                   0x14
#define H2DCE_DEBUG_WRITEDEBUG                  0x12
#define H2DCE_MBOX_PAYLOAD_SIZE_QUERY           0x16

/* Message Codes: MEI-to-Winhost */
#if 0
#define DCE2H_DEBUG_RESET_ACK                   0x03
#define DCE2H_DEBUG_REBOOT_ACK                  0x05
#define DCE2H_ERROR_OPCODE_UNKNOWN              0x0B
#define DCE2H_ERROR_ADDR_UNKNOWN                0x0D
#endif

#define DCE2H_RESPONSE                          0x0F
#define DCE2H_DEBUG_READ_MEI_REPLY              0x07
#define DCE2H_DEBUG_WRITE_MEI_REPLY             0x09
#define DCE2H_DEBUG_WRITEDEBUG_ACK              0x13
#define  DCE2H_DEBUG_READDEBUG_ACK               0x15
#define DCE2H_MBOX_PAYLOAD_SIZE_REPLY           0x17

/* Message Result Codes: MEI-to-Winhost Response */
#define DCE2H_RESPONSE_SUCCESS                  0x00
#define DCE2H_RESPONSE_TIMEOUT                  0x10
#define DCE2H_RESPONSE_MEI_REG_RD_ERROR         0x20
#define DCE2H_RESPONSE_MEI_REG_WR_ERROR         0x30
#define DCE2H_RESPONSE_HOST_DBG_PORT_RD_ERROR   0x40
#define DCE2H_RESPONSE_HOST_DBG_PORT_WR_ERROR   DCE2H_RESPONSE_TIMEOUT
#define DCE2H_RESPONSE_MODEM_READY_TIMEOUT      0x50
#define DCE2H_RESPONSE_MODEM_READY_AC_CLK       0x51

DSL_int_t DSL_CPE_DEV_DeviceOpen(DSL_char_t *pDevName, DSL_uint32_t dev_num);

#endif /* #ifdef DSL_DEBUG_TOOL_INTERFACE*/

#endif /* _DSL_CPE_DEBUG_VINAX_H */
