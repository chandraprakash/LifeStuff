/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Unit tests for Passport class
* Version:      1.0
* Created:      2010-10-19-23.59.27
* Revision:     none
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

#include "boost/lexical_cast.hpp"
#include "gtest/gtest.h"
#include "maidsafe-dht/common/utils.h"
#include "maidsafe/passport/passport.h"

namespace maidsafe {

namespace passport {

namespace test {

const boost::uint16_t kRsaKeySize(4096);
const boost::uint8_t kMaxThreadCount(5);

class PassportTest : public testing::Test {
 public:
  PassportTest()
      : passport_(kRsaKeySize, kMaxThreadCount),
        kUsername_(RandomAlphaNumericString(15)),
        kPin_(boost::lexical_cast<std::string>(RandomUint32())),
        kPassword_(RandomAlphaNumericString(20)),
        kPlainTextMasterData_(RandomString(10000)),
        mid_name_(),
        smid_name_() {}
 protected:
  typedef std::shared_ptr<pki::Packet> PacketPtr;
  typedef std::shared_ptr<MidPacket> MidPtr;
  typedef std::shared_ptr<TmidPacket> TmidPtr;
  typedef std::shared_ptr<SignaturePacket> SignaturePtr;
  void SetUp() {
    passport_.Init();
  }
  void TearDown() {}
  bool CreateUser(MidPtr mid, MidPtr smid, TmidPtr tmid) {
    if (!mid || !smid || !tmid)
      return false;
    SignaturePtr sig_packet(new SignaturePacket);
    bool result =
        passport_.InitialiseSignaturePacket(ANMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess &&
        passport_.InitialiseSignaturePacket(ANSMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess &&
        passport_.InitialiseSignaturePacket(ANTMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess;
    if (!result)
      return false;
    if (passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_, &smid_name_)
        != kSuccess)
      return false;
    if (passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid, smid,
                                 tmid) != kSuccess)
      return false;
    if (passport_.ConfirmNewUserData(mid, smid, tmid) != kSuccess)
      return false;
    return passport_.GetPacket(MID, true).get() && 
           passport_.GetPacket(SMID, true).get() &&
           passport_.GetPacket(TMID, true).get();
  }
  Passport passport_;
  const std::string kUsername_, kPin_, kPassword_, kPlainTextMasterData_;
  std::string mid_name_, smid_name_;
};

TEST_F(PassportTest, BEH_PASSPORT_SignaturePacketFunctions) {
  EXPECT_EQ(kNullPointer,
            passport_.InitialiseSignaturePacket(ANMID, SignaturePtr()));

  SignaturePtr signature_packet(new SignaturePacket);
  EXPECT_EQ(kPassportError,
            passport_.InitialiseSignaturePacket(MID, signature_packet));
  EXPECT_TRUE(signature_packet->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());

  EXPECT_EQ(kNoSigningPacket,
            passport_.InitialiseSignaturePacket(MAID, signature_packet));
  EXPECT_TRUE(signature_packet->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, true).get());

  SignaturePtr anmaid1(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid1));
  EXPECT_FALSE(anmaid1->name().empty());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false)->Equals(anmaid1.get()));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());

  SignaturePtr anmaid2(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid2));
  EXPECT_FALSE(anmaid2->name().empty());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false)->Equals(anmaid2.get()));
  EXPECT_FALSE(anmaid1->Equals(anmaid2.get()));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());

  EXPECT_EQ(kNoSigningPacket,
            passport_.InitialiseSignaturePacket(MAID, signature_packet));
  EXPECT_TRUE(signature_packet->name().empty());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, true).get());

  EXPECT_EQ(kPassportError, passport_.ConfirmSignaturePacket(SignaturePtr()));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());

  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmSignaturePacket(anmaid1));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());

  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(anmaid2));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));

  SignaturePtr anmaid3(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_FALSE(anmaid3->name().empty());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false)->Equals(anmaid3.get()));
  EXPECT_FALSE(anmaid2->Equals(anmaid3.get()));
  ASSERT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));

  EXPECT_TRUE(passport_.SignaturePacketName(MID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKey(MID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPrivateKey(MID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKeySignature(MID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketName(MID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKey(MID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPrivateKey(MID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKeySignature(MID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketName(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKey(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPrivateKey(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKeySignature(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketName(MAID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKey(MAID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPrivateKey(MAID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKeySignature(MAID, true).empty());
  EXPECT_EQ(anmaid3->name(), passport_.SignaturePacketName(ANMAID, false));
  EXPECT_EQ(anmaid3->value(),
            passport_.SignaturePacketPublicKey(ANMAID, false));
  EXPECT_EQ(anmaid3->private_key(),
            passport_.SignaturePacketPrivateKey(ANMAID, false));
  EXPECT_EQ(anmaid3->public_key_signature(),
            passport_.SignaturePacketPublicKeySignature(ANMAID, false));
  EXPECT_EQ(anmaid2->name(), passport_.SignaturePacketName(ANMAID, true));
  EXPECT_EQ(anmaid2->value(), passport_.SignaturePacketPublicKey(ANMAID, true));
  EXPECT_EQ(anmaid2->private_key(),
            passport_.SignaturePacketPrivateKey(ANMAID, true));
  EXPECT_EQ(anmaid2->public_key_signature(),
            passport_.SignaturePacketPublicKeySignature(ANMAID, true));

  EXPECT_EQ(kPassportError, passport_.RevertSignaturePacket(MID));
  EXPECT_EQ(kPassportError, passport_.RevertSignaturePacket(MAID));
  EXPECT_EQ(kSuccess, passport_.RevertSignaturePacket(ANMAID));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));

  SignaturePtr maid(new SignaturePacket);
  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  std::string original_maid_name(maid->name());
  EXPECT_FALSE(original_maid_name.empty());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));
  ASSERT_TRUE(passport_.GetPacket(MAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MAID, false)->Equals(maid.get()));
  EXPECT_FALSE(passport_.GetPacket(MAID, true).get());

  EXPECT_EQ(kSuccess, passport_.RevertSignaturePacket(MAID));
  EXPECT_FALSE(passport_.GetPacket(MAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, true).get());

  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  EXPECT_FALSE(maid->name().empty());
  EXPECT_NE(original_maid_name, maid->name());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));
  ASSERT_TRUE(passport_.GetPacket(MAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MAID, false)->Equals(maid.get()));
  EXPECT_FALSE(passport_.GetPacket(MAID, true).get());

  EXPECT_EQ(kNoPacket, passport_.DeletePacket(MID));
  EXPECT_EQ(kSuccess, passport_.DeletePacket(MAID));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, true).get());

  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MAID, false).get());

  passport_.Clear();
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, false).get());

  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(anmaid3));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MAID, false).get());

  passport_.ClearKeyring();
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MAID, false).get());
}

