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


#include "maidsafe/lifestuff/store_components/fake_store_manager.h"

#include "boost/filesystem.hpp"

#include "maidsafe/common/chunk_validation.h"
#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/dht/contact.h"

#include "maidsafe/lifestuff/data_handler.h"
#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/session.h"
#include "maidsafe/lifestuff/lifestuff_messages_pb.h"

namespace fs = boost::filesystem;

namespace maidsafe {

namespace lifestuff {

namespace {

void PrintDebugInfo(const std::string &packet_name,
                    const std::string &value1,
                    const std::string &value2,
                    const std::string &op_type) {
  if (value2.empty())
    DLOG(WARNING) << "FakeStoreManager::" << op_type << " - <key, value>("
                  << HexSubstr(packet_name) << ", " << HexSubstr(value1) << ")";
  else
    DLOG(WARNING) << "FakeStoreManager::" << op_type << " - <key>("
                  << HexSubstr(packet_name) << ") value(" << HexSubstr(value1)
                  << " --> " << HexSubstr(value2) << ")";
}

class VeritasChunkValidation : public ChunkValidation {
 public:
  VeritasChunkValidation() : ChunkValidation() {}
  ~VeritasChunkValidation() {}

  bool ValidName(const std::string &/*name*/) { return true; }
  bool Hashable(const std::string &/*name*/) { return true; }
  bool Modifiable(const std::string &/*name*/) { return true; }
  bool ValidChunk(const std::string &/*name*/, const std::string &/*content*/) {
    return true;
  }
  bool ValidChunk(const std::string &/*name*/, const fs::path &/*path*/) {
    return true;
  }
  std::string Version(const std::string &/*name*/,
                      const std::string &/*content*/) {
    return "";
  }
  std::string Version(const std::string &/*name*/, const fs::path &/*path*/) {
    return "";
  }

