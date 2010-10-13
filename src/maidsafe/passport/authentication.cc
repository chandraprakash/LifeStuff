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

#include "maidsafe/passport/authentication.h"

//#include <boost/array.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/regex.hpp>
//#include <boost/thread/mutex.hpp>
//
//#include <vector>

//#include "maidsafe/passport/systempackets.h"
//#include "maidsafe/maidsafe.h"
//#include "protobuf/datamaps.pb.h"
//#include "protobuf/maidsafe_messages.pb.h"
//#include "protobuf/maidsafe_service.pb.h"

namespace maidsafe {

namespace passport {

void Authentication::Init(const boost::uint16_t &crypto_key_buffer_count) {
  crypto_key_pairs_.StartToCreateKeyPairs(crypto_key_buffer_count);
}

int Authentication::GetUserInfo(const std::string &username,
                                const std::string &pin) {
  if (username.empty() || pin.empty()) {
    user_info_result_ = kAuthenticationError;
    get_smidtimid_result_ = kAuthenticationError;
    return kAuthenticationError;
  }
  user_info_result_ = kPendingResult;
  get_smidtimid_result_ = kPendingResult;
  PacketParams params;
  params["username"] = username;
  params["pin"] = pin;
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  boost::shared_ptr<UserInfo> ui(new UserInfo());
  ui->func = boost::bind(&Authentication::GetUserInfoCallback, this, _1);
  ui->username = username;
  ui->pin = pin;

  sm_->LoadPacket(midPacket->PacketName(params),
      boost::bind(&Authentication::GetMidCallback, this, _1, _2, ui));
  sm_->LoadPacket(smidPacket->PacketName(params),
      boost::bind(&Authentication::GetSmidCallback, this, _1, _2, ui));

  while (user_info_result_ == kPendingResult)
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

  ss_->SetUsername(username);
  ss_->SetPin(pin);
  return user_info_result_;
}

void Authentication::GetMidCallback(const std::vector<std::string> &values,
                                    const ReturnCode &return_code,
                                    boost::shared_ptr<UserInfo> ui) {
  bool smid;
  {
    boost::mutex::scoped_lock loch(ui->m);
    smid = ui->smid_calledback;
  }

  if (return_code != kSuccess) {
    ss_->SetMidRid(0);
    {
      boost::mutex::scoped_lock loch(ui->m);
      ui->mid_calledback = true;
    }
    if (smid && ss_->SmidRid() == 0) {
#ifdef DEBUG
      printf("Authentication::GetMidCallback - No MID or SMID.\n");
#endif
      ui->func(kUserDoesntExist);
    }
    return;
  }

#ifdef DEBUG
  if (values.size() != 1)
    printf("Authentication::GetMidCallback - Values: %d\n", values.size());
#endif
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  PacketParams params;
  params["username"] = ui->username;
  params["pin"] = ui->pin;
  PacketParams info = midPacket->GetData(values[0], params);
  ss_->SetMidRid(boost::any_cast<boost::uint32_t>(info["data"]));
  {
    boost::mutex::scoped_lock loch(ui->m);
    ui->mid_calledback = true;
    smid = ui->smid_calledback;
  }
  if (ss_->MidRid() == 0) {
    if (smid && ss_->SmidRid() == 0) {
      ui->func(kUserDoesntExist);
    }
    return;
  } else {
    GetMidTmid(ui);
  }
}

void Authentication::GetSmidCallback(const std::vector<std::string> &values,
                                     const ReturnCode &return_code,
                                     boost::shared_ptr<UserInfo> ui) {
  bool mid;
  {
    boost::mutex::scoped_lock loch(ui->m);
    mid = ui->mid_calledback;
  }

  if (return_code != kSuccess) {
    ss_->SetSmidRid(0);
    {
      boost::mutex::scoped_lock loch(ui->m);
      ui->smid_calledback = true;
    }
    if (mid && ss_->MidRid() == 0) {
#ifdef DEBUG
      printf("Authentication::GetSMidCallback - No MID or SMID.\n");
#endif
      ui->func(kUserDoesntExist);
    }
    get_smidtimid_result_ = return_code;
    return;
  }

#ifdef DEBUG
  if (values.size() != 1)
    printf("Authentication::GetSMidCallback - Values: %d\n", values.size());
#endif
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  PacketParams params;
  params["username"] = ui->username;
  params["pin"] = ui->pin;
  PacketParams info = smidPacket->GetData(values[0], params);
  ss_->SetSmidRid(boost::any_cast<boost::uint32_t>(info["data"]));
  {
    boost::mutex::scoped_lock loch(ui->m);
    ui->smid_calledback = true;
    mid = ui->mid_calledback;
  }
  if (ss_->SmidRid() == 0) {
    if (mid && ss_->MidRid() == 0) {
      ui->func(kUserDoesntExist);
    }
    get_smidtimid_result_ = return_code;
    return;
  } else {
    GetSmidTmid(ui);
  }
}

void Authentication::GetMidTmid(boost::shared_ptr<UserInfo> ui) {
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
  PacketParams params;
  params["username"] = ui->username;
  params["pin"] = ui->pin;
  params["rid"] = ss_->MidRid();
  sm_->LoadPacket(tmidPacket->PacketName(params),
      boost::bind(&Authentication::GetMidTmidCallback, this, _1, _2, ui));
}

void Authentication::GetSmidTmid(boost::shared_ptr<UserInfo> ui) {
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
  PacketParams params;
  params["username"] = ui->username;
  params["pin"] = ui->pin;
  params["rid"] = ss_->SmidRid();
  sm_->LoadPacket(tmidPacket->PacketName(params),
      boost::bind(&Authentication::GetSmidTmidCallback, this, _1, _2, ui));
}

void Authentication::GetMidTmidCallback(const std::vector<std::string> &values,
                                        const ReturnCode &return_code,
                                        boost::shared_ptr<UserInfo> ui) {
  bool smid;
  {
    boost::mutex::scoped_lock loch(ui->m);
    ui->tmid_mid_calledback = true;
    smid = ui->tmid_smid_calledback;
  }

  if (return_code != kSuccess) {
    ss_->SetTmidContent("");
    if (smid && ss_->SmidTmidContent() == "") {
#ifdef DEBUG
      printf("Authentication::GetMidTmidCallback - No TMIDS found.\n");
#endif
      ui->func(kAuthenticationError);
    }
    return;
  }

#ifdef DEBUG
  if (values.size() != 1)
    printf("Authentication::GetMidTmidCallback - Values: %d\n", values.size());
#endif
  ss_->SetTmidContent(values[0]);
  ui->func(kUserExists);
}

void Authentication::GetSmidTmidCallback(const std::vector<std::string> &values,
                                         const ReturnCode &return_code,
                                         boost::shared_ptr<UserInfo> ui) {
  bool mid;
  {
    boost::mutex::scoped_lock loch(ui->m);
    ui->tmid_smid_calledback = true;
    mid = ui->tmid_mid_calledback;
  }

  if (return_code != kSuccess) {
    ss_->SetTmidContent("");
    if (mid && ss_->TmidContent() == "") {
#ifdef DEBUG
      printf("Authentication::GetSmidTmidCallback - No TMIDS found.\n");
#endif
      ui->func(kAuthenticationError);
    }
    get_smidtimid_result_ = return_code;
    return;
  }

#ifdef DEBUG
  if (values.size() != 1)
    printf("Authentication::GetSmidTmidCallback - Values: %d\n", values.size());
#endif
  ss_->SetSmidTmidContent(values[0]);
  get_smidtimid_result_ = return_code;
}

int Authentication::GetUserData(const std::string &password,
                                std::string *ser_da) {
  //  still have not recovered the tmid
  crypto::RsaKeyPair kp;
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
  PacketParams params;
  params["password"] = password;
  params["rid"] = ss_->MidRid();
  PacketParams rec_data = tmidPacket->GetData(ss_->TmidContent(), params);
  *ser_da = boost::any_cast<std::string>(rec_data["data"]);

  DataMap dm;
  if (!dm.ParseFromString(*ser_da)) {
#ifdef DEBUG
    printf("Authentication::GetUserData - Ser DM doesn't parse (%s).\n",
           HexSubstr(ss_->TmidContent()).c_str());
#endif
    while (get_smidtimid_result_ == kPendingResult)
      boost::this_thread::sleep(boost::posix_time::milliseconds(50));

    if (get_smidtimid_result_ == kSuccess) {
      rec_data = tmidPacket->GetData(ss_->SmidTmidContent(), params);
      *ser_da = boost::any_cast<std::string>(rec_data["data"]);
      if (dm.ParseFromString(*ser_da)) {
        ss_->SetPassword(password);
        return kSuccess;
      }
    }
    return kPasswordFailure;
  }
#ifdef DEBUG
  printf("Authentication::GetUserData - Using MID TMID\n");
#endif
  ss_->SetPassword(password);
  return kSuccess;
}

int Authentication::CreateUserSysPackets(const std::string &username,
                                         const std::string &pin) {
  system_packets_result_ = kPendingResult;
  PacketParams params;
  params["username"] = username;
  params["pin"] = pin;
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));

