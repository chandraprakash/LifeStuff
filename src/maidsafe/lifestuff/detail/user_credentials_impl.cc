/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Creates, stores and accesses user details
* Version:      1.0
* Created:      2009-01-28-22.18.47
* Revision:     none
* Author:       Team
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

#include "maidsafe/lifestuff/detail/user_credentials_impl.h"

#include <memory>
#include <vector>

#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/thread.hpp"

#include "maidsafe/common/utils.h"

#include "maidsafe/private/chunk_actions/chunk_action_authority.h"
#include "maidsafe/private/chunk_actions/chunk_pb.h"
#include "maidsafe/private/chunk_actions/chunk_types.h"
#include "maidsafe/private/chunk_store/remote_chunk_store.h"

#include "maidsafe/passport/passport.h"

#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/detail/session.h"
#include "maidsafe/lifestuff/detail/utils.h"

namespace args = std::placeholders;
namespace pca = maidsafe::priv::chunk_actions;

namespace maidsafe {

namespace lifestuff {

typedef pcs::RemoteChunkStore::ValidationData ValidationData;

struct OperationResults {
  OperationResults(boost::mutex &mutex_in,  // NOLINT (Dan)
                   boost::condition_variable &conditional_variable_in,  // NOLINT (Dan)
                   std::vector<int> &individual_results_in)  // NOLINT (Dan)
      : mutex(mutex_in),
        conditional_variable(conditional_variable_in),
        individual_results(individual_results_in) {}
  boost::mutex &mutex;
  boost::condition_variable &conditional_variable;
  std::vector<int> &individual_results;
};

namespace {

int KeysToValidationData(std::shared_ptr<asymm::Keys> packet, ValidationData *validation_data) {
  pca::SignedData signed_data;
  signed_data.set_data(RandomString(16));
  asymm::Signature random_data_signature;
  asymm::Sign(signed_data.data(), packet->private_key, &random_data_signature);
  if (random_data_signature.empty()) {
    DLOG(ERROR) << "Ownership proof not properly created.";
    return kCreateSignaturePacketInfoFailure;
  }
  signed_data.set_signature(random_data_signature);

  validation_data->ownership_proof = signed_data.SerializeAsString();
  validation_data->key_pair.identity = packet->identity;
  validation_data->key_pair.public_key = packet->public_key;
  validation_data->key_pair.private_key = packet->private_key;
  validation_data->key_pair.validation_token = packet->validation_token;

  return kSuccess;
}

int CreateSignaturePacketInfo(std::shared_ptr<asymm::Keys> packet,
                              std::string *packet_name,
                              std::string *packet_content,
                              ValidationData *validation_data) {
  BOOST_ASSERT(packet && packet_name && packet_content && validation_data);
  *packet_name = pca::ApplyTypeToName(packet->identity, pca::kSignaturePacket);
  pca::SignedData signed_data;
  std::string public_key;
  asymm::EncodePublicKey(packet->public_key, &public_key);
  if (public_key.empty()) {
    DLOG(ERROR) << "Public key not properly enconded.";
    return kCreateSignaturePacketInfoFailure;
  }

  signed_data.set_data(public_key);
  signed_data.set_signature(packet->validation_token);
  if (!signed_data.SerializeToString(packet_content) ||
      packet_content->empty()) {
    DLOG(ERROR) << "SignedData not properly serialised.";
    return kCreateSignaturePacketInfoFailure;
  }

  return KeysToValidationData(packet, validation_data);
}

void OperationCallback(bool result, OperationResults &results, int index) {  // NOLINT (Dan)
  boost::mutex::scoped_lock barra_loch_an_duin(results.mutex);
  results.individual_results.at(index) = result ? kSuccess : kRemoteChunkStoreFailure;
  results.conditional_variable.notify_one();
}

int AssessJointResult(const std::vector<int> &results) {
  auto it(std::find_if(results.begin(),
                       results.end(),
                       [&](const int &element)->bool {
                         return element != kSuccess;
                       }));
  if (it != results.end())
    return kAtLeastOneFailure;

  return kSuccess;
}

}  // namespace

UserCredentialsImpl::UserCredentialsImpl(std::shared_ptr<pcs::RemoteChunkStore> remote_chunk_store,
                                         std::shared_ptr<Session> session)
    : remote_chunk_store_(remote_chunk_store),
      session_(session),
      passport_(session_->passport()),
      single_threaded_class_mutex_() {}

UserCredentialsImpl::~UserCredentialsImpl() {}

int UserCredentialsImpl::GetUserInfo(const std::string &keyword,
                                     const std::string &pin,
                                     const std::string &password) {
  boost::mutex::scoped_lock loch_a_phuill(single_threaded_class_mutex_);

  // Obtain MID, TMID
  int mid_tmid_result(kSuccess);
  std::string tmid_packet;
  boost::thread mid_tmid_thread(std::bind(&UserCredentialsImpl::GetIdAndTemporaryId, this,
                                          keyword, pin, password, false, &mid_tmid_result,
                                          &tmid_packet));
  // Obtain SMID, STMID
  int smid_stmid_result(kSuccess);
  std::string stmid_packet;
  boost::thread smid_stmid_thread(std::bind(&UserCredentialsImpl::GetIdAndTemporaryId, this,
                                            keyword, pin, password, true, &smid_stmid_result,
                                            &stmid_packet));

  // Wait for them to finish
  mid_tmid_thread.join();
  smid_stmid_thread.join();

  // Evaluate results
  if (mid_tmid_result == kIdPacketNotFound && smid_stmid_result == kIdPacketNotFound) {
    DLOG(INFO) << "User doesn't exist: " << keyword << ", " << pin;
    return kUserDoesntExist;
  }

  if (mid_tmid_result == kCorruptedPacket && smid_stmid_result == kCorruptedPacket) {
    DLOG(ERROR) << "Account corrupted. Should never happen: "
                << keyword << ", " << pin;
    return kAccountCorrupted;
  }

  int result(HandleSerialisedDataMaps(keyword, pin, password, tmid_packet, stmid_packet));
  if (result != kSuccess) {
    if (result != kUsingNextToLastSession) {
      DLOG(ERROR) << "Failed to initialise session: " << result;
      result = kAccountCorrupted;
    }
    return result;
  }

  session_->set_keyword(keyword);
  session_->set_pin(pin);
  session_->set_password(password);
  if (!session_->set_session_name()) {
    DLOG(ERROR) << "Failed to set session.";
    return kSessionFailure;
  }

  return kSuccess;
}

void UserCredentialsImpl::GetIdAndTemporaryId(const std::string &keyword,
                                              const std::string &pin,
                                              const std::string &password,
                                              bool surrogate,
                                              int *result,
                                              std::string *temporary_packet) {
  std::string id_name(pca::ApplyTypeToName(passport::MidName(keyword, pin, surrogate),
                                           pca::kModifiableByOwner));
  std::string id_packet(remote_chunk_store_->Get(id_name));
  if (id_packet.empty()) {
    DLOG(ERROR) << "No " << (surrogate ? "SMID" : "MID") << " found.";
    *result = kIdPacketNotFound;
    return;
  }

  pca::SignedData packet;
  if (!packet.ParseFromString(id_packet) || packet.data().empty()) {
    DLOG(ERROR) << (surrogate ? "SMID" : "MID") << " packet corrupted: Failed parse.";
    *result = kCorruptedPacket;
    return;
  }

  std::string decrypted_rid(passport::DecryptRid(keyword, pin, packet.data()));
  if (decrypted_rid.empty()) {
    DLOG(ERROR) << (surrogate ? "SMID" : "MID") << " packet corrupted: Failed decryption.";
    *result = kCorruptedPacket;
    return;
  }
  decrypted_rid = pca::ApplyTypeToName(decrypted_rid, pca::kModifiableByOwner);

  std::string temporary_id_packet(remote_chunk_store_->Get(decrypted_rid));
  if (temporary_id_packet.empty()) {
    DLOG(ERROR) << "No " << (surrogate ? "STMID" : "TMID") << " found.";
    *result = kTemporaryIdPacketNotFound;
    return;
  }

  packet.Clear();
  if (!packet.ParseFromString(temporary_id_packet) || packet.data().empty()) {
    DLOG(ERROR) << (surrogate ? "STMID" : "TMID") << " packet corrupted: "
                << "Failed parse.";
    *result = kCorruptedPacket;
    return;
  }

  *temporary_packet = passport::DecryptMasterData(keyword, pin, password, packet.data());
  if (temporary_packet->empty()) {
    DLOG(ERROR) << (surrogate ? "STMID" : "TMID") << " packet corrupted: "
                << "Failed decryption.";
    *result = kCorruptedPacket;
    return;
  }
}

int UserCredentialsImpl::HandleSerialisedDataMaps(const std::string &keyword,
                                                  const std::string &pin,
                                                  const std::string &password,
                                                  const std::string &tmid_serialised_data_atlas,
                                                  const std::string &stmid_serialised_data_atlas) {
  int result(kSuccess);
  std::string tmid_da, stmid_da;
  if (!tmid_serialised_data_atlas.empty()) {
    tmid_da = tmid_serialised_data_atlas;
    result = session_->ParseDataAtlas(tmid_serialised_data_atlas);
    if (result == kSuccess)
      session_->set_serialised_data_atlas(tmid_serialised_data_atlas);
  } else if (!stmid_serialised_data_atlas.empty()) {
    tmid_da = stmid_serialised_data_atlas;
    stmid_da = stmid_serialised_data_atlas;
    result = session_->ParseDataAtlas(stmid_serialised_data_atlas);
    if (result == kSuccess) {
      session_->set_serialised_data_atlas(stmid_serialised_data_atlas);
      result = kUsingNextToLastSession;
    }
  }

  if (stmid_da.empty() && !stmid_serialised_data_atlas.empty())
    stmid_da = stmid_serialised_data_atlas;

  result = passport_.SetIdentityPackets(keyword, pin, password, tmid_da, stmid_da);
  result += passport_.ConfirmIdentityPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failure to set and confirm identity packets.";
    return kSetIdentityPacketsFailure;
  }

