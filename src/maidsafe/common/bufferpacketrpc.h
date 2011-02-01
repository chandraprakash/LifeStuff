/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  RPCs used by maidsafe client
* Version:      1.0
* Created:      2009-02-22-00.18.57
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

#ifndef MAIDSAFE_COMMON_BUFFERPACKETRPC_H_
#define MAIDSAFE_COMMON_BUFFERPACKETRPC_H_

#include <boost/cstdint.hpp>

#include <string>

#include "maidsafe/common/maidsafe_service.pb.h"

namespace kad { class Contact; }
namespace transport { class TransportHandler; }
namespace rpcprotocol {
class Controller;
class ChannelManager;
}  // namespace rpcprotocol

namespace maidsafe {

class BufferPacketRpcs {
 public:
  virtual ~BufferPacketRpcs() {}
  virtual void CreateBP(const kad::Contact &peer,
                        const bool &local,
                        const boost::int16_t &transport_id,
                        const CreateBPRequest *create_request,
                        CreateBPResponse *create_response,
                        rpcprotocol::Controller *controller,
                        google::protobuf::Closure *done) = 0;
  virtual void ModifyBPInfo(const kad::Contact &peer,
                            const bool &local,
                            const boost::int16_t &transport_id,
                            const ModifyBPInfoRequest *modify_request,
                            ModifyBPInfoResponse *modify_response,
                            rpcprotocol::Controller *controller,
                            google::protobuf::Closure *done) = 0;
  virtual void GetBPMessages(const kad::Contact &peer,
                             const bool &local,
                             const boost::int16_t &transport_id,
                             const GetBPMessagesRequest *get_messages_request,
                             GetBPMessagesResponse *get_messages_response,
                             rpcprotocol::Controller *controller,
                             google::protobuf::Closure *done) = 0;
  virtual void AddBPMessage(const kad::Contact &peer,
                            const bool &local,
                            const boost::int16_t &transport_id,
                            const AddBPMessageRequest *add_message_request,
                            AddBPMessageResponse *add_message_response,
                            rpcprotocol::Controller *controller,
                            google::protobuf::Closure *done) = 0;
  virtual void GetBPPresence(const kad::Contact &peer,
                             const bool &local,
                             const boost::int16_t &transport_id,
                             const GetBPPresenceRequest *get_presence_request,
                             GetBPPresenceResponse *get_presence_response,
                             rpcprotocol::Controller *controller,
                             google::protobuf::Closure *done) = 0;
  virtual void AddBPPresence(const kad::Contact &peer,
                             const bool &local,
                             const boost::int16_t &transport_id,
                             const AddBPPresenceRequest *add_presence_request,
                             AddBPPresenceResponse *add_presence_response,
                             rpcprotocol::Controller *controller,
                             google::protobuf::Closure *done) = 0;
};

class BufferPacketRpcsImpl : public BufferPacketRpcs {
// public:
//  BufferPacketRpcsImpl(transport::TransportHandler *transport_handler,
//                       rpcprotocol::ChannelManager *channel_manager)
//      : transport_handler_(transport_handler),
//        channel_manager_(channel_manager) {}
//  ~BufferPacketRpcsImpl() {}
//  void CreateBP(const kad::Contact &peer,
//                const bool &local,
//                const boost::int16_t &transport_id,
//                const CreateBPRequest *create_request,
//                CreateBPResponse *create_response,
//                rpcprotocol::Controller *controller,
//                google::protobuf::Closure *done);
//  void ModifyBPInfo(const kad::Contact &peer,
//                    const bool &local,
//                    const boost::int16_t &transport_id,
//                    const ModifyBPInfoRequest *modify_request,
//                    ModifyBPInfoResponse *modify_response,
//                    rpcprotocol::Controller *controller,
//                    google::protobuf::Closure *done);
//  void GetBPMessages(const kad::Contact &peer,
//                     const bool &local,
//                     const boost::int16_t &transport_id,
//                     const GetBPMessagesRequest *get_messages_request,
//                     GetBPMessagesResponse *get_messages_response,
//                     rpcprotocol::Controller *controller,
//                     google::protobuf::Closure *done);
//  void AddBPMessage(const kad::Contact &peer,
//                    const bool &local,
//                    const boost::int16_t &transport_id,
//                    const AddBPMessageRequest *add_message_request,
//                    AddBPMessageResponse *add_message_response,
//                    rpcprotocol::Controller *controller,
//                    google::protobuf::Closure *done);
//  void GetBPPresence(const kad::Contact &peer,
//                     const bool &local,
//                     const boost::int16_t &transport_id,
//                     const GetBPPresenceRequest *get_presence_request,
//                     GetBPPresenceResponse *get_presence_response,
//                     rpcprotocol::Controller *controller,
//                     google::protobuf::Closure *done);
//  void AddBPPresence(const kad::Contact &peer,
//                     const bool &local,
//                     const boost::int16_t &transport_id,
//                     const AddBPPresenceRequest *add_presence_request,
//                     AddBPPresenceResponse *add_presence_response,
//                     rpcprotocol::Controller *controller,
//                     google::protobuf::Closure *done);
// private:
//  BufferPacketRpcsImpl(const BufferPacketRpcsImpl&);
//  BufferPacketRpcsImpl& operator=(const BufferPacketRpcsImpl&);
//  transport::TransportHandler *transport_handler_;
//  rpcprotocol::ChannelManager *channel_manager_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_BUFFERPACKETRPC_H_
