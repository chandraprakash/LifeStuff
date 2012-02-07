/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Unit tests for Authentication
* Version:      1.0
* Created:      2009-01-29-03.19.59
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

#include "maidsafe/common/buffered_chunk_store.h"
#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/private/chunk_actions/chunk_action_authority.h"
#include "maidsafe/private/chunk_actions/chunk_types.h"

#include "maidsafe/pd/client/client_container.h"
#include "maidsafe/pd/client/remote_chunk_store.h"

#include "maidsafe/lifestuff/authentication.h"
#include "maidsafe/lifestuff/local_chunk_manager.h"
#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/session.h"
#include "maidsafe/lifestuff/ye_olde_signal_to_callback_converter.h"

namespace args = std::placeholders;
namespace fs = boost::filesystem;
namespace pca = maidsafe::priv::chunk_actions;

namespace maidsafe {

namespace lifestuff {

namespace test {

class AuthenticationTest : public testing::TestWithParam<bool> {
 public:
  AuthenticationTest()
      : test_dir_(maidsafe::test::CreateTestPath()),
        session_(new Session),
        remote_chunk_store_(),
        authentication_(session_),
        username_("user"),
        pin_("1234"),
        password_("password1"),
        ser_dm_(RandomString(1000)),
        surrogate_ser_dm_(RandomString(1000)),
        converter_(new YeOldeSignalToCallbackConverter),
        service_(),
        work_(new boost::asio::io_service::work(service_)),
        threads_() {}

 protected:
  void SetUp() {
    for (int i(0); i != 10; ++i)
      threads_.create_thread(std::bind(
          static_cast<std::size_t(boost::asio::io_service::*)()>
              (&boost::asio::io_service::run), &service_));

    std::shared_ptr<BufferedChunkStore> bcs(new BufferedChunkStore(service_));
    bcs->Init(*test_dir_ / "buffered_chunk_store");
    std::shared_ptr<priv::ChunkActionAuthority> caa(
        new priv::ChunkActionAuthority(bcs));
    std::shared_ptr<LocalChunkManager> local_chunk_manager(
        new LocalChunkManager(bcs, *test_dir_ / "local_chunk_manager"));
    remote_chunk_store_.reset(new pd::RemoteChunkStore(bcs,
                                                       local_chunk_manager,
                                                       caa));

    session_->ResetSession();
    remote_chunk_store_->sig_chunk_stored()->connect(
        std::bind(&YeOldeSignalToCallbackConverter::Stored, converter_.get(),
                  args::_1, args::_2));
    remote_chunk_store_->sig_chunk_deleted()->connect(
        std::bind(&YeOldeSignalToCallbackConverter::Deleted, converter_.get(),
                  args::_1, args::_2));
    remote_chunk_store_->sig_chunk_modified()->connect(
        std::bind(&YeOldeSignalToCallbackConverter::Modified, converter_.get(),
                  args::_1, args::_2));
    authentication_.Init(remote_chunk_store_, converter_);
  }

  void TearDown() {
    work_.reset();
    service_.stop();
    threads_.join_all();
  }

  int GetMasterDataMap(std::string *ser_dm_login) {
    return GetMasterDataMap(ser_dm_login, password_);
  }

  int GetMasterDataMap(std::string *ser_dm_login, const std::string &password) {
    std::string serialised_data_atlas, surrogate_serialised_data_atlas;
    int res =
        authentication_.GetMasterDataMap(password,
                                         &serialised_data_atlas,
                                         &surrogate_serialised_data_atlas);
    if (res != 0) {
      return kPasswordFailure;
    }

    if (!serialised_data_atlas.empty()) {
      *ser_dm_login = serialised_data_atlas;
    } else if (!surrogate_serialised_data_atlas.empty()) {
      *ser_dm_login = surrogate_serialised_data_atlas;
    } else {
      ser_dm_login->clear();
      return kPasswordFailure;
    }

    return kSuccess;
  }

  void InitAndCloseCallback(int /*i*/) {}

