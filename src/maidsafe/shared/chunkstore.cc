/*
* copyright maidsafe.net limited 2008
* The following source code is property of maidsafe.net limited and
* is not meant for external use. The use of this code is governed
* by the license file LICENSE.TXT found in teh root of this directory and also
* on www.maidsafe.net.
*
* You are not free to copy, amend or otherwise use this source code without
* explicit written permission of the board of directors of maidsafe.net
*
* Created on: Sep 29, 2008
* Author: Team
*/

#include "maidsafe/shared/chunkstore.h"

#include "boost/filesystem/fstream.hpp"
#include "boost/scoped_array.hpp"
#include "maidsafe/common/crypto.h"
#include "maidsafe/shared/config.h"

namespace maidsafe {

ChunkStore::ChunkStore(const std::string &chunkstore_dir,
                       const boost::uint64_t &available_space,
                       const boost::uint64_t &used_space)
    : chunkstore_set_(), path_map_(), kChunkstorePath_(chunkstore_dir),
      is_initialised_(false), initialised_mutex_(), chunkstore_set_mutex_(),
      kHashableLeaf_("Hashable"), kNonHashableLeaf_("NonHashable"),
      kNormalLeaf_("Normal"), kCacheLeaf_("Cache"), kOutgoingLeaf_("Outgoing"),
      kTempCacheLeaf_("TempCache"), available_space_(available_space),
      used_space_(used_space) {}

bool ChunkStore::is_initialised() {
  bool init_result(false);
  {
    boost::mutex::scoped_lock lock(initialised_mutex_);
    init_result = is_initialised_;
  }
  return init_result;
}

void ChunkStore::set_is_initialised(bool value) {
  boost::mutex::scoped_lock lock(initialised_mutex_);
  is_initialised_ = value;
}

bool ChunkStore::Init() {
  if (is_initialised())
    return true;
  if (!PopulatePathMap()) {
#ifdef DEBUG
    printf("ChunkStore::Init failed to populate path map.\n");
#endif
    set_is_initialised(false);
    return false;
  }
  chunkstore_set_.clear();
  // Check root directories exist and if not, create them.
  bool temp_result = true;
  try {
    for (path_map_iterator path_map_itr = path_map_.begin();
         path_map_itr != path_map_.end(); ++path_map_itr) {
      if (fs3::exists((*path_map_itr).second)) {
        temp_result = PopulateChunkSet((*path_map_itr).first,
            (*path_map_itr).second) && temp_result;
// printf("Found %s\n", (*path_map_itr).second.string().c_str());
      } else {
        temp_result = temp_result &&
            fs3::create_directories((*path_map_itr).second);
// printf("Created %s\n", (*path_map_itr).second.string().c_str());
      }
    }
    set_is_initialised(temp_result);
    for (int i = 0; i < kDefaultChunkCount; ++i) {
      std::string key = DecodeFromHex(kDefaultChunks[i][0]);
      std::string value = DecodeFromHex(kDefaultChunks[i][1]);
      ChunkType type(kHashable | kNormal);
      fs3::path chunk_path(GetChunkPath(key, type, true));
      boost::uint64_t chunk_size(value.size());
      fs3::ofstream fstr;
      fstr.open(chunk_path, std::ios_base::binary);
      fstr.write(value.c_str(), chunk_size);
      fstr.close();
      ChunkInfo chunk(key, boost::posix_time::microsec_clock::local_time(),
          type, chunk_size);
      {
        boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
        chunkstore_set_.insert(chunk);
      }
    }
  }
  catch(const std::exception &ex) {
#ifdef DEBUG
    printf("ChunkStore::Init failed.\nException: %s\n", ex.what());
#endif
    set_is_initialised(false);
  }
  return is_initialised();
}

bool ChunkStore::PopulatePathMap() {
  try {
    path_map_iterator path_map_itr;
    fs3::path hashable_parent(kChunkstorePath_ / kHashableLeaf_);
    path_map_.insert(std::pair<ChunkType, fs3::path>(kHashable | kNormal,
        fs3::path(hashable_parent / kNormalLeaf_)));
    path_map_itr = path_map_.begin();
    path_map_.insert(path_map_itr, std::pair<ChunkType, fs3::path>
        (kHashable | kCache, fs3::path(hashable_parent / kCacheLeaf_)));
    ++path_map_itr;
    path_map_.insert(path_map_itr, std::pair<ChunkType, fs3::path>
        (kHashable | kOutgoing, fs3::path(hashable_parent / kOutgoingLeaf_)));
    ++path_map_itr;
    path_map_.insert(path_map_itr, std::pair<ChunkType, fs3::path>
        (kHashable | kTempCache, fs3::path(hashable_parent / kTempCacheLeaf_)));
    ++path_map_itr;
    fs3::path non_hashable_parent(kChunkstorePath_ / kNonHashableLeaf_);
    path_map_.insert(path_map_itr,
                     std::pair<ChunkType, fs3::path> (kNonHashable |
                       kNormal, fs3::path(non_hashable_parent / kNormalLeaf_)));
    ++path_map_itr;
    path_map_.insert(path_map_itr, std::pair<ChunkType, fs3::path>
        (kNonHashable | kCache, fs3::path(non_hashable_parent / kCacheLeaf_)));
    ++path_map_itr;
    path_map_.insert(path_map_itr, std::pair<ChunkType, fs3::path>
        (kNonHashable | kOutgoing,
        fs3::path(non_hashable_parent / kOutgoingLeaf_)));
    ++path_map_itr;
    path_map_.insert(path_map_itr, std::pair<ChunkType, fs3::path>
        (kNonHashable | kTempCache,
        fs3::path(non_hashable_parent / kTempCacheLeaf_)));
    if (size_t(8) != path_map_.size())
      return false;
    else
      return true;
  }
  catch(const std::exception &ex) {
#ifdef DEBUG
    printf("%s\n", ex.what());
#endif
    return false;
  }
}

void ChunkStore::FindFiles(const fs3::path &root_dir_path, ChunkType type,
                           bool hash_check, bool delete_failures,
                           boost::uint64_t *filecount,
                           std::list<std::string> *failed_keys) {
  try {
    if (!fs3::exists(root_dir_path))
      return;
    std::string non_hex_name;
    fs3::directory_iterator end_itr;
    for (fs3::directory_iterator itr(root_dir_path); itr != end_itr; ++itr) {
  // printf("Iter at %s\n", itr->path().filename().string().c_str());
      if (fs3::is_directory(itr->status())) {
        FindFiles(itr->path(), type, hash_check, delete_failures, filecount,
                  failed_keys);
      } else {
        ++(*filecount);
        non_hex_name = DecodeFromHex(itr->path().filename().string());
        boost::uint64_t size = fs3::file_size(itr->path());
        if (size >= 2) {
          ChunkInfo chunk(non_hex_name,
              boost::posix_time::microsec_clock::local_time(), type, size);
          {
            boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
            chunkstore_set_.insert(chunk);
          }
          if ((type & kHashable) && hash_check) {
            if (HashCheckChunk(non_hex_name, itr->path()) != kSuccess) {
              failed_keys->push_back(non_hex_name);
              if (delete_failures) {
                if (DeleteChunkFunction(non_hex_name, itr->path()) == kSuccess)
                  --(*filecount);
              }
            }
          }
        }
      }
    }
  }
  catch(const std::exception &ex) {
#ifdef DEBUG
    printf("ChunkStore::FindFiles - %s\n", ex.what());
#endif
  }
}

bool ChunkStore::PopulateChunkSet(ChunkType type, const fs3::path &dir_path) {
  boost::posix_time::ptime now(boost::posix_time::microsec_clock::local_time());
  boost::uint64_t original_size;
  {
    boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
    original_size = chunkstore_set_.size();
  }
  boost::uint64_t filecount = 0;
  std::list<std::string> failed_keys;
  FindFiles(dir_path, type, true, true, &filecount, &failed_keys);
// std::list<std::string>::iterator itr;
// for (itr = failed_keys.begin(); itr != failed_keys.end(); ++itr) {
// if (DeleteChunk((*itr)) == kSuccess)
// --filecount;
// }
  boost::uint64_t current_size;
  {
    boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
    current_size = chunkstore_set_.size();
  }
#ifdef DEBUG
// printf("In ChunkStore::PopulateChunkSet %s, current_size = %llu, "
// "original_size = %llu, filecount = %llu\n",
// dir_path.string().c_str(), current_size, original_size, filecount);
#endif
  return ((current_size - original_size) == filecount);
}

bool ChunkStore::Has(const std::string &key) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return false;
  bool result(false);
  {
    boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
    chunk_set_by_non_hex_name::iterator itr =
        chunkstore_set_.get<non_hex_name>().find(key);
    result = (itr != chunkstore_set_.end());
  }
  return result;
}

