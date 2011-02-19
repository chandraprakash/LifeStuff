//  /*
//  * ============================================================================
//  *
//  * Copyright [2009] maidsafe.net limited
//  *
//  * Description:  Allows creation of gtest environment where pdvaults are set up
//  *               and started
//  * Version:      1.0
//  * Created:      2009-06-22-15.51.35
//  * Revision:     none
//  * Compiler:     gcc
//  * Author:       Fraser Hutchison (fh), fraser.hutchison@maidsafe.net
//  * Company:      maidsafe.net limited
//  *
//  * The following source code is property of maidsafe.net limited and is not
//  * meant for external use.  The use of this code is governed by the license
//  * file LICENSE.TXT found in the root of this directory and also on
//  * www.maidsafe.net.
//  *
//  * You are not free to copy, amend or otherwise use this source code without
//  * the explicit written permission of the board of directors of maidsafe.net.
//  *
//  * ============================================================================
//  */
//
//  #ifndef MAIDSAFE_SHAREDTEST_LOCALVAULTS_H_
//  #define MAIDSAFE_SHAREDTEST_LOCALVAULTS_H_
//
//  #include <boost/filesystem.hpp>
//  #include <boost/filesystem/fstream.hpp>
//  #include <boost/thread/thread.hpp>
//  #include <gtest/gtest.h>
//
//  #include <string>
//  #include <vector>
//
//  #include "maidsafe/common/commonutils.h"
//  #include "maidsafe/common/filesystem.h"
//  #include "maidsafe/common/kadops.h"
//
//  namespace fs = boost::filesystem;
//
//  namespace maidsafe {
//
//  namespace test {
//
//  namespace localvaults {
//
//  inline void PrintRpcTimings(const rpcprotocol::RpcStatsMap &rpc_timings) {
//    printf("Calls  RPC Name                                            "
//           "min/avg/max/total\n");
//    for (rpcprotocol::RpcStatsMap::const_iterator it = rpc_timings.begin();
//         it != rpc_timings.end();
//         ++it) {
//      printf("%5llux %-50s  %.2f/%.2f/%.2f/%04.1f s\n",
//             it->second.Size(),
//             it->first.c_str(),
//             it->second.Min() / 1000.0,
//             it->second.Mean() / 1000.0,
//             it->second.Max() / 1000.0,
//             it->second.Sum() / 1000.0);
//    }
//  }
//
//  inline void GeneratePmidStuff(std::string *public_key,
//                                std::string *private_key,
//                                std::string *signed_key,
//                                std::string *pmid) {
//    crypto::RsaKeyPair keys;
//    keys.GenerateKeys(maidsafe::kRsaKeySize);
//    *signed_key = maidsafe::RSASign(keys.public_key(), keys.private_key());
//    *public_key = keys.public_key();
//    *private_key = keys.private_key();
//    *pmid = maidsafe::SHA512String(*public_key + *signed_key);
//  }
//
//  class Env: public testing::Environment {
//   public:
//    Env(const boost::uint8_t &k,
//        const int kNetworkSize,
//        std::vector<boost::shared_ptr<vault::PDVault> > *pdvaults,
//        fs::path *kadconfig)
//        : K_(k),
//          vault_dir_(file_system::TempDir() / ("maidsafe_TestVaults_" +
//                     base::RandomAlphaNumericString(6))),
//          kad_config_file_(vault_dir_ / ".kadconfig"),
//          pdvaults_(pdvaults),
//          kNetworkSize_(kNetworkSize),
//          current_nodes_created_(0) {
//      *kadconfig = kad_config_file_;
//    }
//
//    ~Env() {
//      transport::TransportUDT::CleanUp();
//    }
//
//    virtual void SetUp() {
//      ASSERT_LE(K_, kNetworkSize_) << "Need at least K nodes!";
//      pdvaults_->clear();
//      try {
//        if (fs::exists(vault_dir_))
//          fs::remove_all(vault_dir_);
//      }
//      catch(const std::exception &e_) {
//        printf("%s\n", e_.what());
//      }
//      fs::create_directories(vault_dir_);
//
//      // Construct and start vaults
//      printf("Creating vaults...\n");
//      for (int i = 0; i < kNetworkSize_; ++i) {
//        std::string public_key, private_key, signed_key, node_id;
//        GeneratePmidStuff(&public_key, &private_key, &signed_key, &node_id);
//        fs::path local_dir(vault_dir_ / ("Vault_" +
//            base::EncodeToHex(node_id).substr(0, 8)));
//        if (!fs::exists(fs::path(local_dir))) {
//          printf("creating_directories - %s\n", local_dir.string().c_str());
//          fs::create_directories(local_dir);
//        }
//
//        boost::shared_ptr<vault::PDVault>
//            pdvault_local(new vault::PDVault(public_key, private_key, signed_key,
//                          local_dir, 0, false, false, kad_config_file_,
//                          1073741824, 0, K_));
//        pdvaults_->push_back(pdvault_local);
//        ++current_nodes_created_;
//
//        // Start vault
//        (*pdvaults_)[i]->Start(i == 0);
//        ASSERT_TRUE((*pdvaults_)[i]->WaitForStartup(10));
//
//        // Set first node as bootstrapping node for others
//        if (i == 0) {
//          base::KadConfig kad_config;
//          kad::ContactInfo contact_info =
//              (*pdvaults_)[0]->kad_ops_->contact_info();
//          base::KadConfig::Contact *kad_contact = kad_config.add_contact();
//          kad_contact->set_node_id(base::EncodeToHex(contact_info.node_id()));
//          kad_contact->set_ip(contact_info.ip());
//          kad_contact->set_port(contact_info.port());
//          kad_contact->set_local_ip(contact_info.local_ip());
//          kad_contact->set_local_port(contact_info.local_port());
//          std::fstream output(kad_config_file_.string().c_str(),
//                              std::ios::out | std::ios::trunc | std::ios::binary);
//          ASSERT_TRUE(kad_config.SerializeToOstream(&output));
//          output.close();
//        }
//        printf("Vault %i started.\n", i);
//      }
//
//  //    // Make kad config file in ./ for clients' use.
//      if (fs::exists(".kadconfig"))
//        fs::remove(".kadconfig");
//  //    fs::copy_file(kad_config_file_, ".kadconfig");
//
//      // Wait for account creation and syncing
//      for (int i = 0; i < kNetworkSize_; ++i)
//        ASSERT_TRUE((*pdvaults_)[i]->WaitForSync());
//
//  #ifdef WIN32
//      HANDLE hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
//      SetConsoleTextAttribute(hconsole, 10 | 0 << 4);
//  #endif
//      printf("\n*-----------------------------------------------*\n");
//      if (kNetworkSize_ < 10 )
//        printf("*            %i local vaults running             *\n",
//               kNetworkSize_);
//      else
//        printf("*            %i local vaults running            *\n",
//               kNetworkSize_);
//      printf("*                                               *\n");
//      printf("* No. Port   ID                                 *\n");
//      for (int l = 0; l < kNetworkSize_; ++l)
//        printf("* %2i  %5i  %s *\n", l,
//               (*pdvaults_)[l]->kad_ops_->contact_info().port(),
//               (base::EncodeToHex(
//               (*pdvaults_)[l]->kad_ops_->contact_info().node_id()).substr(0, 31)
//               + "...").c_str());
//      printf("*                                               *\n");
//      printf("*-----------------------------------------------*\n\n");
//  #ifdef WIN32
//      SetConsoleTextAttribute(hconsole, 11 | 0 << 4);
//  #endif
//    }
//
//    virtual void TearDown() {
//  #ifdef WIN32
//      HANDLE hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
//      SetConsoleTextAttribute(hconsole, 7 | 0 << 4);
//  #endif
//      printf("In vault tear down.\n");
//      bool success(false);
//      StopCommunications();
//      for (int i = 0; i < current_nodes_created_; ++i) {
//        printf("\nStatistics for vault %i:\n", i);
//        PrintRpcTimings((*pdvaults_)[i]->channel_manager_.RpcTimings());
//        printf("\nTrying to stop vault %i.\n", i);
//        success = false;
//        (*pdvaults_)[i]->Stop();
//        if ((*pdvaults_)[i]->vault_status() != vault::kVaultStarted)
//          printf("Vault %i stopped.\n", i);
//        else
//          printf("Vault %i failed to stop correctly.\n", i);
//        pdvaults_->at(i).reset();
//      }
//      try {
//        if (fs::exists(vault_dir_))
//          fs::remove_all(vault_dir_);
//        if (fs::exists(".kadconfig"))
//          fs::remove(".kadconfig");
//      }
//      catch(const std::exception &e_) {
//        printf("%s\n", e_.what());
//      }
//      printf("\nFinished vault tear down.\n");
//    }
//
//    void StopCommunications() {
//      for (int i = 0; i < current_nodes_created_; ++i) {
//        (*pdvaults_)[i]->StopRvPing();
//      }
//    }
//
//   private:
//    Env(const Env&);
//    Env &operator=(const Env&);
//    boost::uint8_t K_;
//    fs::path vault_dir_, chunkstore_dir_, kad_config_file_;
//    std::vector< boost::shared_ptr<vault::PDVault> > *pdvaults_;
//    const int kNetworkSize_;
//    int current_nodes_created_;
//  };
//
//  }  // namespace localvaults
//  }  // namespace test
//  }  // namespace maidsafe
//
//
//  #endif  // MAIDSAFE_SHAREDTEST_LOCALVAULTS_H_