  return result;
}

int UserCredentialsImpl::CreateUser(const std::string &keyword,
                                    const std::string &pin,
                                    const std::string &password) {
  boost::mutex::scoped_lock loch_a_phuill(single_threaded_class_mutex_);

  int result(ProcessSigningPackets());
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed processing signature packets: " << result;
    return kSessionFailure;
  }

  result = ProcessIdentityPackets(keyword, pin, password);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed processing identity packets: " << result;
    return kSessionFailure;
  }

  session_->set_keyword(keyword);
  session_->set_pin(pin);
  session_->set_password(password);
  if (!session_->set_session_name()) {
    DLOG(ERROR) << "Failed to set session.";
    return kSessionFailure;
  }

  return kSuccess;
}

int UserCredentialsImpl::ProcessSigningPackets() {
  int result(passport_.CreateSigningPackets());
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed creating signature packets: " << result;
    return kSessionFailure;
  }

  result = StoreAnonymousPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failure to Store Anonymous packets: " << result;
    return result;
  }

  result = passport_.ConfirmSigningPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed confirming signature packets: " << result;
    return kSessionFailure;
  }

  return kSuccess;
}

int UserCredentialsImpl::StoreAnonymousPackets() {
  std::vector<int> individual_results(4, kPendingResult);
  boost::condition_variable condition_variable;
  boost::mutex mutex;
  OperationResults results(mutex, condition_variable, individual_results);

  // ANMID path
  StoreAnmid(results);
  // ANSMID path
  StoreAnsmid(results);
  // ANTMID path
  StoreAntmid(results);
  // PMID path: ANMAID, MAID, PMID
  StoreAnmaid(results);

  int result(WaitForResults(mutex, condition_variable, individual_results));
  if (result != kSuccess) {
    DLOG(ERROR) << "Wait for results timed out: " << result;
    DLOG(ERROR) << "ANMID: " << individual_results.at(0)
              << ", ANSMID: " << individual_results.at(1)
              << ", ANTMID: " << individual_results.at(2)
              << ", PMID path: " << individual_results.at(3);
    return result;
  }
  DLOG(INFO) << "ANMID: " << individual_results.at(0)
             << ", ANSMID: " << individual_results.at(1)
             << ", ANTMID: " << individual_results.at(2)
             << ", PMID path: " << individual_results.at(3);

  result = AssessJointResult(individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "One of the operations for Anonymous Packets failed. "
                << "Turn on INFO for feedback on which one. ";
    return kCreateSignaturePacketsFailure;
  }

  return kSuccess;
}

