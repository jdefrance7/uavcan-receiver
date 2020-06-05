//##############################################################################
// File Information
//##############################################################################

bool node_status_received = false;
bool node_status_processed = false;
bool angular_command_received = false;
bool angular_command_processed = false;

//##############################################################################
// Arduino Support Library
//##############################################################################

#include <Arduino.h>

//##############################################################################
// UAVCAN Driver and Data Types
//##############################################################################

#include <ast-uavcan.h>

//##############################################################################
// UAVCAN Driver Instance
//##############################################################################

UAVCAN_Driver uavcan;

//##############################################################################
// Canard Callback - On Transfer Received
//##############################################################################

static void onTransferReceived(
  CanardInstance* ins,
  CanardRxTransfer* transfer)
{
  if(transfer->transfer_type == CanardTransferTypeResponse)
  {
    // Add response handlers here...
  }
  else if(transfer->transfer_type == CanardTransferTypeRequest)
  {
    // Add request handlers here...
  }
  else if(transfer->transfer_type == CanardTransferTypeBroadcast)
  {
    // Add braodcast handlers here...
    if(transfer->data_type_id == NODE_STATUS_DATA_TYPE_ID)
    {
      node_status_processed = true;

      NodeStatus node_status;

      uint32_t bit_offset = 0;

      canardDecodeScalar(transfer, bit_offset, 32, false, &node_status.uptime_sec);
      bit_offset += 32;
      canardDecodeScalar(transfer, bit_offset, 2, false, &node_status.health);
      bit_offset += 2;
      canardDecodeScalar(transfer, bit_offset, 3, false, &node_status.mode);
      bit_offset += 3;
      canardDecodeScalar(transfer, bit_offset, 3, false, &node_status.sub_mode);
      bit_offset += 3;
      canardDecodeScalar(transfer, bit_offset, 16, false, &node_status.vendor_specific_status_code);
      bit_offset += 16;

      Serial.println("\nNodeStatus");
      Serial.print("  uptime_sec: ");     Serial.println(node_status.uptime_sec);
      Serial.print("  health: ");         Serial.println(node_status.health);
      Serial.print("  mode: ");           Serial.println(node_status.mode);
      Serial.print("  sub_mode: ");       Serial.println(node_status.sub_mode);
      Serial.print("  vendor: ");         Serial.println(node_status.vendor_specific_status_code);

      canardReleaseRxTransferPayload(ins, transfer);
      return;
    }

    if(transfer->data_type_id == ANGULAR_COMMAND_DATA_TYPE_ID)
    {
      angular_command_processed = true;

      uint16_t quaternion_xyzw[4];

      AngularCommand angular_command;

      uint32_t bit_offset = 0;

      canardDecodeScalar(transfer, bit_offset, 8, false, &angular_command.gimbal_id);
      bit_offset += 8;
      canardDecodeScalar(transfer, bit_offset, 8, false, &angular_command.mode);
      bit_offset += 8;
      canardDecodeScalar(transfer, bit_offset, 16, true, &quaternion_xyzw[0]);
      angular_command.quaternion_xyzw[0] = canardConvertFloat16ToNativeFloat(quaternion_xyzw[0]);
      bit_offset += 8;
      canardDecodeScalar(transfer, bit_offset, 16, true, &quaternion_xyzw[1]);
      angular_command.quaternion_xyzw[0] = canardConvertFloat16ToNativeFloat(quaternion_xyzw[1]);
      bit_offset += 8;
      canardDecodeScalar(transfer, bit_offset, 16, true, &quaternion_xyzw[2]);
      angular_command.quaternion_xyzw[0] = canardConvertFloat16ToNativeFloat(quaternion_xyzw[2]);
      bit_offset += 8;
      canardDecodeScalar(transfer, bit_offset, 16, true, &quaternion_xyzw[3]);
      angular_command.quaternion_xyzw[0] = canardConvertFloat16ToNativeFloat(quaternion_xyzw[3]);
      bit_offset += 8;

      Serial.println("\nAngularCommand");
      Serial.print("  gimbal_id: ");      Serial.println(angular_command.gimbal_id);
      Serial.print("  mode: ");           Serial.println(angular_command.mode);
      Serial.print("  x: ");              Serial.println(angular_command.quaternion_xyzw[0], 3);
      Serial.print("  y: ");              Serial.println(angular_command.quaternion_xyzw[1], 3);
      Serial.print("  z: ");              Serial.println(angular_command.quaternion_xyzw[2], 3);
      Serial.print("  w: ");              Serial.println(angular_command.quaternion_xyzw[3], 3);

      canardReleaseRxTransferPayload(ins, transfer);
      return;
    }
  }
}

//##############################################################################
// Canard Callback - Should Accept Transfer
//##############################################################################

static bool shouldAcceptTransfer(
  const CanardInstance* ins,
  uint64_t* out_data_type_signature,
  uint16_t data_type_id,
  CanardTransferType transfer_type,
  uint8_t source_node_id)
{
  (void)source_node_id;

  if(transfer_type == CanardTransferTypeResponse)
  {
    // Add response handlers here...
  }
  else if(transfer_type == CanardTransferTypeRequest)
  {
    // Add request handlers here...
  }
  else if(transfer_type == CanardTransferTypeBroadcast)
  {
    if(data_type_id == NODE_STATUS_DATA_TYPE_ID)
    {
      node_status_received = true;

      *out_data_type_signature = NODE_STATUS_DATA_TYPE_SIGNATURE;
      return true;
    }

    if(data_type_id == ANGULAR_COMMAND_DATA_TYPE_ID)
    {
      angular_command_received = true;

      *out_data_type_signature = ANGULAR_COMMAND_DATA_TYPE_SIGNATURE;
      return true;
    }
  }

  return false;
}

//##############################################################################
// Setup Function
//##############################################################################

void setup()
{
  uavcan.setId(7);
  uavcan.setBitrate(1000000);
  uavcan.setCallbacks(onTransferReceived, shouldAcceptTransfer);
  uavcan.begin();

  Serial.begin(115200);
  while(!Serial);

  Serial.println("Initialization Complete!");
}

//##############################################################################
// Main Loop
//##############################################################################

void loop()
{
  static int poll;

  poll = uavcan.poll();

  Serial.print("Poll: "); Serial.println(poll);
  Serial.print("Node Status Received: "); Serial.println(node_status_received);
  Serial.print("Node Status Processed: "); Serial.println(node_status_processed);
  Serial.print("Angular Command Received: "); Serial.println(angular_command_received);
  Serial.print("Angualr Command Processed: "); Serial.println(angular_command_processed);

  delay(10);
}

//##############################################################################
// End of File
//##############################################################################
