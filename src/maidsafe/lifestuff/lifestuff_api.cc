/*
* ============================================================================
*
* Copyright [2012] maidsafe.net limited
*
* Description:  Definition of system-wide constants/enums/structs
* Version:      1.0
* Created:      2012-03-27
* Revision:     none
* Compiler:     gcc
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

#include "maidsafe/lifestuff/lifestuff_api.h"

#include <algorithm>
#include <vector>

#include "maidsafe/common/asio_service.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/encrypt/data_map.h"
#include "maidsafe/pd/client/client_container.h"

#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/return_codes.h"

#include "maidsafe/lifestuff/contacts.h"
#include "maidsafe/lifestuff/message_handler.h"
#include "maidsafe/lifestuff/public_id.h"
#include "maidsafe/lifestuff/session.h"
#include "maidsafe/lifestuff/user_credentials.h"
#include "maidsafe/lifestuff/user_storage.h"
#include "maidsafe/lifestuff/utils.h"

namespace maidsafe {

namespace lifestuff {

struct LifeStuff::Elements {
  Elements() : thread_count(kThreads),
               state(kZeroth),
               buffered_path(),
#ifdef LOCAL_TARGETS_ONLY
               simulation_path(),
#endif
               interval(kSecondsInterval),
               asio_service(),
               remote_chunk_store(),
#ifndef LOCAL_TARGETS_ONLY
               client_container(),
#endif
               session(),
               user_credentials(),
               user_storage(),
               public_id(),
               message_handler() {}

  int thread_count;
  LifeStuffState state;
  fs::path buffered_path;
#ifdef LOCAL_TARGETS_ONLY
  fs::path simulation_path;
#endif
  bptime::seconds interval;
  AsioService asio_service;
  std::shared_ptr<pcs::RemoteChunkStore> remote_chunk_store;
#ifndef LOCAL_TARGETS_ONLY
  std::shared_ptr<pd::ClientContainer> client_container;
#endif
  std::shared_ptr<Session> session;
  std::shared_ptr<UserCredentials> user_credentials;
  std::shared_ptr<UserStorage> user_storage;
  std::shared_ptr<PublicId> public_id;
  std::shared_ptr<MessageHandler> message_handler;
};

LifeStuff::LifeStuff() : lifestuff_elements(new Elements) {}

LifeStuff::~LifeStuff() {}

int LifeStuff::Initialise(const boost::filesystem::path &base_directory) {
  if (lifestuff_elements->state != kZeroth) {
    DLOG(ERROR) << "Make sure that object is in the original Zeroth state. "
                << "Asimov rules.";
    return kGeneralError;
  }

  // Initialisation
  lifestuff_elements->asio_service.Start(lifestuff_elements->thread_count);
  lifestuff_elements->session.reset(new Session);

  fs::path base_path, buffered_chunk_store_path, network_simulation_path;
  if (base_directory.empty()) {
    // Not a test: everything in $HOME/.lifestuff
    base_path = GetHomeDir() / kAppHomeDirectory;
    buffered_chunk_store_path = base_path / RandomAlphaNumericString(16);
    boost::system::error_code error_code;
    network_simulation_path = fs::temp_directory_path(error_code) /
                              "lifestuff_simulation";
  } else {
    // Presumably a test
    base_path = base_directory;
    buffered_chunk_store_path = base_path / RandomAlphaNumericString(16);
    network_simulation_path = base_path / "simulated_network";
  }

#ifdef LOCAL_TARGETS_ONLY
  lifestuff_elements->remote_chunk_store =
      BuildChunkStore(buffered_chunk_store_path,
                      network_simulation_path,
                      lifestuff_elements->asio_service.service());
  lifestuff_elements->simulation_path = network_simulation_path;
#else
  lifestuff_elements->remote_chunk_store =
      BuildChunkStore(buffered_chunk_store_path,
                      &lifestuff_elements->client_container);
#endif
  lifestuff_elements->buffered_path = buffered_chunk_store_path;

  lifestuff_elements->user_credentials.reset(
      new UserCredentials(lifestuff_elements->remote_chunk_store,
                          lifestuff_elements->session));

  lifestuff_elements->public_id.reset(
      new PublicId(lifestuff_elements->remote_chunk_store,
                   lifestuff_elements->session,
                   lifestuff_elements->asio_service.service()));

  lifestuff_elements->message_handler.reset(
      new MessageHandler(lifestuff_elements->remote_chunk_store,
                         lifestuff_elements->session,
                         lifestuff_elements->asio_service.service()));

  lifestuff_elements->user_storage.reset(
      new UserStorage(lifestuff_elements->remote_chunk_store,
                      lifestuff_elements->message_handler));

  lifestuff_elements->message_handler->ConnectToParseAndSaveDataMapSignal(
      std::bind(&UserStorage::ParseAndSaveDataMap,
                lifestuff_elements->user_storage, args::_1, args::_2));

  lifestuff_elements->public_id->ConnectToContactConfirmedSignal(
      std::bind(&MessageHandler::InformConfirmedContactOnline,
                lifestuff_elements->message_handler, args::_1, args::_2));

  lifestuff_elements->state = kInitialised;

  return kSuccess;
}

int LifeStuff::ConnectToSignals(
    const ChatFunction &chat_slot,
    const FileTransferFunction &file_slot,
    const ShareInvitationFunction &/*share_slot*/,
    const NewContactFunction &new_contact_slot,
    const ContactConfirmationFunction &confirmed_contact_slot,
    const ContactProfilePictureFunction &profile_picture_slot,
    const ContactPresenceFunction &contact_presence_slot) {
  if (lifestuff_elements->state != kInitialised) {
    DLOG(ERROR) << "Make sure that object is initialised";
    return kGeneralError;
  }

  int connects(0);
  if (chat_slot) {
    lifestuff_elements->message_handler->ConnectToChatSignal(chat_slot);
    ++connects;
  }
  if (file_slot) {
    lifestuff_elements->message_handler->ConnectToFileTransferSignal(file_slot);
    ++connects;
  }
  if (new_contact_slot) {
    lifestuff_elements->public_id->ConnectToNewContactSignal(new_contact_slot);
    ++connects;
  }
  if (confirmed_contact_slot) {
    lifestuff_elements->public_id->ConnectToContactConfirmedSignal(
        confirmed_contact_slot);
    ++connects;
  }
  if (profile_picture_slot) {
    lifestuff_elements->message_handler->ConnectToContactProfilePictureSignal(
        profile_picture_slot);
    ++connects;
  }
  if (contact_presence_slot) {
    lifestuff_elements->message_handler->ConnectToContactPresenceSignal(
        contact_presence_slot);
    ++connects;
  }

  if (connects > 0) {
    lifestuff_elements->state = kConnected;
    return kSuccess;
  }

  return kGeneralError;
}