void UserCredentialsImpl::StoreAnmid(OperationResults &results) {  // NOLINT (Dan)
  std::shared_ptr<asymm::Keys> anmid(passport_.SignaturePacketDetails(passport::kAnmid, false));
  StoreSignaturePacket(anmid, results, 0);
}

void UserCredentialsImpl::StoreAnsmid(OperationResults &results) {  // NOLINT (Dan)
  std::shared_ptr<asymm::Keys> ansmid(passport_.SignaturePacketDetails(passport::kAnsmid, false));
  StoreSignaturePacket(ansmid, results, 1);
}

void UserCredentialsImpl::StoreAntmid(OperationResults &results) {  // NOLINT (Dan)
  std::shared_ptr<asymm::Keys> antmid(passport_.SignaturePacketDetails(passport::kAntmid, false));
  StoreSignaturePacket(antmid, results, 2);
}

void UserCredentialsImpl::StoreSignaturePacket(
    std::shared_ptr<asymm::Keys> packet,
    OperationResults &results,  // NOLINT (Dan)
    int index) {
  std::string packet_name, packet_content;
  ValidationData validation_data;
  CreateSignaturePacketInfo(packet, &packet_name, &packet_content, &validation_data);
  if (!remote_chunk_store_->Store(packet_name,
                                  packet_content,
                                  std::bind(&OperationCallback, args::_1, results, index),
                                  validation_data)) {
    DLOG(ERROR) << "Failed to store: " << index;
    OperationCallback(false, results, index);
  }
}