TEST_F(PassportTest, BEH_PASSPORT_MpidFunctions) {
  const std::string kPublicName(RandomAlphaNumericString(10));
  EXPECT_EQ(kNullPointer,
            passport_.InitialiseMpid(kPublicName, SignaturePtr()));

  SignaturePtr mpid(new SignaturePacket);
  EXPECT_EQ(kNoSigningPacket, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_TRUE(mpid->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MPID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  SignaturePtr anmpid(new SignaturePacket);
  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(ANMPID, anmpid));
  EXPECT_FALSE(anmpid->name().empty());
  ASSERT_TRUE(passport_.GetPacket(ANMPID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, false)->Equals(anmpid.get()));
  EXPECT_FALSE(passport_.GetPacket(ANMPID, true).get());

  EXPECT_EQ(kNoSigningPacket, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_TRUE(mpid->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MPID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(anmpid));
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));

  EXPECT_EQ(kPassportError, passport_.InitialiseSignaturePacket(MPID, mpid));
  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName, mpid));
  std::string original_mpid_name(mpid->name());
  EXPECT_FALSE(original_mpid_name.empty());
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));
  ASSERT_TRUE(passport_.GetPacket(MPID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MPID, false)->Equals(mpid.get()));
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  EXPECT_EQ(kSuccess, passport_.RevertSignaturePacket(MPID));
  EXPECT_FALSE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_FALSE(mpid->name().empty());
  EXPECT_EQ(original_mpid_name, mpid->name());
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));
  ASSERT_TRUE(passport_.GetPacket(MPID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MPID, false)->Equals(mpid.get()));
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  EXPECT_EQ(kSuccess, passport_.DeletePacket(MPID));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MPID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  SignaturePtr other_mpid(new SignaturePacket);
  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName + "a", other_mpid));
  EXPECT_FALSE(other_mpid->name().empty());
  EXPECT_NE(original_mpid_name, other_mpid->name());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MPID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmSignaturePacket(mpid));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MPID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MPID, true).get());

  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(mpid));
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false).get());
  ASSERT_TRUE(passport_.GetPacket(ANMPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));
  EXPECT_FALSE(passport_.GetPacket(MPID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MPID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MPID, true)->Equals(mpid.get()));
  EXPECT_EQ(original_mpid_name, mpid->name());
}

TEST_F(PassportTest, BEH_PASSPORT_SetInitialDetails) {
  // Invalid data and null pointers
  std::string invalid_pin("Non-numerical");
  mid_name_ = smid_name_ = "a";
  EXPECT_EQ(kNullPointer, passport_.SetInitialDetails(kUsername_, kPin_, NULL,
                                                      &smid_name_));
  EXPECT_EQ(kNullPointer, passport_.SetInitialDetails(kUsername_, kPin_,
                                                      &mid_name_, NULL));

  EXPECT_EQ(kPassportError,
            passport_.SetInitialDetails(kUsername_, invalid_pin, &mid_name_,
                                        &smid_name_));
  EXPECT_TRUE(mid_name_.empty());
  EXPECT_TRUE(smid_name_.empty());

  mid_name_ = smid_name_ = "a";
  EXPECT_EQ(kPassportError, passport_.SetInitialDetails("", kPin_, &mid_name_,
                                                        &smid_name_));
  EXPECT_TRUE(mid_name_.empty());
  EXPECT_TRUE(smid_name_.empty());

  // Good initial data
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_FALSE(mid_name_.empty());
  EXPECT_FALSE(smid_name_.empty());
  EXPECT_NE(mid_name_, smid_name_);
  PacketPtr pending_mid(passport_.GetPacket(MID, false));
  PacketPtr pending_smid(passport_.GetPacket(SMID, false));
  PacketPtr confirmed_mid(passport_.GetPacket(MID, true));
  PacketPtr confirmed_smid(passport_.GetPacket(SMID, true));
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_EQ(mid_name_, pending_mid->name());
  EXPECT_EQ(smid_name_, pending_smid->name());

  // Different username should generate different mid and smid
  std::string different_username(kUsername_ + "a");
  std::string different_username_mid_name, different_username_smid_name;
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(different_username, kPin_,
                                        &different_username_mid_name,
                                        &different_username_smid_name));
  EXPECT_FALSE(different_username_mid_name.empty());
  EXPECT_FALSE(different_username_smid_name.empty());
  EXPECT_NE(different_username_mid_name, different_username_smid_name);
  EXPECT_NE(mid_name_, different_username_mid_name);
  EXPECT_NE(smid_name_, different_username_mid_name);
  EXPECT_NE(mid_name_, different_username_smid_name);
  EXPECT_NE(smid_name_, different_username_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_EQ(different_username_mid_name, pending_mid->name());
  EXPECT_EQ(different_username_smid_name, pending_smid->name());

  // Different pin should generate different mid and smid
  std::string different_pin(boost::lexical_cast<std::string>(
                            boost::lexical_cast<boost::uint32_t>(kPin_) + 1));
  std::string different_pin_mid_name, different_pin_smid_name;
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(kUsername_, different_pin,
                                        &different_pin_mid_name,
                                        &different_pin_smid_name));
  EXPECT_FALSE(different_pin_mid_name.empty());
  EXPECT_FALSE(different_pin_smid_name.empty());
  EXPECT_NE(different_pin_mid_name, different_pin_smid_name);
  EXPECT_NE(mid_name_, different_pin_mid_name);
  EXPECT_NE(smid_name_, different_pin_mid_name);
  EXPECT_NE(mid_name_, different_pin_smid_name);
  EXPECT_NE(smid_name_, different_pin_smid_name);
  EXPECT_NE(different_username_mid_name, different_pin_mid_name);
  EXPECT_NE(different_username_smid_name, different_pin_mid_name);
  EXPECT_NE(different_username_mid_name, different_pin_smid_name);
  EXPECT_NE(different_username_smid_name, different_pin_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_EQ(different_pin_mid_name, pending_mid->name());
  EXPECT_EQ(different_pin_smid_name, pending_smid->name());

  // Different username & pin should generate different mid and smid
  std::string different_both_mid_name, different_both_smid_name;
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(different_username, different_pin,
                                        &different_both_mid_name,
                                        &different_both_smid_name));
  EXPECT_FALSE(different_both_mid_name.empty());
  EXPECT_FALSE(different_both_smid_name.empty());
  EXPECT_NE(different_both_mid_name, different_both_smid_name);
  EXPECT_NE(mid_name_, different_both_mid_name);
  EXPECT_NE(smid_name_, different_both_mid_name);
  EXPECT_NE(mid_name_, different_both_smid_name);
  EXPECT_NE(smid_name_, different_both_smid_name);
  EXPECT_NE(different_username_mid_name, different_both_mid_name);
  EXPECT_NE(different_username_smid_name, different_both_mid_name);
  EXPECT_NE(different_username_mid_name, different_both_smid_name);
  EXPECT_NE(different_username_smid_name, different_both_smid_name);
  EXPECT_NE(different_pin_mid_name, different_both_mid_name);
  EXPECT_NE(different_pin_smid_name, different_both_mid_name);
  EXPECT_NE(different_pin_mid_name, different_both_smid_name);
  EXPECT_NE(different_pin_smid_name, different_both_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_EQ(different_both_mid_name, pending_mid->name());
  EXPECT_EQ(different_both_smid_name, pending_smid->name());

  // Original username & pin should generate original mid and smid
  std::string original_mid_name, original_smid_name;
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_,
                                                  &original_mid_name,
                                                  &original_smid_name));
  EXPECT_EQ(mid_name_, original_mid_name);
  EXPECT_EQ(smid_name_, original_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_EQ(mid_name_, pending_mid->name());
  EXPECT_EQ(smid_name_, pending_smid->name());
}

