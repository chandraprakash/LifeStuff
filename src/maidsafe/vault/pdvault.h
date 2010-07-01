/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Class containing vault logic
* Version:      1.0
* Created:      2009-02-21-23.55.54
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

#ifndef MAIDSAFE_VAULT_PDVAULT_H_
#define MAIDSAFE_VAULT_PDVAULT_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <maidsafe/base/crypto.h>
#include <maidsafe/maidsafe-dht.h>
#include <maidsafe/transport/transportudt.h>
#include <maidsafe/base/utils.h>
#include <QThreadPool>

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "maidsafe/maidsafevalidator.h"
#include "maidsafe/opdata.h"
#include "maidsafe/vault/vaultchunkstore.h"
#include "maidsafe/vault/vaultrpc.h"
#include "maidsafe/vault/vaultservice.h"
#include "maidsafe/vault/vaultservicelogic.h"


namespace maidsafe {
namespace test {
class CBPHandlerTest;
class ImMessagingTest;
class CCImMessagingTest;
// This forward declaration is to allow gtest environment to be declared as a
// friend class
namespace localvaults {
class Env;
}  // namespace localvaults
}  // namespace test
}  // namespace maidsafe

namespace maidsafe_vault {

enum VaultStatus {kVaultStarted, kVaultStopping, kVaultStopped};

class RunPDVaults;

namespace test {
class PDVaultTest;
class PDVaultTest_FUNC_MAID_NET_StoreAndGetChunks_Test;
class PDVaultTest_FUNC_MAID_NET_Cachechunk_Test;
}  // namespace test

/* struct SyncVaultData {
  SyncVaultData() : chunk_names(), num_updated_chunks(0), num_chunks(0),
    active_updating(0), cb(), is_callbacked(false) {}
  std::list<std::string> chunk_names;  // chunks to be updated
  int num_updated_chunks;  // number of chunks updated
  int num_chunks;
  int active_updating;
  kad::VoidFunctorOneString cb;
  bool is_callbacked;
}; */

/* struct GetAlivePartner {
  GetAlivePartner(const int &num_parners, const std::string &chunk_name) :
    data(), is_found(false), contacted_partners(0),
    number_partners(num_parners), chunk_name(chunk_name) {}
  boost::shared_ptr<SyncVaultData> data;
  bool is_found;
  int contacted_partners;
  int number_partners;
  std::string chunk_name;
}; */

/* struct RepublishChunkRefData {
  RepublishChunkRefData() : chunk_names(), num_republished_chunks(0),
    num_chunks(0), cb(), is_callbacked(false) {}
  std::list<std::string> chunk_names;  // chunks to be updated
  int num_republished_chunks;  // number of chunks updated
  int num_chunks;
  kad::VoidFunctorOneString cb;
  bool is_callbacked;
}; */

struct LoadChunkData {
  LoadChunkData(const std::string &chunkname, kad::VoidFunctorOneString cb)
    : chunk_holders(), failed_chunk_holders(), number_holders(0),
      failed_holders(0), is_active(false), chunk_name(chunkname), retry(0),
      cb(cb), is_callbacked(false), get_msgs(false), pub_key(""),
      sig_pub_key("") {}
  std::vector<kad::Contact> chunk_holders;
  std::vector<kad::Contact> failed_chunk_holders;
  int number_holders;
  int failed_holders;
  bool is_active;
  std::string chunk_name;
  int retry;
  kad::VoidFunctorOneString cb;
  bool is_callbacked;
  // used only for get msgs
  bool get_msgs;
  std::string pub_key;
  std::string sig_pub_key;
};

/* struct SynchArgs {
  SynchArgs(const std::string &chunk_name,
            const kad::Contact &chunk_holder,
            boost::shared_ptr<SyncVaultData> data)
                : chunk_name_(chunk_name),
                  chunk_holder_(chunk_holder),
                  data_(data) {}
  const std::string chunk_name_;
  const kad::Contact chunk_holder_;
  boost::shared_ptr<SyncVaultData> data_;
}; */

/* struct ValidityCheckArgs {
  ValidityCheckArgs(const std::string &chunk_name,
                    const std::string &random_data,
                    const kad::Contact &chunk_holder,
                    kad::VoidFunctorOneString cb)
                        : chunk_name_(chunk_name),
                          random_data_(random_data),
                          chunk_holder_(chunk_holder),
                          cb_(cb), retry_remote(false) {}
  const std::string chunk_name_;
  const std::string random_data_;
  const kad::Contact chunk_holder_;
  kad::VoidFunctorOneString cb_;
  bool retry_remote;
}; */

struct GetArgs {
  GetArgs(const kad::Contact &chunk_holder,
          boost::shared_ptr<struct LoadChunkData> data)
      : chunk_holder_(chunk_holder),
        data_(data),
        retry_remote_(false),
        controller_(new rpcprotocol::Controller) {}
  const kad::Contact chunk_holder_;
  boost::shared_ptr<struct LoadChunkData> data_;
  bool retry_remote_;
  boost::shared_ptr<rpcprotocol::Controller> controller_;
};

struct SwapChunkArgs {
  SwapChunkArgs(const std::string &chunkname,
                const std::string &remote_ip,
                const boost::uint16_t &remote_port,
                const std::string &rendezvous_ip,
                const boost::uint16_t &rendezvous_port,
                kad::VoidFunctorOneString cb)
     : chunkname_(chunkname),
       remote_ip_(remote_ip),
       remote_port_(remote_port),
       rendezvous_ip_(rendezvous_ip),
       rendezvous_port_(rendezvous_port),
       cb_(cb) {}
  std::string chunkname_;
  std::string remote_ip_;
  boost::uint16_t remote_port_;
  std::string rendezvous_ip_;
  boost::uint16_t rendezvous_port_;
  kad::VoidFunctorOneString cb_;
};

class PDVault {
 public:
  // vault_dir will contain .kadconfig (copied from read_only_kad_config_file)
  // and "Chunkstore" directory.
  PDVault(const std::string &pmid_public,
          const std::string &pmid_private,
          const std::string &signed_pmid_public,
          const fs::path &vault_dir,
          const boost::uint16_t &port,
          bool port_forwarded,
          bool use_upnp,
          const fs::path &read_only_kad_config_file,
          const boost::uint64_t &available_space,
          const boost::uint64_t &used_space,
          const boost::uint8_t &k);
  ~PDVault() {}
  void Start(bool first_node);
  int Stop();
  void CleanUp();
  VaultStatus vault_status();
  void SetVaultStatus(const VaultStatus &vault_status);
  bool WaitForStartup(const boost::uint16_t &timeout);
  bool WaitForSync();
  std::string pmid() const { return pmid_; }
  inline boost::uint64_t available_space() {
    return vault_chunkstore_->available_space();
  }
  inline boost::uint64_t UsedSpace() { return vault_chunkstore_->used_space(); }
  inline boost::uint64_t FreeSpace() { return vault_chunkstore_->FreeSpace(); }