void UserCredentialsImpl::StoreAnmaid(OperationResults &results) {  // NOLINT (Dan)
  std::shared_ptr<asymm::Keys> anmaid(
        passport_.SignaturePacketDetails(passport::kAnmaid, false));
  std::string packet_name, packet_content;
  ValidationData validation_data;
  CreateSignaturePacketInfo(anmaid, &packet_name, &packet_content, &validation_data);
  if (!remote_chunk_store_->Store(packet_name,
                                  packet_content,
                                  std::bind(&UserCredentialsImpl::StoreMaid, this,
                                            args::_1, results),
                                  validation_data)) {
    DLOG(ERROR) << "Failed to store ANMAID.";
    StoreMaid(false, results);
  }
}

void UserCredentialsImpl::StoreMaid(bool result, OperationResults &results) {  // NOLINT (Dan)
  if (!result) {
    DLOG(ERROR) << "Anmaid failed to store.";
    OperationCallback(false, results, 3);
    return;
  }

  std::shared_ptr<asymm::Keys> maid(passport_.SignaturePacketDetails(passport::kMaid, false));
  std::shared_ptr<asymm::Keys> anmaid(passport_.SignaturePacketDetails(passport::kAnmaid, false));

  ValidationData validation_data;
  int var(KeysToValidationData(anmaid, &validation_data));
  if (var != kSuccess) {
    DLOG(ERROR) << "Failed to procure MAID's validation data: " << var;
    StorePmid(false, results);
    return;
  }

  std::string maid_name(pca::ApplyTypeToName(maid->identity, pca::kSignaturePacket));
  pca::SignedData signed_maid;
  signed_maid.set_signature(maid->validation_token);
  std::string maid_string_public_key;
  asymm::EncodePublicKey(maid->public_key, &maid_string_public_key);
  if (maid_string_public_key.empty()) {
    DLOG(ERROR) << "Failed to procure sign MAID's public key.";
    StorePmid(false, results);
    return;
  }
  signed_maid.set_data(maid_string_public_key);

  if (!remote_chunk_store_->Store(maid_name,
                                  signed_maid.SerializeAsString(),
                                  std::bind(&UserCredentialsImpl::StorePmid, this,
                                            args::_1, results),
                                  validation_data)) {
    DLOG(ERROR) << "Failed to store MAID.";
    StorePmid(false, results);
  }
}