TEST_F(PassportTest, BEH_PASSPORT_SetNewUserData) {
  // Invalid data and null pointers
  MidPtr null_mid, mid(new MidPacket), null_smid, smid(new MidPacket);
  TmidPtr null_tmid, tmid(new TmidPacket);

  EXPECT_EQ(kNoMid,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(smid->name().empty());
  EXPECT_TRUE(tmid->name().empty());

  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kSuccess, passport_.packet_handler_.DeletePacket(SMID));
  EXPECT_EQ(kNoSmid,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(smid->name().empty());
  EXPECT_TRUE(tmid->name().empty());

  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kNullPointer,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     null_mid, smid, tmid));
  EXPECT_TRUE(smid->name().empty());
  EXPECT_TRUE(tmid->name().empty());
  EXPECT_EQ(kNullPointer,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     mid, null_smid, tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(tmid->name().empty());
  EXPECT_EQ(kNullPointer,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     mid, smid, null_tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(smid->name().empty());

  // Good initial data
  EXPECT_EQ(kSuccess,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  MidPtr pending_mid(std::static_pointer_cast<MidPacket>(
                     passport_.GetPacket(MID, false)));
  MidPtr pending_smid(std::static_pointer_cast<MidPacket>(
                      passport_.GetPacket(SMID, false)));
  TmidPtr pending_tmid(std::static_pointer_cast<TmidPacket>(
                       passport_.GetPacket(TMID, false)));
  MidPtr confirmed_mid(std::static_pointer_cast<MidPacket>(
                       passport_.GetPacket(MID, true)));
  MidPtr confirmed_smid(std::static_pointer_cast<MidPacket>(
                        passport_.GetPacket(SMID, true)));
  TmidPtr confirmed_tmid(std::static_pointer_cast<TmidPacket>(
                         passport_.GetPacket(TMID, true)));
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  ASSERT_TRUE(pending_tmid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_FALSE(confirmed_tmid.get());
  std::string mid_name(pending_mid->name()), smid_name(pending_smid->name());
  std::string tmid_name(pending_tmid->name());
  EXPECT_FALSE(mid_name.empty());
  EXPECT_FALSE(smid_name.empty());
  EXPECT_FALSE(tmid_name.empty());
  EXPECT_TRUE(pending_mid->Equals(mid.get()));
  EXPECT_TRUE(pending_smid->Equals(smid.get()));
  EXPECT_TRUE(pending_tmid->Equals(tmid.get()));
  EXPECT_EQ(kUsername_, pending_mid->username());
  EXPECT_EQ(kUsername_, pending_smid->username());
  EXPECT_EQ(kUsername_, pending_tmid->username());
  EXPECT_EQ(kPin_, pending_mid->pin());
  EXPECT_EQ(kPin_, pending_smid->pin());
  EXPECT_EQ(kPin_, pending_tmid->pin());
  std::string rid(pending_mid->rid());
  EXPECT_FALSE(rid.empty());
  EXPECT_EQ(rid, pending_smid->rid());
  EXPECT_EQ(kPassword_, pending_tmid->password());
  // Check *copies* of pointers are returned
  EXPECT_EQ(1UL, mid.use_count());
  EXPECT_EQ(1UL, smid.use_count());
  EXPECT_EQ(1UL, tmid.use_count());

  // Check retry with same data generates new rid and hence new tmid name
  MidPtr retry_mid(new MidPacket), retry_smid(new MidPacket);
  TmidPtr retry_tmid(new TmidPacket);
  EXPECT_EQ(kSuccess,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     retry_mid, retry_smid, retry_tmid));
  pending_mid = std::static_pointer_cast<MidPacket>(
                passport_.GetPacket(MID, false));
  pending_smid = std::static_pointer_cast<MidPacket>(
                 passport_.GetPacket(SMID, false));
  pending_tmid = std::static_pointer_cast<TmidPacket>(
                 passport_.GetPacket(TMID, false));
  confirmed_mid = std::static_pointer_cast<MidPacket>(
                  passport_.GetPacket(MID, true));
  confirmed_smid = std::static_pointer_cast<MidPacket>(
                   passport_.GetPacket(SMID, true));
  confirmed_tmid = std::static_pointer_cast<TmidPacket>(
                   passport_.GetPacket(TMID, true));
  ASSERT_TRUE(pending_mid.get());
  ASSERT_TRUE(pending_smid.get());
  ASSERT_TRUE(pending_tmid.get());
  EXPECT_FALSE(confirmed_mid.get());
  EXPECT_FALSE(confirmed_smid.get());
  EXPECT_FALSE(confirmed_tmid.get());
  EXPECT_EQ(mid_name, pending_mid->name());
  EXPECT_EQ(smid_name, pending_smid->name());
  EXPECT_NE(tmid_name, pending_tmid->name());
  EXPECT_FALSE(pending_tmid->name().empty());
  EXPECT_TRUE(pending_mid->Equals(retry_mid.get()));
  EXPECT_TRUE(pending_smid->Equals(retry_smid.get()));
  EXPECT_TRUE(pending_tmid->Equals(retry_tmid.get()));
  EXPECT_FALSE(pending_mid->Equals(mid.get()));
  EXPECT_FALSE(pending_smid->Equals(smid.get()));
  EXPECT_FALSE(pending_tmid->Equals(tmid.get()));
  EXPECT_EQ(kUsername_, pending_mid->username());
  EXPECT_EQ(kUsername_, pending_smid->username());
  EXPECT_EQ(kUsername_, pending_tmid->username());
  EXPECT_EQ(kPin_, pending_mid->pin());
  EXPECT_EQ(kPin_, pending_smid->pin());
  EXPECT_EQ(kPin_, pending_tmid->pin());
  EXPECT_FALSE(pending_mid->rid().empty());
  EXPECT_NE(rid, pending_mid->rid());
  EXPECT_EQ(pending_mid->rid(), pending_smid->rid());
  EXPECT_EQ(kPassword_, pending_tmid->password());
}

TEST_F(PassportTest, BEH_PASSPORT_ConfirmNewUserData) {
  MidPtr null_mid, different_username_mid(new MidPacket);
  MidPtr null_smid, different_username_smid(new MidPacket);
  TmidPtr null_tmid, different_username_tmid(new TmidPacket);
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails("Different", kPin_,
                                                  &mid_name_, &smid_name_));
  EXPECT_EQ(kSuccess, passport_.SetNewUserData(kPassword_,
                      kPlainTextMasterData_, different_username_mid,
                      different_username_smid, different_username_tmid));
  MidPtr mid(new MidPacket), smid(new MidPacket);
  TmidPtr tmid(new TmidPacket);
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kSuccess,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  MidPtr pending_mid(std::static_pointer_cast<MidPacket>(
                     passport_.GetPacket(MID, false)));
  MidPtr pending_smid(std::static_pointer_cast<MidPacket>(
                      passport_.GetPacket(SMID, false)));
  TmidPtr pending_tmid(std::static_pointer_cast<TmidPacket>(
                       passport_.GetPacket(TMID, false)));
  EXPECT_TRUE(pending_mid.get());
  EXPECT_TRUE(pending_smid.get());
  EXPECT_TRUE(pending_tmid.get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kNullPointer, passport_.ConfirmNewUserData(null_mid, smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kNullPointer, passport_.ConfirmNewUserData(mid, null_smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kNullPointer, passport_.ConfirmNewUserData(mid, smid, null_tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kMissingDependentPackets,
            passport_.ConfirmNewUserData(mid, smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  SignaturePtr signature_packet(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMID, signature_packet));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(signature_packet));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANSMID, signature_packet));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(signature_packet));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANTMID, signature_packet));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(signature_packet));

  EXPECT_EQ(kPacketsNotEqual,
            passport_.ConfirmNewUserData(different_username_mid, smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kPacketsNotEqual,
            passport_.ConfirmNewUserData(mid, different_username_smid, tmid));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kPacketsNotEqual,
            passport_.ConfirmNewUserData(mid, smid, different_username_tmid));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());

  EXPECT_EQ(kSuccess, passport_.ConfirmNewUserData(mid, smid, tmid));
  MidPtr confirmed_mid(std::static_pointer_cast<MidPacket>(
                       passport_.GetPacket(MID, true)));
  MidPtr confirmed_smid(std::static_pointer_cast<MidPacket>(
                        passport_.GetPacket(SMID, true)));
  TmidPtr confirmed_tmid(std::static_pointer_cast<TmidPacket>(
                         passport_.GetPacket(TMID, true)));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(confirmed_mid.get());
  EXPECT_TRUE(confirmed_smid.get());
  EXPECT_TRUE(confirmed_tmid.get());

  EXPECT_TRUE(mid->Equals(pending_mid.get()));
  EXPECT_TRUE(smid->Equals(pending_smid.get()));
  EXPECT_TRUE(tmid->Equals(pending_tmid.get()));
  EXPECT_TRUE(mid->Equals(confirmed_mid.get()));
  EXPECT_TRUE(smid->Equals(confirmed_smid.get()));
  EXPECT_TRUE(tmid->Equals(confirmed_tmid.get()));

  EXPECT_EQ(kSuccess, passport_.ConfirmNewUserData(mid, smid, tmid));
}

