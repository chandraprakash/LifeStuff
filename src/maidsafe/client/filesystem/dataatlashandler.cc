/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Manages databases for directories
* Version:      1.0
* Created:      09/09/2008 12:14:35 PM
* Revision:     none
* Compiler:     gcc
* Author:       David Irvine (di), david.irvine@maidsafe.net
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

#include "maidsafe/client/filesystem/dataatlashandler.h"
#include "boost/filesystem.hpp"
#include <exception>
#include "maidsafe/common/commonutils.h"
#include "maidsafe/client/filesystem/pddir.h"
#include "maidsafe/client/clientutils.h"
#include "maidsafe/client/sessionsingleton.h"

namespace fs3 = boost::filesystem3;

namespace maidsafe {

DataAtlasHandler::DataAtlasHandler() : db_dir_() {
  SessionSingleton *ss = SessionSingleton::getInstance();
  if (!ss->SessionName().empty()) {
    db_dir_ = file_system::DbDir(ss->SessionName());
  } else {
    std::string username("user1");
    std::string pin("1234");
    std::string s(".maidsafe" + EncodeToHex(SHA1String(pin + username)));
    db_dir_ = fs3::path(file_system::TempDir() / s / "dir");
  }
}

int DataAtlasHandler::Init(bool new_user) {
  if (!new_user) {
    return kSuccess;
  } else {
    // create root db
    int result;
    std::shared_ptr<PdDir> da(GetPdDir("/", CREATE, &result));
    // create keys db
//    std::string keys_db_name_;
//    GetKeyDbPath(&keys_db_name_);
//    int result_ = -1;
//    std::shared_ptr<KeyAtlas> key_db_(GetKeysDb(CREATE, &result_));
    return kSuccess;
  }
}

std::string DataAtlasHandler::GetElementNameFromPath(
    const std::string &element_path) {
#ifdef DEBUG
//   printf("\t\tGetElementNameFromPath::GetMetaDataMap %s\n",
//          element_path.c_str());
#endif
  fs3::path path(element_path);
  return path.filename().string();
}

void DataAtlasHandler::GetDbPath(const std::string &element_path,
                                 DbInitFlag flag,
                                 std::string *db_path) {
  fs3::path path(element_path);

  // unless we're creating a new dir db, the one we want is the branch of
  // element_path
  std::string pre_hash_db_name;
  if (flag != CREATE) {
    pre_hash_db_name = path.parent_path().string() + db_dir_.string();
    // if the branch is null, we're making an element in the root, so set
    // pre_hash_db_name to "/"+db_dir_
    if (path.parent_path().filename().empty())
      pre_hash_db_name = fs3::path("/" + db_dir_.string()).string();
  } else {
    pre_hash_db_name = path.string() + db_dir_.string();
  }

  *db_path = EncodeToHex(SHA1String(StringToLowercase(pre_hash_db_name)));
  fs3::path db_path1(db_dir_);
  db_path1 /= *db_path;
  *db_path = db_path1.string();
}

std::shared_ptr<PdDir> DataAtlasHandler::GetPdDir(
    const std::string &element_path,
    DbInitFlag flag,
    int *result) {
  std::string db_name;
#ifdef DEBUG
//   printf("In getpddir: element_path = %s\tand flag = %i\n",
//           element_path.c_str(), flag);
#endif
  GetDbPath(element_path, flag, &db_name);
#ifdef DEBUG
//   printf("In getpddir: getdbpath returned db_name as %s\n",
//          db_name.c_str());
#endif
//  int res = kDataAtlasError;
  std::shared_ptr<PdDir> da(new PdDir(db_name, flag, result));
#ifdef DEBUG
//   printf("In getpddir: made new db with result %i\n", res);
#endif
//  *result = res;
  return da;
}

int DataAtlasHandler::AddElement(const std::string &element_path,
                                 const std::string &ser_mdm,
                                 const std::string &ser_dm,
                                 const std::string &dir_key,
                                 bool make_new_db) {
  int result = kDataAtlasError;
  // create the new database if the element is a dir and make_new_db == true
  if (ser_dm == "" && make_new_db) {
#ifdef DEBUG
    // printf("This is a dir(%s)\n", element_path.c_str());
#endif
    std::shared_ptr<PdDir> da_newdir_(GetPdDir(element_path,
                                                 CREATE, &result));
#ifdef DEBUG
    // printf("New dir's db added with result %i", result);
#endif
    if (result != kSuccess)
      return result;
  }

#ifdef DEBUG
  // printf("Getting db.\n");
#endif
  std::shared_ptr<PdDir> da_(GetPdDir(element_path, CONNECT, &result));
#ifdef DEBUG
  // printf("Got db with result %i\n", result);
#endif
  if (result != kSuccess)
    return result;
#ifdef DEBUG
  // printf("Adding to db.\n");
#endif
  result = da_->AddElement(ser_mdm, ser_dm, dir_key);
#ifdef DEBUG
  // printf("Added to db with result %i\n", result);
#endif
  return result;
}

int DataAtlasHandler::ModifyMetaDataMap(const std::string &element_path,
                                        const std::string &ser_mdm,
                                        const std::string &ser_dm) {
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da_(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess)
    return result;
  return da_->ModifyMetaDataMap(ser_mdm, ser_dm);
}

int DataAtlasHandler::RemoveElement(const std::string &element_path) {
  int result = kDataAtlasError;
  std::string ser_mdm;
  result = GetMetaDataMap(element_path, &ser_mdm);
  if (result != kSuccess) {
    printf("DataAtlasHandler::RemoveElement fail 0\n");
    return result;
  }
  MetaDataMap mdm;
  if (!mdm.ParseFromString(ser_mdm)) {
    printf("DataAtlasHandler::RemoveElement fail 0.5\n");
    return result;
  }
  std::string ser_dm;
  if (mdm.type() != DIRECTORY && mdm.type() != EMPTY_DIRECTORY) {
    result = GetDataMap(element_path, &ser_dm);
    if (result != kSuccess) {
      printf("DataAtlasHandler::RemoveElement fail 1\n");
      return result;
    }
  }
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess) {
    printf("DataAtlasHandler::RemoveElement fail 2\n");
    return result;
  }
  result = da->RemoveElement(GetElementNameFromPath(element_path));
  if (result != kSuccess) {
    printf("DataAtlasHandler::RemoveElement fail 3\n");
    return result;
  }
  if (ser_dm == "") {
    try {
      std::string db_to_delete;
      GetDbPath(element_path, CREATE, &db_to_delete);
//      printf("Deleting DB: %s", db_to_delete.c_str());
      fs3::remove(db_to_delete);
    }
    catch(const std::exception &e) {
#ifdef DEBUG
      printf("%s", e.what());
#endif
      return kDataAtlasException;
    }
  }
  return kSuccess;
}

