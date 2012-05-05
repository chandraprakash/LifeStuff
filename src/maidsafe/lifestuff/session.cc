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

#include "maidsafe/lifestuff/session.h"

#include <memory>
#include <vector>
#include <limits>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/lifestuff/contacts.h"
#include "maidsafe/lifestuff/data_atlas_pb.h"
#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/return_codes.h"

namespace maidsafe {

namespace lifestuff {

struct UserDetails {
  UserDetails()
      : defconlevel(kDefCon3),
        username(),
        pin(),
        password(),
        session_name(),
        unique_user_id(),
        root_parent_id() {}
  DefConLevels defconlevel;
  std::string username, pin, password, session_name, unique_user_id,
              root_parent_id;
};

Session::Session()
    : user_details_(new UserDetails),
      passport_(new passport::Passport),
      contact_handler_map_(),
      profile_picture_map_() {}

Session::~Session() {}

bool Session::Reset() {
  user_details_->defconlevel = kDefCon3;
  user_details_->username.clear();
  user_details_->pin.clear();
  user_details_->password.clear();
  user_details_->session_name.clear();
  user_details_->unique_user_id.clear();
  user_details_->root_parent_id.clear();
  // TODO(Fraser#5#): 2011-11-17 - Implement in passport
  passport_->ClearKeyChain(true, true, true);
  contact_handler_map_.clear();
  profile_picture_map_.clear();
  return true;
}

ContactHandlerMap& Session::contact_handler_map() {
  return contact_handler_map_;
}

PublicIdContactMap Session::GetAllContacts(ContactStatus status) {
  std::vector<Contact> contacts;
  PublicIdContactMap result;
  auto it(contact_handler_map_.begin());
  for (; it != contact_handler_map_.end(); ++it) {
    result[(*it).first] = std::set<std::string>();
    BOOST_ASSERT(status <= std::numeric_limits<uint16_t>::max());
    (*it).second->OrderedContacts(&contacts,
                                  kAlphabetical,
                                  static_cast<uint16_t>(status));
    for (auto item(contacts.begin()); item != contacts.end(); ++item)
      result[(*it).first].insert((*item).public_id);
  }

  return result;
}

DefConLevels Session::def_con_level() const {
  return user_details_->defconlevel;
}
std::string Session::username() const { return user_details_->username; }
std::string Session::pin() const { return user_details_->pin; }
std::string Session::password() const { return user_details_->password; }
std::string Session::session_name() const {
  return user_details_->session_name;
}
std::string Session::unique_user_id() const {
  return user_details_->unique_user_id;
}
std::string Session::root_parent_id() const {
  return user_details_->root_parent_id;
}
std::string Session::profile_picture_data_map(
    const std::string &public_id) const {
  auto it(profile_picture_map_.find(public_id));
  if (it == profile_picture_map_.end()) {
    DLOG(ERROR) << "no such public ID: " << public_id;
    return "";
  }

  return (*it).second;
}

void Session::set_def_con_level(DefConLevels defconlevel) {
  user_details_->defconlevel = defconlevel;
}
void Session::set_username(const std::string &username) {
  user_details_->username = username;
}
void Session::set_pin(const std::string &pin) { user_details_->pin = pin; }
void Session::set_password(const std::string &password) {
  user_details_->password = password;
}
bool Session::set_session_name() {
  if (username().empty() || pin().empty()) {
    DLOG(ERROR) << "username: " << std::boolalpha << username().empty()
                << ", pin: " << std::boolalpha << pin().empty();
    return false;
  }
  user_details_->session_name =
      EncodeToHex(crypto::Hash<crypto::SHA1>(pin() + username()));
  return true;
}
void Session::clear_session_name() { user_details_->session_name.clear(); }
void Session::set_unique_user_id(const std::string &unique_user_id) {
  user_details_->unique_user_id = unique_user_id;
}
void Session::set_root_parent_id(const std::string &root_parent_id) {
  user_details_->root_parent_id = root_parent_id;
}
bool Session::set_profile_picture_data_map(
    const std::string &public_id,
    const std::string &profile_picture_data_map) {
  if (contact_handler_map_.find(public_id) == contact_handler_map_.end()) {
    DLOG(ERROR) << "no such public ID: " << public_id;
    return false;
  }
  profile_picture_map_[public_id] = profile_picture_data_map;

  return true;
}

int Session::ParseDataAtlas(const std::string &serialised_data_atlas) {
  DataAtlas data_atlas;
  if (serialised_data_atlas.empty()) {
    DLOG(ERROR) << "TMID brought is empty.";
    return -9000;
  }
  if (!data_atlas.ParseFromString(serialised_data_atlas)) {
    DLOG(ERROR) << "TMID doesn't parse.";
    return -9000;
  }

  set_unique_user_id(data_atlas.drive_data().unique_user_id());
  set_root_parent_id(data_atlas.drive_data().root_parent_id());

  int result(0);
  result = ParseKeyChain(data_atlas.passport_data().serialised_keyring(),
                         data_atlas.passport_data().serialised_selectables());
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed ParseKeyChain: " << result;
    return -9003;
  }