TEST_F(PassportTest, BEH_PASSPORT_UpdateMasterData) {
  // Setup
  MidPtr original_mid(new MidPacket), original_smid(new MidPacket);
  TmidPtr original_tmid(new TmidPacket);
  MidPtr null_mid, different_smid(new MidPacket(kUsername_ + "a", kPin_, "1"));
  TmidPtr null_tmid;
  std::string updated_master_data1(RandomString(10000));
  std::string mid_old_value, smid_old_value;
  MidPtr updated_mid1(new MidPacket), updated_smid1(new MidPacket);
  TmidPtr new_tmid1(new TmidPacket), tmid_for_deletion1(new TmidPacket);

  // Invalid data and null pointers
  ASSERT_TRUE(CreateUser(original_mid, original_smid, original_tmid));
  ASSERT_EQ(kSuccess, passport_.DeletePacket(TMID));
  EXPECT_EQ(kNoTmid, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  ASSERT_EQ(kSuccess, passport_.DeletePacket(SMID));
  EXPECT_EQ(kNoSmid, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  passport_.Clear();
  EXPECT_EQ(kNoMid, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));

  ASSERT_TRUE(CreateUser(original_mid, original_smid, original_tmid));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1, NULL,
            &smid_old_value, updated_mid1, updated_smid1, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, NULL, updated_mid1, updated_smid1, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, null_mid, updated_smid1, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, null_mid, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            null_tmid, tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, null_tmid));
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(original_mid.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(original_smid.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(original_tmid.get()));

  // Good initial data
  *tmid_for_deletion1 = *original_tmid;
  EXPECT_FALSE(tmid_for_deletion1->name().empty());
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  EXPECT_EQ(original_mid->value(), mid_old_value);
  EXPECT_EQ(original_smid->value(), smid_old_value);
  ASSERT_TRUE(updated_mid1.get());
  ASSERT_TRUE(updated_smid1.get());
  ASSERT_TRUE(new_tmid1.get());
  ASSERT_TRUE(tmid_for_deletion1.get());
  EXPECT_EQ(1UL, updated_mid1.use_count());
  EXPECT_EQ(1UL, updated_smid1.use_count());
  EXPECT_EQ(1UL, new_tmid1.use_count());
  EXPECT_EQ(1UL, tmid_for_deletion1.use_count());
  EXPECT_TRUE(tmid_for_deletion1->name().empty());
  ASSERT_TRUE(passport_.GetPacket(MID, false).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MID, false)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, false)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, false)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, false)->value(), original_tmid->value());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(original_mid.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(original_smid.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(original_tmid.get()));
  // As first ever update for this user, original SMID == updated SMID
  EXPECT_TRUE(original_smid->Equals(updated_smid1.get()));

  // Bad confirm
  EXPECT_EQ(kNullPointer, passport_.ConfirmMasterDataUpdate(null_mid,
                          updated_smid1, new_tmid1));
  EXPECT_EQ(kNullPointer, passport_.ConfirmMasterDataUpdate(updated_mid1,
                          null_mid, new_tmid1));
  EXPECT_EQ(kNullPointer, passport_.ConfirmMasterDataUpdate(updated_mid1,
                          updated_smid1, null_tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmMasterDataUpdate(original_mid,
                              updated_smid1, new_tmid1));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmMasterDataUpdate(updated_mid1,
                              different_smid, new_tmid1));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmMasterDataUpdate(updated_mid1,
                              updated_smid1, original_tmid));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());

  // Confirm to populate STMID
  EXPECT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(updated_mid1,
                      updated_smid1, new_tmid1));
  EXPECT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(updated_mid1,
                      updated_smid1, new_tmid1));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());

  // Retry with same data
  std::string updated_master_data2(RandomString(10000));
  MidPtr updated_mid2(new MidPacket), updated_smid2(new MidPacket);
  TmidPtr new_tmid2(new TmidPacket), tmid_for_deletion2(new TmidPacket);
  *tmid_for_deletion2 = *original_tmid;
  EXPECT_FALSE(tmid_for_deletion2->name().empty());
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(updated_master_data2,
            &mid_old_value, &smid_old_value, updated_mid2, updated_smid2,
            new_tmid2, tmid_for_deletion2));
  EXPECT_EQ(updated_mid1->value(), mid_old_value);
  EXPECT_EQ(updated_smid1->value(), smid_old_value);
  EXPECT_NE(original_mid->value(), mid_old_value);
  EXPECT_EQ(original_smid->value(), smid_old_value);
  ASSERT_TRUE(updated_mid2.get());
  ASSERT_TRUE(updated_smid2.get());
  ASSERT_TRUE(new_tmid2.get());
  ASSERT_TRUE(tmid_for_deletion2.get());
  EXPECT_EQ(1UL, updated_mid2.use_count());
  EXPECT_EQ(1UL, updated_smid2.use_count());
  EXPECT_EQ(1UL, new_tmid2.use_count());
  EXPECT_EQ(1UL, tmid_for_deletion2.use_count());
  EXPECT_EQ(original_tmid->name(), tmid_for_deletion2->name());
  EXPECT_EQ(original_tmid->value(), tmid_for_deletion2->value());
  ASSERT_TRUE(passport_.GetPacket(MID, false).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MID, false)->Equals(updated_mid2.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, false)->Equals(updated_smid2.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->Equals(new_tmid2.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, false)->name(), new_tmid1->name());
  EXPECT_EQ(passport_.GetPacket(STMID, false)->value(), new_tmid1->value());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());
  EXPECT_FALSE(updated_smid1->Equals(updated_smid2.get()));

  // Retry with same data - shouldn't return tmid_for_deletion this time
  MidPtr updated_mid3(new MidPacket), updated_smid3(new MidPacket);
  TmidPtr new_tmid3(new TmidPacket), tmid_for_deletion3(new TmidPacket);
  *tmid_for_deletion3 = *original_tmid;
  EXPECT_FALSE(tmid_for_deletion3->name().empty());
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(updated_master_data2,
            &mid_old_value, &smid_old_value, updated_mid3, updated_smid3,
            new_tmid3, tmid_for_deletion3));
  EXPECT_EQ(updated_mid1->value(), mid_old_value);
  EXPECT_EQ(updated_smid1->value(), smid_old_value);
  EXPECT_NE(original_mid->value(), mid_old_value);
  EXPECT_EQ(original_smid->value(), smid_old_value);
  ASSERT_TRUE(updated_mid3.get());
  ASSERT_TRUE(updated_smid3.get());
  ASSERT_TRUE(new_tmid3.get());
  ASSERT_TRUE(tmid_for_deletion3.get());
  EXPECT_EQ(1UL, updated_mid3.use_count());
  EXPECT_EQ(1UL, updated_smid3.use_count());
  EXPECT_EQ(1UL, new_tmid3.use_count());
  EXPECT_EQ(1UL, tmid_for_deletion3.use_count());
  EXPECT_TRUE(tmid_for_deletion3->name().empty());
  EXPECT_TRUE(tmid_for_deletion3->value().empty());
  ASSERT_TRUE(passport_.GetPacket(MID, false).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MID, false)->Equals(updated_mid3.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, false)->Equals(updated_smid3.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->Equals(new_tmid3.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, false)->name(), new_tmid1->name());
  EXPECT_EQ(passport_.GetPacket(STMID, false)->value(), new_tmid1->value());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());
  EXPECT_FALSE(updated_smid1->Equals(updated_smid3.get()));

  // Revert
  EXPECT_EQ(kSuccess, passport_.RevertMasterDataUpdate());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());

  // Revert again when no pending packets exist
  EXPECT_EQ(kSuccess, passport_.RevertMasterDataUpdate());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());
}

