/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Class that Validates and creates messages for im
* Version:      1.0
* Created:      2010-04-13
* Revision:     none
* Compiler:     gcc
* Author:
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

//  #include <maidsafe/maidsafe-dht_config.h>

#include "maidsafe/common/crypto.h"
#include "maidsafe/lifestuff/client/imhandler.h"
#include "maidsafe/lifestuff/client/sessionsingleton.h"

namespace maidsafe {

namespace lifestuff {
//
//IMHandler::IMHandler() : ss_(SessionSingleton::getInstance()) {}
//
//std::string IMHandler::CreateMessage(const std::string &msg,
//                                     const std::string &receiver) {
//  maidsafe::BufferPacketMessage bpmsg;
//  bpmsg.set_sender_id(ss_->PublicUsername());
//  bpmsg.set_type(INSTANT_MSG);
//  std::string aes_key(RandomString(crypto::AES256_KeySize +
//                                   crypto::AES256_IVSize));
//  bpmsg.set_aesenc_message(AESEncrypt(msg, aes_key));
//  std::string rec_pub_key(ss_->GetContactPublicKey(receiver));
//  bpmsg.set_rsaenc_key(RSAEncrypt(aes_key, rec_pub_key));
//  std::string mpid_private;
//  ss_->MPublicID(NULL, NULL, &mpid_private, NULL);
//  GenericPacket gp;
//  gp.set_data(bpmsg.SerializeAsString());
//  gp.set_signature(RSASign(gp.data(), mpid_private));
//  return gp.SerializeAsString();
//}
//
//std::string IMHandler::CreateMessageEndpoint(const std::string &receiver) {
//  InstantMessage msg;
//  msg.set_sender(ss_->PublicUsername());
//  msg.set_message("");
//  msg.set_date(0/*GetDurationSinceEpoch()*/);
//  msg.set_status(ss_->ConnectionStatus());
//  EndPoint *endpoint = msg.mutable_endpoint();
//  *endpoint = ss_->Ep();
//  std::string ser_msg(msg.SerializeAsString());
//
//  BufferPacketMessage bpmsg;
//  bpmsg.set_sender_id(ss_->PublicUsername());
//  bpmsg.set_type(HELLO_PING);
//  std::string aes_key(RandomString(crypto::AES256_KeySize +
//                                   crypto::AES256_IVSize));
//  bpmsg.set_aesenc_message(AESEncrypt(ser_msg, aes_key));
//  std::string rec_pub_key(ss_->GetContactPublicKey(receiver));
//  bpmsg.set_rsaenc_key(RSAEncrypt(aes_key, rec_pub_key));
//
//  std::string mpid_private;
//  ss_->MPublicID(NULL, NULL, &mpid_private, NULL);
//  GenericPacket gp;
//  gp.set_data(bpmsg.SerializeAsString());
//  gp.set_signature(RSASign(gp.data(), mpid_private));
//  return gp.SerializeAsString();
//}
//
//std::string IMHandler::CreateLogOutMessage(const std::string &receiver) {
//  InstantMessage msg;
//  msg.set_sender(ss_->PublicUsername());
//  msg.set_message("");
//  msg.set_date(/*GetDurationSinceEpoch()*/0);
//  msg.set_status(ss_->ConnectionStatus());
//  std::string ser_msg(msg.SerializeAsString());
//
//  BufferPacketMessage bpmsg;
//  bpmsg.set_sender_id(ss_->PublicUsername());
//  bpmsg.set_type(LOGOUT_PING);
//  std::string aes_key(RandomString(crypto::AES256_KeySize +
//                                   crypto::AES256_IVSize));
//  bpmsg.set_aesenc_message(AESEncrypt(ser_msg, aes_key));
//  std::string rec_pub_key(ss_->GetContactPublicKey(receiver));
//  bpmsg.set_rsaenc_key(RSAEncrypt(aes_key, rec_pub_key));
//
//  std::string mpid_private;
//  ss_->MPublicID(NULL, NULL, &mpid_private, NULL);
//  GenericPacket gp;
//  gp.set_data(bpmsg.SerializeAsString());
//  gp.set_signature(RSASign(gp.data(), mpid_private));
//  return gp.SerializeAsString();
//}
//
//bool IMHandler::ValidateMessage(const std::string &ser_msg,
//                                MessageType *type,
//                                std::string *validated_msg) {
//  validated_msg->clear();
//  GenericPacket gp;
//  if (!gp.ParseFromString(ser_msg)) {
//    return false;
//  }
//  BufferPacketMessage bpmsg;
//  if (!bpmsg.ParseFromString(gp.data())) {
//    return false;
//  }
//  std::string send_pub_key(ss_->GetContactPublicKey(bpmsg.sender_id()));
//  if (!RSACheckSignedData(gp.data(), gp.signature(), send_pub_key)) {
//    return false;
//  }
//
//  std::string mpid_private;
//  ss_->MPublicID(NULL, NULL, &mpid_private, NULL);
//  std::string aes_key(RSADecrypt(bpmsg.rsaenc_key(), mpid_private));
//  *validated_msg = AESDecrypt(bpmsg.aesenc_message(), aes_key);
//  *type = bpmsg.type();
//  return true;
//}
//
}  // namespace lifestuff

}  // namespace maidsafe