void UserCredentialsImpl::StorePmid(bool result, OperationResults &results) {  // NOLINT (Dan)
  if (!result) {
    DLOG(ERROR) << "Maid failed to store.";
    OperationCallback(false, results, 3);
    return;
  }

  std::shared_ptr<asymm::Keys> pmid(passport_.SignaturePacketDetails(passport::kPmid, false));
  std::shared_ptr<asymm::Keys> maid(passport_.SignaturePacketDetails(passport::kMaid, false));

  ValidationData validation_data;
  int var(KeysToValidationData(maid, &validation_data));
  if (var != kSuccess) {
    DLOG(ERROR) << "Failed to procure PMID's validation data: " << var;
    StorePmid(false, results);
    return;
  }

  std::string pmid_name(pca::ApplyTypeToName(pmid->identity, pca::kSignaturePacket));
  pca::SignedData signed_pmid;
  signed_pmid.set_signature(pmid->validation_token);
  std::string pmid_string_public_key;
  asymm::EncodePublicKey(pmid->public_key, &pmid_string_public_key);
  if (pmid_string_public_key.empty()) {
    DLOG(ERROR) << "Failed to procure sign PMID's public key.";
    StorePmid(false, results);
    return;
  }
  signed_pmid.set_data(pmid_string_public_key);

  if (!remote_chunk_store_->Store(pmid_name,
                                  signed_pmid.SerializeAsString(),
                                  std::bind(&OperationCallback, args::_1, results, 3),
                                  validation_data)) {
    DLOG(ERROR) << "Failed to store PMID.";
    OperationCallback(false, results, 3);
  }
}

int UserCredentialsImpl::ProcessIdentityPackets(const std::string &keyword,
                                                const std::string &pin,
                                                const std::string &password) {
  std::string serialised_data_atlas, surrogate_serialised_data_atlas;
  int result(session_->SerialiseDataAtlas(&serialised_data_atlas));
  Sleep(boost::posix_time::milliseconds(1));  // Need different timestamps
  result += session_->SerialiseDataAtlas(&surrogate_serialised_data_atlas);
  if (result != kSuccess ||
      serialised_data_atlas.empty() ||
      surrogate_serialised_data_atlas.empty()) {
    DLOG(ERROR) << "Don't have the appropriate elements to save on ID packets.";
    return kSessionSerialisationFailure;
  }

  result = passport_.SetIdentityPackets(keyword,
                                        pin,
                                        password,
                                        serialised_data_atlas,
                                        surrogate_serialised_data_atlas);
  if (result!= kSuccess) {
    DLOG(ERROR) << "Creation of ID packets failed.";
    return kSessionSerialisationFailure;
  }

  result = StoreIdentityPackets();
  if (result!= kSuccess) {
    DLOG(ERROR) << "Storing of ID packets failed.";
    return result;
  }

  result = passport_.ConfirmIdentityPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed confirming identity packets: " << result;
    return kSessionFailure;
  }

  session_->set_serialised_data_atlas(serialised_data_atlas);

  return kSuccess;
}

int UserCredentialsImpl::StoreIdentityPackets() {
  std::vector<int> individual_results(4, kPendingResult);
  boost::condition_variable condition_variable;
  boost::mutex mutex;
  OperationResults results(mutex, condition_variable, individual_results);

  // MID path
  StoreMid(results);
  // SMID path
  StoreSmid(results);
  // TMID path
  StoreTmid(results);
  // STMID
  StoreStmid(results);

  int result(WaitForResults(mutex, condition_variable, individual_results));
  if (result != kSuccess) {
    DLOG(ERROR) << "Wait for results timed out.";
    return result;
  }
  DLOG(INFO) << "MID: " << individual_results.at(0)
             << ", SMID: " << individual_results.at(1)
             << ", TMID: " << individual_results.at(2)
             << ", STMID: " << individual_results.at(3);

  result = AssessJointResult(individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "One of the operations for Identity Packets failed. "
                << "Turn on INFO for feedback on which one. ";
    return kStoreIdentityPacketsFailure;
  }

  return kSuccess;
}

void UserCredentialsImpl::StoreMid(OperationResults &results) {  // NOLINT (Dan)
  StoreIdentity(results, passport::kMid, passport::kAnmid, 0);
}

void UserCredentialsImpl::StoreSmid(OperationResults &results) {  // NOLINT (Dan)
  StoreIdentity(results, passport::kSmid, passport::kAnsmid, 1);
}