  boost::uint16_t count(0);
  bool calledback(false);
  VoidFuncOneInt func = boost::bind(
      &Authentication::CreateSystemPacketsCallback, this, _1);
  sm_->KeyUnique(midPacket->PacketName(params), false, boost::bind(
      &Authentication::CreateUserSysPackets, this, _1, username, pin, func,
      &count, &calledback));
  sm_->KeyUnique(smidPacket->PacketName(params), false, boost::bind(
      &Authentication::CreateUserSysPackets, this, _1, username, pin, func,
      &count, &calledback));

  while (system_packets_result_ == kPendingResult) {
#ifdef DEBUG
    printf(".");
#endif
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
  }
#ifdef DEBUG
  printf("\n");
#endif

  return system_packets_result_;
}

void Authentication::CreateUserSysPackets(const ReturnCode &return_code,
                                          const std::string &username,
                                          const std::string &pin,
                                          VoidFuncOneInt vfoi,
                                          boost::uint16_t *count,
                                          bool *calledback) {
  if (*calledback)
    return;
  if (return_code == kKeyUnique) {
    if (*count == 0) {
      ++*count;
      return;
    }
  } else {
    *calledback = true;
    vfoi(kUserExists);
    return;
  }
  boost::array<PacketType, 3> anonymous = { {ANMAID, ANMID, ANTMID} };
  boost::shared_ptr<SystemPacketCreation> data(new SystemPacketCreation());
  data->username = username;
  data->pin = pin;
  data->vfoi = vfoi;
  for (size_t n = 0; n < anonymous.size(); ++n) {
    CreateSignaturePacket(data, anonymous[n]);
  }
}

void Authentication::CreateSignaturePacket(
    boost::shared_ptr<SystemPacketCreation> spc, const PacketType &type_da) {
  crypto::RsaKeyPair kp;
  while (!crypto_key_pairs_.GetKeyPair(&kp)) {
    kp.ClearKeys();
    crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
  }
  PacketParams params;
  params["publicKey"] = kp.public_key();
  params["privateKey"] = kp.private_key();
  boost::shared_ptr<Packet> sigPacket(PacketFactory::Factory(type_da));
  PacketParams result = sigPacket->Create(params);
  boost::shared_ptr<FindSystemPacket> find_system_packet_data(new FindSystemPacket());
  find_system_packet_data->spc = spc;
  find_system_packet_data->pp = result;
  find_system_packet_data->pt = type_da;

  VoidFuncOneInt func = boost::bind(
      &Authentication::CreateSignaturePacketKeyUnique, this, _1, find_system_packet_data);
  sm_->KeyUnique(boost::any_cast<std::string>(result["name"]), false, func);
}

void Authentication::CreateSignaturePacketKeyUnique(
    const ReturnCode &return_code, boost::shared_ptr<FindSystemPacket> find_system_packet_data) {
  if (return_code == kKeyUnique) {
    int n = ss_->AddKey(find_system_packet_data->pt,
                        boost::any_cast<std::string>(find_system_packet_data->pp["name"]),
                        boost::any_cast<std::string>(find_system_packet_data->pp["privateKey"]),
                        boost::any_cast<std::string>(find_system_packet_data->pp["publicKey"]),
                        "");
    if (n != 0) {
      // return to CreateSignaturePacket
    }

    VoidFuncOneInt func = boost::bind(
        &Authentication::CreateSignaturePacketStore, this, _1, find_system_packet_data);
    sm_->StorePacket(boost::any_cast<std::string>(find_system_packet_data->pp["name"]),
                     boost::any_cast<std::string>(find_system_packet_data->pp["publicKey"]),
                     find_system_packet_data->pt, PRIVATE, "", func);
  } else {
    // return to CreateSignaturePacket
  }
}

