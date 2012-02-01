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


#include "maidsafe/lifestuff/local_chunk_manager.h"

#include "maidsafe/common/file_chunk_store.h"
#include "maidsafe/common/threadsafe_chunk_store.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/private/chunk_actions/chunk_action_authority.h"
#include "maidsafe/private/chunk_actions/chunk_types.h"

#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/return_codes.h"

namespace pca = maidsafe::priv::chunk_actions;

namespace maidsafe {

namespace lifestuff {

LocalChunkManager::LocalChunkManager(
    std::shared_ptr<ChunkStore> normal_local_chunk_store,
    const fs::path &simulation_directory)
    : ChunkManager(normal_local_chunk_store),
      simulation_chunk_store_(),
      simulation_chunk_action_authority_() {
  std::shared_ptr<FileChunkStore> file_chunk_store(new FileChunkStore);
  fs::path local_version_directory;
  if (simulation_directory.empty()) {
    boost::system::error_code error_code;
    local_version_directory = fs::temp_directory_path(error_code);
    if (error_code) {
      DLOG(ERROR) << "Failed to get temp directory: " << error_code.message();
      return;
    }
    local_version_directory /= "LocalUserCredentials";
  } else {
    local_version_directory = simulation_directory;
  }

  if (!file_chunk_store->Init(simulation_directory)) {
    DLOG(ERROR) << "Failed to initialise file chunk store";
    return;
  }

  simulation_chunk_store_.reset(new ThreadsafeChunkStore(file_chunk_store));
  simulation_chunk_action_authority_.reset(
      new priv::ChunkActionAuthority(simulation_chunk_store_));
}

LocalChunkManager::~LocalChunkManager() {}

void LocalChunkManager::GetChunk(const std::string &name,
                                 const rsa::Identity &/*owner_key_id*/,
                                 const rsa::PublicKey &owner_public_key,
                                 const std::string &/*ownership_proof*/) {
  // TODO(Team): Add check of ID on network
  unsigned char chunk_type(pca::GetDataType(name));
  bool chunk_exists(chunk_store_->Has(name));
  if (chunk_type == pca::kDefaultType && chunk_exists) {
    (*sig_chunk_got_)(name, kSuccess);
    return;
  }

  std::string content(
      simulation_chunk_action_authority_->Get(name, "", owner_public_key));
  if (content.empty()) {
    DLOG(ERROR) << "CAA failure on network chunkstore " << Base32Substr(name);
    (*sig_chunk_got_)(name, kGetPacketFailure);
  }

  if (chunk_exists) {
    if (!chunk_store_->Modify(name, content)) {
      DLOG(ERROR) << "Failed to modify locally " << Base32Substr(name);
      (*sig_chunk_got_)(name, kGetPacketFailure);
    }
  } else {
    if (!chunk_store_->Store(name, content)) {
      DLOG(ERROR) << "Failed to store locally " << Base32Substr(name);
      (*sig_chunk_got_)(name, kGetPacketFailure);
    }
  }

  (*sig_chunk_got_)(name, kSuccess);
}

void LocalChunkManager::StoreChunk(const std::string &name,
                                   const rsa::Identity &/*owner_key_id*/,
                                   const rsa::PublicKey &owner_public_key) {
  // TODO(Team): Add check of ID on network
  unsigned char chunk_type(pca::GetDataType(name));
  std::string content(chunk_store_->Get(name));
  if (content.empty()) {
    DLOG(ERROR) << "No chunk in local chunk store" << Base32Substr(name);
    (*sig_chunk_stored_)(name, kStorePacketFailure);
    return;
  }

  if (!simulation_chunk_action_authority_->Store(name,
                                                 content,
                                                 owner_public_key)) {
    DLOG(ERROR) << "CAA failure on network chunkstore " << Base32Substr(name);
    (*sig_chunk_stored_)(name, kStorePacketFailure);
    return;
  }

  if (chunk_type != pca::kDefaultType)
    chunk_store_->Delete(name);

  (*sig_chunk_stored_)(name, kSuccess);
}

void LocalChunkManager::DeleteChunk(const std::string &name,
                                    const rsa::Identity &/*owner_key_id*/,
                                    const rsa::PublicKey &owner_public_key,
                                    const std::string &ownership_proof) {
  // TODO(Team): Add check of ID on network
  if (!simulation_chunk_action_authority_->Delete(name,
                                                  "",
                                                  ownership_proof,
                                                  owner_public_key)) {
    DLOG(ERROR) << "CAA failure on network chunkstore " << Base32Substr(name);
    (*sig_chunk_deleted_)(name, kDeletePacketFailure);
    return;
  }

  (*sig_chunk_deleted_)(name, kSuccess);
}

void LocalChunkManager::ModifyChunk(const std::string &name,
                                    const std::string &content,
                                    const rsa::Identity &/*owner_key_id*/,
                                    const rsa::PublicKey &owner_public_key) {
  int64_t operation_diff;
  if (!simulation_chunk_action_authority_->Modify(name,
                                                  content,
                                                  "",
                                                  owner_public_key,
                                                  &operation_diff)) {
    DLOG(ERROR) << "CAA failure on network chunkstore " << Base32Substr(name);
    (*sig_chunk_modified_)(name, kUpdatePacketFailure);
    return;
  }

  (*sig_chunk_modified_)(name, kSuccess);
}

}  // namespace pd

}  // namespace maidsafe