TEST_F(PassportTest, BEH_PASSPORT_Login) {
  // Setup
  MidPtr original_mid(new MidPacket), original_smid(new MidPacket);
  TmidPtr original_tmid(new TmidPacket);
  const std::string kPlainTextMasterData1(RandomString(10000));
  std::string mid_old_value, smid_old_value;
  MidPtr updated_mid1(new MidPacket), updated_smid1(new MidPacket);
  TmidPtr new_tmid1(new TmidPacket), tmid_for_deletion1(new TmidPacket);

  ASSERT_TRUE(CreateUser(original_mid, original_smid, original_tmid));
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(kPlainTextMasterData1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  EXPECT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(updated_mid1,
                      updated_smid1, new_tmid1));
  const std::string kPlainTextMasterData2(RandomString(10000));
  MidPtr updated_mid2(new MidPacket), updated_smid2(new MidPacket);
  TmidPtr new_tmid2(new TmidPacket), tmid_for_deletion2(new TmidPacket);
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(kPlainTextMasterData2,
            &mid_old_value, &smid_old_value, updated_mid2, updated_smid2,
            new_tmid2, tmid_for_deletion2));
  EXPECT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(updated_mid2,
                      updated_smid2, new_tmid2));
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  const std::string kEncryptedRidMain(
      passport_.GetPacket(MID, true)->value());
  const std::string kEncryptedRidSurrogate(
      passport_.GetPacket(SMID, true)->value());
  const std::string kEncryptedMasterDataMain(
      passport_.GetPacket(TMID, true)->value());
  const std::string kEncryptedMasterDataSurrogate(
      passport_.GetPacket(STMID, true)->value());
  const std::string kSerialisedKeyring(passport_.SerialiseKeyring());
  EXPECT_FALSE(kSerialisedKeyring.empty());
  passport_.Clear();

  // Invalid data and null pointers
  EXPECT_EQ(kNullPointer,
            passport_.InitialiseTmid(false, kEncryptedRidMain, NULL));
  EXPECT_EQ(kNullPointer,
            passport_.InitialiseTmid(true, kEncryptedRidSurrogate, NULL));
  std::string tmid_name, stmid_name;
  EXPECT_EQ(kNoPendingMid,
            passport_.InitialiseTmid(false, kEncryptedRidMain, &tmid_name));
  EXPECT_EQ(kNoPendingSmid, passport_.InitialiseTmid(true,
                            kEncryptedRidSurrogate, &stmid_name));
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());

  EXPECT_EQ(kNullPointer, passport_.GetUserData(kPassword_, false,
                          kEncryptedMasterDataMain, NULL));
  EXPECT_EQ(kNullPointer, passport_.GetUserData(kPassword_, true,
                          kEncryptedMasterDataSurrogate, NULL));
  std::string recovered_plain_text_main, recovered_plain_text_surrogate;
  EXPECT_EQ(kNoPendingTmid, passport_.GetUserData(kPassword_, false,
            kEncryptedMasterDataMain, &recovered_plain_text_main));
  EXPECT_EQ(kNoPendingStmid, passport_.GetUserData(kPassword_, true,
            kEncryptedMasterDataSurrogate, &recovered_plain_text_surrogate));

  // Good data
  std::string mid_name, smid_name;
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());
  EXPECT_EQ(updated_mid2->name(), mid_name_);
  EXPECT_EQ(updated_smid2->name(), smid_name_);

  EXPECT_EQ(kSuccess, passport_.InitialiseTmid(false, kEncryptedRidMain,
                                               &tmid_name));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());

  EXPECT_EQ(kSuccess, passport_.InitialiseTmid(true, kEncryptedRidSurrogate,
                                               &stmid_name));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());
  EXPECT_EQ(new_tmid2->name(), tmid_name);
  EXPECT_EQ(new_tmid1->name(), stmid_name);

  std::string original_plain_text1, original_plain_text2;
  EXPECT_EQ(kSuccess, passport_.GetUserData(kPassword_, false,
                      kEncryptedMasterDataMain, &original_plain_text2));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());

  EXPECT_EQ(kSuccess, passport_.GetUserData(kPassword_, true,
                      kEncryptedMasterDataSurrogate, &original_plain_text1));
  EXPECT_TRUE(passport_.GetPacket(MID, false).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(MID, true).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, true).get());
  EXPECT_EQ(kPlainTextMasterData1, original_plain_text1);
  EXPECT_EQ(kPlainTextMasterData2, original_plain_text2);

  EXPECT_FALSE(passport_.GetPacket(ANMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, true).get());
  EXPECT_EQ(kSuccess, passport_.ParseKeyring(kSerialisedKeyring));
  EXPECT_FALSE(passport_.GetPacket(ANMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANSMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANTMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_EQ(passport_.GetPacket(MID, true)->name(), updated_mid2->name());
  EXPECT_EQ(passport_.GetPacket(MID, true)->value(), updated_mid2->value());
  EXPECT_EQ(passport_.GetPacket(SMID, true)->name(), updated_smid2->name());
  EXPECT_EQ(passport_.GetPacket(SMID, true)->value(), updated_smid2->value());
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid2.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), new_tmid1->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), new_tmid1->value());

  // Try to parse keyring while signature packets pre-exist
  EXPECT_EQ(kKeyringNotEmpty, passport_.ParseKeyring(kSerialisedKeyring));

  // Try to parse keyring without pending MID, SMID, TMID or STMID packets
  passport_.ClearKeyring();
  EXPECT_FALSE(passport_.GetPacket(ANMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true).get());
  EXPECT_TRUE(passport_.GetPacket(SMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_EQ(kPassportError, passport_.ParseKeyring(kSerialisedKeyring));

  // Try to GetUserData with wrong encrypted data
  passport_.Clear();
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kSuccess, passport_.InitialiseTmid(false, kEncryptedRidMain,
                                               &tmid_name));
  EXPECT_EQ(kSuccess, passport_.InitialiseTmid(true, kEncryptedRidSurrogate,
                                               &stmid_name));
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  EXPECT_EQ(kBadSerialisedTmidData, passport_.GetUserData(kPassword_, false, "",
                                    &original_plain_text2));
  EXPECT_EQ(kBadSerialisedStmidData, passport_.GetUserData(kPassword_, true, "",
                                     &original_plain_text1));
}

