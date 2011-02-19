/* copyright maidsafe.net limited 2008
 * ==========================================================================
 *
 *       Filename:  filesystem.cc
 *
 *    Description:  filesystem mount / unmount commands.
 *    Also includes on-access scanner.
 *
 *        Version:  1.0
 *        Created:  09/11/2008 11:51:39 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  David Irvine (di), david.irvine@maidsafe.net
 *        Company:  maidsafe.net limited
 *
 *
 *
 * The following source code is property of maidsafe.net limited and
 * is not meant for external use. The use of this code is governed
 * by the license file LICENSE.TXT found in the root of this directory and also
 * on www.maidsafe.net.
 *
 * You are not free to copy, amend or otherwise use this source code without
 * explicit written permission of the board of directors of maidsafe.net
 *
 *
 * ==========================================================================
 */

#include "maidsafe/common/filesystem.h"
#include <boost/thread.hpp>
#include <stdio.h>
#ifdef LifeStuff_WIN32
#include <shlwapi.h>
#include <shlobj.h>
#include <sstream>
#endif
#include "maidsafe/common/returncodes.h"

namespace fs = boost::filesystem;

namespace file_system {

fs::path HomeDir() {
  fs::path dirname;
  if (std::getenv("USERPROFILE"))
    dirname = std::getenv("USERPROFILE");
  else if (std::getenv("userprofile"))
    dirname = std::getenv("userprofile");
  else if (std::getenv("HOME"))
    dirname = std::getenv("HOME");
  return dirname;
}

fs::path ApplicationDataDir() {
#if defined(LifeStuff_POSIX)
  return fs::path("/var/cache/maidsafe/");
#elif defined(LifeStuff_WIN32)
  fs::path app_path("");
  TCHAR szpth[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0,
      szpth))) {
    std::ostringstream stm;
    const std::ctype<char> &ctfacet =
        std::use_facet< std::ctype<char> >(stm.getloc());
    for (size_t i = 0; i < wcslen(szpth); ++i)
      stm << ctfacet.narrow(szpth[i], 0);
    app_path = fs::path(stm.str());
    app_path /= "maidsafe";
  }
  return app_path;
#elif defined(LifeStuff_APPLE)
  return fs::path("/Library/maidsafe/");
#endif
return fs::path("/tmp");
}

fs::path TempDir() {
#if defined(LifeStuff_WIN32)
  fs::path temp_dir("");
  if (std::getenv("TEMP"))
    temp_dir = std::getenv("TEMP");
  else if (std::getenv("TMP"))
    temp_dir = std::getenv("TMP");
#elif defined(P_tmpdir)
  fs::path temp_dir(P_tmpdir);
#else
  fs::path temp_dir("");
  if (std::getenv("TMPDIR")) {
    temp_dir = std::getenv("TMPDIR");
  } else {
    temp_dir = fs::path("/tmp");
    try {
      if (!fs::exists(temp_dir))
        temp_dir.clear();
    }
    catch(const std::exception &e) {
#ifdef DEBUG
      printf("In file_system::TempDir: %s\n", e.what());
#endif
      temp_dir.clear();
    }
  }
#endif
  size_t last_char = temp_dir.string().size() - 1;
  if (temp_dir.string()[last_char] == '/' ||
      temp_dir.string()[last_char] == '\\') {
    std::string temp_str = temp_dir.string();
    temp_str.resize(last_char);
    temp_dir = fs::path(temp_str);
  }
  return temp_dir;
}

fs::path LocalStoreManagerDir() {
  return TempDir() / "maidsafe_LocalStoreManager";
}

fs::path MaidsafeDir(const std::string &session_name) {
  return HomeDir() / (".maidsafe" + session_name);
}

fs::path MaidsafeHomeDir(const std::string &session_name) {
  return MaidsafeDir(session_name) / "msroot";
}

fs::path MaidsafeFuseDir(const std::string &session_name) {
  return MaidsafeDir(session_name) / ("maidsafe-" + session_name.substr(0, 8));
}

fs::path DbDir(const std::string &session_name) {
  return MaidsafeDir(session_name) / "dir";
}