ChunkType ChunkStore::chunk_type(const std::string &key) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
  chunk_set_by_non_hex_name::iterator itr =
      chunkstore_set_.get<non_hex_name>().find(key);
  return itr != chunkstore_set_.end() ? (*itr).type_ :
      static_cast<ChunkType>(kInvalidChunkType);
}

ChunkType ChunkStore::GetChunkType(const std::string &key,
                                   const std::string &value, bool outgoing) {
  // Return type if we already have the chunk's details
  ChunkType type = chunk_type(key);
  if (type != kInvalidChunkType)
    return type;
  // otherwise this is a new chunk
  if (outgoing)
    type = kOutgoing;
  else
    type = kNormal;
  if (key == crypto::Hash<crypto::SHA512>(value)) {
    type |= kHashable;
  } else {
    type |= kNonHashable;
  }
  return type;
}

ChunkType ChunkStore::GetChunkType(const std::string &key,
                                   const fs3::path &file,
                                   bool outgoing) {
  // Return type if we already have the chunk's details
  ChunkType type = chunk_type(key);
  if (type != kInvalidChunkType)
    return type;
  // otherwise this is a new chunk
  if (outgoing)
    type = kOutgoing;
  else
    type = kNormal;
  try {
    if (fs3::exists(file)) {
      if (key == crypto::HashFile<crypto::SHA512>(file)) {
        type |= kHashable;
      } else {
        type |= kNonHashable;
      }
    } else {
      type = kInvalidChunkType;
    }
    return type;
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("%s\n", e.what());
#endif
    return kInvalidChunkType;
  }
}

