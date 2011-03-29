/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Manages databases for directories
* Version:      1.0
* Created:      2009-01-28-11.23.16
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

#ifndef MAIDSAFE_CLIENT_FILESYSTEM_DATAATLASHANDLER_H_
#define MAIDSAFE_CLIENT_FILESYSTEM_DATAATLASHANDLER_H_

#include "boost/filesystem.hpp"
#include <map>
#include <string>
#include <vector>
#include "maidsafe/shared/maidsafe.h"
#include "maidsafe/client/filesystem/distributed_filesystem.pb.h"

namespace fs = boost::filesystem;

namespace maidsafe {

class PdDir;
class KeyAtlas;

class DataAtlasHandler {
 public:
  DataAtlasHandler();
  ~DataAtlasHandler() {}
  int Init(bool new_user);
  // methods for normal folder Data Atlases
  void GetDbPath(const std::string &element_path,
                 DbInitFlag flag,
                 std::string *db_path);
  int AddElement(const std::string &element_path,
                 const std::string &ser_mdm,
                 const std::string &ser_dm,
                 const std::string &dir_key,
                 bool make_new_db);
  int ModifyMetaDataMap(const std::string &element_path,
                        const std::string &ser_mdm,
                        const std::string &ser_dm);
                        // amend MDMs for files only
  int RemoveElement(const std::string &element_path);
  int ListFolder(const std::string &element_path,
                 std::map<fs::path, ItemType> *children);
  int RenameElement(const std::string &original_path,
                    const std::string &target_path,
                    bool force);
  int CopyElement(const std::string &original_path,
                  const std::string &target_path,
                  const std::string &new_dir_key,
                  bool force);
  int GetDirKey(const std::string &element_path, std::string *dir_key);
  int GetDataMap(const std::string &element_path, std::string *ser_dm);
  int GetMetaDataMap(const std::string &element_path, std::string *ser_mdm);
  int ChangeCtime(const std::string &element_path);
  int ChangeMtime(const std::string &element_path);
  int ChangeAtime(const std::string &element_path);
  int DisconnectPdDir(const std::string &branch_path);

 private:
  std::string GetElementNameFromPath(const std::string &element_path);
  std::shared_ptr<PdDir> GetPdDir(const std::string &element_path,
                                    DbInitFlag flag,
                                    int *result);
  int CopyDb(const std::string &original_path_,
             const std::string &target_path_);
  int ListSubDirs(const std::string &element_path,
                  std::vector<fs::path> *subdirs_);
  int CopySubDbs(const std::string &original_path_,
                 const std::string &target_path_);
  std::shared_ptr<KeyAtlas> GetKeysDb(DbInitFlag flag_, int *result);
  fs::path db_dir_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_CLIENT_FILESYSTEM_DATAATLASHANDLER_H_