int LifeStuff::Finalise() {
  if (lifestuff_elements->state != kLoggedOut) {
    DLOG(ERROR) << "Need to be logged out to finalise.";
    return kGeneralError;
  }

  boost::system::error_code error_code;
  fs::remove_all(lifestuff_elements->buffered_path, error_code);


  lifestuff_elements->asio_service.Stop();
  lifestuff_elements->remote_chunk_store.reset();
#ifndef LOCAL_TARGETS_ONLY
  lifestuff_elements->client_container.reset();
#endif
  lifestuff_elements->message_handler.reset();
  lifestuff_elements->public_id.reset();
  lifestuff_elements->session.reset();
  lifestuff_elements->user_credentials.reset();
  lifestuff_elements->user_storage.reset();
  lifestuff_elements->state = kZeroth;

  return kSuccess;
}

int LifeStuff::SetValidPmid() {
  int result = dht::kSuccess;
#ifndef LOCAL_TARGETS_ONLY
  std::cout << lifestuff_elements->client_container->key_pair_->identity << std::endl;
  std::vector<dht::Contact> bootstrap_contacts;
  lifestuff_elements->client_container->Stop(&bootstrap_contacts);
  lifestuff_elements->client_container->set_key_pair(
      lifestuff_elements->session->GetPmidKeys());
  lifestuff_elements->client_container->Init(
      lifestuff_elements->buffered_path / "buffered_chunk_store", 10, 4);
  result = lifestuff_elements->client_container->Start(bootstrap_contacts);
  lifestuff_elements->remote_chunk_store.reset(
      new pcs::RemoteChunkStore(lifestuff_elements->client_container->chunk_store(),
            lifestuff_elements->client_container->chunk_manager(),
            lifestuff_elements->client_container->chunk_action_authority()));
  lifestuff_elements->user_credentials.reset(
      new UserCredentials(lifestuff_elements->remote_chunk_store,
                          lifestuff_elements->session));  
  std::cout << "Joined: " << lifestuff_elements->client_container->node_->joined() << std::endl;
  std::cout << lifestuff_elements->client_container->key_pair_->identity << std::endl;  
#endif
  return result;
}