int DataAtlasHandler::ListFolder(const std::string &element_path,
                                 std::map<fs3::path, ItemType> *children) {
  int result = kDataAtlasError;

  if (element_path == "\\" || element_path == "/") {
    for (int i = 0 ; i < kRootSubdirSize ; i++) {
      children->insert(std::pair<fs3::path, ItemType>(
      TidyPath(kRootSubdir[i][0]), DIRECTORY));
    }
    return kSuccess;
  }

  // append "/a" to element_path so that GetPdDir finds correct branch
  fs3::path path_(element_path);
  path_ /= "a";
  std::string element_path_modified = path_.string();
  std::shared_ptr<PdDir> da_(GetPdDir(element_path_modified,
                                        CONNECT, &result));
  if (result != kSuccess)
    return result;
  return da_->ListFolder(children);
}

int DataAtlasHandler::RenameElement(const std::string &original_path,
                                    const std::string &target_path,
                                    bool force) {
  // As this is a rename, where the element is a dir, the original dir key can
  // be used.
  std::string dir_key;
  int result = GetDirKey(original_path, &dir_key);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Could not get original_path dirkey.\n");
#endif
    return result;
  }

  if (CopyElement(original_path, target_path, dir_key, force) != kSuccess) {
#ifdef DEBUG
    printf("Element could not be copied.\n");
#endif
    return kRenameElementError;
  }

  if (RemoveElement(original_path) != kSuccess) {
#ifdef DEBUG
    printf("Original element could not be removed.\n");
#endif
    return kRenameElementError;
  }
  return kSuccess;
}

