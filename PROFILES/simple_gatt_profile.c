/******************************************************************************

 @file  simple_gatt_profile.c

 @brief This file contains the Simple GATT profile sample GATT service profile
        for use with the BLE sample application.

 Group: WCS, BTS
 Target Device: CC1350

 ******************************************************************************
 
 Copyright (c) 2010-2017, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 Release Name: ti-ble-2.3.1-stack-sdk_1_60_xx
 Release Date: 2017-12-16 12:03:51
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "bcomdef.h"
#include "osal.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "simple_gatt_profile.h"


/*********************************************************************
 * MACROS
 */

#ifndef min
#define min(a, b)               (((a) < (b)) ? (a) : (b))
#endif

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        12

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 simpleProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

CONST uint8 UserChallangeProfileCharUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(USER_CHALLANGE_UUID), HI_UINT16(USER_CHALLANGE_UUID)
};

CONST uint8 ServerResponseProfileCharUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SERVER_RESPONSE_UUID), HI_UINT16(SERVER_RESPONSE_UUID)
};

CONST uint8 ResponseReadyProfileCharUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(RESPONSE_READY_UUID), HI_UINT16(RESPONSE_READY_UUID)
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static simpleProfileCBs_t *simpleProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Simple Profile Service attribute
static CONST gattAttrType_t simpleProfileService = { ATT_BT_UUID_SIZE, simpleProfileServUUID };


// Simple Profile Characteristic 1 Properties
static uint8 UserChallangeProfileCharProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 1 Value
static uint8 UserChllangeProfileBuffer[USER_CHALLANGE_CHAR_LENGTH] = "";

// Simple Profile Characteristic 1 User Description
static uint8 UserChallangeCharDesp[15] = "User Challange";


// Simple Profile Characteristic 2 Properties
static uint8 ServerResponseProfileCharProps = GATT_PROP_READ;

// Characteristic 2 Value
static uint8 ServerResonseProfileBuffer[SERVER_RESPONSE_CHAR_LENGTH] = "";

// Simple Profile Characteristic 2 User Description
static uint8 ServerResponseProfileDesp[16] = "Server Response";

// Simple Profile Characteristic 2 Properties
static uint8 ResponseReadyProfileCharProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY;

// Characteristic 2 Value
static uint8 ResponseReadyProfileBuffer[RESPONSE_READY_CHAR_LENGTH] = "";

// Simple Profile Characteristic 2 User Description
static uint8 ResponseReadyProfileDesp[16] = "Response Status";

static gattCharCfg_t *ResponseReadyCharConfig;

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t simpleProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Simple Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&simpleProfileService            /* pValue */
  },

    // Characteristic 1 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &UserChallangeProfileCharProps
    },

      // Characteristic Value 1
      { 
        { ATT_BT_UUID_SIZE, UserChallangeProfileCharUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        UserChllangeProfileBuffer
      },

      // Characteristic 1 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        UserChallangeCharDesp
      },      

    // Characteristic 2 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &ServerResponseProfileCharProps
    },

      // Characteristic Value 2
      { 
        { ATT_BT_UUID_SIZE, ServerResponseProfileCharUUID },
        GATT_PERMIT_READ, 
        0, 
        ServerResonseProfileBuffer
      },

      // Characteristic 2 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        ServerResponseProfileDesp
      },           

      // Characteristic 3 Declaration
      {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &ResponseReadyProfileCharProps
      },

        // Characteristic Value 3
        {
          { ATT_BT_UUID_SIZE, ResponseReadyProfileCharUUID },
          GATT_PERMIT_READ | GATT_PERMIT_WRITE,
          0,
          ResponseReadyProfileBuffer
        },

        // Characteristic 3 configuration
        {
          { ATT_BT_UUID_SIZE, clientCharCfgUUID },
          GATT_PERMIT_READ | GATT_PERMIT_WRITE,
          0,
          (uint8 *)&ResponseReadyCharConfig
        },

        // Characteristic 3 User Description
        {
          { ATT_BT_UUID_SIZE, charUserDescUUID },
          GATT_PERMIT_READ,
          0,
          ResponseReadyProfileDesp
        },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr, 
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method);
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Simple Profile Service Callbacks
// Note: When an operation on a characteristic requires authorization and 
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the 
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an 
// operation on a characteristic requires authorization the Stack will call 
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be 
// made within these functions.
CONST gattServiceCBs_t simpleProfileCBs =
{
  simpleProfile_ReadAttrCB,  // Read callback function pointer
  simpleProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t SimpleProfile_AddService( uint32 services )
{
  uint8 status;
  
  // Allocate Client Characteristic Configuration table
    ResponseReadyCharConfig = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                              linkDBNumConns );
    if ( ResponseReadyCharConfig == NULL )
    {
      return ( bleMemAllocError );
    }

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, ResponseReadyCharConfig );

  if ( services & SIMPLEPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( simpleProfileAttrTbl, 
                                          GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &simpleProfileCBs );
  }
  else
  {
    status = SUCCESS;
  }

  return ( status );
}