fs3::path ChunkStore::GetChunkPath(const std::string &key, ChunkType type,
                                  bool create_path) {
  if (key.size() != kKeySize) {
#ifdef DEBUG
    printf("In ChunkStore::GetChunkPath, %s has size %u, not %u\n",
           HexSubstr(key).c_str(), key.size(), kKeySize);
#endif
    return "";
  }
  path_map_iterator path_map_itr = path_map_.find(type);
  if (path_map_itr == path_map_.end()) {
#ifdef DEBUG
    printf("In ChunkStore::GetChunkPath, %i is not a valid type\n", type);
#endif
    return "";
  }
  std::string hex_key = EncodeToHex(key);
  std::string dir_one, dir_two, dir_three;
/*MAHMOUD dir_one = hex_key.substr(0, 1);
dir_two = hex_key.substr(1, 1);
dir_three = hex_key.substr(2, 1);
fs3::path chunk_path((*path_map_itr).second / dir_one / dir_two / dir_three);*/
  // Added to store all chunks in one directory
  fs3::path chunk_path((*path_map_itr).second);
  try {
    if (!fs3::exists(chunk_path)) {
      if (create_path) {
          fs3::create_directories(chunk_path);
        chunk_path /= hex_key;
      } else {
        chunk_path.clear();
      }
    } else {
      chunk_path /= hex_key;
    }
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("%s\n", e.what());
#endif
    chunk_path.clear();
  }
#ifdef DEBUG
// printf("Chunk path: %s\n", chunk_path.string().c_str());
#endif
  return chunk_path;
}

boost::uint64_t ChunkStore::GetChunkSize(const std::string &key) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return 0;
  boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
  chunk_set_by_non_hex_name::iterator itr =
      chunkstore_set_.get<non_hex_name>().find(key);
  return itr != chunkstore_set_.end() ? itr->size_ : 0;
}

