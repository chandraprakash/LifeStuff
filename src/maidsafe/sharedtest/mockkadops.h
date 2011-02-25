/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  A mock KadOps object, and related helper methods
* Created:      2010-02-11
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

#ifndef MAIDSAFE_SHAREDTEST_MOCKKADOPS_H_
#define MAIDSAFE_SHAREDTEST_MOCKKADOPS_H_

#include gmock/gmock.h

#include <vector>
#include <string>

#include "maidsafe/common/kadops.h"
#include "maidsafe/sharedtest/threadpool.h"

namespace mock_kadops {

enum FindNodesResponseType {
  kFailParse,  // response won't parse
  kResultFail,  // negative response
  kCloseContacts,  // returned contacts are close to target
  kFarContacts  // returned contacts are far from target
};

std::string MakeFindNodesResponse(const FindNodesResponseType &type,
                                  const std::string &target,
                                  const boost::uint8_t n,
                                  std::vector<kad::Contact> *nodes);

}  // namespace mock_kadops

namespace maidsafe {

class MockKadOps : public KadOps {
// public:
//  MockKadOps(transport::TransportHandler *transport_handler,
//             rpcprotocol::ChannelManager *channel_manager,
//             kad::NodeType type,
//             const std::string &private_key,
//             const std::string &public_key,
//             bool port_forwarded,
//             bool use_upnp,
//             boost::uint8_t k,
//             std::shared_ptr<ChunkStore> chunkstore)
//      : KadOps(transport_handler, channel_manager, type, private_key,
//               public_key, port_forwarded, use_upnp, k, chunkstore),
//        tp_(1) {}
//  MOCK_METHOD1(AddressIsLocal, bool(const kad::Contact &peer));
//  MOCK_METHOD1(AddressIsLocal, bool(const kad::ContactInfo &peer));
//  MOCK_METHOD3(FindValue, void(const std::string &key,
//                               bool check_local,
//                               kad::VoidFunctorOneString callback));
//  MOCK_METHOD3(GetNodeContactDetails, void(const std::string &node_id,
//                                           VoidFuncIntContact callback,
//                                           bool local));
//  MOCK_METHOD2(FindKClosestNodes, void(const std::string &key,
//                                       VoidFuncIntContacts callback));
//  MOCK_METHOD4(GetStorePeer, int(const double &ideal_rtt,
//                                 const std::vector<kad::Contact> &exclude,
//                                 kad::Contact *new_peer,
//                                 bool *local));
//  void RealGetNodeContactDetails(const std::string &node_id,
//                                 VoidFuncIntContact callback,
//                                 bool local) {
//    KadOps::GetNodeContactDetails(node_id, callback, local);
//  }
//  void RealFindKClosestNodes(const std::string &key,
//                             VoidFuncIntContacts callback) {
//    KadOps::FindKClosestNodes(key, callback);
//  }
//  void ThreadedGetNodeContactDetailsCallback(const std::string &response,
//                                             VoidFuncIntContact callback) {
//    tp_.EnqueueTask(boost::bind(&KadOps::GetNodeContactDetailsCallback, this,
//                                response, callback));
//  }
//void ThreadedFindKClosestNodesCallback(const std::string &response,
//                                         VoidFuncIntContacts callback) {
//    tp_.EnqueueTask(boost::bind(&KadOps::FindKClosestNodesCallback, this,
//                                response, callback));
//  }
//  bool Wait() {
//    return tp_.WaitForTasksToFinish(boost::posix_time::milliseconds(3000));
//  }
// private:
//  base::Threadpool tp_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_SHAREDTEST_MOCKKADOPS_H_
