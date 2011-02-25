/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Manages buffer packet messages to the maidsafe client
* Version:      1.0
* Created:      2009-01-28-23.10.42
* Revision:     none
* Compiler:     gcc
* Author:       Fraser Hutchison (fh), fraser.hutchison@maidsafe.net
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

#ifndef MAIDSAFE_COMMON_CLIENTBUFFERPACKETHANDLER_H_
#define MAIDSAFE_COMMON_CLIENTBUFFERPACKETHANDLER_H_

#include "boost/cstdint.hpp"
#include "boost/function.hpp"
#include "maidsafe-dht/kademlia/contact.h"
#include <list>
#include <string>
#include <vector>
#include "maidsafe/common/maidsafe.h"
#include "maidsafe/common/returncodes.h"
#include "maidsafe/common/maidsafe_service.pb.h"
#include "maidsafe/common/packet.pb.h"

namespace rpcprotocol { class Controller; }

namespace maidsafe {

typedef boost::function<void(const ReturnCode&)> bp_operations_cb;
typedef boost::function<void(const ReturnCode&,
                             const std::list<ValidatedBufferPacketMessage>&,
                             bool)> bp_getmessages_cb;
typedef boost::function<void(const ReturnCode&,
                             const std::list<std::string>&,
                             bool)> bp_getpresence_cb;

enum BpOpType {
  CREATEBP,
  MODIFY_INFO,
  ADD_MESSAGE,
  GET_MESSAGES,
  ADD_PRESENCE,
  GET_PRESENCE
};

//class BufferPacketRpcs;
//class KadOps;

struct ChangeBPData {
//  ChangeBPData() : create_request(), modify_request(), get_msgs_request(),
//                   add_msg_request(), get_presence_request(),
//                   add_presence_request(), successful_ops(0),
//                   is_calledback(false), type(), cb(0), cb_getmsgs(0),
//                   cb_getpresence(0), private_key() {}
//  CreateBPRequest create_request;
//  ModifyBPInfoRequest modify_request;
//  GetBPMessagesRequest get_msgs_request;
//  AddBPMessageRequest add_msg_request;
//  GetBPPresenceRequest get_presence_request;
//  AddBPPresenceRequest add_presence_request;
//  boost::uint16_t successful_ops;
//  bool is_calledback;
//  BpOpType type;
//  bp_operations_cb cb;
//  bp_getmessages_cb cb_getmsgs;
//  bp_getpresence_cb cb_getpresence;
//  std::string private_key;
};

struct ModifyBPCallbackData {
//  ModifyBPCallbackData() : ctrl(NULL),
//                           create_response(NULL),
//                           modify_response(NULL),
//                           get_msgs_response(NULL),
//                           add_msg_response(NULL),
//                           get_presence_response(NULL),
//                           add_presence_response(NULL),
//                           ctc(),
//                           data(),
//                           transport_id(0),
//                           is_calledback(false) {}
//  rpcprotocol::Controller *ctrl;
//  CreateBPResponse *create_response;
//  ModifyBPInfoResponse *modify_response;
//  GetBPMessagesResponse *get_msgs_response;
//  AddBPMessageResponse *add_msg_response;
//  GetBPPresenceResponse *get_presence_response;
//  AddBPPresenceResponse *add_presence_response;
//  kademlia::Contact ctc;
//  std::shared_ptr<ChangeBPData> data;
//  boost::int16_t transport_id;
//  bool is_calledback;
};

struct BPInputParameters {
//  BPInputParameters(const std::string &public_username,
//                    const std::string &mpid_public_key,
//                    const std::string &mpid_private_key)
//    : kPublicUsername(public_username),
//      kMpidPublicKey(mpid_public_key),
//      kMpidPrivateKey(mpid_private_key) {}
//  const std::string kPublicUsername, kMpidPublicKey, kMpidPrivateKey;
};

class ClientBufferPacketHandler {
//  static const boost::uint16_t kParallelStores = 1;
//  static const boost::uint16_t kParallelFindCtcs = 1;
// public:
//  ClientBufferPacketHandler(std::shared_ptr<BufferPacketRpcs> rpcs,
//                            std::shared_ptr<KadOps> kadops,
//                            boost::uint8_t upper_threshold)
//    : rpcs_(rpcs), kad_ops_(kadops), kUpperThreshold_(upper_threshold) {}
//  virtual ~ClientBufferPacketHandler() {}
//  void CreateBufferPacket(const BPInputParameters &args,
//                          bp_operations_cb cb,
//                          const boost::int16_t &transport_id);
//  void ModifyOwnerInfo(const BPInputParameters &args,
//                       const std::vector<std::string> &users,
//                       bp_operations_cb cb,
//                       const boost::int16_t &transport_id);
//  void GetMessages(const BPInputParameters &args,
//                   bp_getmessages_cb cb,
//                   const boost::int16_t &transport_id);
//  void AddMessage(const BPInputParameters &args,
//                  const std::string &my_pu,
//                  const std::string &recver_public_key,
//                  const std::string &receiver_id,
//                  const std::string &message,
//                  const MessageType &m_type,
//                  bp_operations_cb cb,
//                  const boost::int16_t &transport_id);
//  void GetPresence(const BPInputParameters &args,
//                   bp_getpresence_cb cb,
//                   const boost::int16_t &transport_id);
//  void AddPresence(const BPInputParameters &args,
//                   const std::string &my_pu,
//                   const std::string &recver_public_key,
//                   const std::string &receiver_id,
//                   bp_operations_cb cb,
//                   const boost::int16_t &transport_id);
// private:
//  virtual void FindNodes(VoidFuncIntContacts cb,
//                         std::shared_ptr<ChangeBPData> data);
//  virtual void FindNodesCallback(
//      const ReturnCode &result,
//      const std::vector<kad::Contact> &closest_nodes,
//      std::shared_ptr<ChangeBPData> data,
//      const boost::int16_t &transport_id);
//  void ActionOnBpDone(
//      std::shared_ptr<std::vector<ModifyBPCallbackData> > cb_datas,
//      boost::int16_t index);
//  std::list<ValidatedBufferPacketMessage> ValidateMsgs(
//      const GetBPMessagesResponse *response,
//      const std::string &private_key);
//  std::list<LivePresence> ValidatePresence(
//      const GetBPPresenceResponse *response,
//      const std::string &private_key);
//  ClientBufferPacketHandler &operator=(const ClientBufferPacketHandler);
//  ClientBufferPacketHandler(const ClientBufferPacketHandler&);
//  std::shared_ptr<BufferPacketRpcs> rpcs_;
//  std::shared_ptr<KadOps> kad_ops_;
//  const boost::uint16_t kUpperThreshold_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_CLIENTBUFFERPACKETHANDLER_H_