void Authentication::CreateSignaturePacketStore(
    const ReturnCode &return_code, boost::shared_ptr<FindSystemPacket> find_system_packet_data) {
  if (return_code == kSuccess) {
    ++find_system_packet_data->spc->packet_count;
    switch (find_system_packet_data->pt) {
      case ANMAID:
        find_system_packet_data->pt = MAID;
        CreateMaidPmidPacket(find_system_packet_data);
        break;
      case ANMID:
        CreateMidPacket(find_system_packet_data);
        break;
      case MID:
        CreateSignaturePacket(find_system_packet_data->spc, ANSMID);
        break;
      case ANSMID:
        CreateSmidPacket(find_system_packet_data);
        break;
      case MAID:
        find_system_packet_data->pt = PMID;
        CreateMaidPmidPacket(find_system_packet_data);
        break;
      default:
        if (find_system_packet_data->spc->packet_count == kNoOfSystemPackets)
          find_system_packet_data->spc->vfoi(kSuccess);
    }
  } else {
    ss_->RemoveKey(find_system_packet_data->pt);
  }
}

void Authentication::CreateMidPacket(boost::shared_ptr<FindSystemPacket> find_system_packet_data) {
  find_system_packet_data->pt = MID;
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  PacketParams user_params;
  user_params["username"] = find_system_packet_data->spc->username;
  user_params["pin"] = find_system_packet_data->spc->pin;

  PacketParams mid_result = midPacket->Create(user_params);
  ss_->SetMidRid(boost::any_cast<boost::uint32_t>(mid_result["rid"]));

  sm_->StorePacket(boost::any_cast<std::string>(mid_result["name"]),
                   boost::any_cast<std::string>(mid_result["encRid"]),
                   MID, PRIVATE, "",
                   boost::bind(&Authentication::CreateSignaturePacketStore,
                               this, _1, find_system_packet_data));
}

void Authentication::CreateSmidPacket(boost::shared_ptr<FindSystemPacket> find_system_packet_data) {
  find_system_packet_data->pt = SMID;
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  PacketParams user_params;
  user_params["username"] = find_system_packet_data->spc->username;
  user_params["pin"] = find_system_packet_data->spc->pin;
  user_params["rid"] = ss_->MidRid();

  PacketParams smid_result = smidPacket->Create(user_params);
  ss_->SetSmidRid(ss_->MidRid());
  sm_->StorePacket(boost::any_cast<std::string>(smid_result["name"]),
                   boost::any_cast<std::string>(smid_result["encRid"]),
                   SMID, PRIVATE, "",
                   boost::bind(&Authentication::CreateSignaturePacketStore,
                               this, _1, find_system_packet_data));
}

void Authentication::CreateMaidPmidPacket(
    boost::shared_ptr<FindSystemPacket> find_system_packet_data) {
  crypto::RsaKeyPair kp;
  while (!crypto_key_pairs_.GetKeyPair(&kp)) {
    kp.ClearKeys();
    crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
  }
  boost::shared_ptr<Packet> packet(PacketFactory::Factory(PMID));
  PacketParams user_params;
  if (find_system_packet_data->pt == PMID) {
    user_params["signerPrivateKey"] = ss_->PrivateKey(MAID);
  } else {
    user_params["signerPrivateKey"] = ss_->PrivateKey(ANMAID);
  }
  user_params["publicKey"] = kp.public_key();
  user_params["privateKey"] = kp.private_key();

  PacketParams result = packet->Create(user_params);
  int n = ss_->AddKey(find_system_packet_data->pt,
                      boost::any_cast<std::string>(result["name"]),
                      boost::any_cast<std::string>(result["privateKey"]),
                      boost::any_cast<std::string>(result["publicKey"]),
                      boost::any_cast<std::string>(result["signature"]));
  if (n != 0) {
    // return to CreateSignaturePacket
  }
  sm_->StorePacket(boost::any_cast<std::string>(result["name"]),
                   boost::any_cast<std::string>(result["publicKey"]),
                   find_system_packet_data->pt, PRIVATE, "",
                   boost::bind(&Authentication::CreateSignaturePacketStore,
                               this, _1, find_system_packet_data));
}

int Authentication::CreateTmidPacket(const std::string &username,
                                     const std::string &pin,
                                     const std::string &password,
                                     const std::string &ser_dm) {
  PacketParams user_params;
  user_params["username"] = username;
  user_params["pin"] = pin;
  user_params["password"] = password;
  user_params["rid"] = ss_->MidRid();

  crypto::RsaKeyPair kp;
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));

  // STORING SERLIALISED DATA MAP OF DATA ATLAS
  user_params["data"] = ser_dm;
  PacketParams tmid_result = tmidPacket->Create(user_params);
  std::string enc_tmid(boost::any_cast<std::string>(tmid_result["data"]));
  std::string name_tmid(boost::any_cast<std::string>(tmid_result["name"]));
  if (StorePacket(name_tmid, enc_tmid, TMID, "") != kSuccess) {
    ss_->SetMidRid(0);
    ss_->SetSmidRid(0);
    return kAuthenticationError;
  }

  GenericPacket gp;
  gp.set_data(enc_tmid);
  gp.set_signature(crypto_.AsymSign(gp.data(), "", ss_->PrivateKey(ANTMID),
      crypto::STRING_STRING));
  ss_->SetUsername(username);
  ss_->SetPin(pin);
  ss_->SetPassword(password);
  ss_->SetTmidContent(gp.SerializeAsString());
  ss_->SetSmidTmidContent(gp.SerializeAsString());

  return kSuccess;
}

void Authentication::SaveSession(const std::string &ser_da,
                                 const VoidFuncOneInt &cb) {
  PacketParams params, result;
  params["username"] = ss_->Username();
  params["pin"] = ss_->Pin();

  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  boost::shared_ptr<SaveSessionData> ssd(new SaveSessionData);
  ssd->ser_da = ser_da;
  ssd->vfoi = cb;
  ssd->same_mid_smid = false;

  // Need to update SMID with MID content
  params["rid"] = ss_->MidRid();
  result = smidPacket->Create(params);
  std::string new_value(boost::any_cast<std::string>(result["encRid"]));
  ssd->current_encripted_mid = new_value;
  if (ss_->MidRid() != ss_->SmidRid()) {
    params["rid"] = ss_->SmidRid();
    result = smidPacket->Create(params);
    std::string old_value(boost::any_cast<std::string>(result["encRid"]));
    std::string packet_name(boost::any_cast<std::string>(result["name"]));
    sm_->UpdatePacket(packet_name, old_value, new_value, SMID, PRIVATE, "",
                      boost::bind(&Authentication::UpdateSmidCallback,
                                  this, _1, ssd));
  } else {
    ssd->same_mid_smid = true;
    UpdateSmidCallback(kSuccess, ssd);
  }
}