/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t SimpleProfile_RegisterAppCBs( simpleProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    simpleProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case USER_CHALLANGE_CHAR_VALUE:
      if ( len == USER_CHALLANGE_CHAR_LENGTH ) {
         VOID memcpy( UserChllangeProfileBuffer, value, USER_CHALLANGE_CHAR_LENGTH );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SERVER_RESPONSE_CHAR_VALUE:
      if ( len == SERVER_RESPONSE_CHAR_LENGTH ) {
         VOID memcpy( ServerResonseProfileBuffer, value, SERVER_RESPONSE_CHAR_LENGTH );
      } else {
        ret = bleInvalidRange;
      }
      break;

    case RESPONSE_READY_CHAR_VALUE:
        // This is a single character value indicating the status of the result
        if (len == RESPONSE_READY_CHAR_LENGTH){
            VOID memcpy( ResponseReadyProfileBuffer, value, RESPONSE_READY_CHAR_LENGTH );

        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( ResponseReadyCharConfig, ResponseReadyProfileBuffer, FALSE,
                                    simpleProfileAttrTbl, GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                    INVALID_TASK_ID, simpleProfile_ReadAttrCB );

        } else {
            ret = bleInvalidRange;
        }

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case USER_CHALLANGE_CHAR_VALUE:
      VOID memcpy( value, UserChllangeProfileBuffer, USER_CHALLANGE_CHAR_LENGTH );
      break;

    case SERVER_RESPONSE_CHAR_VALUE:
      VOID memcpy( value, ServerResonseProfileBuffer, SERVER_RESPONSE_CHAR_LENGTH );
      break;      
    case RESPONSE_READY_CHAR_VALUE:
      VOID memcpy( value, ResponseReadyProfileBuffer , RESPONSE_READY_CHAR_LENGTH );
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          simpleProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method)
{
  bStatus_t status = SUCCESS;
  uint16_t bytes_left_to_read;
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those reads
      case USER_CHALLANGE_UUID:
          bytes_left_to_read = USER_CHALLANGE_CHAR_LENGTH - offset;
          break;

      case SERVER_RESPONSE_UUID:
          bytes_left_to_read = SERVER_RESPONSE_CHAR_LENGTH - offset;
          break;

      case RESPONSE_READY_UUID:
          bytes_left_to_read = RESPONSE_READY_CHAR_LENGTH - offset;
          break;

      default:
        bytes_left_to_read = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }

    // Copy the request value to the correct buffer
    bytes_left_to_read = min(bytes_left_to_read, maxLen);
    *pLen = bytes_left_to_read;
    VOID memcpy( pValue, &(pAttr->pValue[offset]), bytes_left_to_read );
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method)
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case USER_CHALLANGE_UUID:
        if ( offset == 0 )
        {
          if ( len >  USER_CHALLANGE_CHAR_LENGTH )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }
        
        //Write the value
        if ( status == SUCCESS )
        {
          uint8 *pCurValue = (uint8 *)pAttr->pValue;        
          VOID memcpy( pCurValue, pValue, len );

          notifyApp = USER_CHALLANGE_CHAR_VALUE;
        }
             
        break;

      case RESPONSE_READY_UUID:
          // The user wants to tell us he has read the response
          if ( (offset != 0) || (len != RESPONSE_READY_CHAR_LENGTH) ) {
              status = ATT_ERR_INVALID_VALUE_SIZE;
          } else {
              notifyApp = RESPONSE_READY_CHAR_VALUE;
              uint8 *pCurValue = (uint8 *)pAttr->pValue;
              VOID memcpy( pCurValue, pValue, len);
          }
          break;

      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        break;
        
      default:
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  // If a characteristic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnSimpleProfileChange )
  {
    simpleProfile_AppCBs->pfnSimpleProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
*********************************************************************/