/// Credential operations
int LifeStuff::CreateUser(const std::string &username,
                          const std::string &pin,
                          const std::string &password) {
  if (lifestuff_elements->state != kConnected) {
    DLOG(ERROR) << "Make sure that object is initialised and connected";
    return kGeneralError;
  }

  if (!lifestuff_elements->user_credentials->CreateUser(username,
                                                        pin,
                                                        password)) {
    DLOG(ERROR) << "Failed to Create User.";
    return kGeneralError;
  }

   if (SetValidPmid() != kSuccess)  {
     DLOG(ERROR) << "Failed to set valid PMID";
     return kGeneralError;
   }

//   boost::system::error_code error_code;
//   fs::path mount_dir(GetHomeDir() /
//                      kAppHomeDirectory /
//                      lifestuff_elements->session->session_name());
//   if (!fs::exists(mount_dir, error_code)) {
//     fs::create_directories(mount_dir, error_code);
//     if (error_code) {
//       DLOG(ERROR) << "Failed to create app directories - " << error_code.value()
//                   << ": " << error_code.message();
//       return kGeneralError;
//     }
//   }
// 
//   lifestuff_elements->user_storage->MountDrive(mount_dir,
//                                                lifestuff_elements->session,
//                                                true);
//   if (!lifestuff_elements->user_storage->mount_status()) {
//     DLOG(ERROR) << "Failed to mount";
//     return kGeneralError;
//   }
// 
//   fs::create_directory(lifestuff_elements->user_storage->mount_dir() /
//                        fs::path("/").make_preferred() /
//                        "My Stuff", error_code);
//   if (error_code) {
//     DLOG(ERROR) << "Failed creating My Stuff: " << error_code.message();
//     return kGeneralError;
//   }
//   fs::create_directory(lifestuff_elements->user_storage->mount_dir() /
//                            fs::path("/").make_preferred() / "Shared Stuff",
//                        error_code);
//   if (error_code) {
//     DLOG(ERROR) << "Failed creating Shared Stuff: " << error_code.message();
//     return kGeneralError;
//   }

  int result(lifestuff_elements->user_credentials->SaveSession());
  if (result != kSuccess) {
    DLOG(WARNING) << "Failed to save session.";
  }

  lifestuff_elements->state = kLoggedIn;

  return kSuccess;
}

int LifeStuff::CreatePublicId(const std::string &public_id) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state to create a public ID.";
    return kGeneralError;
  }

  // Check if it's the 1st one
  bool first_public_id(false);
  if (lifestuff_elements->session->contact_handler_map().empty())
    first_public_id = true;

  int result(lifestuff_elements->public_id->CreatePublicId(public_id, true));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to create public ID.";
    return result;
  }

  if (first_public_id) {
    lifestuff_elements->public_id->StartCheckingForNewContacts(
        lifestuff_elements->interval);
    lifestuff_elements->message_handler->StartUp(
        lifestuff_elements->interval);
  }

  return kSuccess;
}