void Authentication::UpdateSmidCallback(
    const ReturnCode &return_code, boost::shared_ptr<SaveSessionData> ssd) {
  if (return_code != kSuccess) {
    ssd->vfoi(return_code);
#ifdef DEBUG
    printf("Auth::UpdateSmidCallback - Smid kad update failed(%d)\n", return_code);
#endif
    return;
  }

  // Need to delete the TMID related to the SMID from the network
  if (!ssd->same_mid_smid) {
    PacketParams params;
    params["username"] = ss_->Username();
    params["pin"] = ss_->Pin();
    params["rid"] = ss_->SmidRid();
    boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
    std::string tmidname(tmidPacket->PacketName(params));
    GenericPacket gp;
    if (!gp.ParseFromString(ss_->SmidTmidContent())) {
#ifdef DEBUG
      printf("Auth::UpdateSmidCallback - SmidTmidContent failed to parse.\n");
#endif
    }
    std::vector<std::string> values(1, gp.data());
    sm_->DeletePacket(tmidname, values, TMID, PRIVATE, "",
                      boost::bind(&Authentication::DeleteSmidTmidCallback,
                                  this, _1, ssd));
  } else {
    DeleteSmidTmidCallback(kSuccess, ssd);
  }
}

void Authentication::DeleteSmidTmidCallback(
    const ReturnCode &return_code, boost::shared_ptr<SaveSessionData> ssd) {
  if (return_code != kSuccess) {
    ssd->vfoi(return_code);
#ifdef DEBUG
    printf("Auth::DeleteSmidTmidCallback - SmidTmid delete failed(%d)\n", return_code);
#endif
    return;
  }
  ss_->SetSmidRid(ss_->MidRid());
  ss_->SetSmidTmidContent(ss_->TmidContent());

  PacketParams params;
  params["username"] = ss_->Username();
  params["pin"] = ss_->Pin();
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  PacketParams mid_result = midPacket->Create(params);
  while (ss_->MidRid() == boost::any_cast<boost::uint32_t>(mid_result["rid"]))
    mid_result = midPacket->Create(params);

  ssd->new_mid = boost::any_cast<boost::uint32_t>(mid_result["rid"]);
  std::string new_value(boost::any_cast<std::string>(mid_result["encRid"]));
  std::string packet_name(boost::any_cast<std::string>(mid_result["name"]));
  sm_->UpdatePacket(packet_name, ssd->current_encripted_mid, new_value, MID,
                    PRIVATE, "", boost::bind(&Authentication::UpdateMidCallback,
                                             this, _1, ssd));
}

void Authentication::UpdateMidCallback(
    const ReturnCode &return_code, boost::shared_ptr<SaveSessionData> ssd) {
  if (return_code != kSuccess) {
    ssd->vfoi(return_code);
#ifdef DEBUG
    printf("Auth::UpdateMidCallback - Mid kad update failed(%d)\n", return_code);
#endif
    return;
  }

  PacketParams params;
  params["username"] = ss_->Username();
  params["pin"] = ss_->Pin();
  params["rid"] = ssd->new_mid;
  params["password"] = ss_->Password();
  params["data"] = ssd->ser_da;
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
  PacketParams tmidresult = tmidPacket->Create(params);

  ss_->SetMidRid(ssd->new_mid);
  ssd->mid_tmid_data = boost::any_cast<std::string>(tmidresult["data"]);

  sm_->StorePacket(boost::any_cast<std::string>(tmidresult["name"]),
                   boost::any_cast<std::string>(tmidresult["data"]),
                   TMID, PRIVATE, "",
                   boost::bind(&Authentication::StoreMidTmidCallback,
                               this, _1, ssd));
}

void Authentication::StoreMidTmidCallback(
    const ReturnCode &return_code, boost::shared_ptr<SaveSessionData> ssd) {
  if (return_code != kSuccess) {
    ssd->vfoi(return_code);
#ifdef DEBUG
    printf("Auth::StoreMidTmidCallback - Mid Tmid store failed(%d)\n", return_code);
#endif
    return;
  }

  GenericPacket packet;
  packet.set_data(ssd->mid_tmid_data);
  packet.set_signature(crypto_.AsymSign(packet.data(), "",
                       ss_->PrivateKey(ANTMID), crypto::STRING_STRING));
  ss_->SetTmidContent(packet.SerializeAsString());

  ssd->vfoi(return_code);
}

int Authentication::SaveSession(const std::string &ser_da) {
  boost::mutex mutex;
  boost::condition_variable cond_var;
  int result(kPendingResult);
  SaveSession(ser_da, boost::bind(&Authentication::PacketOpCallback, this, _1,
                                  &mutex, &cond_var, &result));
  {
    boost::mutex::scoped_lock lock(mutex);
    while (result == kPendingResult)
      cond_var.wait(lock);
  }
  return result;
}

int Authentication::RemoveMe(std::list<KeyAtlasRow> sig_keys) {
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));

  PacketParams params;
  params["username"] = ss_->Username();
  params["pin"] = ss_->Pin();

  params["rid"] = ss_->MidRid();
  std::string mpid_name, pmid_name;

  while (!sig_keys.empty()) {
    KeyAtlasRow kt = sig_keys.front();
    sig_keys.pop_front();
    switch (kt.type_) {
      case ANMID:
          DeletePacket(midPacket->PacketName(params), "",
                       static_cast<PacketType>(kt.type_));
          break;
      case ANSMID:
          DeletePacket(smidPacket->PacketName(params), "",
                       static_cast<PacketType>(kt.type_));
          break;
      case ANTMID:
          DeletePacket(tmidPacket->PacketName(params), "",
                       static_cast<PacketType>(kt.type_));
          params["rid"] = ss_->SmidRid();
          if (ss_->SmidRid() != ss_->MidRid())
            DeletePacket(tmidPacket->PacketName(params), "",
                         static_cast<PacketType>(kt.type_));
          break;
      case ANMPID:
          DeletePacket(mpid_name, "", static_cast<PacketType>(kt.type_));
          break;
      case MAID:
          if (!pmid_name.empty())
            DeletePacket(pmid_name, "", static_cast<PacketType>(kt.type_));
          break;
      case MPID: mpid_name = kt.id_; break;
      case PMID: pmid_name = kt.id_; break;
    }
    DeletePacket(kt.id_, "", static_cast<PacketType>(kt.type_));
  }
  return kSuccess;
}

