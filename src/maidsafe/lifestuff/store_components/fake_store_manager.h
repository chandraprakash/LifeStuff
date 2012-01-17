/*
* ============================================================================
*
* Copyright [2011] maidsafe.net limited
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

#ifndef MAIDSAFE_LIFESTUFF_STORE_COMPONENTS_FAKE_STORE_MANAGER_H_
#define MAIDSAFE_LIFESTUFF_STORE_COMPONENTS_FAKE_STORE_MANAGER_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"

#include "maidsafe/common/rsa.h"

#include "maidsafe/lifestuff/return_codes.h"
#include "maidsafe/lifestuff/version.h"
#include "maidsafe/lifestuff/store_components/packet_manager.h"

#if MAIDSAFE_LIFESTUFF_VERSION != 111
#  error This API is not compatible with the installed library.\
    Please update the maidsafe-lifestuff library.
#endif


namespace maidsafe {

namespace priv { class ChunkActionAuthority; }

class ChunkStore;

namespace lifestuff {

class Session;

class FakeStoreManager : public PacketManager {
 public:
  explicit FakeStoreManager(std::shared_ptr<Session> session);
  virtual ~FakeStoreManager();
  int Close(bool cancel_pending_ops);
  bool KeyUnique(const std::string &key, const std::string &signing_key_id);
  void KeyUnique(const std::string &key,
                 const std::string &signing_key_id,
                 const VoidFuncOneInt &cb);
  int GetPacket(const std::string &packet_name,
                const std::string &signing_key_id,
                std::vector<std::string> *results);
  void GetPacket(const std::string &packet_name,
                 const std::string &signing_key_id,
                 const GetPacketFunctor &lpf);
  void StorePacket(const std::string &packet_name,
                   const std::string &value,
                   const std::string &signing_key_id,
                   const VoidFuncOneInt &cb);
  void DeletePacket(const std::string &packet_name,
                    const std::string &signing_key_id,
                    const VoidFuncOneInt &cb);
  void ModifyPacket(const std::string &packet_name,
                    const std::string &value,
                    const std::string &signing_key_id,
                    const VoidFuncOneInt &cb);
  std::shared_ptr<ChunkStore> chunk_store() const;

 protected:
//  ReturnCode Init(const boost::filesystem::path &buffered_chunk_store_dir);
  void ExecReturnCodeCallback(VoidFuncOneInt callback, ReturnCode return_code);
  void ExecReturnLoadPacketCallback(GetPacketFunctor callback,
                                    std::vector<std::string> results,
                                    ReturnCode return_code);

  boost::asio::io_service asio_service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  boost::thread_group thread_group_;
  std::shared_ptr<ChunkStore> client_chunk_store_;
  std::shared_ptr<priv::ChunkActionAuthority> chunk_action_authority_;
  std::shared_ptr<Session> session_;
  boost::filesystem::path temp_directory_path_;

 private:
  FakeStoreManager &operator=(const FakeStoreManager&);
  FakeStoreManager(const FakeStoreManager&);
};

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_STORE_COMPONENTS_FAKE_STORE_MANAGER_H_
