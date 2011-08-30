/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Singleton class which controls all maidsafe client operations
* Version:      1.0
* Created:      2009-01-28-11.09.12
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

#ifndef MAIDSAFE_LIFESTUFF_CLIENTCONTROLLER_H_
#define MAIDSAFE_LIFESTUFF_CLIENTCONTROLLER_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "boost/function.hpp"
#include "boost/signals2.hpp"
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

#include "maidsafe/lifestuff/maidsafe.h"
#include "maidsafe/lifestuff/returncodes.h"
#include "maidsafe/lifestuff/user_credentials_api.h"

#if MAIDSAFE_LIFESTUFF_VERSION != 106
#  error This API is not compatible with the installed library.\
    Please update the maidsafe-lifestuff library.
#endif


namespace bs2 = boost::signals2;

namespace maidsafe {

namespace dht {
namespace kademlia {
class Contact;
}  // namespace kademlia
}  // namespace dht

class ChunkStore;

namespace lifestuff {

namespace test {
class ClientControllerTest;
}  // namespace test

class Authentication;
class Contact;
class PacketManager;
class PrivateShare;
class SessionSingleton;
class MockClientController;
struct private_share;

class CCCallback {
 public:
  CCCallback()
      : result_(),
        return_code_(kPendingResult),
        mutex_(),
        cv_() {}
  void StringCallback(const std::string &result);
  void ReturnCodeCallback(const ReturnCode &return_code);
  std::string WaitForStringResult();
  ReturnCode WaitForReturnCodeResult();

 private:
  std::string result_;
  ReturnCode return_code_;
  boost::mutex mutex_;
  boost::condition_variable cv_;
};

class BPCallback {
 public:
  BPCallback()
      : result(kGeneralError),
        status(0) {}
  void ContactInfoCallback(const ReturnCode &res,
                           const boost::uint32_t &st) {
    result = res;
    status = st;
  }

  void Reset() { result = kGeneralError; }

  ReturnCode result;
  boost::uint32_t status;
};

struct VaultConfigParameters {
  VaultConfigParameters() : vault_type(0), space(0), port(0), directory() {}
  int vault_type;
  boost::uint32_t space;
  boost::uint32_t port;
  std::string directory;
};

class ClientController : public lifestuff::UserCredentials {
 public:
  ClientController()
      : client_chunkstore_(),
        local_sm_(),
        auth_(),
        ss_(),
        ser_da_(),
        client_store_(),
        initialised_(false),
        logging_out_(false),
        logged_in_(false),
        K_(0),
        upper_threshold_(0) {}

  ClientController &operator=(const ClientController&);
  ClientController(const ClientController&);

  ~ClientController() {}
  int Init(boost::uint8_t k);
  inline bool initialised() { return initialised_; }

  // User credential operations
  virtual int CheckUserExists(const std::string &username,
                              const std::string &pin);
  virtual bool ValidateUser(const std::string &password);
  virtual bool CreateUser(const std::string &username,
                          const std::string &pin,
                          const std::string &password);
  virtual bool Logout();
  virtual int SaveSession();
  virtual bool ChangeUsername(const std::string &new_username);
  virtual bool ChangePin(const std::string &new_pin);
  virtual bool ChangePassword(const std::string &new_password);
  virtual bool LeaveMaidsafeNetwork();

  virtual std::string Username();
  virtual std::string Pin();
  virtual std::string Password();

 private:
  friend class MockClientController;
  friend class test::ClientControllerTest;

  // Functions
  bool JoinKademlia();
  int ParseDa();
  int SerialiseDa();

  // Variables
  std::shared_ptr<ChunkStore> client_chunkstore_;
  std::shared_ptr<PacketManager> local_sm_;
  std::shared_ptr<Authentication> auth_;
  SessionSingleton *ss_;
  std::string ser_da_;
  std::string client_store_;
  bool initialised_;
  bool logging_out_;
  bool logged_in_;
  boost::uint8_t K_;
  boost::uint16_t upper_threshold_;
};

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_CLIENTCONTROLLER_H_