int LifeStuff::LogIn(const std::string &username,
                     const std::string &pin,
                     const std::string &password) {
  if (!(lifestuff_elements->state == kConnected ||
        lifestuff_elements->state == kLoggedOut)) {
    DLOG(ERROR) << "Make sure that object is initialised and connected";
    return kGeneralError;
  }

  int result(lifestuff_elements->user_credentials->CheckUserExists(username,
                                                                   pin));
  if (result != kUserExists) {
    DLOG(ERROR) << "User doesn't exist.";
    return result;
  }

  if (!lifestuff_elements->user_credentials->ValidateUser(password)) {
    DLOG(ERROR) << "Wrong password.";
    return kGeneralError;
  }

  if (SetValidPmid() != dht::kSuccess)  {
    DLOG(ERROR) << "Failed to set valid PMID";
    return kGeneralError;
  }

  boost::system::error_code error_code;
  fs::path mount_dir(GetHomeDir() /
                     kAppHomeDirectory /
                     lifestuff_elements->session->session_name());
  if (!fs::exists(mount_dir, error_code)) {
    fs::create_directories(mount_dir, error_code);
    if (error_code) {
      DLOG(ERROR) << "Failed to create app directories - " << error_code.value()
                  << ": " << error_code.message();
      return kGeneralError;
    }
  }

  lifestuff_elements->user_storage->MountDrive(mount_dir,
                                               lifestuff_elements->session,
                                               false);

  if (!lifestuff_elements->user_storage->mount_status()) {
    DLOG(ERROR) << "Failed to mount";
    return kGeneralError;
  }

  if (!lifestuff_elements->session->contact_handler_map().empty()) {
    lifestuff_elements->public_id->StartUp(lifestuff_elements->interval);
    lifestuff_elements->message_handler->StartUp(lifestuff_elements->interval);
  }

  lifestuff_elements->state = kLoggedIn;

  return kSuccess;
}

int LifeStuff::LogOut() {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Should be logged in to log out.";
    return kGeneralError;
  }

  lifestuff_elements->user_storage->UnMountDrive();
  if (lifestuff_elements->user_storage->mount_status()) {
    DLOG(ERROR) << "Failed to un-mount.";
    return kGeneralError;
  }

  lifestuff_elements->public_id->ShutDown();
  lifestuff_elements->message_handler->ShutDown();

  if (!lifestuff_elements->user_credentials->Logout()) {
    DLOG(ERROR) << "Failed to log out.";
    return kGeneralError;
  }

  if (!lifestuff_elements->remote_chunk_store->WaitForCompletion()) {
    DLOG(ERROR) << "Failed complete chunk operations.";
    return kGeneralError;
  }

  // Delete mount directory
  boost::system::error_code error_code;
  fs::remove_all(lifestuff_elements->user_storage->mount_dir(), error_code);
  if (error_code)
    DLOG(WARNING) << "Failed to delete mount directory: "
                  << lifestuff_elements->user_storage->mount_dir();
  lifestuff_elements->session->Reset();

  lifestuff_elements->state = kLoggedOut;

  return kSuccess;
}

int LifeStuff::CheckPassword(const std::string &password) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Should be logged in to log out.";
    return kGeneralError;
  }

  return lifestuff_elements->session->password() == password ? kSuccess :
                                                               kGeneralError;
}

int LifeStuff::ChangeKeyword(const std::string &old_username,
                             const std::string &new_username,
                             const std::string &password) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Should be logged in to log out.";
    return kGeneralError;
  }

  int result(CheckPassword(password));
  if (result != kSuccess) {
    DLOG(ERROR) << "Password verification failed.";
    return result;
  }

  if (lifestuff_elements->session->username() != old_username) {
    DLOG(ERROR) << "Keyword verification failed.";
    return kGeneralError;
  }

  return lifestuff_elements->user_credentials->ChangeUsername(new_username);
}

