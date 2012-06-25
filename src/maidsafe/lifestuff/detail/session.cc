/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Singleton for setting/getting session info
* Version:      1.0
* Created:      2009-01-28-16.56.20
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

#include "maidsafe/lifestuff/detail/session.h"

#include <memory>
#include <vector>
#include <limits>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/lifestuff/return_codes.h"
#include "maidsafe/lifestuff/detail/contacts.h"
#include "maidsafe/lifestuff/detail/data_atlas_pb.h"

namespace maidsafe {

namespace lifestuff {

Session::Session()
    : passport_(),
      user_details_(),
      public_id_details_() {}

Session::~Session() {}

void Session::Reset() {
  user_details_.defconlevel = kDefCon3;
  user_details_.keyword.clear();
  user_details_.pin.clear();
  user_details_.password.clear();
  user_details_.session_name.clear();
  user_details_.unique_user_id.clear();
  user_details_.root_parent_id.clear();
  user_details_.max_space = 1073741824;
  user_details_.used_space = 0;
  user_details_.serialised_data_atlas.clear();

  passport_.Clear(true, true, true);
  public_id_details_.clear();
}

passport::Passport& Session::passport() { return passport_; }

int Session::AddPublicId(const std::string &public_id) {
  auto result(public_id_details_.insert(std::make_pair(public_id, PublicIdDetails())));
  if (!result.second) {
    LOG(kError) << "Failure to add public id to session: " << public_id;
    return kPublicIdInsertionFailure;
  }

  return kSuccess;
}

bool Session::OwnPublicId(const std::string &public_id) {
  return public_id_details_.find(public_id) != public_id_details_.end();
}

ContactsHandler& Session::contacts_handler(const std::string &public_id, int &result) {
  auto it(public_id_details_.find(public_id));
  if (it == public_id_details_.end()) {
    LOG(kError) << "Failure to find public id: " << public_id;
    result = kPublicIdNotFoundFailure;
    ContactsHandler ch;
    return ch;
  }

  result = kSuccess;
  return (*it).second.contacts_handler;
}

ShareInformation& Session::share_information(const std::string &public_id, int &result) {
  auto it(public_id_details_.find(public_id));
  if (it == public_id_details_.end()) {
    LOG(kError) << "Failure to find public id: " << public_id;
    result = kPublicIdNotFoundFailure;
    ShareInformation si;
    return si;
  }

  result = kSuccess;
  return (*it).second.share_information;
}

std::string& Session::profile_picture_data_map(const std::string &public_id, int &result) {
  auto it(public_id_details_.find(public_id));
  if (it == public_id_details_.end()) {
    LOG(kError) << "Failure to find public id: " << public_id;
    result = kPublicIdNotFoundFailure;
    std::string s;
    return s;
  }

  result = kSuccess;
  return (*it).second.profile_picture_data_map;
}

DefConLevels Session::def_con_level() const { return user_details_.defconlevel; }
std::string Session::keyword() const { return user_details_.keyword; }
std::string Session::pin() const { return user_details_.pin; }
std::string Session::password() const { return user_details_.password; }
std::string Session::session_name() const { return user_details_.session_name; }
std::string Session::unique_user_id() const { return user_details_.unique_user_id; }
std::string Session::root_parent_id() const { return user_details_.root_parent_id; }
int64_t Session::max_space() const { return user_details_.max_space; }
int64_t Session::used_space() const { return user_details_.used_space; }
std::string Session::serialised_data_atlas() const { return user_details_.serialised_data_atlas; }

void Session::set_def_con_level(DefConLevels defconlevel) {
  user_details_.defconlevel = defconlevel;
}
void Session::set_keyword(const std::string &keyword) { user_details_.keyword = keyword; }
void Session::set_pin(const std::string &pin) { user_details_.pin = pin; }
void Session::set_password(const std::string &password) { user_details_.password = password; }
bool Session::set_session_name() {
  if (keyword().empty() || pin().empty()) {
    LOG(kError) << "keyword: " << std::boolalpha << keyword().empty()
                << ", pin: " << std::boolalpha << pin().empty();
    return false;
  }
  user_details_.session_name = EncodeToHex(crypto::Hash<crypto::SHA1>(pin() + keyword()));
  return true;
}
void Session::clear_session_name() { user_details_.session_name.clear(); }
void Session::set_unique_user_id(const std::string &unique_user_id) {
  if (unique_user_id.empty())
    LOG(kWarning) << "Passed empty unique user ID.";
  user_details_.unique_user_id = unique_user_id;
}
void Session::set_root_parent_id(const std::string &root_parent_id) {
  if (root_parent_id.empty())
    LOG(kWarning) << "Passed empty root parent ID.";
  user_details_.root_parent_id = root_parent_id;
}
void Session::set_max_space(const int64_t &max_space) {
  if (max_space == 0)
    LOG(kWarning) << "Passed zero maximum space.";
  user_details_.max_space = max_space;
}
void Session::set_used_space(const int64_t &used_space) {
  if (used_space > user_details_.max_space)
    LOG(kWarning) << "Passed used space greater than maximum.";
  user_details_.used_space = used_space;
}
void Session::set_serialised_data_atlas(const std::string &serialised_data_atlas) {
  user_details_.serialised_data_atlas = serialised_data_atlas;
}

int Session::ParseDataAtlas(const std::string &serialised_data_atlas) {
  DataAtlas data_atlas;
  if (serialised_data_atlas.empty()) {
    LOG(kError) << "TMID brought is empty.";
    return -9000;
  }
  if (!data_atlas.ParseFromString(serialised_data_atlas)) {
    LOG(kError) << "TMID doesn't parse.";
    return -9000;
  }

  set_unique_user_id(data_atlas.drive_data().unique_user_id());
  set_root_parent_id(data_atlas.drive_data().root_parent_id());
  set_max_space(data_atlas.drive_data().max_space());
  set_used_space(data_atlas.drive_data().used_space());

  int result(passport_.Parse(data_atlas.passport_data().serialised_keyring()));
  if (result != kSuccess) {
    LOG(kError) << "Failed ParseKeyChain: " << result;
    return -9003;
  }

  std::string pub_id;
  for (int id_count(0); id_count < data_atlas.public_ids_size(); ++id_count) {
    pub_id = data_atlas.public_ids(id_count).public_id();
    PublicIdDetails public_id_details;
    public_id_details.profile_picture_data_map =
        data_atlas.public_ids(id_count).profile_picture_data_map();

    for (int contact_count(0);
         contact_count < data_atlas.public_ids(id_count).contacts_size();
         ++contact_count) {
      Contact contact(data_atlas.public_ids(id_count).contacts(contact_count));
      asymm::DecodePublicKey(
          data_atlas.public_ids(id_count).contacts(contact_count).mpid_public_key(),
          &contact.mpid_public_key);
      asymm::DecodePublicKey(
          data_atlas.public_ids(id_count).contacts(contact_count).inbox_public_key(),
          &contact.inbox_public_key);
      int result(public_id_details.contacts_handler.AddContact(contact));
      LOG(kInfo) << "Result of adding " << contact.public_id << " to " << pub_id << ":  " << result;
    }

    public_id_details_[pub_id] = public_id_details;
  }

  return kSuccess;
}

int Session::SerialiseDataAtlas(std::string *serialised_data_atlas) {
  BOOST_ASSERT(serialised_data_atlas);
  DataAtlas data_atlas;
  DriveData *drive_data(data_atlas.mutable_drive_data());
  drive_data->set_unique_user_id(unique_user_id());
  drive_data->set_root_parent_id(root_parent_id());
  drive_data->set_max_space(max_space());
  drive_data->set_used_space(used_space());

  data_atlas.set_timestamp(boost::lexical_cast<std::string>(
      GetDurationSinceEpoch().total_microseconds()));

  std::string serialised_keyring(passport_.Serialise());
  if (serialised_keyring.empty()) {
    LOG(kError) << "Serialising keyring failed.";
    return -1;
  }

  PassportData *passport_data(data_atlas.mutable_passport_data());
  passport_data->set_serialised_keyring(serialised_keyring);

  std::vector<Contact> contacts;
  for (auto it(public_id_details_.begin()); it != public_id_details_.end(); ++it) {
    PublicIdentity *pub_id(data_atlas.add_public_ids());
    pub_id->set_public_id((*it).first);
    pub_id->set_profile_picture_data_map((*it).second.profile_picture_data_map);
    (*it).second.contacts_handler.OrderedContacts(&contacts, kAlphabetical, kRequestSent |
                                                                            kPendingResponse |
                                                                            kConfirmed |
                                                                            kBlocked);
    for (size_t n(0); n < contacts.size(); ++n) {
      PublicContact *pc(pub_id->add_contacts());
      pc->set_public_id(contacts[n].public_id);
      pc->set_mpid_name(contacts[n].mpid_name);
      pc->set_inbox_name(contacts[n].inbox_name);
      std::string serialised_mpid_public_key, serialised_inbox_public_key;
      asymm::EncodePublicKey(contacts[n].mpid_public_key, &serialised_mpid_public_key);
      pc->set_mpid_public_key(serialised_mpid_public_key);
      asymm::EncodePublicKey(contacts[n].inbox_public_key, &serialised_inbox_public_key);
      pc->set_inbox_public_key(serialised_inbox_public_key);
      pc->set_status(contacts[n].status);
      pc->set_rank(contacts[n].rank);
      pc->set_last_contact(contacts[n].last_contact);
      pc->set_profile_picture_data_map(contacts[n].profile_picture_data_map);
      LOG(kInfo) << "Added contact " << contacts[n].public_id << " to " << (*it).first << " map.";
    }
  }

  if (!data_atlas.SerializeToString(serialised_data_atlas)) {
    LOG(kError) << "Failed to serialise.";
    return -1;
  }

  return kSuccess;
}

bool Session::CreateTestPackets(bool with_public_ids) {
  if (passport_.CreateSigningPackets() != kSuccess)
    return false;
  if (passport_.ConfirmSigningPackets() != kSuccess)
    return false;

  if (with_public_ids) {
    for (size_t n(0); n < 5; ++n) {
      std::string public_id(RandomAlphaNumericString(5));
      if (passport_.CreateSelectableIdentity(public_id) != kSuccess)
        return false;
      if (passport_.ConfirmSelectableIdentity(public_id) != kSuccess)
        return false;
    }
  }

  return true;
}


std::vector<std::string> Session::PublicIdentities() const {
  std::vector<std::string> public_identities;
  typedef std::map<std::string, PublicIdDetails> PublicIdDetailsMap;
  std::for_each(public_id_details_.begin(),
                public_id_details_.end(),
                [&public_identities] (const PublicIdDetailsMap::value_type &el) {
                  public_identities.push_back(el.first);
                });
  return public_identities;
}

}  // namespace lifestuff

}  // namespace maidsafe