int Authentication::CreatePublicName(const std::string &public_username) {
  PacketParams params;
  PacketParams local_result;
  params["publicname"] = public_username;
  boost::shared_ptr<Packet> mpidPacket(PacketFactory::Factory(MPID));
  std::string mpidname = mpidPacket->PacketName(params);

  if (!sm_->KeyUnique(mpidname, false)) {
#ifdef DEBUG
    printf("Authentication::CreatePublicName - Exists.\n");
#endif
    return kPublicUsernameExists;
  }
  crypto::RsaKeyPair kp;
  while (!crypto_key_pairs_.GetKeyPair(&kp)) {
    kp.ClearKeys();
    crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
  }
  params["publicKey"] = kp.public_key();
  params["privateKey"]= kp.private_key();

  kp.ClearKeys();
  while (!crypto_key_pairs_.GetKeyPair(&kp)) {
    kp.ClearKeys();
    crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
  }
  boost::shared_ptr<Packet> sigPacket(PacketFactory::Factory(ANMPID));
  PacketParams spacket_params;
  spacket_params["publicKey"] = kp.public_key();
  spacket_params["privateKey"] = kp.private_key();
  PacketParams result = sigPacket->Create(spacket_params);
  while (!sm_->KeyUnique(boost::any_cast<std::string>(result["name"]),
         false)) {
    kp.ClearKeys();
    while (!crypto_key_pairs_.GetKeyPair(&kp)) {
      kp.ClearKeys();
      crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
    }
    spacket_params["publicKey"] = kp.public_key();
    spacket_params["privateKey"] = kp.private_key();
    result = sigPacket->Create(spacket_params);
  }

  ss_->AddKey(ANMPID, boost::any_cast<std::string>(result["name"]),
              boost::any_cast<std::string>(result["privateKey"]),
              boost::any_cast<std::string>(result["publicKey"]),
              "");
  if (StorePacket(boost::any_cast<std::string>(result["name"]),
                  boost::any_cast<std::string>(result["publicKey"]), ANMPID, "")
      != kSuccess) {
#ifdef DEBUG
    printf("Authentication::CreatePublicName - Buggered in ANMPID\n");
#endif
    ss_->RemoveKey(ANMPID);
    return kAuthenticationError;
  }

  PacketParams mpid_result = mpidPacket->Create(params);
  std::string data = boost::any_cast<std::string>(mpid_result["publicKey"]);
  std::string pubkey_signature = crypto_.AsymSign(data, "",
                                 ss_->PrivateKey(ANMPID),
                                 crypto::STRING_STRING);

  if (StorePacket(boost::any_cast<std::string>(mpid_result["name"]), data, MPID,
                  "") != kSuccess) {
#ifdef DEBUG
    printf("Authentication::CreatePublicName - Buggered in MPID\n");
#endif
    ss_->RemoveKey(ANMPID);
    return kAuthenticationError;
  }

  ss_->AddKey(MPID,
              public_username,
              boost::any_cast<std::string>(mpid_result["privateKey"]),
              boost::any_cast<std::string>(mpid_result["publicKey"]),
              pubkey_signature);

  return kSuccess;
}

int Authentication::ChangeUsername(const std::string &ser_da,
                                   const std::string &new_username) {
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
  PacketParams user_params;
  user_params["username"] = ss_->Username();
  user_params["pin"] = ss_->Pin();

  // Backing up current session details
  boost::uint32_t old_mid(ss_->MidRid());
  boost::uint32_t old_smid(ss_->SmidRid());
  std::string old_mid_name(midPacket->PacketName(user_params));
  std::string old_smid_name(smidPacket->PacketName(user_params));
  std::string old_mid_tmid(ss_->TmidContent());
  std::string old_smid_tmid(ss_->SmidTmidContent());
  user_params["rid"] = old_mid;
  std::string old_mid_tmid_name(tmidPacket->PacketName(user_params));
  user_params["rid"] = old_smid;
  std::string old_smid_tmid_name(tmidPacket->PacketName(user_params));
  std::string old_username(ss_->Username());

  // Verifying uniqueness of new MID
  user_params["username"] = new_username;
  user_params["pin"] = ss_->Pin();
  std::string mid_name = midPacket->PacketName(user_params);
  if (!sm_->KeyUnique(mid_name, false))
    return kUserExists;

  // Verifying uniqueness of new SMID
  std::string smid_name = smidPacket->PacketName(user_params);
  if (!sm_->KeyUnique(smid_name, false))
    return kUserExists;

  //  Creating and storing new MID packet with new username
  PacketParams mid_result = midPacket->Create(user_params);
  while (ss_->MidRid() == boost::any_cast<boost::uint32_t>(mid_result["rid"]))
    mid_result = midPacket->Create(user_params);

  if (StorePacket(boost::any_cast<std::string>(mid_result["name"]),
                  boost::any_cast<std::string>(mid_result["encRid"]), MID, "")
      != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername: Can't store new MID\n");
#endif
    return kAuthenticationError;
  }

  //  Creating and storing new SMID packet with new username
  boost::uint32_t new_smid_rid(0);
  while (new_smid_rid == 0 || new_smid_rid == old_mid ||
         new_smid_rid == old_smid) {
    new_smid_rid = base::RandomUint32();
  }
  user_params["rid"] = new_smid_rid;
  PacketParams smid_result = smidPacket->Create(user_params);
  if (StorePacket(boost::any_cast<std::string>(smid_result["name"]),
                  boost::any_cast<std::string>(smid_result["encRid"]), SMID, "")
      != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername: Can't store new SMID\n");
#endif
    return kAuthenticationError;
  }

  //  Creating new MID TMID
  user_params["password"] = ss_->Password();
  user_params["rid"] = boost::any_cast<boost::uint32_t>(mid_result["rid"]);
  user_params["data"] = ser_da;
  PacketParams tmid_result = tmidPacket->Create(user_params);
  std::string new_mid_tmid(boost::any_cast<std::string>(tmid_result["data"]));
  if (StorePacket(boost::any_cast<std::string>(tmid_result["name"]),
                  boost::any_cast<std::string>(tmid_result["data"]), TMID, "")
      != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername: Can't store new TMID\n");
#endif
    return kAuthenticationError;
  }

  //  Creating new SMID TMID
  PacketParams old_user_params;
  old_user_params["username"] = ss_->Username();
  old_user_params["pin"] = ss_->Pin();
  old_user_params["rid"] = ss_->MidRid();
  old_user_params["password"] = ss_->Password();
  PacketParams rec_tmid = tmidPacket->GetData(old_mid_tmid, old_user_params);
  std::string tmid_data = boost::any_cast<std::string>(rec_tmid["data"]);
  if (tmid_data.empty())
    return kAuthenticationError;
  old_user_params["data"] = tmid_data;
  old_user_params["username"] = new_username;
  old_user_params["rid"] = new_smid_rid;
  tmid_result = tmidPacket->Create(old_user_params);
  std::string new_smid_tmid(boost::any_cast<std::string>(tmid_result["data"]));
  if (StorePacket(boost::any_cast<std::string>(tmid_result["name"]),
      new_smid_tmid, TMID, "") != kSuccess) {
    return kAuthenticationError;
  }

  // Deleting old MID
  int result = DeletePacket(old_mid_name, EncryptedDataMidSmid(old_mid), MID);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername - Failed to delete MID.\n");
#endif
    return kAuthenticationError;
  }

  // Deleting old SMID
  result = DeletePacket(old_smid_name, EncryptedDataMidSmid(old_smid), SMID);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername - Failed to delete SMID.\n");