int ChunkStore::Store(const std::string &key, const std::string &value) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  if (Has(key)) {
#ifdef DEBUG
    printf("Chunk %s already exists in ChunkStore::StoreChunk.\n",
           HexSubstr(key).c_str());
#endif
// If chunk is cached and is hashable, change type to kNormal. If chunk is
// in Outgoing dir, leave it there. If chunk is non-hashable, update it.
    ChunkType type = chunk_type(key);
    if ((type & kOutgoing) == kOutgoing)
      return kSuccess;
    if (type == (kHashable | kCache) || type == (kHashable | kTempCache))
      return ChangeChunkType(key, kHashable | kNormal);
    if ((type & kNonHashable) != kNonHashable)
      return kInvalidChunkType;
  }
  ChunkType type = GetChunkType(key, value, false);
  fs3::path chunk_path(GetChunkPath(key, type, true));
  return StoreChunkFunction(key, value, chunk_path, type);
}

int ChunkStore::Store(const std::string &key, const fs3::path &file) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  if (Has(key)) {
// If chunk is cached and is hashable, change type to kNormal. If chunk is
// in Outgoing dir, leave it there. If chunk is non-hashable, update it.
    ChunkType type = chunk_type(key);
    if ((type & kOutgoing) == kOutgoing)
      return kSuccess;
// #ifdef DEBUG
// printf("Chunk already exists in ChunkStore::StoreChunk.\n");
// #endif
    if (type == (kHashable | kCache) || type == (kHashable | kTempCache))
      return ChangeChunkType(key, kHashable | kNormal);
    if ((type & kNonHashable) != kNonHashable)
      return kInvalidChunkType;
  }
  ChunkType type = GetChunkType(key, file, false);
  fs3::path chunk_path(GetChunkPath(key, type, true));
  return StoreChunkFunction(key, file, chunk_path, type);
}

int ChunkStore::AddChunkToOutgoing(const std::string &key,
                                   const std::string &value) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  if (Has(key)) {
// #ifdef DEBUG
// printf("Chunk already exists in ChunkStore::AddChunkToOutgoing.\n");
// #endif
    return kChunkExistsInChunkstore;
  }
  ChunkType type = GetChunkType(key, value, true);
  fs3::path chunk_path(GetChunkPath(key, type, true));
  return (StoreChunkFunction(key, value, chunk_path, type) == kSuccess) ?
      kSuccess : kChunkstoreFailedStore;
}

int ChunkStore::AddChunkToOutgoing(const std::string &key,
                                   const fs3::path &file) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  if (Has(key)) {
// #ifdef DEBUG
// printf("Chunk already exists in ChunkStore::AddChunkToOutgoing.\n");
// #endif
    return kChunkExistsInChunkstore;
  }
  ChunkType type = GetChunkType(key, file, true);
  fs3::path chunk_path(GetChunkPath(key, type, true));
  return (StoreChunkFunction(key, file, chunk_path, type) == kSuccess) ?
      kSuccess: kChunkstoreFailedStore;
}

int ChunkStore::StoreChunkFunction(const std::string &key,
                                   const std::string &value,
                                   const fs3::path &chunk_path,
                                   ChunkType type) {
  try {
    boost::uint64_t chunk_size(value.size());
    fs3::ofstream fstr;
    fstr.open(chunk_path, std::ios_base::binary);
    fstr.write(value.c_str(), chunk_size);
    fstr.close();
    // If the chunk is hashable then set last checked time to now, otherwise
    // set it to max allowable time.
    boost::posix_time::ptime lastcheckedtime(boost::posix_time::max_date_time);
    if (type & kHashable) {
      lastcheckedtime = boost::posix_time::microsec_clock::local_time();
    }
    ChunkInfo chunk(key, lastcheckedtime, type, chunk_size);
    {
      boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
      chunkstore_set_.insert(chunk);
      if (!(type & kCache))
        IncrementUsedSpace(chunk_size);
    }
    return kSuccess;
  }
  catch(const std::exception&) {
#ifdef DEBUG
// printf("ChunkStore::StoreChunk exception writing chunk: %s\n", ex.what());
#endif
    return kChunkstoreException;
  }
}

