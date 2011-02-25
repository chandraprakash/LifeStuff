/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Macro definitions and class to allow network and non-network
*               versions of tests to use the same source files.
* Created:      2010-06-03
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

#ifndef MAIDSAFE_SHAREDTEST_NETWORKTEST_H_
#define MAIDSAFE_SHAREDTEST_NETWORKTEST_H_

#include "boost/cstdint.hpp"
#include "boost/filesystem.hpp"
#include "gtest/gtest.h"
//  #include <maidsafe/maidsafe-dht_config.h>
//  #include <maidsafe/rpcprotocol/channelmanager-api.h>
//  #include <maidsafe/transport/transporthandler-api.h>
//  #include <maidsafe/transport/transportudt.h>

#include <string>
#include <vector>

#ifdef MS_NETWORK_TEST

#include "maidsafe/client/maidstoremanager.h"
#include "maidsafe/sharedtest/localvaults.h"

#define TEST_MS_NET(test_fixture, test_type, test_name_prefix, test_name)\
  TEST_F(test_fixture, FUNC_##test_name_prefix##_NET_##test_name)

namespace fs3 = boost::filesystem3;

namespace maidsafe {

class ChunkStore;
class KadOps;

namespace test {

typedef std::vector< std::shared_ptr<vault::PDVault> > LocalVaults;
typedef MaidsafeStoreManager TestStoreManager;
typedef std::shared_ptr<TestStoreManager> TestStoreManagerPtr;

boost::uint8_t K();
int kNetworkSize();
LocalVaults *pdvaults();
fs3::path *kadconfig();

#else  // MS_NETWORK_TEST

#include "maidsafe/client/localstoremanager.h"

#define TEST_MS_NET(test_fixture, test_type, test_name_prefix, test_name)\
  TEST_F(test_fixture, test_type##_##test_name_prefix##_##test_name)
namespace maidsafe {

class ChunkStore;
class KadOps;

namespace test {

typedef LocalStoreManager TestStoreManager;
typedef std::shared_ptr<TestStoreManager> TestStoreManagerPtr;

boost::uint8_t K();

#endif  // MS_NETWORK_TEST

class NetworkTest {
 public:
  NetworkTest();
  ~NetworkTest();
  bool Init();
  bool IsLastTest();
  boost::int16_t transport_id() const { return transport_id_; }
  fs3::path test_dir() const { return test_dir_; }
//  transport::Transport *transport() const { return transport_; }
//  transport::TransportHandler *transport_handler() const {
//    return transport_handler_;
//  }
//  rpcprotocol::ChannelManager *channel_manager() const {
//    return channel_manager_;
//  }
  std::shared_ptr<maidsafe::ChunkStore> chunkstore() const {
    return chunkstore_;
  }
  std::shared_ptr<maidsafe::KadOps> kad_ops() const { return kad_ops_; }
  TestStoreManagerPtr store_manager() const { return store_manager_; }
  boost::uint8_t K() const { return K_; }
  boost::uint8_t kUpperThreshold() const { return kUpperThreshold_; }
  boost::uint8_t kLowerThreshold() const { return kLowerThreshold_; }

 private:
  NetworkTest(const NetworkTest&);
  NetworkTest &operator=(const NetworkTest&);
  const testing::TestInfo* const test_info_;
  std::string test_case_name_;
  boost::int16_t transport_id_;
  fs3::path test_dir_;
//  transport::Transport *transport_;
//  transport::TransportHandler *transport_handler_;
//  rpcprotocol::ChannelManager *channel_manager_;
  std::shared_ptr<maidsafe::ChunkStore> chunkstore_;
  std::shared_ptr<maidsafe::KadOps> kad_ops_;
  TestStoreManagerPtr store_manager_;
  const boost::uint8_t K_, kUpperThreshold_, kLowerThreshold_;
};

}  // namespace test

}  // namespace maidsafe

#endif  // MAIDSAFE_SHAREDTEST_NETWORKTEST_H_
