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

#ifndef MAIDSAFE_LIFESTUFF_CLIENT_CLIENTRPC_H_
#define MAIDSAFE_LIFESTUFF_CLIENT_CLIENTRPC_H_

#include <string>
#include "maidsafe/common/maidsafe.h"
#include "maidsafe/common/maidsafe_service.pb.h"
#include "maidsafe/maidsafe-dht.h"

namespace maidsafe {

namespace lifestuff {

class ClientRpcs {
 public:
  ClientRpcs(transport::TransportHandler *transport_handler,
             rpcprotocol::ChannelManager *channel_manager)
      : transport_handler_(transport_handler),
        channel_manager_(channel_manager) {}
  virtual ~ClientRpcs() {}
  virtual void StorePrep(const kad::Contact &peer,
                         bool local,
                         const boost::int16_t &transport_id,
                         StorePrepRequest *store_prep_request,
                         StorePrepResponse *store_prep_response,
                         rpcprotocol::Controller *controller,
                         google::protobuf::Closure *done);
  virtual void StoreChunk(const kad::Contact &peer,
                          bool local,
                          const boost::int16_t &transport_id,
                          StoreChunkRequest *store_chunk_request,
                          StoreChunkResponse *store_chunk_response,
                          rpcprotocol::Controller *controller,
                          google::protobuf::Closure *done);
  virtual void AddToWatchList(
      const kad::Contact &peer,
      bool local,
      const boost::int16_t &transport_id,
      AddToWatchListRequest *add_to_watch_list_request,
      AddToWatchListResponse *add_to_watch_list_response,
      rpcprotocol::Controller *controller,
      google::protobuf::Closure *done);
  virtual void RemoveFromWatchList(
      const kad::Contact &peer,
      bool local,
      const boost::int16_t &transport_id,
      RemoveFromWatchListRequest *remove_from_watch_list_request,
      RemoveFromWatchListResponse *remove_from_watch_list_response,
      rpcprotocol::Controller *controller,
      google::protobuf::Closure *done);
  virtual void GetChunkReferences(
      const kad::Contact &peer,
      bool local,
      const boost::int16_t &transport_id,
      GetChunkReferencesRequest *get_chunk_references_request,
      GetChunkReferencesResponse *get_chunk_references_response,
      rpcprotocol::Controller *controller,
      google::protobuf::Closure *done);
  virtual void AmendAccount(const kad::Contact &peer,
                            bool local,
                            const boost::int16_t &transport_id,
                            AmendAccountRequest *amend_account_request,
                            AmendAccountResponse *amend_account_response,
                            rpcprotocol::Controller *controller,
                            google::protobuf::Closure *done);
  virtual void ExpectAmendment(
      const kad::Contact &peer,
      bool local,
      const boost::int16_t &transport_id,
      ExpectAmendmentRequest *expect_amendment_request,
      ExpectAmendmentResponse *expect_amendment_response,
      rpcprotocol::Controller *controller,
      google::protobuf::Closure *done);
  virtual void AccountStatus(const kad::Contact &peer,
                             bool local,
                             const boost::int16_t &transport_id,
                             AccountStatusRequest *account_status_request,
                             AccountStatusResponse *account_status_response,
                             rpcprotocol::Controller *controller,
                             google::protobuf::Closure *done);
  void CheckChunk(const kad::Contact &peer,
                  bool local,
                  const boost::int16_t &transport_id,
                  CheckChunkRequest *check_chunk_request,
                  CheckChunkResponse *check_chunk_response,
                  rpcprotocol::Controller *controller,
                  google::protobuf::Closure *done);
  void GetChunk(const kad::Contact &peer,
                bool local,
                const boost::int16_t &transport_id,
                GetChunkRequest *get_chunk_request,
                GetChunkResponse *get_chunk_response,
                rpcprotocol::Controller *controller,
                google::protobuf::Closure *done);
  void LocalVaultOwned(LocalVaultOwnedResponse *response,
                       rpcprotocol::Controller *controller,
                       rpcprotocol::Channel *channel,
                       google::protobuf::Closure *done);
  void SetLocalVaultOwned(SetLocalVaultOwnedRequest *request,
                          SetLocalVaultOwnedResponse *response,
                          rpcprotocol::Controller *controller,
                          rpcprotocol::Channel *channel,
                          google::protobuf::Closure *done);

 private:
  ClientRpcs(const ClientRpcs&);
  ClientRpcs& operator=(const ClientRpcs&);
  transport::TransportHandler *transport_handler_;
  rpcprotocol::ChannelManager *channel_manager_;
};

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_CLIENT_CLIENTRPC_H_