enum ChangeType {
  kChangeUsername,
  kChangePin,
  kChangeUsernameAndPin,
  kChangePassword
};

class PassportVPTest : public testing::TestWithParam<ChangeType> {
 public:
  PassportVPTest()
      : passport_(kRsaKeySize, kMaxThreadCount),
        kUsername_(RandomAlphaNumericString(15)),
        kPin_(boost::lexical_cast<std::string>(RandomUint32())),
        kPassword_(RandomAlphaNumericString(20)),
        kNewUsername_((GetParam() == kChangeUsername ||
                      GetParam() == kChangeUsernameAndPin) ? kUsername_ + "a" :
                      kUsername_),
        kNewPin_((GetParam() == kChangePin ||
                 GetParam() == kChangeUsernameAndPin) ?
                 boost::lexical_cast<std::string>(
                     boost::lexical_cast<boost::uint32_t>(kPin_) + 1) : kPin_),
        kNewPassword_(GetParam() == kChangePassword ? kPassword_ + "a" :
                     kPassword_),
        kPlainTextMasterDataTmid_(RandomString(10000)),
        kPlainTextMasterDataStmid_(RandomString(10000)),
        kPlainTextMasterDataAfterChange_(RandomString(10000)),
        mid_before_change_(new MidPacket),
        smid_before_change_(new MidPacket),
        tmid_before_change_(new TmidPacket),
        stmid_before_change_(new TmidPacket),
        mid_after_change_(new MidPacket),
        smid_after_change_(new MidPacket),
        tmid_after_change_(new TmidPacket),
        stmid_after_change_(new TmidPacket),
        mid_for_deletion_(new MidPacket),
        smid_for_deletion_(new MidPacket),
        tmid_for_deletion_(new TmidPacket),
        stmid_for_deletion_(new TmidPacket),
        kChangePassword_(GetParam() == kChangePassword) {}
 protected:
  typedef std::shared_ptr<pki::Packet> PacketPtr;
  typedef std::shared_ptr<MidPacket> MidPtr;
  typedef std::shared_ptr<TmidPacket> TmidPtr;
  typedef std::shared_ptr<SignaturePacket> SignaturePtr;
  void SetUp() {
    passport_.Init();
    MidPtr mid(new MidPacket), smid(new MidPacket);
    TmidPtr tmid(new TmidPacket);
    SignaturePtr sig_packet(new SignaturePacket);
    ASSERT_TRUE(
        passport_.InitialiseSignaturePacket(ANMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess &&
        passport_.InitialiseSignaturePacket(ANSMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess &&
        passport_.InitialiseSignaturePacket(ANTMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess);
    std::string t;
    ASSERT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &t, &t));
    ASSERT_EQ(kSuccess,
              passport_.SetNewUserData(kPassword_, "ab", mid, smid, tmid));
    ASSERT_EQ(kSuccess, passport_.ConfirmNewUserData(mid, smid, tmid));
    ASSERT_EQ(kSuccess, passport_.UpdateMasterData(kPlainTextMasterDataStmid_,
              &t, &t, mid, smid, stmid_before_change_, tmid_for_deletion_));
    ASSERT_EQ(kSuccess,
        passport_.ConfirmMasterDataUpdate(mid, smid, stmid_before_change_));
    stmid_before_change_->SetToSurrogate();
    ASSERT_EQ(kSuccess, passport_.UpdateMasterData(kPlainTextMasterDataTmid_,
              &t, &t, mid_before_change_, smid_before_change_,
              tmid_before_change_, tmid_for_deletion_));
    ASSERT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(mid_before_change_,
                        smid_before_change_, tmid_before_change_));
    ASSERT_TRUE(passport_.GetPacket(MID, true).get());
    ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
    ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
    ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  }
  void TearDown() {}
  Passport passport_;
  const std::string kUsername_, kPin_, kPassword_;
  const std::string kNewUsername_, kNewPin_, kNewPassword_;
  const std::string kPlainTextMasterDataTmid_;
  const std::string kPlainTextMasterDataStmid_;
  const std::string kPlainTextMasterDataAfterChange_;
  MidPtr mid_before_change_, smid_before_change_;
  TmidPtr tmid_before_change_, stmid_before_change_;
  MidPtr mid_after_change_, smid_after_change_;
  TmidPtr tmid_after_change_, stmid_after_change_;
  MidPtr mid_for_deletion_, smid_for_deletion_;
  TmidPtr tmid_for_deletion_, stmid_for_deletion_;
  const bool kChangePassword_;
};