  std::string PacketValueFromSession(passport::PacketType packet_type,
                                     bool confirmed) {
    return session_->passport_->PacketName(packet_type, confirmed);
  }

  std::string PacketSignerFromSession(passport::PacketType packet_type,
                                      bool confirmed) {
    switch (packet_type) {
      case passport::kMid:
          return session_->passport_->PacketName(passport::kAnmid, confirmed);
      case passport::kSmid:
          return session_->passport_->PacketName(passport::kAnsmid, confirmed);
      case passport::kTmid:
      case passport::kStmid:
          return session_->passport_->PacketName(passport::kAntmid, confirmed);
      default: return "";
    }
  }

  std::shared_ptr<fs::path> test_dir_;
  std::shared_ptr<Session> session_;
  std::shared_ptr<pd::RemoteChunkStore> remote_chunk_store_;
  Authentication authentication_;
  std::string username_, pin_, password_, ser_dm_, surrogate_ser_dm_;
  std::shared_ptr<YeOldeSignalToCallbackConverter> converter_;
  boost::asio::io_service service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  boost::thread_group threads_;

 private:
  AuthenticationTest(const AuthenticationTest&);
  AuthenticationTest &operator=(const AuthenticationTest&);
};

TEST_F(AuthenticationTest, FUNC_CreateUserSysPackets) {
  username_ += "01";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
}

TEST_F(AuthenticationTest, FUNC_GoodLogin) {
  username_ += "02";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, pin_));
  std::string ser_dm_login;
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login));
  ASSERT_EQ(ser_dm_, ser_dm_login);
  ASSERT_EQ(username_, session_->username());
  ASSERT_EQ(pin_, session_->pin());
  ASSERT_EQ(password_, session_->password());

  ASSERT_EQ(kSuccess, authentication_.SaveSession(ser_dm_ + "1"));

  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, pin_));

  ser_dm_login.clear();
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login));
  ASSERT_EQ(ser_dm_ + "1", ser_dm_login);
  ASSERT_EQ(username_, session_->username());
  ASSERT_EQ(pin_, session_->pin());
}

TEST_F(AuthenticationTest, FUNC_LoginNoUser) {
  username_ += "03";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, pin_));
  std::string ser_dm_login;
  password_ = "password_tonto";
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login));
  ASSERT_NE(ser_dm_, ser_dm_login);
}

TEST_F(AuthenticationTest, FUNC_RegisterUserOnce) {
  username_ += "041";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
  ASSERT_EQ(username_, session_->username());
  ASSERT_EQ(pin_, session_->pin());
//  Sleep(boost::posix_time::milliseconds(100));
  ASSERT_EQ(password_, session_->password());
}

TEST_F(AuthenticationTest, FUNC_RegisterUserWithoutNetworkCheck) {
  username_ += "042";
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
  ASSERT_EQ(username_, session_->username());
  ASSERT_EQ(pin_, session_->pin());
//  Sleep(boost::posix_time::milliseconds(100));
  ASSERT_EQ(password_, session_->password());
}

TEST_F(AuthenticationTest, FUNC_RegisterUserTwice) {
  username_ += "05";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
  session_->ResetSession();
  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, pin_));
}

TEST_F(AuthenticationTest, FUNC_RepeatedSaveSessionBlocking) {
  username_ += "06";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
  std::string original_tmidname(PacketValueFromSession(passport::kTmid, true));
  ASSERT_FALSE(original_tmidname.empty());

  // store current mid, smid and tmid details to check later whether they remain
  // on the network
  ser_dm_ = RandomString(1000);
  ASSERT_EQ(kSuccess, authentication_.SaveSession(ser_dm_));

  ser_dm_ = RandomString(1000);
  ASSERT_EQ(kSuccess, authentication_.SaveSession(ser_dm_));
  std::string tmidname(PacketValueFromSession(passport::kTmid, true));
  std::string stmidname(PacketValueFromSession(passport::kStmid, true));

//  ASSERT_TRUE(packet_manager_->KeyUnique(
//                  pca::ApplyTypeToName(original_tmidname,
//                                       pca::kModifiableByOwner),
//                  PacketSignerFromSession(passport::kTmid, true)));
//  ASSERT_FALSE(packet_manager_->KeyUnique(
//                   pca::ApplyTypeToName(stmidname,
//                                        pca::kModifiableByOwner),
//                   PacketSignerFromSession(passport::kStmid, true)));
//  ASSERT_FALSE(packet_manager_->KeyUnique(
//                   pca::ApplyTypeToName(tmidname,
//                                        pca::kModifiableByOwner),
//                   PacketSignerFromSession(passport::kTmid, true)));
}