  std::string pub_id;
  for (int id_count(0); id_count < data_atlas.public_ids_size(); ++id_count) {
    pub_id = data_atlas.public_ids(id_count).public_id();
    contact_handler_map().insert(
        std::make_pair(pub_id,
                       std::make_shared<ContactsHandler>()));
    set_profile_picture_data_map(
        pub_id,
        data_atlas.public_ids(id_count).profile_picture_data_map());
    for (int contact_count(0);
         contact_count < data_atlas.public_ids(id_count).contacts_size();
         ++contact_count) {
      Contact contact(data_atlas.public_ids(id_count).contacts(contact_count));
      int result(contact_handler_map()[pub_id]->AddContact(contact));
      DLOG(INFO) << "Result of adding contact " << contact.public_id << " to "
                 << pub_id << ":  " << result;
    }
  }

  return 0;
}

int Session::SerialiseDataAtlas(std::string *serialised_data_atlas) {
  BOOST_ASSERT(serialised_data_atlas);
  DataAtlas data_atlas;
  DriveData *drive_data(data_atlas.mutable_drive_data());
  drive_data->set_unique_user_id(unique_user_id());
  drive_data->set_root_parent_id(root_parent_id());

  data_atlas.set_timestamp(boost::lexical_cast<std::string>(
      GetDurationSinceEpoch().total_microseconds()));

  std::string serialised_keyring, serialised_selectables;
  SerialiseKeyChain(&serialised_keyring, &serialised_selectables);
  if (serialised_keyring.empty()) {
    DLOG(ERROR) << "Serialising keyring failed.";
    return -1;
  }

  PassportData *passport_data(data_atlas.mutable_passport_data());
  passport_data->set_serialised_keyring(serialised_keyring);
  passport_data->set_serialised_selectables(serialised_selectables);

  std::vector<Contact> contacts;
  for (auto it(contact_handler_map().begin());
       it != contact_handler_map().end();
       ++it) {
    contacts.clear();
    PublicIdentity *pub_id(data_atlas.add_public_ids());
    pub_id->set_public_id((*it).first);
    pub_id->set_profile_picture_data_map(profile_picture_data_map((*it).first));
    (*it).second->OrderedContacts(&contacts, kAlphabetical, kRequestSent |
                                                            kPendingResponse |
                                                            kConfirmed |
                                                            kBlocked);
    for (size_t n(0); n < contacts.size(); ++n) {
      PublicContact *pc(pub_id->add_contacts());
      pc->set_public_id(contacts[n].public_id);
      pc->set_mpid_name(contacts[n].mpid_name);
      pc->set_inbox_name(contacts[n].inbox_name);
      pc->set_status(contacts[n].status);
      pc->set_rank(contacts[n].rank);
      pc->set_last_contact(contacts[n].last_contact);
      pc->set_profile_picture_data_map(contacts[n].profile_picture_data_map);
      DLOG(INFO) << "Added contact " << contacts[n].public_id
                  << " to " << (*it).first << " map.";
    }
  }

  if (!data_atlas.SerializeToString(serialised_data_atlas)) {
    DLOG(ERROR) << "Failed to serialise.";
    return -1;
  }

  return 0;
}

int Session::ParseKeyChain(const std::string &serialised_keyring,
                           const std::string &serialised_selectables) {
  return passport_->ParseKeyChain(serialised_keyring, serialised_selectables);
}
void Session::SerialiseKeyChain(std::string *serialised_keyring,
                                std::string *serialised_selectables) {
  passport_->SerialiseKeyChain(serialised_keyring, serialised_selectables);
}

bool Session::CreateTestPackets(bool with_public_ids) {
  if (passport_->CreateSigningPackets() != kSuccess)
    return false;
  if (passport_->ConfirmSigningPackets() != kSuccess)
    return false;

  if (with_public_ids) {
    for (size_t n(0); n < 5; ++n) {
      std::string public_id(RandomAlphaNumericString(5));
      if (passport_->CreateSelectableIdentity(public_id) != kSuccess)
        return false;
      if (passport_->ConfirmSelectableIdentity(public_id) != kSuccess)
        return false;
    }
  }

  return true;
}

std::vector<std::string> Session::GetPublicIdentities() {
  std::vector<std::string> public_identities;
  std::for_each(contact_handler_map_.begin(),
                contact_handler_map_.end(),
                [&public_identities] (const ContactHandlerMap::value_type &el) {
                  public_identities.push_back(el.first);
                });
  return public_identities;
}

}  // namespace lifestuff

}  // namespace maidsafe