int ChunkStore::StoreChunkFunction(const std::string &key,
                                   const fs3::path &input_file,
                                   const fs3::path &chunk_path,
                                   ChunkType type) {
  try {
    if (fs3::exists(chunk_path))
      fs3::remove_all(chunk_path);
    fs3::rename(input_file, chunk_path);
    boost::uint64_t chunk_size(fs3::file_size(chunk_path));
    // If the chunk is hashable then set last checked time to now, otherwise
    // set it to max allowable time.
    boost::posix_time::ptime lastcheckedtime(boost::posix_time::max_date_time);
    if (type & kHashable) {
      lastcheckedtime = boost::posix_time::microsec_clock::local_time();
    }
    ChunkInfo chunk(key, lastcheckedtime, type, chunk_size);
    {
      boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
      chunkstore_set_.insert(chunk);
      if (!(type & kCache))
        IncrementUsedSpace(chunk_size);
    }
    return kSuccess;
  }
  catch(const std::exception&) {
#ifdef DEBUG
// printf("ChunkStore::StoreChunk exception writing chunk %s: %s\n",
// chunk_path.string().c_str(), ex.what());
#endif
    return kChunkstoreException;
  }
}

int ChunkStore::DeleteChunk(const std::string &key) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  ChunkType type = chunk_type(key);
  // Chunk is not in multi-index
  if (type == kInvalidChunkType)
    return kSuccess;
  fs3::path chunk_path(GetChunkPath(key, type, false));
  return DeleteChunkFunction(key, chunk_path);
}

int ChunkStore::DeleteChunkFunction(const std::string &key,
                                    const fs3::path &chunk_path) {
  {
    boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
    chunk_set_by_non_hex_name::iterator itr =
        chunkstore_set_.get<non_hex_name>().find(key);
    if (itr != chunkstore_set_.end()) {  // i.e. we have the chunk's details
      DecrementUsedSpace(itr->size_);
      chunkstore_set_.erase(itr);
    } else {
      DecrementUsedSpace(fs3::file_size(chunk_path));
    }
  }
  // Doesn't matter if we don't actually remove chunk file.
  try {
    fs3::remove(chunk_path);
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("Couldn't remove file %s in ChunkStore::DeleteChunk - %s\n",
           chunk_path.string().c_str(), e.what());
#endif
  }
  /*
bool result(false);
{
boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
chunk_set_by_non_hex_name::iterator itr =
chunkstore_set_.get<non_hex_name>().find(key);
result = (itr == chunkstore_set_.end());
}
return result ? kSuccess : kChunkstoreFailedDelete;
*/
  return kSuccess;
}

int ChunkStore::Clear() {
  // Delete the directory
  try {
    if (fs3::exists(kChunkstorePath_)) {
      fs3::remove_all(kChunkstorePath_);
    }
  } catch(const std::exception &e) {
#ifdef DEBUG
    printf("Couldn't remove directory %s in ChunkStore::DeleteDir - %s\n",
           kChunkstorePath_.string().c_str(), e.what());
    return kChunkstoreFailedDelete;
#endif
  }
  chunkstore_set_.clear();
  set_is_initialised(false);
  return kSuccess;
}

int ChunkStore::Load(const std::string &key, std::string *value) {
  value->clear();
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  ChunkType type = chunk_type(key);
  if (type == kInvalidChunkType)
    return kInvalidChunkType;
  fs3::path chunk_path(GetChunkPath(key, type, false));
  boost::uint64_t chunk_size(0);
  try {
    if (!fs3::exists(chunk_path)) {
#ifdef DEBUG
      printf("ChunkStore::Load - file %s doesn't exist.\n",
             chunk_path.string().c_str());
#endif
      DeleteChunkFunction(key, chunk_path);
      return kChunkFileDoesntExist;
    }
    chunk_size = fs3::file_size(chunk_path);
    if (chunk_size != GetChunkSize(key)) {
#ifdef DEBUG
      printf("ChunkStore::Load - file %s has wrong size.\n",
             chunk_path.string().c_str());
#endif
      // TODO(Team#) enqueue chunk file for checking/repairing/deleting
      return kHashCheckFailure;
    }
    boost::scoped_array<char> temp(new char[chunk_size]);
    fs3::ifstream fstr;
    fstr.open(chunk_path, std::ios_base::binary);
    fstr.read(temp.get(), chunk_size);
    fstr.close();
    std::string result(static_cast<const char*>(temp.get()), chunk_size);
    *value = result;
    return kSuccess;
  }
  catch(const std::exception &ex) {
#ifdef DEBUG
    printf("ChunkStore::Load - %s - path: %s\n", ex.what(),
           chunk_path.string().c_str());
#endif
    return kChunkstoreException;
  }
}