TEST_F(AuthenticationTest, FUNC_ChangeUsername) {
  username_ += "08";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));

  std::string original_tmidname(PacketValueFromSession(passport::kTmid, true));
  std::string original_stmidname(PacketValueFromSession(passport::kStmid,
                                                        true));
  ASSERT_FALSE(original_tmidname.empty());
  ASSERT_FALSE(original_stmidname.empty());

  ASSERT_EQ(kSuccess, authentication_.ChangeUsername(ser_dm_ + "2",
                                                     "el iuserneim"));
  ASSERT_EQ("el iuserneim", session_->username());

  ASSERT_EQ(kUserExists, authentication_.GetUserInfo("el iuserneim", pin_));
  std::string ser_dm_login;
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login));
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
//  ASSERT_TRUE(packet_manager_->KeyUnique(
//                  pca::ApplyTypeToName(original_stmidname,
//                                       pca::kModifiableByOwner),
//                  PacketSignerFromSession(passport::kTmid, true)));
}

TEST_F(AuthenticationTest, FUNC_ChangePin) {
  username_ += "09";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));

  std::string original_tmidname(PacketValueFromSession(passport::kTmid, true));
  std::string original_stmidname(PacketValueFromSession(passport::kStmid,
                                                        true));
  ASSERT_FALSE(original_tmidname.empty());
  ASSERT_FALSE(original_stmidname.empty());

  ASSERT_EQ(kSuccess, authentication_.ChangePin(ser_dm_ + "2", "7894"));
  ASSERT_EQ("7894", session_->pin());

  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, "7894"));
  std::string ser_dm_login;
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login));
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
//  ASSERT_TRUE(packet_manager_->KeyUnique(
//                  pca::ApplyTypeToName(original_stmidname,
//                                       pca::kModifiableByOwner),
//                  PacketSignerFromSession(passport::kTmid, true)));
}

TEST_F(AuthenticationTest, FUNC_ChangePassword) {
  username_ += "10";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));

  ASSERT_EQ(kSuccess, authentication_.ChangePassword(ser_dm_ + "2",
                                                     "password_new"));
  ASSERT_EQ("password_new", session_->password());

  std::string ser_dm_login;
  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login));
  ASSERT_NE(ser_dm_, ser_dm_login);

  ser_dm_login.clear();
  ASSERT_EQ(kUserExists, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, GetMasterDataMap(&ser_dm_login, "password_new"));
  ASSERT_NE(ser_dm_, ser_dm_login);
}

TEST_F(AuthenticationTest, FUNC_RegisterLeaveRegister) {
  username_ += "13";
  ASSERT_EQ(kUserDoesntExist, authentication_.GetUserInfo(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));

  //  Remove user.
  ASSERT_EQ(kSuccess, authentication_.RemoveMe());

  //  Check user no longer registered.
  session_->ResetSession();
  ASSERT_NE(kUserExists, authentication_.GetUserInfo(username_, pin_));

  session_->ResetSession();
  ASSERT_EQ(kSuccess, authentication_.CreateUserSysPackets(username_, pin_));
  ASSERT_EQ(kSuccess, authentication_.CreateTmidPacket(password_,
                                                       ser_dm_,
                                                       surrogate_ser_dm_));
}

INSTANTIATE_TEST_CASE_P(LocalRemote,
                        AuthenticationTest,
                        testing::Values(true));

}  // namespace test

}  // namespace lifestuff

}  // namespace maidsafe