void UserCredentialsImpl::StoreTmid(OperationResults &results) {  // NOLINT (Dan)
  StoreIdentity(results, passport::kTmid, passport::kAntmid, 2);
}

void UserCredentialsImpl::StoreStmid(OperationResults &results) {  // NOLINT (Dan)
  StoreIdentity(results, passport::kStmid, passport::kAntmid, 3);
}

void UserCredentialsImpl::StoreIdentity(OperationResults &results,  // NOLINT (Dan)
                                        int identity_type,
                                        int signer_type,
                                        int index) {
  passport::PacketType id_pt(static_cast<passport::PacketType>(identity_type));
  passport::PacketType sign_pt(static_cast<passport::PacketType>(signer_type));
  std::string packet_name(passport_.PacketName(id_pt, false)),
              packet_content(passport_.IdentityPacketValue(id_pt, false));
  packet_name = pca::ApplyTypeToName(packet_name, pca::kModifiableByOwner);
  std::shared_ptr<asymm::Keys> signer(passport_.SignaturePacketDetails(sign_pt, true));
  ValidationData validation_data;
  int result(KeysToValidationData(signer, &validation_data));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to put keys into validation data: " << result;
    OperationCallback(false, results, index);
    return;
  }

  asymm::Signature signature;
  result = asymm::Sign(packet_content, signer->private_key, &signature);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to sign content: " << result;
    OperationCallback(false, results, index);
    return;
  }

  pca::SignedData signed_data;
  signed_data.set_data(packet_content);
  signed_data.set_signature(signature);

  if (!remote_chunk_store_->Store(packet_name,
                                  signed_data.SerializeAsString(),
                                  std::bind(&OperationCallback, args::_1, results, index),
                                  validation_data)) {
    DLOG(ERROR) << "Failed to store: " << index;
    OperationCallback(false, results, index);
  }
}

int UserCredentialsImpl::SaveSession() {
  boost::mutex::scoped_lock loch_a_phuill(single_threaded_class_mutex_);

  std::string serialised_data_atlas;
  int result(SerialiseAndSetIdentity("", "", "", &serialised_data_atlas));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failure setting details of new session: " << result;
    return result;
  }

  std::vector<int> individual_results(4, kPendingResult);
  boost::condition_variable condition_variable;
  boost::mutex mutex;
  OperationResults results(mutex, condition_variable, individual_results);

  ModifyMid(results);
  ModifySmid(results);
  StoreTmid(results);
  DeleteStmid(results);

  result = WaitForResults(mutex, condition_variable, individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to store new identity packets: Time out.";
    return kSaveSessionFailure;
  }

  DLOG(INFO) << "MID: " << individual_results.at(0)
             << ", SMID: " << individual_results.at(1)
             << ", TMID: " << individual_results.at(2)
             << ", STMID: " << individual_results.at(3);

  result = AssessJointResult(individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "One of the operations for Identity Packets failed. "
                << "Turn on INFO for feedback on which one. ";
    return kSaveSessionFailure;
  }

  session_->set_serialised_data_atlas(serialised_data_atlas);

  return kSuccess;
}

void UserCredentialsImpl::ModifyMid(OperationResults &results) {  // NOLINT (Dan)
  ModifyIdentity(results, passport::kMid, passport::kAnmid, 0);
}

void UserCredentialsImpl::ModifySmid(OperationResults &results) {  // NOLINT (Dan)
  ModifyIdentity(results, passport::kSmid, passport::kAnsmid, 1);
}