int ChunkStore::HashCheckChunk(const std::string &key) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  ChunkType type = chunk_type(key);
  if (type == kInvalidChunkType)
    return kInvalidChunkType;
  fs3::path chunk_path(GetChunkPath(key, type, false));
  if (chunk_path.empty())
    return kChunkstoreError;
  return HashCheckChunk(key, chunk_path);
}

int ChunkStore::HashCheckChunk(const std::string &key,
                               const fs3::path &chunk_path) {
  std::string non_hex_filename;
  boost::uint64_t chunk_size(0);
  boost::posix_time::ptime now(boost::posix_time::microsec_clock::local_time());
  {
    boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
    chunk_set_by_non_hex_name &non_hex_name_index =
        chunkstore_set_.get<non_hex_name>();
    chunk_set_by_non_hex_name::iterator itr = non_hex_name_index.find(key);
    if (itr == chunkstore_set_.end())
      return kChunkstoreError;
    non_hex_filename = itr->non_hex_name_;
    chunk_size = itr->size_;
    non_hex_name_index.modify(itr, change_last_checked(now));
  }

  if (chunk_size != fs3::file_size(chunk_path))
    return kHashCheckFailure;

  std::string file_hash = crypto::HashFile<crypto::SHA512>(chunk_path);
  return file_hash == non_hex_filename ? kSuccess : kHashCheckFailure;
}

int ChunkStore::ChangeChunkType(const std::string &key, ChunkType type) {
  int valid = InitialOperationVerification(key);
  if (valid != kSuccess)
    return valid;
  ChunkType current_type = chunk_type(key);
  if (current_type == kInvalidChunkType) {
#ifdef DEBUG
    printf("In ChunkStore::ChangeChunkType: chunk doesn't exist.\n");
#endif
    return kChunkstoreError;
  }
  if (current_type == type)
    return kSuccess;
  fs3::path current_chunk_path(GetChunkPath(key, current_type, false));
  fs3::path new_chunk_path(GetChunkPath(key, type, true));
  if (new_chunk_path.empty()) {
#ifdef DEBUG
    printf("In ChunkStore::ChangeChunkType, %i is not a valid type\n", type);
#endif
    return kInvalidChunkType;
  }
  // Try to rename file.
  bool copied(false);
  try {
    fs3::copy_file(current_chunk_path, new_chunk_path);
    copied = fs3::exists(new_chunk_path);
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("%s\n", e.what());
#endif
    return kChunkstoreException;
  }
  if (copied) {
    try {
      fs3::remove(current_chunk_path);
    }
    catch(const std::exception &e) {
#ifdef DEBUG
      printf("%s\n", e.what());
#endif
    }
  }
  {
    boost::mutex::scoped_lock lock(chunkstore_set_mutex_);
    chunk_set_by_non_hex_name &non_hex_name_index =
        chunkstore_set_.get<non_hex_name>();
    chunk_set_by_non_hex_name::iterator itr = non_hex_name_index.find(key);
    non_hex_name_index.modify(itr, change_type(type));
  }
  return kSuccess;
}

int ChunkStore::InitialOperationVerification(const std::string &key) {
  if (!is_initialised()) {
#ifdef DEBUG
    printf("ChunkStore not initialised.\n");
#endif
    return kChunkstoreUninitialised;
  }
  if (key.size() != kKeySize) {
#ifdef DEBUG
    printf("In ChunkStore, key size passed is %u, not %u.\n",
           key.size(), kKeySize);
#endif
    return kIncorrectKeySize;
  }
  return kSuccess;
}

}  // namespace maidsafe

