/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Created:      2010-03-10
* Author:       Team www.maidsafe.net
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

//  #include <maidsafe/transport/transporthandler-api.h>
//  #include <maidsafe/rpcprotocol/channelmanager-api.h>
//
//  #include "maidsafe/sharedtest/mockkadops.h"
//  #include "maidsafe/common/commonutils.h"
//  #include "maidsafe/common/chunkstore.h"
//
//  namespace test_kadops {
//  static const boost::uint8_t K(4);
//  }  // namespace test_kadops
//
//  namespace maidsafe {
//
//  class KadOpsTest : public testing::Test {
//   protected:
//    KadOpsTest()
//      : transport_handler_(),
//        channel_manager_(&transport_handler_),
//        chunkstore_(new ChunkStore("Chunkstore", 9999999, 0)),
//        mko_(&transport_handler_, &channel_manager_, kad::CLIENT, "", "", false,
//             false, test_kadops::K, chunkstore_) {}
//    transport::TransportHandler transport_handler_;
//    rpcprotocol::ChannelManager channel_manager_;
//    std::shared_ptr<ChunkStore> chunkstore_;
//    MockKadOps mko_;
//  };
//
//  TEST_F(KadOpsTest, BEH_MAID_BlockingGetNodeContactDetails) {
//    std::string dummy_id(SHA512String("Dummy"));
//    kad::Contact contact, dummy_contact(kad::Contact(dummy_id, "192.168.1.0", 7));
//    std::string ser_contact, ser_dummy_contact;
//    contact.SerialiseToString(&ser_contact);
//    dummy_contact.SerialiseToString(&ser_dummy_contact);
//    kad::FindNodeResult find_result;
//    find_result.set_result(kad::kRpcResultFailure);
//    find_result.set_contact(ser_contact);
//    std::string fail_response_1(find_result.SerializeAsString());
//    find_result.set_result(kad::kRpcResultSuccess);
//    find_result.set_contact("fail");
//    std::string fail_response_2(find_result.SerializeAsString());
//    find_result.set_contact(ser_dummy_contact);
//    std::string good_response(find_result.SerializeAsString());
//
//    ASSERT_EQ(kad::kZeroId, contact.node_id().String());
//
//    // Expectations
//    EXPECT_CALL(mko_, GetNodeContactDetails("fail",
//        testing::An<VoidFuncIntContact>(), false))
//        .WillOnce(testing::WithArgs<0, 1, 2>(testing::Invoke(
//            boost::bind(&MockKadOps::RealGetNodeContactDetails, &mko_,
//                        _1, _2, _3))));                                  // Call 2
//    EXPECT_CALL(mko_, GetNodeContactDetails("",
//        testing::An<VoidFuncIntContact>(), false))
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedGetNodeContactDetailsCallback, &mko_,
//                        "fail", _1))))                                   // Call 3
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedGetNodeContactDetailsCallback, &mko_,
//                        fail_response_1, _1))))                          // Call 4
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedGetNodeContactDetailsCallback, &mko_,
//                        fail_response_2, _1))))                          // Call 5
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedGetNodeContactDetailsCallback, &mko_,
//                        good_response, _1))));                           // Call 6
//
//    // Call 1
//    ASSERT_EQ(kFindNodesError,
//              mko_.BlockingGetNodeContactDetails("", NULL, false));
//
//    // Call 2
//    ASSERT_EQ(kFindNodesError,
//              mko_.BlockingGetNodeContactDetails("fail", &contact, false));
//
//    // Call 3
//    ASSERT_EQ(kFindNodesParseError,
//              mko_.BlockingGetNodeContactDetails("", &contact, false));
//    ASSERT_EQ(kad::kZeroId, contact.node_id().String());
//
//    // Call 4
//    ASSERT_EQ(kFindNodesFailure,
//              mko_.BlockingGetNodeContactDetails("", &contact, false));
//    ASSERT_EQ(kad::kZeroId, contact.node_id().String());
//
//    // Call 5
//    ASSERT_EQ(kFindNodesFailure,
//              mko_.BlockingGetNodeContactDetails("", &contact, false));
//    ASSERT_EQ(kad::kZeroId, contact.node_id().String());
//
//    // Call 6
//    ASSERT_EQ(kSuccess, mko_.BlockingGetNodeContactDetails("", &contact, false));
//    ASSERT_EQ(dummy_id, contact.node_id().String());
//  }
//
//  TEST_F(KadOpsTest, BEH_MAID_BlockingFindKClosestNodes) {
//    std::vector<kad::Contact> contacts;
//    std::string dummy_id(SHA512String("Dummy"));
//    kad::Contact dummy_contact(dummy_id, "192.168.1.0", 4999);
//    std::vector<kad::Contact> few_contacts, good_contacts;
//
//    std::string fail_parse_result(mock_kadops::MakeFindNodesResponse(
//        mock_kadops::kFailParse, dummy_id, test_kadops::K, NULL));
//    std::string fail_result(mock_kadops::MakeFindNodesResponse(
//        mock_kadops::kResultFail, dummy_id, test_kadops::K, NULL));
//    std::string few_result(mock_kadops::MakeFindNodesResponse(
//        mock_kadops::kFarContacts, dummy_id, 1, &few_contacts));
//    std::string good_result(mock_kadops::MakeFindNodesResponse(
//        mock_kadops::kFarContacts, dummy_id, test_kadops::K, &good_contacts));
//
//    // Expectations
//    EXPECT_CALL(mko_, FindKClosestNodes("fail",
//        testing::An<VoidFuncIntContacts>()))
//        .WillOnce(testing::WithArgs<0, 1>(testing::Invoke(
//            boost::bind(&MockKadOps::RealFindKClosestNodes, &mko_, _1, _2))));
//    EXPECT_CALL(mko_, FindKClosestNodes(dummy_id,
//        testing::An<VoidFuncIntContacts>()))
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedFindKClosestNodesCallback, &mko_,
//                        fail_parse_result, _1))))                        // Call 3
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedFindKClosestNodesCallback, &mko_,
//                        fail_result, _1))))                              // Call 4
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedFindKClosestNodesCallback, &mko_,
//                        few_result, _1))))                               // Call 5
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedFindKClosestNodesCallback, &mko_,
//                        good_result, _1))))                              // Call 6
//        .WillOnce(testing::WithArgs<1>(testing::Invoke(
//            boost::bind(&MockKadOps::ThreadedFindKClosestNodesCallback, &mko_,
//                        good_result, _1))));                             // Call 7
//
//    // Call 1
//    ASSERT_EQ(kFindNodesError,
//              mko_.BlockingFindKClosestNodes("", NULL));
//
//    // Call 2
//    contacts.push_back(dummy_contact);
//    ASSERT_EQ(size_t(1), contacts.size());
//    ASSERT_EQ(kFindNodesError,
//              mko_.BlockingFindKClosestNodes("fail", &contacts));
//    ASSERT_EQ(size_t(0), contacts.size());
//
//    // Call 3
//    contacts.push_back(dummy_contact);
//    ASSERT_EQ(size_t(1), contacts.size());
//    ASSERT_EQ(kFindNodesParseError,
//              mko_.BlockingFindKClosestNodes(dummy_id, &contacts));
//    ASSERT_EQ(size_t(0), contacts.size());
//
//    // Call 4
//    contacts.push_back(dummy_contact);
//    ASSERT_EQ(size_t(1), contacts.size());
//    ASSERT_EQ(kFindNodesFailure,
//              mko_.BlockingFindKClosestNodes(dummy_id, &contacts));
//    ASSERT_EQ(size_t(0), contacts.size());
//
//    // Call 5
//    ASSERT_EQ(kSuccess, mko_.BlockingFindKClosestNodes(dummy_id, &contacts));
//    ASSERT_EQ(few_contacts.size(), contacts.size());
//
//    // Call 6
//    ASSERT_EQ(kSuccess, mko_.BlockingFindKClosestNodes(dummy_id, &contacts));
//    ASSERT_EQ(good_contacts.size(), contacts.size());
//
//    // Call 7
//    contacts.push_back(dummy_contact);
//    ASSERT_EQ(kSuccess, mko_.BlockingFindKClosestNodes(dummy_id, &contacts));
//    ASSERT_EQ(good_contacts.size(), contacts.size());
//
//    for (size_t i = 0; i < contacts.size(); ++i)
//      ASSERT_TRUE(good_contacts[i].Equals(contacts[i]));
//  }
//
//  TEST_F(KadOpsTest, DISABLED_BEH_MAID_GetStorePeer) {
//    ASSERT_TRUE(false) << "Not implemented.";
//  }
//
//  TEST_F(KadOpsTest, BEH_MAID_ContactWithinClosest) {
//    std::vector<kad::Contact> ctc;
//    kad::Contact contact1(base::DecodeFromHex(std::string(2 * kKeySize, '1')),
//                          "127.0.0.1", 0);
//    ctc.push_back(contact1);
//    kad::Contact contact2(base::DecodeFromHex(std::string(2 * kKeySize, '7')),
//                          "127.0.0.1", 0);
//    ctc.push_back(contact2);
//    kad::Contact close(base::DecodeFromHex(std::string(2 * kKeySize, '3')),
//                       "127.0.0.1", 0);
//    kad::Contact not_close(base::DecodeFromHex(std::string(2 * kKeySize, 'f')),
//                           "127.0.0.1", 0);
//    std::string key(kKeySize, '0');
//
//    ASSERT_TRUE(ContactWithinClosest(key, close, ctc));
//    ASSERT_FALSE(ContactWithinClosest(key, not_close, ctc));
//  }
//
//  TEST_F(KadOpsTest, BEH_MAID_RemoveKadContact) {
//    std::vector<kad::Contact> ctc;
//    ctc.push_back(kad::Contact(SHA512String("aaa"), "127.0.0.1", 0));
//    ctc.push_back(kad::Contact(SHA512String("bbb"), "127.0.0.1", 0));
//    ctc.push_back(kad::Contact(SHA512String("ccc"), "127.0.0.1", 0));
//    ASSERT_EQ(size_t(3), ctc.size());
//    ASSERT_FALSE(RemoveKadContact(SHA512String("ddd"), &ctc));
//    ASSERT_EQ(size_t(3), ctc.size());
//    ASSERT_TRUE(RemoveKadContact(SHA512String("bbb"), &ctc));
//    ASSERT_EQ(size_t(2), ctc.size());
//  }
//
//  TEST_F(KadOpsTest, BEH_MAID_MakeFindNodesResponse) {
//    std::string id(base::RandomString(kKeySize)),
//                tid(SHA512String(id + kAccount));
//    kad::Contact contact(id, "127.0.0.1", 0);
//    std::vector<kad::Contact> contacts;
//    std::string response;
//    kad::FindResponse find_response;
//
//    contacts.push_back(kad::Contact("abc", "", 0));
//    ASSERT_EQ(size_t(1), contacts.size());
//    response = mock_kadops::MakeFindNodesResponse(mock_kadops::kFailParse, tid,
//                                                  test_kadops::K, &contacts);
//    ASSERT_EQ(size_t(0), contacts.size());
//    ASSERT_FALSE(find_response.ParseFromString(response));
//
//    response = mock_kadops::MakeFindNodesResponse(mock_kadops::kResultFail, tid,
//                                                  test_kadops::K, &contacts);
//    ASSERT_TRUE(find_response.ParseFromString(response));
//    ASSERT_EQ(test_kadops::K, contacts.size());
//    ASSERT_EQ(test_kadops::K, find_response.closest_nodes_size());
//
//    response = mock_kadops::MakeFindNodesResponse(mock_kadops::kCloseContacts,
//                                                  tid, test_kadops::K, &contacts);
//    ASSERT_TRUE(find_response.ParseFromString(response));
//    ASSERT_EQ(test_kadops::K, contacts.size());
//    ASSERT_EQ(test_kadops::K, find_response.closest_nodes_size());
//    EXPECT_FALSE(ContactWithinClosest(tid, contact, contacts));
//
//    response = mock_kadops::MakeFindNodesResponse(mock_kadops::kFarContacts, tid,
//                                                  test_kadops::K, &contacts);
//    ASSERT_TRUE(find_response.ParseFromString(response));
//    ASSERT_EQ(test_kadops::K, contacts.size());
//    ASSERT_EQ(test_kadops::K, find_response.closest_nodes_size());
//    EXPECT_TRUE(ContactWithinClosest(tid, contact, contacts));
//  }
//
//  }  // namespace maidsafe