int LifeStuff::ChangePin(const std::string &old_pin,
                         const std::string &new_pin,
                         const std::string &password) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Should be logged in to log out.";
    return kGeneralError;
  }

  int result(CheckPassword(password));
  if (result != kSuccess) {
    DLOG(ERROR) << "Password verification failed.";
    return result;
  }

  if (lifestuff_elements->session->pin() != old_pin) {
    DLOG(ERROR) << "Keyword verification failed.";
    return kGeneralError;
  }

  return lifestuff_elements->user_credentials->ChangePin(new_pin);
}

int LifeStuff::ChangePassword(const std::string &old_password,
                              const std::string &new_password) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Should be logged in to log out.";
    return kGeneralError;
  }

  int result(CheckPassword(old_password));
  if (result != kSuccess) {
    DLOG(ERROR) << "Password verification failed.";
    return result;
  }

  return lifestuff_elements->user_credentials->ChangePassword(new_password);
}

/// Contact operations
int PreContactChecks(const LifeStuffState &state,
                     const std::string &my_public_id,
                     std::shared_ptr<Session> session) {
  if (state != kLoggedIn) {
    DLOG(ERROR) << "Incorrect state. Should be logged in.";
    return kGeneralError;
  }

  auto it(session->contact_handler_map().find(my_public_id));
  if (it == session->contact_handler_map().end()) {
    DLOG(ERROR) << "No such public ID.";
    return kGeneralError;
  }

  return kSuccess;
}

int LifeStuff::AddContact(const std::string &my_public_id,
                          const std::string &contact_public_id) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in AddContact.";
    return result;
  }

  return lifestuff_elements->public_id->SendContactInfo(my_public_id,
                                                        contact_public_id,
                                                        true);
}

int LifeStuff::ConfirmContact(const std::string &my_public_id,
                              const std::string &contact_public_id) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in ConfirmContact.";
    return result;
  }

  result = lifestuff_elements->public_id->ConfirmContact(my_public_id,
                                                         contact_public_id,
                                                         true);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to Confirm Contact.";
    return result;
  }

  return lifestuff_elements->message_handler->SendPresenceMessage(
             my_public_id,
             contact_public_id,
             kOnline);
}

int LifeStuff::DeclineContact(const std::string &my_public_id,
                              const std::string &contact_public_id) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in DeclineContact.";
    return result;
  }

  return lifestuff_elements->public_id->ConfirmContact(my_public_id,
                                                       contact_public_id,
                                                       false);
}

int LifeStuff::RemoveContact(const std::string &my_public_id,
                             const std::string &contact_public_id) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in RemoveContact.";
    return result;
  }

  return lifestuff_elements->public_id->RemoveContact(my_public_id,
                                                      contact_public_id);
}

int LifeStuff::ChangeProfilePicture(
    const std::string &my_public_id,
    const std::string &profile_picture_contents) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in ChangeProfilePicture.";
    return result;
  }

  if (profile_picture_contents.empty() ||
      profile_picture_contents.size() > kFileRecontructionLimit) {
    DLOG(ERROR) << "Contents of picture inadequate("
                << profile_picture_contents.size() << "). Good day!";
    return kGeneralError;
  }

  // Write contents somewhere
  fs::path profile_picture_path(lifestuff_elements->user_storage->mount_dir() /
                                fs::path("/").make_preferred() /
                                std::string(my_public_id +
                                            "_profile_picture" +
                                            drive::kMsHidden.string()));
  if (WriteHiddenFile(profile_picture_path,
                      profile_picture_contents,
                      true) != kSuccess) {
    DLOG(ERROR) << "Failed to write profile picture file: "
                << profile_picture_path;
    return kGeneralError;
  }


  // Get datamap
  std::string data_map;
  std::string reconstructed;
  int count(0), limit(100);
  while (reconstructed != profile_picture_contents && count++ < limit) {
    data_map.clear();
    result = lifestuff_elements->user_storage->GetHiddenFileDataMap(
                profile_picture_path,
                &data_map);
    if ((result != kSuccess || data_map.empty()) && count == limit) {
      DLOG(ERROR) << "Failed obtaining DM of profile picture: " << result
                  << ", file: " << profile_picture_path;
      return result;
    }

    reconstructed = lifestuff_elements->user_storage->ConstructFile(data_map);
    Sleep(bptime::milliseconds(50));
  }