int DataAtlasHandler::CopyElement(const std::string &original_path,
                                  const std::string &target_path,
                                  const std::string &new_dir_key,
                                  bool force) {
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da_original(GetPdDir(original_path, CONNECT,
      &result));
  if (result != kSuccess)
    return result;
  std::shared_ptr<PdDir> da_target(GetPdDir(target_path, CONNECT, &result));
  if (result != kSuccess)
    return result;
  std::string ser_mdm, ser_dm, original_name, target_name;
  original_name = GetElementNameFromPath(original_path);
  target_name = GetElementNameFromPath(target_path);

  // Check if target exists.  If so, and force==false, abort.
  result = da_target->GetMetaDataMap(target_name, &ser_mdm);
  if (result == kSuccess) {  // i.e. target exists
    if (!force) {
#ifdef DEBUG
      printf("Target element already exists.\n");
#endif
      return kCopyElementError;
    } else {
      // i.e. force==true, so delete old target in preparation of adding new.
      result = da_target->RemoveElement(target_name);
      if (result != kSuccess) {
#ifdef DEBUG
        printf("Couldn't remove existing target element before copying.\n");
#endif
        return kCopyElementError;
      }
    }
  }

  // Get original mdm and dm
  ser_mdm = "";
  result = da_original->GetMetaDataMap(original_name, &ser_mdm);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Can't retrieve original mdm for copying.\n");
#endif
    return kCopyElementError;
  }
  da_original->GetDataMap(original_name, &ser_dm);

  // Amend mdm
  MetaDataMap mdm;
  mdm.ParseFromString(ser_mdm);
  mdm.set_id(-2);
  mdm.set_display_name(target_name);
  mdm.SerializeToString(&ser_mdm);

#ifdef DEBUG
  // Add these under target path
  // printf("In DAH::Cpyelmnt, addingelement: %s with sermdm %s & serdm %s\n",
  //        target_path.c_str(), ser_mdm.c_str(), ser_dm.c_str());
#endif
  result = AddElement(target_path, ser_mdm, ser_dm, new_dir_key, false);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("In copyelement, result of addelement = %i\n", result);
#endif
    return kCopyElementError;
  }

  // If the element is a dir, the original db must be copied to the new one as
  // do any subdirs, sub-subdirs, etc.
  if (mdm.type() == 4 || mdm.type() == 5) {
    if (CopySubDbs(original_path, target_path) != kSuccess) {
#ifdef DEBUG
      printf("In copyelement, result of addelement = %i\n", result);
#endif
      return kCopyElementError;
    }
  }
  return kSuccess;
}

int DataAtlasHandler::CopyDb(const std::string &original_path,
                             const std::string &target_path) {
  std::string original_db_path, target_db_path;
  GetDbPath(original_path, CREATE, &original_db_path);
  GetDbPath(target_path, CREATE, &target_db_path);
#ifdef DEBUG
  // printf("In DAH::CopyDb:\noriginal_db_path_ = %s\ntarget_db_path_ = %s\n\n",
  //        original_db_path_.c_str(), target_db_path_.c_str());
#endif
  try {
    if (fs3::exists(target_db_path))
      fs3::remove(target_db_path);
//    fs3::copy_file(original_db_path, target_db_path);
    return kSuccess;
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("%s", e.what());
#endif
    return kDataAtlasException;
  }
}