#endif
    return kAuthenticationError;
  }

  // Deleting old MID TMID
  GenericPacket gp;
  std::string tmidcontent;
  if (gp.ParseFromString(ss_->TmidContent()))
    tmidcontent = gp.data();
  result = DeletePacket(old_mid_tmid_name, tmidcontent, TMID);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername - Failed to delete midTMID {%s}.\n",
           ss_->TmidContent().c_str());
#endif
    return kAuthenticationError;
  }

  // Deleting old SMID TMID
  if (ss_->MidRid() != ss_->SmidRid()) {
    gp.Clear();
    if (gp.ParseFromString(ss_->SmidTmidContent()))
      tmidcontent = gp.data();
    result = DeletePacket(old_smid_tmid_name, tmidcontent, TMID);
    if (result != kSuccess) {
#ifdef DEBUG
      printf("Authentication::ChangeUsername - Failed to delete smidTMID.\n");
#endif
      return kAuthenticationError;
    }
  }

  ss_->SetUsername(new_username);
  ss_->SetSmidRid(new_smid_rid);
  ss_->SetMidRid(boost::any_cast<boost::uint32_t>(mid_result["rid"]));
  gp.Clear();
  gp.set_data(new_mid_tmid);
  gp.set_signature(crypto_.AsymSign(gp.data(), "", ss_->PrivateKey(ANTMID),
    crypto::STRING_STRING));
  ss_->SetTmidContent(gp.SerializeAsString());
  gp.Clear();
  gp.set_data(new_smid_tmid);
  gp.set_signature(crypto_.AsymSign(gp.data(), "", ss_->PrivateKey(ANTMID),
    crypto::STRING_STRING));
  ss_->SetSmidTmidContent(gp.SerializeAsString());

  return kSuccess;
}

int Authentication::ChangePin(const std::string &ser_da,
                              const std::string &new_pin) {
  boost::shared_ptr<Packet> midPacket(PacketFactory::Factory(MID));
  boost::shared_ptr<Packet> smidPacket(PacketFactory::Factory(SMID));
  boost::shared_ptr<Packet> tmidPacket(PacketFactory::Factory(TMID));
  PacketParams user_params;
  user_params["username"] = ss_->Username();
  user_params["pin"] = ss_->Pin();

  // Backing up current session details
  boost::uint32_t old_mid(ss_->MidRid());
  boost::uint32_t old_smid(ss_->SmidRid());
  std::string old_mid_name(midPacket->PacketName(user_params));
  std::string old_smid_name(smidPacket->PacketName(user_params));
  std::string old_mid_tmid(ss_->TmidContent());
  std::string old_smid_tmid(ss_->SmidTmidContent());
  user_params["rid"] = old_mid;
  std::string old_mid_tmid_name(tmidPacket->PacketName(user_params));
  user_params["rid"] = old_smid;
  std::string old_smid_tmid_name(tmidPacket->PacketName(user_params));
  std::string old_pin(ss_->Pin());

  // Verifying uniqueness of new MID
  user_params["username"] = ss_->Username();
  user_params["pin"] = new_pin;
  std::string mid_name = midPacket->PacketName(user_params);
  if (!sm_->KeyUnique(mid_name, false))
    return kUserExists;

  // Verifying uniqueness of new SMID
  std::string smid_name = smidPacket->PacketName(user_params);
  if (!sm_->KeyUnique(smid_name, false))
    return kUserExists;

  //  Creating and storing new MID packet with new username
  PacketParams mid_result = midPacket->Create(user_params);
  while (ss_->MidRid() == boost::any_cast<boost::uint32_t>(mid_result["rid"]))
    mid_result = midPacket->Create(user_params);

  if (StorePacket(boost::any_cast<std::string>(mid_result["name"]),
                  boost::any_cast<std::string>(mid_result["encRid"]), MID, "")
      != kSuccess) {
    return kAuthenticationError;
  }

  //  Creating and storing new SMID packet with new username
  boost::uint32_t new_smid_rid(0);
  while (new_smid_rid == 0 || new_smid_rid == old_mid ||
         new_smid_rid == old_smid) {
    new_smid_rid = base::RandomUint32();
  }
  user_params["rid"] = new_smid_rid;
  PacketParams smid_result = smidPacket->Create(user_params);
  if (StorePacket(boost::any_cast<std::string>(smid_result["name"]),
                  boost::any_cast<std::string>(smid_result["encRid"]), SMID, "")
      != kSuccess) {
    return kAuthenticationError;
  }

  //  Creating new MID TMID
  user_params["password"] = ss_->Password();
  user_params["rid"] = boost::any_cast<boost::uint32_t>(mid_result["rid"]);
  user_params["data"] = ser_da;
  PacketParams tmid_result = tmidPacket->Create(user_params);
  std::string new_mid_tmid(boost::any_cast<std::string>(tmid_result["data"]));
  if (StorePacket(boost::any_cast<std::string>(tmid_result["name"]),
                  boost::any_cast<std::string>(tmid_result["data"]), TMID, "")
      != kSuccess) {
    return kAuthenticationError;
  }

  //  Creating new SMID TMID
  PacketParams old_user_params;
  old_user_params["username"] = ss_->Username();
  old_user_params["pin"] = ss_->Pin();
  old_user_params["rid"] = ss_->MidRid();
  old_user_params["password"] = ss_->Password();
  PacketParams rec_tmid = tmidPacket->GetData(old_mid_tmid, old_user_params);
  std::string tmid_data = boost::any_cast<std::string>(rec_tmid["data"]);
  if (tmid_data.empty())
    return kAuthenticationError;
  old_user_params["data"] = tmid_data;
  old_user_params["pin"] = new_pin;
  old_user_params["rid"] = new_smid_rid;
  tmid_result = tmidPacket->Create(old_user_params);
  std::string new_smid_tmid(boost::any_cast<std::string>(tmid_result["data"]));
  if (StorePacket(boost::any_cast<std::string>(tmid_result["name"]),
      new_smid_tmid, TMID, "") != kSuccess) {
    return kAuthenticationError;
  }

  // Deleting old MID
  int result = DeletePacket(old_mid_name, EncryptedDataMidSmid(old_mid), MID);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername - Failed to delete MID.\n");
#endif
    return kAuthenticationError;
  }

  // Deleting old SMID
  result = DeletePacket(old_smid_name, EncryptedDataMidSmid(old_smid), SMID);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername - Failed to delete SMID.\n");