//   boost::system::error_code error_code;
//   int count(0), limit(100);
//   std::string reconstructed();
//   while (fs::file_size(profile_picture_path, error_code) !=
//          profile_picture_contents.size() && count++ < limit)
//     Sleep(bptime::milliseconds(50));
//   if (count == limit || error_code) {
//     DLOG(ERROR) << "Failed to obtain correct size after write, error_code: "
//                 << error_code.value() << ", count: " << count;
//     return kGeneralError;
//   }

  // Set in session
  lifestuff_elements->session->set_profile_picture_data_map(my_public_id,
                                                            data_map);

  // Message construction
  InboxItem message(kContactProfilePicture);
  message.sender_public_id = my_public_id;
  message.content.push_back(data_map);

  // Send to everybody
  lifestuff_elements->message_handler->SendEveryone(message);

  return kSuccess;
}

std::string LifeStuff::GetOwnProfilePicture(const std::string &my_public_id) {
  // Read contents, put them in a string, give them back. Should not be a file
  // over a certain size (kFileRecontructionLimit).
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in ChangeProfilePicture.";
    return "";
  }

  fs::path profile_picture_path(lifestuff_elements->user_storage->mount_dir() /
                                fs::path("/").make_preferred() /
                                std::string(my_public_id +
                                            "_profile_picture" +
                                            drive::kMsHidden.string()));
  std::string profile_picture_contents;
  if (ReadHiddenFile(profile_picture_path,
                     &profile_picture_contents) != kSuccess ||
      profile_picture_contents.empty()) {
    DLOG(ERROR) << "Failed reading profile picture: " << profile_picture_path;
    return "";
  }

  return profile_picture_contents;
}

std::string LifeStuff::GetContactProfilePicture(
    const std::string &my_public_id,
    const std::string &contact_public_id) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in GetContactProfilePicture.";
    return "";
  }

  // Look up data map in session.
  Contact contact;
  result = lifestuff_elements->session->contact_handler_map()
               [my_public_id]->ContactInfo(contact_public_id, &contact);
  if (result != kSuccess || contact.profile_picture_data_map.empty()) {
    DLOG(ERROR) << "No such contact(" << result << "): " << contact_public_id;
    return "";
  }

  // Read contents, put them in a string, give them back. Should not be
  // over a certain size (kFileRecontructionLimit).
  return lifestuff_elements->user_storage->ConstructFile(
            contact.profile_picture_data_map);
}

ContactMap LifeStuff::GetContacts(const std::string &my_public_id,
                                  uint16_t bitwise_status) {
  int result(PreContactChecks(lifestuff_elements->state,
                              my_public_id,
                              lifestuff_elements->session));
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed pre checks in GetContacts.";
    return ContactMap();
  }

  return lifestuff_elements->session->contact_handler_map()
             [my_public_id]->GetContacts(bitwise_status);
}

std::vector<std::string> LifeStuff::PublicIdsList() const {
  std::vector<std::string> public_ids;
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return public_ids;
  }


  // Retrieve all keys
  std::transform(lifestuff_elements->session->contact_handler_map().begin(),
                 lifestuff_elements->session->contact_handler_map().end(),
                 std::back_inserter(public_ids),
                 std::bind(&ContactHandlerMap::value_type::first, args::_1));


  return public_ids;
}