void UserCredentialsImpl::ModifyIdentity(OperationResults &results,  // NOLINT (Dan)
                                         int identity_type,
                                         int signer_type,
                                         int index) {
  passport::PacketType id_pt(static_cast<passport::PacketType>(identity_type));
  passport::PacketType sign_pt(static_cast<passport::PacketType>(signer_type));
  std::string name(passport_.PacketName(id_pt, false)),
              content(passport_.IdentityPacketValue(id_pt, false));
  name = pca::ApplyTypeToName(name, pca::kModifiableByOwner);
  std::shared_ptr<asymm::Keys> signer(passport_.SignaturePacketDetails(sign_pt, true));
  ValidationData validation_data;
  int result(KeysToValidationData(signer, &validation_data));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to put keys into validation data: " << result;
    OperationCallback(false, results, index);
    return;
  }

  asymm::Signature signature;
  result = asymm::Sign(content, signer->private_key, &signature);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to sign content: " << result;
    OperationCallback(false, results, index);
    return;
  }

  pca::SignedData signed_data;
  signed_data.set_data(content);
  signed_data.set_signature(signature);

  if (!remote_chunk_store_->Modify(name,
                                   signed_data.SerializeAsString(),
                                   std::bind(&OperationCallback, args::_1, results, index),
                                   validation_data)) {
    DLOG(ERROR) << "Failed to modify: " << index;
    OperationCallback(false, results, index);
  }
}

int UserCredentialsImpl::ChangeUsernamePin(const std::string &new_keyword,
                                           const std::string &new_pin) {
  boost::mutex::scoped_lock loch_a_phuill(single_threaded_class_mutex_);

  BOOST_ASSERT(!new_keyword.empty());
  BOOST_ASSERT(!new_pin.empty());

  std::string serialised_data_atlas;
  int result(SerialiseAndSetIdentity(new_keyword, new_pin, "", &serialised_data_atlas));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failure setting details of new session: " << result;
    return result;
  }

  result = StoreIdentityPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to store new identity packets: " << result;
    return result;
  }

  result = DeleteOldIdentityPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to delete old identity packets: " << result;
    return result;
  }

  result = passport_.ConfirmIdentityPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to set new identity packets: " << result;
    return kSetIdentityPacketsFailure;
  }

  session_->set_keyword(new_keyword);
  session_->set_pin(new_pin);
  session_->set_serialised_data_atlas(serialised_data_atlas);

  return kSuccess;
}

int UserCredentialsImpl::DeleteOldIdentityPackets() {
  std::vector<int> individual_results(4, kPendingResult);
  boost::condition_variable condition_variable;
  boost::mutex mutex;
  OperationResults results(mutex, condition_variable, individual_results);

  DeleteMid(results);
  DeleteSmid(results);
  DeleteTmid(results);
  DeleteStmid(results);

  int result(WaitForResults(mutex, condition_variable, individual_results));
  if (result != kSuccess) {
    DLOG(ERROR) << "Wait for results timed out.";
    return result;
  }
  DLOG(INFO) << "MID: " << individual_results.at(0)
             << ", SMID: " << individual_results.at(1)
             << ", TMID: " << individual_results.at(2)
             << ", STMID: " << individual_results.at(3);

  result = AssessJointResult(individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "One of the operations for Identity Packets failed. "
                << "Turn on INFO for feedback on which one. ";
    return kDeleteIdentityPacketsFailure;
  }

  return kSuccess;
}

void UserCredentialsImpl::DeleteMid(OperationResults &results) {  // NOLINT (Dan)
  DeleteIdentity(results, passport::kMid, passport::kAnmid, 0);
}

void UserCredentialsImpl::DeleteSmid(OperationResults &results) {  // NOLINT (Dan)
  DeleteIdentity(results, passport::kSmid, passport::kAnsmid, 1);
}

void UserCredentialsImpl::DeleteTmid(OperationResults &results) {  // NOLINT (Dan)
  DeleteIdentity(results, passport::kTmid, passport::kAntmid, 2);
}

void UserCredentialsImpl::DeleteStmid(OperationResults &results) {  // NOLINT (Dan)
  DeleteIdentity(results, passport::kStmid, passport::kAntmid, 3);
}

void UserCredentialsImpl::DeleteIdentity(OperationResults &results,  // NOLINT (Dan)
                                         int packet_type,
                                         int signer_type,
                                         int index) {
  passport::PacketType id_type(static_cast<passport::PacketType>(packet_type));
  passport::PacketType sig_type(static_cast<passport::PacketType>(signer_type));
  std::string name(passport_.PacketName(id_type, true));
  if (name.empty()) {
    DLOG(ERROR) << "Failed to get packet name: " << index;
    OperationCallback(false, results, index);
    return;
  }
  name = pca::ApplyTypeToName(name, pca::kModifiableByOwner);
  std::shared_ptr<asymm::Keys> signer(passport_.SignaturePacketDetails(sig_type, true));
  ValidationData validation_data;
  int result(KeysToValidationData(signer, &validation_data));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to put keys into validation data: " << result;
    OperationCallback(false, results, index);
    return;
  }

  if (!remote_chunk_store_->Delete(name,
                                   std::bind(&OperationCallback, args::_1, results, index),
                                   validation_data)) {
    DLOG(ERROR) << "Failed to delete: " << index;
    OperationCallback(false, results, index);
  }
}

