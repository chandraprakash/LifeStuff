/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Struct declarations for use in PDvault and MSM
* Created:      2010-02-08
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

#ifndef MAIDSAFE_SHARED_OPDATA_H_
#define MAIDSAFE_SHARED_OPDATA_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "boost/cstdint.hpp"
#include "maidsafe/passport/passport.h"

#include "maidsafe/shared/chunkstore.h"
#include "maidsafe/shared/maidsafe.h"
#include "maidsafe/shared/maidsafe_service.pb.h"
#include "maidsafe/client/storemanagertaskshandler.h"

namespace maidsafe {

struct StoreData {
  // Default constructor
  StoreData() : data_name(),
                value(),
                size(0),
                msid(),
                key_id(),
                public_key(),
                public_key_signature(),
                private_key(),
                chunk_type(kHashable | kNormal),
                system_packet_type(passport::MID),
                dir_type(PRIVATE),
                callback(),
                exclude_peers(),
                master_task_id(kRootTask),
                watchlist_master_task_id(kRootTask),
                amendment_task_id(kRootTask),
                chunk_copy_master_task_id(kRootTask) {}
  // Store chunk & Delete chunk constructor
  StoreData(const std::string &chunk_name,
            const boost::uint64_t &chunk_size,
            ChunkType ch_type,
            DirType directory_type,
            const std::string &ms_id,
            const std::string &key,
            const std::string &pub_key,
            const std::string &pub_key_signature,
            const std::string &priv_key)
                : data_name(chunk_name),
                  value(),
                  size(chunk_size),
                  msid(ms_id),
                  key_id(key),
                  public_key(pub_key),
                  public_key_signature(pub_key_signature),
                  private_key(priv_key),
                  chunk_type(ch_type),
                  system_packet_type(passport::MID),
                  dir_type(directory_type),
                  callback(),
                  exclude_peers(),
                  master_task_id(kRootTask),
                  watchlist_master_task_id(kRootTask),
                  amendment_task_id(kRootTask),
                  chunk_copy_master_task_id(kRootTask) {}
  // Store packet constructor
  StoreData(const std::string &packet_name,
            const std::string &packet_value,
            passport::PacketType sys_packet_type,
            DirType directory_type,
            const std::string &ms_id,
            const std::string &key,
            const std::string &pub_key,
            const std::string &pub_key_signature,
            const std::string &priv_key,
            VoidFuncOneInt cb)
                : data_name(packet_name),
                  value(packet_value),
                  size(0),
                  msid(ms_id),
                  key_id(key),
                  public_key(pub_key),
                  public_key_signature(pub_key_signature),
                  private_key(priv_key),
                  chunk_type(kHashable | kNormal),
                  system_packet_type(sys_packet_type),
                  dir_type(directory_type),
                  callback(cb),
                  exclude_peers(),
                  master_task_id(kRootTask),
                  watchlist_master_task_id(kRootTask),
                  amendment_task_id(kRootTask),
                  chunk_copy_master_task_id(kRootTask) {}
  std::string data_name, value;
  boost::uint64_t size;
  std::string msid, key_id, public_key, public_key_signature, private_key;
  ChunkType chunk_type;
  passport::PacketType system_packet_type;
  DirType dir_type;
  VoidFuncOneInt callback;
  std::vector<kad::Contact> exclude_peers;
  TaskId master_task_id, watchlist_master_task_id, amendment_task_id;
  TaskId chunk_copy_master_task_id;
};

struct DeletePacketData {
 public:
  DeletePacketData(const std::string &name,
                   const std::vector<std::string> &packet_values,
                   passport::PacketType sys_packet_type,
                   DirType directory_type,
                   const std::string &ms_id,
                   const std::string &key,
                   const std::string &pub_key,
                   const std::string &pub_key_signature,
                   const std::string &priv_key,
                   VoidFuncOneInt cb)
                       : packet_name(name),
                         values(packet_values),
                         msid(ms_id),
                         key_id(key),
                         public_key(pub_key),
                         public_key_signature(pub_key_signature),
                         private_key(priv_key),
                         system_packet_type(sys_packet_type),
                         dir_type(directory_type),
                         callback(cb),
                         mutex(),
                         returned_count(0),
                         called_back(false) {}
  std::string packet_name;
  std::vector<std::string> values;
  std::string msid, key_id, public_key, public_key_signature, private_key;
  passport::PacketType system_packet_type;
  DirType dir_type;
  VoidFuncOneInt callback;
  boost::mutex mutex;
  size_t returned_count;
  bool called_back;
 private:
};

struct UpdatePacketData {
 public:
  UpdatePacketData(const std::string &name,
                   const std::string &oldvalue,
                   const std::string &newvalue,
                   passport::PacketType sys_packet_type,
                   DirType directory_type,
                   const std::string &ms_id,
                   const std::string &key,
                   const std::string &pub_key,
                   const std::string &pub_key_signature,
                   const std::string &priv_key,
                   VoidFuncOneInt cb)
                       : packet_name(name),
                         old_value(oldvalue),
                         new_value(newvalue),
                         msid(ms_id),
                         key_id(key),
                         public_key(pub_key),
                         public_key_signature(pub_key_signature),
                         private_key(priv_key),
                         system_packet_type(sys_packet_type),
                         dir_type(directory_type),
                         callback(cb),
                         mutex() {}
  std::string packet_name;
  std::string old_value, new_value, msid, key_id, public_key,
              public_key_signature, private_key;
  passport::PacketType system_packet_type;
  DirType dir_type;
  VoidFuncOneInt callback;
  boost::mutex mutex;
};

// This is used within following OpData structs to hold details of a single RPC
template <typename ResponseType>
struct SingleOpDataHolder {
  explicit SingleOpDataHolder(const std::string &id)
        : node_id(id), response(), controller(new rpcprotocol::Controller) {
    controller->set_timeout(300);
  }
    std::string node_id;
    ResponseType response;
    std::shared_ptr<rpcprotocol::Controller> controller;
};

// This is used to hold the data required to perform a Kad lookup to get a
// group of Chunk Info holders, send the result as part of k
// ExpectAmendmentRequests, send each CI Holder an AddToWatchListRequest or
// RemoveFromWatchListRequest and assess the responses.
struct WatchListOpData {
  typedef SingleOpDataHolder<AddToWatchListResponse> AddToWatchDataHolder;
  typedef SingleOpDataHolder<RemoveFromWatchListResponse>
      RemoveFromWatchDataHolder;
  explicit WatchListOpData(std::shared_ptr<StoreData> sd)
      : store_data(sd),
        mutex(),
        chunk_info_holders(),
        add_to_watchlist_data_holders(),
        remove_from_watchlist_data_holders(),
        returned_count(0),
        success_count(0),
        required_upload_copies(),
        consensus_upload_copies(-1),
        payment_values(),
        task_id(kRootTask) {}
  std::shared_ptr<StoreData> store_data;
  boost::mutex mutex;
  std::vector<kad::Contact> chunk_info_holders;
  std::vector<AddToWatchDataHolder> add_to_watchlist_data_holders;
  std::vector<RemoveFromWatchDataHolder> remove_from_watchlist_data_holders;
  boost::uint16_t returned_count, success_count;
  std::multiset<int> required_upload_copies;
  int consensus_upload_copies;
  std::vector<boost::uint64_t> payment_values;
  TaskId task_id;
};

struct ExpectAmendmentOpData {
  typedef SingleOpDataHolder<ExpectAmendmentResponse> AccountDataHolder;
  explicit ExpectAmendmentOpData()
      : mutex(),
        account_data_holders(),
        returned_count(0),
        success_count(0),
        callback() {}
  boost::mutex mutex;
  std::vector<AccountDataHolder> account_data_holders;
  boost::uint16_t returned_count, success_count;
  VoidFuncOneInt callback;
};

// This is used to hold the data required to perform a SendChunkPrep followed by
// a SendChunkContent operation.
struct SendChunkData {
  explicit SendChunkData(std::shared_ptr<StoreData> sd)
      : store_data(sd),
        peer(),
        local(false),
        store_prep_request(),
        store_prep_response(),
        store_chunk_request(),
        store_chunk_response(),
        controller(new rpcprotocol::Controller),
        chunk_copy_task_id(kRootTask) {
    controller->set_timeout(120);
  }
  std::shared_ptr<StoreData> store_data;
  kad::Contact peer;
  bool local;
  StorePrepRequest store_prep_request;
  StorePrepResponse store_prep_response;
  StoreChunkRequest store_chunk_request;
  StoreChunkResponse store_chunk_response;
  std::shared_ptr<rpcprotocol::Controller> controller;
  TaskId chunk_copy_task_id;
};

// This is used to hold the data required to send AccountStatusRequests and
// assess the responses.
struct AccountStatusData {
  typedef SingleOpDataHolder<AccountStatusResponse> AccountStatusDataHolder;
  explicit AccountStatusData()
      : mutex(),
        contacts(),
        data_holders(),
        returned_count(0),
        success_count(0),
        offered_values(),
        given_values(),
        taken_values(),
        overall_success(false) {}
  boost::mutex mutex;
  std::vector<kad::Contact> contacts;
  std::vector<AccountStatusDataHolder> data_holders;
  boost::uint16_t returned_count, success_count;
  std::vector<boost::uint64_t> offered_values, given_values, taken_values;
  bool overall_success;
};

// This is used to hold the data required to send AmendAccountRequests and
// assess the responses.
struct AmendAccountData {
  typedef SingleOpDataHolder<AmendAccountResponse> AmendAccountDataHolder;
  explicit AmendAccountData()
      : mutex(),
        condition(),
        contacts(),
        data_holders(),
        returned_count(0),
        success_count(0) {}
  boost::mutex mutex;
  boost::condition_variable condition;
  std::vector<kad::Contact> contacts;
  std::vector<AmendAccountDataHolder> data_holders;
  boost::uint16_t returned_count, success_count;
};

struct GenericConditionData {
 public:
  explicit GenericConditionData(std::shared_ptr<boost::condition_variable> cv)
      : cond_flag(false),
        cond_variable(cv),
        cond_mutex() {}
  ~GenericConditionData() {}
  bool cond_flag;
  std::shared_ptr<boost::condition_variable> cond_variable;
  boost::mutex cond_mutex;
 private:
  GenericConditionData &operator=(const GenericConditionData&);
  GenericConditionData(const GenericConditionData&);
};

enum ChunkHolderStatus {
  kHolderNew,
  kHolderContactable,
  kHolderHasChunk,
  kHolderPending,
  kHolderFailed,
  kHolderStatusCount  // highest index
};

class GetChunkOpData {
 public:
  explicit GetChunkOpData(const std::string &chunk_name_)
    : chunk_name(chunk_name_),
      needs_cache_copy_id(),
      failed(false),
      find_value_done(false),
      found_chunk_holder(false),
      idx_info(0),
      num_info_responses(0),
      chunk_info_holders(),
      chunk_holder_contacts(),
      ref_responses(),
      check_responses(),
      controllers(),
      mutex(),
      condition() {}
  void AddChunkHolder(const std::string &pmid) {
    bool found(false);
    for (int i = 0; i < kHolderStatusCount; ++i) {
      if (chunk_holders[i].count(pmid) != 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      chunk_holders[kHolderNew].insert(pmid);
    }
  }
  void AddChunkHolder(const kad::Contact &contact) {
    chunk_holders[kHolderContactable].insert(
        contact.node_id().String());
    chunk_holder_contacts[contact.node_id().String()] = contact;
  }
  std::string chunk_name, needs_cache_copy_id;
  bool failed, find_value_done, found_chunk_holder;
  size_t idx_info, num_info_responses;
  std::vector<kad::Contact> chunk_info_holders;
  std::set<std::string> chunk_holders[kHolderStatusCount];
  std::map<std::string, kad::Contact> chunk_holder_contacts;
  std::vector<GetChunkReferencesResponse> ref_responses;
  std::vector<CheckChunkResponse> check_responses;
  std::list< std::shared_ptr<rpcprotocol::Controller> > controllers;
  boost::mutex mutex;
  boost::condition_variable condition;
 private:
  GetChunkOpData &operator=(const GetChunkOpData&);
  GetChunkOpData(const GetChunkOpData&);
};

}  // namespace maidsafe

#endif  // MAIDSAFE_SHARED_OPDATA_H_