/// Messaging
int LifeStuff::SendChatMessage(const std::string &sender_public_id,
                               const std::string &receiver_public_id,
                               const std::string &message) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  if (message.size() > kMaxChatMessageSize) {
    DLOG(ERROR) << "Message too large: " << message.size();
    return kGeneralError;
  }

  InboxItem inbox_item(kChat);
  inbox_item.receiver_public_id = receiver_public_id;
  inbox_item.sender_public_id = sender_public_id;
  inbox_item.content.push_back(message);

  return lifestuff_elements->message_handler->Send(sender_public_id,
                                                   receiver_public_id,
                                                   inbox_item);
}

int LifeStuff::SendFile(const std::string &sender_public_id,
                        const std::string &receiver_public_id,
                        const fs::path absolute_path) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  std::string serialised_datamap;
  int result(lifestuff_elements->user_storage->GetDataMap(absolute_path,
                                                          &serialised_datamap));
  if (result != kSuccess || serialised_datamap.empty()) {
    DLOG(ERROR) << "Failed to get DM for " << absolute_path << ": " << result;
    return result;
  }

  InboxItem inbox_item(kFileTransfer);
  inbox_item.receiver_public_id = receiver_public_id;
  inbox_item.sender_public_id = sender_public_id;
  inbox_item.content.push_back(absolute_path.filename().string());
  inbox_item.content.push_back(serialised_datamap);

  result = lifestuff_elements->message_handler->Send(sender_public_id,
                                                     receiver_public_id,
                                                     inbox_item);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed to send message: " << result;
    return result;
  }

  return kSuccess;
}

int LifeStuff::AcceptSentFile(const fs::path absolute_path,
                              const std::string &identifier) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  std::string serialised_data_map;
  int result(
      lifestuff_elements->user_storage->ReadHiddenFile(
          mount_path() / fs::path("/").make_preferred() /
              std::string(identifier + drive::kMsHidden.string()),
          &serialised_data_map));
  if (result != kSuccess || serialised_data_map.empty()) {
    DLOG(ERROR) << "No such identifier found: " << result;
    return result == kSuccess ? kGeneralError : result;
  }

  drive::DataMapPtr data_map_ptr(ParseSerialisedDataMap(serialised_data_map));
  if (!data_map_ptr) {
    DLOG(ERROR) << "Corrupted DM in file";
    return kGeneralError;
  }

  result = lifestuff_elements->user_storage->InsertDataMap(absolute_path,
                                                           serialised_data_map);
  if (result != kSuccess) {
    DLOG(ERROR) << "Failed inserting DM: " << result;
    return result;
  }

  return kSuccess;
}

int LifeStuff::RejectSentFile(const std::string &identifier) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  fs::path hidden_file(mount_path() /
                       fs::path("/").make_preferred() /
                       std::string(identifier + drive::kMsHidden.string()));
  return lifestuff_elements->user_storage->DeleteHiddenFile(hidden_file);
}

/// Filesystem
int LifeStuff::ReadHiddenFile(const fs::path &absolute_path,
                              std::string *content) const {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  return lifestuff_elements->user_storage->ReadHiddenFile(absolute_path,
                                                          content);
}

int LifeStuff::WriteHiddenFile(const fs::path &absolute_path,
                               const std::string &content,
                               bool overwrite_existing) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  return lifestuff_elements->user_storage->WriteHiddenFile(absolute_path,
                                                           content,
                                                           overwrite_existing);
}

int LifeStuff::DeleteHiddenFile(const fs::path &absolute_path) {
  if (lifestuff_elements->state != kLoggedIn) {
    DLOG(ERROR) << "Wrong state: " << lifestuff_elements->state;
    return kGeneralError;
  }

  return lifestuff_elements->user_storage->DeleteHiddenFile(absolute_path);
}

///
int LifeStuff::state() const { return lifestuff_elements->state; }

fs::path LifeStuff::mount_path() const {
  return lifestuff_elements->user_storage->mount_dir();
}


}  // namespace lifestuff

}  // namespace maidsafe
