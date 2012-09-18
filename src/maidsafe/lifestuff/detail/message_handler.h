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

#ifndef MAIDSAFE_LIFESTUFF_DETAIL_MESSAGE_HANDLER_H_
#define MAIDSAFE_LIFESTUFF_DETAIL_MESSAGE_HANDLER_H_


#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "boost/signals2/signal.hpp"

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/date_time/posix_time/posix_time_duration.hpp"

#include "maidsafe/private/chunk_actions/appendable_by_all_pb.h"
#include "maidsafe/private/chunk_store/remote_chunk_store.h"

#include "maidsafe/passport/passport_config.h"

#include "maidsafe/lifestuff/lifestuff.h"
#include "maidsafe/lifestuff/detail/data_atlas_pb.h"
#include "maidsafe/lifestuff/detail/message_handler_signal_types.h"
#include "maidsafe/lifestuff/detail/utils.h"

namespace ba = boost::asio;
namespace bptime = boost::posix_time;
namespace bs2 = boost::signals2;
namespace pcs = maidsafe::priv::chunk_store;

namespace maidsafe {

namespace passport {
class Passport;
}  // namespace passport

namespace lifestuff {

class Message;
class Session;

class MessageHandler {
 public:
  typedef std::map<std::string, uint64_t> ReceivedMessagesMap;

  MessageHandler(std::shared_ptr<pcs::RemoteChunkStore> remote_chunk_store,
                 Session& session,
                 ba::io_service& asio_service);
  ~MessageHandler();

  void StartUp(bptime::seconds interval);
  void ShutDown();

  // Periodically retrieves saved messages from MMID and fires
  // new_message_signal_ for each valid message retrieved.  Checking will only
  // succeed if at least one public username has been successfully created.
  int StartCheckingForNewMessages(boost::posix_time::seconds interval);
  void StopCheckingForNewMessages();

  int Send(const InboxItem& message);
  int SendPresenceMessage(const std::string& own_public_id,
                          const std::string& recipient_public_id,
                          const ContactPresence& presence);
  void InformConfirmedContactOnline(const std::string& own_public_id,
                                    const std::string& recipient_public_id);
  void SendEveryone(const InboxItem& message);

  // Extra library connections
  bs2::connection ConnectToChatSignal(const ChatFunction& function);
  bs2::connection ConnectToFileTransferSignal(const FileTransferFunction& function);
  bs2::connection ConnectToPrivateShareInvitationSignal(
      const PrivateShareInvitationFunction& function);
  bs2::connection ConnectToPrivateShareDeletionSignal(const PrivateShareDeletionFunction& function);
  bs2::connection ConnectToPrivateMemberAccessChangeSignal(
      const PrivateMemberAccessChangeFunction& function);
  bs2::connection ConnectToContactPresenceSignal(const ContactPresenceFunction& function);

  bs2::connection ConnectToContactProfilePictureSignal(
      const ContactProfilePictureFunction& function);

  // Intra and extra library connections
  bs2::connection ConnectToPrivateShareUserLeavingSignal(
      const PrivateShareUserLeavingSignal::slot_type& function);
  bs2::connection ConnectToShareInvitationResponseSignal(
      const ShareInvitationResponseFunction& function);

  // Intra library connections
  bs2::connection ConnectToPrivateShareDetailsSignal(
      boost::function<int(const std::string& share_id, fs::path* relative_path)> function);  // NOLINT (Dan)
//      const PrivateShareDetailsSignal::slot_type& function);
  bs2::connection ConnectToParseAndSaveDataMapSignal(
      const ParseAndSaveDataMapSignal::slot_type& function);
  bs2::connection ConnectToPrivateShareUpdateSignal(
      const PrivateShareUpdateSignal::slot_type& function);
  bs2::connection ConnectToPrivateMemberAccessLevelSignal(
      const PrivateMemberAccessLevelSignal::slot_type& function);
  bs2::connection ConnectToSavePrivateShareDataSignal(
      const SavePrivateShareDataSignal::slot_type& function);
  bs2::connection ConnectToDeletePrivateShareDataSignal(
      const DeletePrivateShareDataSignal::slot_type& function);

 private:
  MessageHandler(const MessageHandler&);
  MessageHandler& operator=(const MessageHandler&);

  bool ProtobufToInbox(const Message& message, InboxItem* inbox_item) const;
  bool InboxToProtobuf(const InboxItem& inbox_item, Message* message) const;
  void GetNewMessages(const bptime::seconds& interval, const boost::system::error_code& error_code);
  void ProcessRetrieved(const std::string& public_id, const std::string& retrieved_mmid_packet);
  void RetrieveMessagesForAllIds();
  bool MessagePreviouslyReceived(const std::string& message);
  void ClearExpiredReceivedMessages();
  void EnqueuePresenceMessages(ContactPresence presence);

  void ProcessContactPresence(const InboxItem& presence_message);
  void ProcessContactProfilePicture(const InboxItem& profile_message);
  void ProcessFileTransfer(const InboxItem& file_transfer_message);
  void ProcessShareInvitationResponse(const InboxItem& inbox_item);
  void ProcessPrivateShare(const InboxItem& private_share_message);

  void ContentsDontParseAsDataMap(const std::string& serialised_dm, std::string* data_map);
  void ProcessPresenceMessages();

  std::shared_ptr<pcs::RemoteChunkStore> remote_chunk_store_;
  Session& session_;
  passport::Passport& passport_;
  ba::deadline_timer get_new_messages_timer_;
  bool get_new_messages_timer_active_;
  ba::io_service& asio_service_;
  bool start_up_done_;
  ReceivedMessagesMap received_messages_;

  /// Extra library signals
  ChatMessageSignal chat_signal_;
  FileTransferSignal file_transfer_signal_;
  ContactPresenceSignal contact_presence_signal_;
  ContactProfilePictureSignal contact_profile_picture_signal_;
  PrivateShareInvitationSignal private_share_invitation_signal_;
  PrivateShareDeletionSignal private_share_deletion_signal_;
  PrivateMemberAccessChangeSignal private_member_access_change_signal_;
  ShareInvitationResponseSignal share_invitation_response_signal_;

  /// Intra and extra library signals
  PrivateShareUserLeavingSignal private_share_user_leaving_signal_;

  /// Intra library signals
  ParseAndSaveDataMapSignal parse_and_save_data_map_signal_;
  PrivateShareDetailsSignal private_share_details_signal_;
  PrivateShareUpdateSignal private_share_update_signal_;
  PrivateMemberAccessLevelSignal private_member_access_level_signal_;
  SavePrivateShareDataSignal save_private_share_data_signal_;
  DeletePrivateShareDataSignal delete_private_share_data_signal_;
};

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_DETAIL_MESSAGE_HANDLER_H_