int UserCredentialsImpl::ChangePassword(const std::string &new_password) {
  boost::mutex::scoped_lock loch_a_phuill(single_threaded_class_mutex_);

  std::string serialised_data_atlas;
  int result(SerialiseAndSetIdentity("", "", new_password, &serialised_data_atlas));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failure setting details of new session: " << result;
    return result;
  }

  result = DoChangePasswordAdditions();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to perform additions.";
    return result;
  }

  result = DoChangePasswordRemovals();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to perform removals.";
    return result;
  }

  result = passport_.ConfirmIdentityPackets();
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to set new identity packets: " << result;
    return kSetIdentityPacketsFailure;
  }

  session_->set_password(new_password);
  session_->set_serialised_data_atlas(serialised_data_atlas);

  return kSuccess;
}

int UserCredentialsImpl::DoChangePasswordAdditions() {
  std::vector<int> individual_results(4, kPendingResult);
  boost::condition_variable condition_variable;
  boost::mutex mutex;
  OperationResults new_results(mutex, condition_variable, individual_results);

  ModifyMid(new_results);
  ModifySmid(new_results);
  StoreTmid(new_results);
  StoreStmid(new_results);

  int result(WaitForResults(mutex, condition_variable, individual_results));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to store new identity packets: Time out.";
    return kChangePasswordFailure;
  }

  DLOG(INFO) << "MID: " << individual_results.at(0)
             << ", SMID: " << individual_results.at(1)
             << ", TMID: " << individual_results.at(2)
             << ", STMID: " << individual_results.at(3);

  result = AssessJointResult(individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "One of the operations for Identity Packets failed. "
                << "Turn on INFO for feedback on which one. ";
    return kChangePasswordFailure;
  }

  return kSuccess;
}

int UserCredentialsImpl::DoChangePasswordRemovals() {
  // Delete old TMID, STMID
  std::vector<int> individual_results(4, kSuccess);
  boost::condition_variable condition_variable;
  boost::mutex mutex;
  individual_results[2] = kPendingResult;
  individual_results[3] = kPendingResult;
  OperationResults del_results(mutex, condition_variable, individual_results);
  DeleteTmid(del_results);
  DeleteStmid(del_results);

  int result(WaitForResults(mutex, condition_variable, individual_results));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to store new identity packets: Time out.";
    return kChangePasswordFailure;
  }

  DLOG(INFO) << "TMID: " << individual_results.at(2)
             << ", STMID: " << individual_results.at(3);

  result = AssessJointResult(individual_results);
  if (result != kSuccess) {
    DLOG(ERROR) << "One of the operations for Identity Packets failed. "
                << "Turn on INFO for feedback on which one. ";
    return kChangePasswordFailure;
  }

  return kSuccess;
}

int UserCredentialsImpl::SerialiseAndSetIdentity(const std::string &keyword,
                                                 const std::string &pin,
                                                 const std::string &password,
                                                 std::string *serialised_data_atlas) {
  BOOST_ASSERT(serialised_data_atlas);
  int result(session_->SerialiseDataAtlas(serialised_data_atlas));
  if (result != kSuccess || serialised_data_atlas->empty()) {
    DLOG(ERROR) << "Failed to serialise session: " << result;
    return kSessionSerialisationFailure;
  }

  result = passport_.SetIdentityPackets(
               keyword.empty()? session_->keyword() : keyword,
               pin. empty() ? session_->pin() : pin,
               password.empty() ? session_->password() : password,
               *serialised_data_atlas,
               session_->serialised_data_atlas());

  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to set new identity packets: " << result;
    return kSetIdentityPacketsFailure;
  }

  return kSuccess;
}

}  // namespace lifestuff

}  // namespace maidsafe