fs::path MakeRelativeMSPath(const std::string &entry,
                            const std::string &session_name) {
  fs::path p(entry);
  std::string result(p.string());
  fs::path ms_path(MaidsafeHomeDir(session_name));
  fs::path home_path(HomeDir());
  fs::path ms_home_path(MaidsafeFuseDir(session_name));
  if (p.string().substr(0, ms_path.string().size()) == ms_path.string()) {
    result.erase(0, ms_path.string().size());
    return fs::path(result);
  } else if (p.string().substr(0, ms_home_path.string().size()) ==
      ms_home_path.string()) {
    result.erase(0, ms_home_path.string().size());
    return fs::path(result);
  } else if (p.string().substr(0, home_path.string().size()) ==
      home_path.string()) {
    result.erase(0, home_path.string().size());
    return fs::path(result);
  } else {
    return p;
  }
}

fs::path FullMSPathFromRelPath(const std::string &entry,
                               const std::string &session_name) {
  return MaidsafeHomeDir(session_name) / entry;
}

int Mount(const std::string &session_name,
          const maidsafe::DefConLevels &defcon) {
  if (session_name.empty())
    return maidsafe::kSessionNameEmpty;
  int result = UnMount(session_name, defcon);
  if (result != maidsafe::kSuccess)
    return result;
  try {
    fs::path db_dir = DbDir(session_name);
    fs::path ms_home_dir = MaidsafeHomeDir(session_name);
    // if DbDir is created OK, so is MaidsafeDir
    bool mount_result(true);
    if (!fs::exists(db_dir)) {
      mount_result = fs::create_directories(db_dir);
    }
    if (!fs::exists(ms_home_dir)) {
      mount_result = mount_result && fs::create_directories(ms_home_dir);
    }
    return mount_result ? maidsafe::kSuccess : maidsafe::kFileSystemMountError;
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("In file_system::CreateDirs: %s\n", e.what());
#endif
    return maidsafe::kFileSystemException;
  }
}

int UnMount(const std::string &session_name,
            const maidsafe::DefConLevels &defcon) {
  if (session_name.empty())
    return maidsafe::kSessionNameEmpty;
  fs::path db_dir = DbDir(session_name);
  fs::path ms_home_dir = MaidsafeHomeDir(session_name);
  fs::path ms_dir = MaidsafeDir(session_name);
  switch (defcon) {
    case maidsafe::kDefCon1:
      return RemoveDir(db_dir, kMaxRemoveDirAttempts) ?  maidsafe::kSuccess :
          maidsafe::kFileSystemUnmountError;
    case maidsafe::kDefCon2: {
      bool success1(RemoveDir(db_dir, kMaxRemoveDirAttempts));
      bool success2(RemoveDir(ms_home_dir, kMaxRemoveDirAttempts));
      return (success1 && success2) ?  maidsafe::kSuccess :
          maidsafe::kFileSystemUnmountError;
    }
    case maidsafe::kDefCon3:
      return RemoveDir(ms_dir, kMaxRemoveDirAttempts) ?  maidsafe::kSuccess :
          maidsafe::kFileSystemUnmountError;
    default:
      return maidsafe::kFileSystemUnmountError;
  }
}

int FuseMountPoint(const std::string &session_name) {
  try {
    fs::path fuse_dir = MaidsafeFuseDir(session_name);
    if (!fs::exists(fuse_dir)) {
      return fs::create_directories(fuse_dir) ? maidsafe::kSuccess :
          maidsafe::kFuseMountPointError;
    } else {
      return maidsafe::kSuccess;
    }
  }
  catch(const std::exception &e) {
#ifdef DEBUG
    printf("In file_system::FuseMountPoint: %s\n", e.what());
#endif
    return maidsafe::kFileSystemException;
  }
}

bool RemoveDir(const fs::path &dir, const boost::uint8_t &max_attempts) {
  bool removed(false);
  for (boost::uint8_t i = 0; (i < max_attempts) && !removed; ++i) {
    try {
      if (fs::exists(dir))
        fs::remove_all(dir);
    }
    catch(const std::exception& e) {
      printf("In RemoveDir A (%i) %s\n", i, e.what());
    }
    try {
      removed = !fs::exists(dir);
    }
    catch(const std::exception& e) {
      printf("In RemoveDir B (%i) %s\n", i, e.what());
    }
    if (!removed)
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  }
  return removed;
}

}  // namespace file_system
