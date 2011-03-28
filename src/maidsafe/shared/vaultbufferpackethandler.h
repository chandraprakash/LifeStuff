/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Manages buffer packet messages to the maidsafe vault
* Version:      1.0
* Created:      2009-01-29-00.59.23
* Revision:     none
* Compiler:     gcc
* Author:       Team maidsafe
* Company:      maidsafe.net limited
*
* The following source code is property of maidsafe.net limited and is not
* meant for external use.  The use of this code is governed by the license
* file LICENSE.TXT found in the root of this directory and also on
* www.maidsafe.net.
*
* You are not free to copy, amend or otherwise use this source code without
* the explicit written permission of the board of directors of maidsafe.net.
*
* ============================================================================
*/

#ifndef MAIDSAFE_COMMON_VAULTBUFFERPACKETHANDLER_H_
#define MAIDSAFE_COMMON_VAULTBUFFERPACKETHANDLER_H_

#include <string>
#include <vector>
#include "maidsafe/common/packet.pb.h"

namespace maidsafe {

class VaultBufferPacketHandler {
 public:
  VaultBufferPacketHandler() {}
  ~VaultBufferPacketHandler() {}
  bool ValidateOwnerSignature(const std::string &public_key,
                              const std::string &ser_bufferpacket);
  bool GetMessages(std::string *ser_bp, std::vector<std::string> *msgs);
  bool ClearMessages(std::string *ser_bufferpacket);
  bool IsOwner(const std::string &owner_id, const GenericPacket &gp_info);
  bool ChangeOwnerInfo(const std::string &ser_gp,
                       const std::string &public_key,
                       std::string *ser_packet);
  bool AddMessage(const std::string &current_bp,
                  const std::string &ser_message,
                  const std::string &signed_public_key,
                  std::string *updated_bp);
  bool CheckMsgStructure(const std::string &ser_message,
                         std::string *sender_id,
                         MessageType *type);
  bool GetPresence(std::string *ser_bp, std::vector<std::string> *msgs);
  bool AddPresence(const std::string &ser_message, std::string *ser_bp);

 private:
  VaultBufferPacketHandler &operator=(const VaultBufferPacketHandler);
  VaultBufferPacketHandler(const VaultBufferPacketHandler&);
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_VAULTBUFFERPACKETHANDLER_H_