TEST_P(PassportVPTest, BEH_PASSPORT_ChangeUserDetails) {
  std::string message("\n\nCHANGING ");
  switch (GetParam()) {
    case kChangeUsername:
      message += "USERNAME ONLY.\n\n";
      break;
    case kChangePin:
      message += "PIN ONLY.\n\n";
      break;
    case kChangeUsernameAndPin:
      message += "USERNAME AND PIN.\n\n";
      break;
    case kChangePassword:
      message += "PASSWORD ONLY.\n\n";
      break;
    default:
      break;
  }
  SCOPED_TRACE(message);
  // Invalid data and null pointers
  MidPtr null_mid;
  TmidPtr null_tmid;
  std::string temp;
  if (kChangePassword_) {
    std::string tmid_old_value, stmid_old_value;
    EXPECT_EQ(kNullPointer, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, NULL, &temp, tmid_after_change_,
        stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, NULL, tmid_after_change_,
        stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, &temp, null_tmid,
        stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, &temp, tmid_after_change_,
        null_tmid));
  } else {
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, null_mid, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, null_mid,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        null_tmid, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, null_tmid, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, null_mid,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        null_mid, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, null_tmid, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, null_tmid));
  }
  EXPECT_FALSE(passport_.GetPacket(ANMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANSMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANTMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());

  std::string serialised_keyring(passport_.SerialiseKeyring());
  EXPECT_EQ(kSuccess, passport_.DeletePacket(STMID));
  if (kChangePassword_) {
    EXPECT_EQ(kNoStmid, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, &temp, tmid_after_change_,
        stmid_after_change_));
  } else {
    EXPECT_EQ(kNoStmid, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
  }

  EXPECT_EQ(kSuccess, passport_.DeletePacket(TMID));
  if (kChangePassword_) {
    EXPECT_EQ(kNoTmid, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, &temp, tmid_after_change_,
        stmid_after_change_));
  } else {
    EXPECT_EQ(kNoTmid, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
  }

  EXPECT_EQ(kSuccess, passport_.DeletePacket(SMID));
  if (kChangePassword_) {
    EXPECT_EQ(kNoSmid, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, &temp, tmid_after_change_,
        stmid_after_change_));
  } else {
    EXPECT_EQ(kNoSmid, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
  }

  EXPECT_EQ(kSuccess, passport_.DeletePacket(MID));
  if (kChangePassword_) {
    EXPECT_EQ(kNoMid, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &temp, &temp, tmid_after_change_,
        stmid_after_change_));
  } else {
    EXPECT_EQ(kNoMid, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
  }

  // Reset passport and test with good data
  passport_.Clear();
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(kUsername_, kPin_, &temp, &temp));
  EXPECT_EQ(kSuccess, passport_.InitialiseTmid(false,
                      mid_before_change_->value(), &temp));
  EXPECT_EQ(kSuccess, passport_.InitialiseTmid(true,
                      smid_before_change_->value(), &temp));
  EXPECT_EQ(kSuccess, passport_.GetUserData(kPassword_, false,
                      tmid_before_change_->value(), &temp));
  EXPECT_EQ(kSuccess, passport_.GetUserData(kPassword_, true,
                      stmid_before_change_->value(), &temp));
  EXPECT_EQ(kSuccess, passport_.ParseKeyring(serialised_keyring));
  EXPECT_FALSE(passport_.GetPacket(ANMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANSMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(ANTMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(ANMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANSMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(ANTMID, true).get());
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(mid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->
              Equals(smid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_before_change_.get()));

  std::string tmid_old_value, stmid_old_value;
  if (kChangePassword_) {
    EXPECT_EQ(kSuccess, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &tmid_old_value, &stmid_old_value,
        tmid_after_change_, stmid_after_change_));
    EXPECT_FALSE(passport_.GetPacket(MID, false).get());
    EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  } else {
    EXPECT_EQ(kSuccess, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    ASSERT_TRUE(passport_.GetPacket(MID, false).get());
    ASSERT_TRUE(passport_.GetPacket(SMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(MID, false)->
                Equals(mid_after_change_.get()));
    EXPECT_TRUE(passport_.GetPacket(SMID, false)->
                Equals(smid_after_change_.get()));
  }
  ASSERT_TRUE(passport_.GetPacket(TMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->
              Equals(tmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, false)->
              Equals(stmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(mid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->
              Equals(smid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_before_change_.get()));

  if (kChangePassword_) {
    EXPECT_EQ(tmid_before_change_->value(), tmid_old_value);
    EXPECT_EQ(stmid_before_change_->value(), stmid_old_value);
    tmid_old_value.clear();
    stmid_old_value.clear();
  } else {
    EXPECT_TRUE(mid_before_change_->Equals(mid_for_deletion_.get()));
    EXPECT_TRUE(smid_before_change_->Equals(smid_for_deletion_.get()));
    EXPECT_TRUE(tmid_before_change_->Equals(tmid_for_deletion_.get()));
    EXPECT_TRUE(stmid_before_change_->Equals(stmid_for_deletion_.get()));
    EXPECT_FALSE(mid_before_change_->Equals(mid_after_change_.get()));
    EXPECT_FALSE(smid_before_change_->Equals(smid_after_change_.get()));
  }
  EXPECT_FALSE(tmid_before_change_->Equals(tmid_after_change_.get()));
  EXPECT_FALSE(stmid_before_change_->Equals(stmid_after_change_.get()));
  EXPECT_EQ(1UL, mid_for_deletion_.use_count());
  EXPECT_EQ(1UL, smid_for_deletion_.use_count());
  EXPECT_EQ(1UL, tmid_for_deletion_.use_count());
  EXPECT_EQ(1UL, stmid_for_deletion_.use_count());
  EXPECT_EQ(1UL, mid_after_change_.use_count());
  EXPECT_EQ(1UL, smid_after_change_.use_count());
  EXPECT_EQ(1UL, tmid_after_change_.use_count());
  EXPECT_EQ(1UL, stmid_after_change_.use_count());

  // Revert
  if (kChangePassword_) {
    EXPECT_EQ(kSuccess, passport_.RevertPasswordChange());
  } else {
    EXPECT_EQ(kSuccess, passport_.RevertUserDataChange());
  }
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(mid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->
              Equals(smid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_before_change_.get()));

  // Reapply change
  if (kChangePassword_) {
    EXPECT_EQ(kSuccess, passport_.ChangePassword(kNewPassword_,
        kPlainTextMasterDataAfterChange_, &tmid_old_value, &stmid_old_value,
        tmid_after_change_, stmid_after_change_));
    EXPECT_FALSE(passport_.GetPacket(MID, false).get());
    EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  } else {
    EXPECT_EQ(kSuccess, passport_.ChangeUserData(kNewUsername_, kNewPin_,
        kPlainTextMasterDataAfterChange_, mid_for_deletion_, smid_for_deletion_,
        tmid_for_deletion_, stmid_for_deletion_, mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    ASSERT_TRUE(passport_.GetPacket(MID, false).get());
    ASSERT_TRUE(passport_.GetPacket(SMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(MID, false)->
                Equals(mid_after_change_.get()));
    EXPECT_TRUE(passport_.GetPacket(SMID, false)->
                Equals(smid_after_change_.get()));
  }
  ASSERT_TRUE(passport_.GetPacket(TMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->
              Equals(tmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, false)->
              Equals(stmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(mid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->
              Equals(smid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_before_change_.get()));

  // Fail to confirm change
  if (kChangePassword_) {
    EXPECT_EQ(kNullPointer,
              passport_.ConfirmPasswordChange(null_tmid, stmid_after_change_));
    EXPECT_EQ(kNullPointer,
              passport_.ConfirmPasswordChange(tmid_after_change_, null_tmid));
    EXPECT_FALSE(passport_.GetPacket(MID, false).get());
    EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  } else {
    EXPECT_EQ(kNullPointer, passport_.ConfirmUserDataChange(null_mid,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ConfirmUserDataChange(mid_after_change_,
        null_mid, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ConfirmUserDataChange(mid_after_change_,
        smid_after_change_, null_tmid, stmid_after_change_));
    EXPECT_EQ(kNullPointer, passport_.ConfirmUserDataChange(mid_after_change_,
        smid_after_change_, tmid_after_change_, null_tmid));
    ASSERT_TRUE(passport_.GetPacket(MID, false).get());
    ASSERT_TRUE(passport_.GetPacket(SMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(MID, false)->
                Equals(mid_after_change_.get()));
    EXPECT_TRUE(passport_.GetPacket(SMID, false)->
                Equals(smid_after_change_.get()));
  }
  ASSERT_TRUE(passport_.GetPacket(TMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->
              Equals(tmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, false)->
              Equals(stmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(mid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->
              Equals(smid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_before_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_before_change_.get()));

  if (!kChangePassword_) {
    EXPECT_EQ(kPacketsNotEqual,
              passport_.ConfirmUserDataChange(mid_before_change_,
              smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_TRUE(passport_.GetPacket(MID, false).get());
    EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
    EXPECT_EQ(kPacketsNotEqual,
              passport_.ConfirmUserDataChange(mid_after_change_,
              smid_before_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_FALSE(passport_.GetPacket(MID, false).get());
    EXPECT_TRUE(passport_.GetPacket(SMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
    EXPECT_TRUE(passport_.GetPacket(STMID, false).get());
  }

  if (kChangePassword_) {
    EXPECT_EQ(kPacketsNotEqual,
              passport_.ConfirmPasswordChange(tmid_before_change_,
                                              stmid_after_change_));
  } else {
    EXPECT_EQ(kPacketsNotEqual,
              passport_.ConfirmUserDataChange(mid_after_change_,
              smid_after_change_, tmid_before_change_, stmid_after_change_));
  }
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());

  if (kChangePassword_) {
    EXPECT_EQ(kPacketsNotEqual,
              passport_.ConfirmPasswordChange(tmid_after_change_,
                                              stmid_before_change_));
  } else {
    EXPECT_EQ(kPacketsNotEqual,
              passport_.ConfirmUserDataChange(mid_after_change_,
              smid_after_change_, tmid_after_change_, stmid_before_change_));
  }
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_TRUE(passport_.GetPacket(STMID, false).get());

  // Confirm change
  if (kChangePassword_) {
    EXPECT_EQ(kSuccess, passport_.ConfirmPasswordChange(tmid_after_change_,
                                                        stmid_after_change_));
  } else {
    EXPECT_EQ(kSuccess, passport_.ConfirmUserDataChange(mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
  }
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  if (!kChangePassword_) {
    EXPECT_TRUE(passport_.GetPacket(MID, true)->
                Equals(mid_after_change_.get()));
    EXPECT_TRUE(passport_.GetPacket(SMID, true)->
                Equals(smid_after_change_.get()));
  }
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_after_change_.get()));

  // Confirm same change
  if (kChangePassword_) {
    EXPECT_EQ(kSuccess, passport_.ConfirmPasswordChange(tmid_after_change_,
                                                        stmid_after_change_));
  } else {
    EXPECT_EQ(kSuccess, passport_.ConfirmUserDataChange(mid_after_change_,
        smid_after_change_, tmid_after_change_, stmid_after_change_));
  }
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  if (!kChangePassword_) {
    EXPECT_TRUE(passport_.GetPacket(MID, true)->
                Equals(mid_after_change_.get()));
    EXPECT_TRUE(passport_.GetPacket(SMID, true)->
                Equals(smid_after_change_.get()));
  }
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_after_change_.get()));

  // Confirm with missing pending packets
  if (kChangePassword_) {
    EXPECT_EQ(kNoPendingPacket,
              passport_.ConfirmPasswordChange(tmid_before_change_,
                                              stmid_after_change_));
    EXPECT_EQ(kNoPendingPacket,
              passport_.ConfirmPasswordChange(tmid_after_change_,
                                              stmid_before_change_));
  } else {
    EXPECT_EQ(kNoPendingPacket,
              passport_.ConfirmUserDataChange(mid_before_change_,
              smid_after_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNoPendingPacket,
              passport_.ConfirmUserDataChange(mid_after_change_,
              smid_before_change_, tmid_after_change_, stmid_after_change_));
    EXPECT_EQ(kNoPendingPacket,
              passport_.ConfirmUserDataChange(mid_after_change_,
              smid_after_change_, tmid_before_change_, stmid_after_change_));
    EXPECT_EQ(kNoPendingPacket,
              passport_.ConfirmUserDataChange(mid_after_change_,
              smid_after_change_, tmid_after_change_, stmid_before_change_));
  }
  EXPECT_FALSE(passport_.GetPacket(MID, false).get());
  EXPECT_FALSE(passport_.GetPacket(SMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(TMID, false).get());
  EXPECT_FALSE(passport_.GetPacket(STMID, false).get());
  ASSERT_TRUE(passport_.GetPacket(MID, true).get());
  ASSERT_TRUE(passport_.GetPacket(SMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(TMID, true).get());
  ASSERT_TRUE(passport_.GetPacket(STMID, true).get());
  if (!kChangePassword_) {
    EXPECT_TRUE(passport_.GetPacket(MID, true)->
                Equals(mid_after_change_.get()));
    EXPECT_TRUE(passport_.GetPacket(SMID, true)->
                Equals(smid_after_change_.get()));
  }
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->
              Equals(tmid_after_change_.get()));
  EXPECT_TRUE(passport_.GetPacket(STMID, true)->
              Equals(stmid_after_change_.get()));
}

INSTANTIATE_TEST_CASE_P(VPTest, PassportVPTest, testing::Values(kChangeUsername,
    kChangePin, kChangeUsernameAndPin, kChangePassword));

}  // namespace test

}  // namespace passport

}  // namespace maidsafe