#endif
    return kAuthenticationError;
  }

  // Deleting old MID TMID
  GenericPacket gp;
  std::string tmidcontent;
  if (gp.ParseFromString(ss_->TmidContent()))
    tmidcontent = gp.data();
  result = DeletePacket(old_mid_tmid_name, tmidcontent, TMID);
  if (result != kSuccess) {
#ifdef DEBUG
    printf("Authentication::ChangeUsername - Failed to delete midTMID {%s}.\n",
           ss_->TmidContent().c_str());
#endif
    return kAuthenticationError;
  }

  // Deleting old SMID TMID
  if (ss_->MidRid() != ss_->SmidRid()) {
    gp.Clear();
    if (gp.ParseFromString(ss_->SmidTmidContent()))
      tmidcontent = gp.data();
    result = DeletePacket(old_smid_tmid_name, tmidcontent, TMID);
    if (result != kSuccess) {
#ifdef DEBUG
      printf("Authentication::ChangeUsername - Failed to delete smidTMID.\n");
#endif
      return kAuthenticationError;
    }
  }

  ss_->SetPin(new_pin);
  ss_->SetSmidRid(new_smid_rid);
  ss_->SetMidRid(boost::any_cast<boost::uint32_t>(mid_result["rid"]));
  gp.Clear();
  gp.set_data(new_mid_tmid);
  gp.set_signature(crypto_.AsymSign(gp.data(), "", ss_->PrivateKey(ANTMID),
    crypto::STRING_STRING));
  ss_->SetTmidContent(gp.SerializeAsString());
  gp.Clear();
  gp.set_data(new_smid_tmid);
  gp.set_signature(crypto_.AsymSign(gp.data(), "", ss_->PrivateKey(ANTMID),
    crypto::STRING_STRING));
  ss_->SetSmidTmidContent(gp.SerializeAsString());

  return kSuccess;
}

int Authentication::ChangePassword(const std::string &ser_da,
                                   const std::string &new_password) {
  std::string old_password = ss_->Password();
  ss_->SetPassword(new_password);
  if (SaveSession(ser_da) == kSuccess) {
    return kSuccess;
  } else {
    ss_->SetPassword(old_password);
    return kAuthenticationError;
  }
}

std::string Authentication::CreateSignaturePackets(const PacketType &type_da,
                                                   std::string *public_key) {
  PacketParams params;
  crypto::RsaKeyPair kp;
  while (!crypto_key_pairs_.GetKeyPair(&kp)) {
    kp.ClearKeys();
    crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
  }
  boost::shared_ptr<Packet> sigPacket(PacketFactory::Factory(type_da));
  params["publicKey"] = kp.public_key();
  params["privateKey"] = kp.private_key();
  PacketParams result = sigPacket->Create(params);

  while (!sm_->KeyUnique(boost::any_cast<std::string>(result["name"]), false)) {
    kp.ClearKeys();
    while (!crypto_key_pairs_.GetKeyPair(&kp)) {
      kp.ClearKeys();
      crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
    }
    params["publicKey"] = kp.public_key();
    params["privateKey"] = kp.private_key();
    result = sigPacket->Create(params);
  }

  ss_->AddKey(type_da,
              boost::any_cast<std::string>(result["name"]),
              boost::any_cast<std::string>(result["privateKey"]),
              boost::any_cast<std::string>(result["publicKey"]),
              "");

  if (StorePacket(boost::any_cast<std::string>(result["name"]),
                  boost::any_cast<std::string>(result["publicKey"]), type_da,
                  "")
      != kSuccess) {
    ss_->RemoveKey(type_da);
    return "";
  }

  *public_key = boost::any_cast<std::string>(result["publicKey"]);
  return boost::any_cast<std::string>(result["privateKey"]);
}

bool Authentication::CheckUsername(const std::string &username) {
  std::string username_ = UtilsTrim(boost::lexical_cast<char*>(username));
  return (username_.length() >= 4);
}

bool Authentication::CheckPin(const std::string &pin) {
  std::string pin_ = UtilsTrim(boost::lexical_cast<char*>(pin));
  if (pin_ == "0000")
    return false;
  boost::regex re("\\d{4}");
  return boost::regex_match(pin_, re);
}

bool Authentication::CheckPassword(const std::string &password) {
  std::string password_ = UtilsTrim(boost::lexical_cast<char*>(password));
  return (password_.length() >= 4);
}

int Authentication::PublicUsernamePublicKey(const std::string &public_username,
                                            std::string *public_key) {
  PacketParams params;
  params["publicname"] = public_username;
  crypto::RsaKeyPair kp;
  boost::shared_ptr<Packet> mpidPacket(PacketFactory::Factory(MPID));

  std::vector<std::string> packet_content;
  int result = sm_->LoadPacket(mpidPacket->PacketName(params),
                                         &packet_content);
  if (result != kSuccess || packet_content.empty())
    return kUserDoesntExist;
  std::string ser_generic_packet = packet_content[0];
  PacketParams mpid_result = mpidPacket->GetData(ser_generic_packet,
      PacketParams());

  std::string data(boost::any_cast<std::string>(mpid_result["data"]));

  if (data.empty()) {
    return kAuthenticationError;  // Packet corrupt
  }

  *public_key = data;

  return kSuccess;
}