  void SyncVault(kad::VoidFunctorOneString) {}
  void RepublishChunkRef(kad::VoidFunctorOneString) {}
/*
  void ValidityCheck(const std::string &chunk_name,
                     const std::string &random_data,
                     const kad::Contact &remote,
                     int attempt,
                     kad::VoidFunctorOneString cb);
*/
  void GetChunk(const std::string &chunk_name, kad::VoidFunctorOneString cb);
  void SwapChunk(const std::string &chunk_name,
                 const std::string &remote_ip,
                 const boost::uint16_t &remote_port,
                 const std::string &rendezvous_ip,
                 const boost::uint16_t &rendezvous_port,
                 kad::VoidFunctorOneString cb);
  void StopRvPing() { transport_handler_.StopPingRendezvous(); }
  friend class maidsafe::test::localvaults::Env;
  friend class test::PDVaultTest;
  friend class test::PDVaultTest_FUNC_MAID_NET_StoreAndGetChunks_Test;
  friend class test::PDVaultTest_FUNC_MAID_NET_Cachechunk_Test;
  friend class RunPDVaults;
  friend class maidsafe::test::CBPHandlerTest;
  friend class maidsafe::test::ImMessagingTest;
  friend class maidsafe::test::CCImMessagingTest;
 private:
  PDVault(const PDVault&);
  PDVault& operator=(const PDVault&);
  void RegisterMaidService();
  void UnRegisterMaidService();
  // This runs in a continuous loop until vault_status_ is not kVaultStarted.
  void PrunePendingOperations();
  // Removes this vault's ID from reference list for chunkname.
  int RemoveFromRefList(const std::string &chunkname,
                        const maidsafe::SignedSize &signed_size);
  // Runs in a worker thread to remove this vault's ID from a chunk ref packet.
  void RemoveFromRefPacket(const std::string &chunkname,
                           const maidsafe::SignedSize &signed_size);
  int AmendAccount(const boost::uint64_t &space_offered);
  void AmendAccountCallback(size_t index,
                            boost::shared_ptr<maidsafe::AmendAccountData> data);
  // Send request to kad-closest and k-th closest peers for their maidsafe info.
  void JoinMaidsafeNet();
  void GetSyncDataCallback(bool *done,
      std::pair<boost::mutex*, boost::condition_variable*> sync);

/*
  void IterativeSyncVault(boost::shared_ptr<SyncVaultData> data);
  void SyncVault_FindAlivePartner(
      const std::string& result,
      boost::shared_ptr<SyncVaultData> data,
      std::string chunk_name);
  void SyncVault_FindAlivePartner_Callback(
      const std::string& result,
      boost::shared_ptr<GetAlivePartner> partner_data,
      kad::Contact remote);
  void ValidityCheckCallback(
    boost::shared_ptr<maidsafe::ValidityCheckResponse> validity_check_response,
    boost::shared_ptr<ValidityCheckArgs> validity_check_args);
  void IterativeSyncVault_SyncChunk(
      const std::string& result,
      boost::shared_ptr<SyncVaultData> data,
      std::string chunk_name, kad::Contact remote);
  void IterativeSyncVault_UpdateChunk(
      boost::shared_ptr<maidsafe::GetChunkResponse> get_chunk_response,
      boost::shared_ptr<SynchArgs> synch_args);
  void IterativePublishChunkRef(
      boost::shared_ptr<RepublishChunkRefData> data);
  void IterativePublishChunkRef_Next(const std::string &result,
      boost::shared_ptr<RepublishChunkRefData> data);
*/
  void CheckChunk(boost::shared_ptr<GetArgs> get_args);
  void CheckChunkCallback(boost::shared_ptr<maidsafe::CheckChunkResponse>
      check_chunk_response, boost::shared_ptr<GetArgs> get_args);
  void GetMessagesCallback(boost::shared_ptr<maidsafe::GetBPMessagesResponse>
      get_messages_response, boost::shared_ptr<GetArgs> get_args);
  void GetChunkCallback(boost::shared_ptr<maidsafe::GetChunkResponse>
      get_chunk_response, boost::shared_ptr<GetArgs> get_args);
  void RetryGetChunk(boost::shared_ptr<struct LoadChunkData> data);
  void FindChunkRef(boost::shared_ptr<struct LoadChunkData> data);
  void FindChunkRefCallback(const std::string &result,
                            boost::shared_ptr<struct LoadChunkData> data);
  void GetMessages(const std::string &chunk_name,
                   const std::string &public_key,
                   const std::string &signed_public_key,
                   kad::VoidFunctorOneString cb);
  void SwapChunkSendChunk(
      boost::shared_ptr<maidsafe::SwapChunkResponse> swap_chunk_response,
      boost::shared_ptr<SwapChunkArgs> swap_chunk_args);
  void SwapChunkAcceptChunk(
      boost::shared_ptr<maidsafe::SwapChunkResponse> swap_chunk_response,
      boost::shared_ptr<SwapChunkArgs> swap_chunk_args);
  boost::uint8_t K_;
  boost::uint16_t upper_threshold_;
  boost::uint16_t lower_threshold_;
  boost::uint16_t port_;
  transport::TransportUDT global_udt_transport_;
  boost::int16_t transport_id_;
  transport::TransportHandler transport_handler_;
  rpcprotocol::ChannelManager channel_manager_;
  maidsafe::MaidsafeValidator validator_;
  boost::shared_ptr<VaultRpcs> vault_rpcs_;
  boost::shared_ptr<VaultChunkStore> vault_chunkstore_;
  boost::shared_ptr<maidsafe::KadOps> kad_ops_;
  boost::shared_ptr<VaultService> vault_service_;
  VaultServiceLogic vault_service_logic_;
  VaultStatus vault_status_;
  boost::mutex vault_status_mutex_;
  boost::condition_variable vault_status_cond_;
  bool sync_done_, sync_succeeded_;
  kad::Contact our_details_;
  std::string pmid_public_, pmid_private_, signed_pmid_public_, pmid_;
  crypto::Crypto co_;
  boost::shared_ptr<rpcprotocol::Channel> svc_channel_;
  fs::path kad_config_file_;
  QThreadPool thread_pool_;
  boost::thread maidsafe_join_thread_;
  boost::shared_ptr<base::PublicRoutingTableHandler> routing_table_;
};

}  // namespace maidsafe_vault

#endif  // MAIDSAFE_VAULT_PDVAULT_H_