int DataAtlasHandler::ListSubDirs(const std::string &element_path,
                                  std::vector<fs3::path> *subdirs) {
  int result = kDataAtlasError;
  // append "/a" to element_path so that GetPdDir finds correct branch
  fs3::path path(element_path);
  path /= "a";
  std::string element_path_modified = path.string();

  std::shared_ptr<PdDir> da(GetPdDir(element_path_modified, CONNECT,
      &result));
  if (result != kSuccess)
    return result;
  return da->ListSubDirs(subdirs);
}

int DataAtlasHandler::CopySubDbs(const std::string &original_path,
                                 const std::string &target_path) {
#ifdef DEBUG
  // printf("In DAH::CopySubDbs:\noriginal_path_ = %s\ntarget_path_ = %s\n\n",
  //        original_path_.c_str(), target_path_.c_str());
#endif
  int result = kDataAtlasError;
  std::vector<fs3::path> subdirs;
  result = ListSubDirs(original_path, &subdirs);
  if (result != kSuccess)
    return result;
  boost::uint16_t i = 0;
  while (i < subdirs.size()) {
    fs3::path orig_path(original_path / subdirs[i]);
    fs3::path targ_path(target_path / subdirs[i]);
    result = CopySubDbs(orig_path.string(), targ_path.string());
    if (result != kSuccess)
      // ie CopySubDbs failed
      return result;
    ++i;
  }
  return CopyDb(original_path, target_path);
}

int DataAtlasHandler::GetDirKey(const std::string &element_path,
                                std::string *dir_key) {
  int result = kDataAtlasError;
#ifdef DEBUG
//  printf("In DAH::GetDirKey, element_path_ = %s\n", element_path.c_str());
#endif
  if (element_path == "" || element_path == "/" || element_path == "\\") {
    *dir_key = maidsafe::SessionSingleton::getInstance()->RootDbKey();
    return kSuccess;
  }
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess) {
#ifdef DEBUG
    printf("In DAH::GetDirKey, result from GetPdDir = %i\n", result);
#endif
    return result;
  }
  return da->GetDirKey(GetElementNameFromPath(element_path), dir_key);
}

int DataAtlasHandler::GetDataMap(const std::string &element_path,
                                 std::string *ser_dm) {
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess) {
#ifdef DEBUG
    printf("In DAH::GetDataMap, result from GetPdDir = %i\n", result);
#endif
    return result;
  }
  return da->GetDataMap(GetElementNameFromPath(element_path), ser_dm);
}

int DataAtlasHandler::GetMetaDataMap(const std::string &element_path,
                                     std::string *ser_mdm) {
#ifdef DEBUG
  // printf("\t\tDataAtlasHandler::GetMetaDataMap %s\n", element_path.c_str());
#endif
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess)
    return result;
  std::string the_path(GetElementNameFromPath(element_path));
#ifdef DEBUG
  // printf("\t\tDataAtlasHandler::GetMetaDataMap the_path: %s\n",
  //   the_path.c_str());
#endif
  result = da->GetMetaDataMap(the_path, ser_mdm);
  return result;
}

int DataAtlasHandler::ChangeCtime(const std::string &element_path) {
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess)
    return result;
  return da->ChangeCtime(GetElementNameFromPath(element_path));
}

int DataAtlasHandler::ChangeMtime(const std::string &element_path) {
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess)
    return result;
  return da->ChangeMtime(GetElementNameFromPath(element_path));
}

int DataAtlasHandler::ChangeAtime(const std::string &element_path) {
  int result = kDataAtlasError;
  std::shared_ptr<PdDir> da(GetPdDir(element_path, CONNECT, &result));
  if (result != kSuccess)
    return result;
  return da->ChangeAtime(GetElementNameFromPath(element_path));
}

int DataAtlasHandler::DisconnectPdDir(const std::string &branch_path) {
  int result = kDataAtlasError;
  // append "/a" to branch_path so that GetPdDir finds correct branch
  fs3::path path(branch_path);
  path /= "a";
  std::string element_path_modified = path.string();

  std::shared_ptr<PdDir> da(GetPdDir(element_path_modified, DISCONNECT,
      &result));
  return result;
}

}  // namespace maidsafe