void Authentication::CreateMSIDPacket(kad::VoidFunctorOneString cb) {
  PacketParams params;
  crypto::RsaKeyPair kp;
  while (!crypto_key_pairs_.GetKeyPair(&kp)) {
    kp.ClearKeys();
    crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
  }
  boost::shared_ptr<Packet> sigPacket(PacketFactory::Factory(MSID));
  params["publicKey"] = kp.public_key();
  params["privateKey"] = kp.private_key();
  PacketParams result = sigPacket->Create(params);

  int count = 0;
  while (!sm_->KeyUnique(boost::any_cast<std::string>(result["name"]),
         false) && count < 10) {
    kp.ClearKeys();
    while (!crypto_key_pairs_.GetKeyPair(&kp)) {
      kp.ClearKeys();
      crypto_key_pairs_.StartToCreateKeyPairs(kNoOfSystemPackets);
    }
    params["publicKey"] = kp.public_key();
    params["privateKey"] = kp.private_key();
    ++count;
  }

  if (count > 9) {
    CreateMSIDResult local_result;
    local_result.set_result(kNack);
    std::string ser_local_result;
    local_result.SerializeToString(&ser_local_result);
    cb(ser_local_result);
    return;
  }

  std::vector<boost::uint32_t> share_stats(2, 0);
  std::vector<std::string> atts;
  atts.push_back(boost::any_cast<std::string>(result["name"]));
  atts.push_back(boost::any_cast<std::string>(result["name"]));
  atts.push_back(boost::any_cast<std::string>(result["publicKey"]));
  atts.push_back(boost::any_cast<std::string>(result["privateKey"]));
  int n = ss_->AddPrivateShare(atts, share_stats, NULL);

  n = StorePacket(boost::any_cast<std::string>(result["name"]),
      boost::any_cast<std::string>(result["publicKey"]), MSID,
      boost::any_cast<std::string>(result["name"]));
  ss_->DeletePrivateShare(atts[0], 0);

  StoreChunkResponse result_msg;
  CreateMSIDResult local_result;
  std::string str_local_result;
  if (n != 0) {
    local_result.set_result(kNack);
  } else {
    local_result.set_result(kAck);
    local_result.set_private_key(boost::any_cast<std::string>(
        result["privateKey"]));
    local_result.set_public_key(boost::any_cast<std::string>(
        result["publicKey"]));
    local_result.set_name(boost::any_cast<std::string>(result["name"]));
  }
  local_result.SerializeToString(&str_local_result);
  cb(str_local_result);
}

int Authentication::StorePacket(const std::string &packet_name,
                                const std::string &value,
                                const PacketType &type,
                                const std::string &msid) {
// TODO(Fraser#5#): 2010-01-28 - Use callbacks properly to allow several stores
//                               to happen concurrently.
  boost::mutex mutex;
  boost::condition_variable cond_var;
  int result(kPendingResult);
  VoidFuncOneInt func = boost::bind(&Authentication::PacketOpCallback, this, _1,
                                    &mutex, &cond_var, &result);
  sm_->StorePacket(packet_name, value, type, PRIVATE_SHARE, msid, func);
  {
    boost::mutex::scoped_lock lock(mutex);
    while (result == kPendingResult)
      cond_var.wait(lock);
  }
#ifdef DEBUG
  if (result != kSuccess)
    printf("!!!!!!!!!!!!!!!!!!!!!\nAuthentication::StorePacket %i\n\n", result);
#endif
  return result;
}

int Authentication::DeletePacket(const std::string &packet_name,
                                 const std::string &value,
                                 const PacketType &type) {
// TODO(Fraser#5#): 2010-01-28 - Use callbacks properly to allow several deletes
//                               to happen concurrently.
  boost::mutex mutex;
  boost::condition_variable cond_var;
  int result(kPendingResult);
  VoidFuncOneInt func = boost::bind(&Authentication::PacketOpCallback, this, _1,
                                    &mutex, &cond_var, &result);
  std::vector<std::string> values;
  if (!value.empty())
    values.push_back(value);
  sm_->DeletePacket(packet_name, values, type, PRIVATE, "", func);
  {
    boost::mutex::scoped_lock lock(mutex);
    while (result == kPendingResult)
      cond_var.wait(lock);
  }
#ifdef DEBUG
  if (result != kSuccess)
    printf("!!!!!!!!!!!!!!!!!!!!\nAuthentication::DeletePacket %i\n\n", result);
#endif
  return result;
}

void Authentication::PacketOpCallback(const int &store_manager_result,
                                      boost::mutex *mutex,
                                      boost::condition_variable *cond_var,
                                      int *op_result) {
  boost::mutex::scoped_lock lock(*mutex);
  *op_result = store_manager_result;
  cond_var->notify_one();
}

std::string Authentication::EncryptedDataMidSmid(boost::uint32_t rid) {
  std::string salt = crypto_.Hash(ss_->Pin() + ss_->Username(), "",
                                  crypto::STRING_STRING, false);
  if (salt.empty())
    return "";
  std::string password = crypto_.SecurePassword(ss_->Username(), salt,
                         boost::lexical_cast<boost::uint16_t>(ss_->Pin()));
  return crypto_.SymmEncrypt(boost::lexical_cast<std::string>(rid), "",
                             crypto::STRING_STRING, password);
}

void Authentication::CreateSystemPacketsCallback(const ReturnCode &return_code) {
  system_packets_result_ = return_code;
}

void Authentication::GetUserInfoCallback(const ReturnCode &return_code) {
  user_info_result_ = return_code;
}

char *Authentication::UtilsTrimRight(char *szSource) {
  char *pszEOS = NULL;
  //  Set pointer to character before terminating NULL
  pszEOS = szSource + strlen(szSource) - 1;
  //  iterate backwards until non '_' is found
  while ((pszEOS >= szSource) && (*pszEOS == ' '))
    --*pszEOS = '\0';
  return szSource;
}

char *Authentication::UtilsTrimLeft(char *szSource) {
  char *pszBOS = NULL;
  //  Set pointer to first character
  pszBOS = szSource;
  //  iterate forwards until non '_' is found
  while (*pszBOS == ' ')
    ++*pszBOS;
  return pszBOS;
}

char *Authentication::UtilsTrim(char *szSource) {
  return UtilsTrimLeft(UtilsTrimRight(UtilsTrimLeft(szSource)));
}

}  // namespace passport

}  // namespace maidsafe