 private:
  VeritasChunkValidation(const VeritasChunkValidation&);
  VeritasChunkValidation& operator=(const VeritasChunkValidation&);
};

void GetDataSlot(const std::string &signal_data, std::string *slot_data) {
  *slot_data = signal_data;
}

}  // namespace

std::string GetPublicKey(const std::string &packet_name,
                         std::shared_ptr<Session> session) {
  std::shared_ptr<passport::Passport> pprt(session->passport_);
  for (int i(passport::kAnmid); i != passport::kMid; ++i) {
    passport::PacketType packet_type(static_cast<passport::PacketType>(i));
    if (pprt->PacketName(packet_type, false) == packet_name)
      return pprt->PacketValue(packet_type, false);
    if (pprt->PacketName(packet_type, true) == packet_name)
      return pprt->PacketValue(packet_type, true);
  }
  return "";
}

FakeStoreManager::FakeStoreManager(std::shared_ptr<Session> session)
    : asio_service_(),
      work_(new boost::asio::io_service::work(asio_service_)),
      thread_group_(),
      chunk_validation_(new VeritasChunkValidation),
      client_chunk_store_(),
      session_(session),
      temp_directory_path_() {
  boost::system::error_code error_code;
  temp_directory_path_ = fs::temp_directory_path(error_code);
  if (error_code)
    DLOG(ERROR) << "Failed to get temp directory: " << error_code.message();
}

ReturnCode FakeStoreManager::Init(const fs::path &buffered_chunk_store_dir) {
  for (int i = 0; i < 3; ++i) {
    thread_group_.create_thread(
        std::bind(static_cast<std::size_t(boost::asio::io_service::*)()>
                      (&boost::asio::io_service::run), &asio_service_));
  }

  boost::system::error_code error_code;
  if (!fs::exists(buffered_chunk_store_dir, error_code)) {
    fs::create_directories(buffered_chunk_store_dir, error_code);
    if (error_code) {
      DLOG(ERROR) << "Failed to create " << buffered_chunk_store_dir
                  << ": " << error_code.message();
      return kStoreManagerInitError;
    }
  }

  return kSuccess;
}

FakeStoreManager::~FakeStoreManager() {
  work_.reset();
  asio_service_.stop();
  thread_group_.join_all();
}

int FakeStoreManager::Close(bool /*cancel_pending_ops*/) { return kSuccess; }

bool FakeStoreManager::KeyUnique(const std::string &key) {
  DataHandler data_handler;
  return data_handler.ProcessData(DataHandler::kHas,
                                  key,
                                  "",
                                  "",
                                  client_chunk_store_) == kKeyUnique;
}

void FakeStoreManager::KeyUnique(const std::string &key,
                                 const VoidFuncOneInt &cb) {
  DataHandler data_handler;
  ReturnCode result(
      static_cast<ReturnCode>(data_handler.ProcessData(DataHandler::kHas,
                                                       key,
                                                       "",
                                                       "",
                                                       client_chunk_store_)));
  ExecReturnCodeCallback(cb, result);
}

int FakeStoreManager::GetPacket(const std::string &packet_name,
                                std::vector<std::string> *results) {
  DataHandler data_handler;
  std::string data;
  data_handler.get_data_signal()->connect(
      DataHandler::GetDataSignalPtr::element_type::slot_type(
          &GetDataSlot, _1, &data));

  int result(data_handler.ProcessData(DataHandler::kGet,
                                      packet_name,
                                      "",
                                      "",
                                      client_chunk_store_));
  if (result != kSuccess) {
    DLOG(ERROR) << "FakeStoreManager::GetPacket - Failure in DH::ProcessData: "
                << result;
    return kGetPacketFailure;
  }

  if (data.empty()) {
    DLOG(ERROR) << "FakeStoreManager::GetPacket - data empty";
    return kGetPacketFailure;
  }

  results->push_back(data);

  return kSuccess;
}

void FakeStoreManager::GetPacket(const std::string &packetname,
                                 const GetPacketFunctor &lpf) {
  std::vector<std::string> results;
  ReturnCode rc(static_cast<ReturnCode>(GetPacket(packetname, &results)));
  ExecReturnLoadPacketCallback(lpf, results, rc);
}

void FakeStoreManager::DeletePacket(const std::string &packet_name,
                                    const std::string &value,
                                    const VoidFuncOneInt &cb) {
  DLOG(INFO) << "Deleting <" << HexSubstr(packet_name) << ", "
             << HexSubstr(value) << ">";

  GenericPacket gp;
  if (!gp.ParseFromString(value)) {
    ExecReturnCodeCallback(cb, kDeletePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::DeletePacket - Failure to parse value";
    return;
  }

  std::string public_key(GetPublicKey(gp.signing_id(), session_));
  if (public_key.empty()) {
    ExecReturnCodeCallback(cb, kNoPublicKeyToCheck);
    DLOG(ERROR) << "FakeStoreManager::StorePacket - No public key";
    return;
  }

  std::string data;
  CreateSerialisedSignedValue(gp, &data);
  DataHandler data_handler;
  int result(data_handler.ProcessData(DataHandler::kDelete,
                                      packet_name,
                                      data,
                                      public_key,
                                      client_chunk_store_));
  if (result != kSuccess) {
    ExecReturnCodeCallback(cb, kDeletePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::DeletePacket - Failure in "
                << "DH::ProcessData: " << result;
    return;
  }

  ExecReturnCodeCallback(cb, kSuccess);
}

void FakeStoreManager::StorePacket(const std::string &packet_name,
                                   const std::string &value,
                                   const VoidFuncOneInt &cb) {
  DLOG(INFO) << "Storing <" << HexSubstr(packet_name) << ", "
             << HexSubstr(value) << ">";

  GenericPacket gp;
  if (!gp.ParseFromString(value)) {
    ExecReturnCodeCallback(cb, kStorePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::StorePacket - Failure to parse value";
    return;
  }

  std::string public_key(GetPublicKey(gp.signing_id(), session_));
  if (public_key.empty()) {
    ExecReturnCodeCallback(cb, kNoPublicKeyToCheck);
    DLOG(ERROR) << "FakeStoreManager::StorePacket - No public key - ID: "
                << HexSubstr(gp.signing_id());
    return;
  }

  std::string data;
  CreateSerialisedSignedValue(gp, &data);
  DataHandler data_handler;
  int result(data_handler.ProcessData(DataHandler::kStore,
                                      packet_name,
                                      data,
                                      public_key,
                                      client_chunk_store_));
  if (result != kSuccess) {
    ExecReturnCodeCallback(cb, kStorePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::StorePacket - Failure in "
                << "DH::ProcessData: " << result;
    return;
  }

  ExecReturnCodeCallback(cb, kSuccess);
}

void FakeStoreManager::UpdatePacket(const std::string &packet_name,
                                    const std::string &old_value,
                                    const std::string &new_value,
                                    const VoidFuncOneInt &cb) {
  DLOG(INFO) << "Updating <" << HexSubstr(packet_name) << ", "
             << HexSubstr(old_value) << "> to <" << HexSubstr(packet_name)
             << ", " << HexSubstr(new_value) << ">";
  PrintDebugInfo(packet_name, old_value, new_value, "UpdatePacket");

  GenericPacket old_gp, new_gp;
  if (!old_gp.ParseFromString(old_value)) {
    ExecReturnCodeCallback(cb, kDeletePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::UpdatePacket - Failure parsing old value";
    return;
  }
  if (!new_gp.ParseFromString(new_value)) {
    ExecReturnCodeCallback(cb, kDeletePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::UpdatePacket - Failure parsing new value";
    return;
  }

  // TODO(Team): Compare both signing ids?

  std::string public_key(GetPublicKey(old_gp.signing_id(), session_));
  if (public_key.empty()) {
    ExecReturnCodeCallback(cb, kNoPublicKeyToCheck);
    DLOG(ERROR) << "FakeStoreManager::StorePacket - No public key";
    return;
  }

  std::string old_data, new_data;
  CreateSerialisedSignedValue(old_gp, &old_data);
  CreateSerialisedSignedValue(new_gp, &new_data);

  DataHandler data_handler;
  int result(data_handler.ProcessData(DataHandler::kUpdate,
                                      packet_name,
                                      new_data,
                                      public_key,
                                      client_chunk_store_));
  if (result != kSuccess) {
    ExecReturnCodeCallback(cb, kUpdatePacketFailure);
    DLOG(ERROR) << "FakeStoreManager::UpdatePacket - Failure in "
                << "DH::ProcessData: " << result;
    return;
  }

  ExecReturnCodeCallback(cb, kSuccess);
}

void FakeStoreManager::ExecReturnCodeCallback(VoidFuncOneInt callback,
                                              ReturnCode return_code) {
  asio_service_.post(std::bind(callback, return_code));
}

void FakeStoreManager::ExecReturnLoadPacketCallback(
    GetPacketFunctor callback,
    std::vector<std::string> results,
    ReturnCode return_code) {
  asio_service_.post(std::bind(callback, results, return_code));
}

bool FakeStoreManager::ValidateGenericPacket(std::string ser_gp,
                                             std::string public_key) {
  GenericPacket gp;
  if (!gp.ParseFromString(ser_gp))
    return false;

  return crypto::AsymCheckSig(gp.data(), gp.signature(), public_key);
}

void FakeStoreManager::CreateSerialisedSignedValue(const GenericPacket &data,
                                                   std::string *ser_gp) {
  ser_gp->clear();
  DataWrapper data_wrapper;
  if (data.hashable())
    data_wrapper.set_data_type(DataWrapper::kHashableSigned);
  else
    data_wrapper.set_data_type(DataWrapper::kNonHashableSigned);
  GenericPacket *gp = data_wrapper.mutable_signed_data();
  *gp = data;
  *ser_gp = data_wrapper.SerializeAsString();
}

}  // namespace lifestuff

}  // namespace maidsafe
