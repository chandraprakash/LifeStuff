/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Definition of system-wide constants/enums/structs
* Version:      1.0
* Created:      2009-01-29-00.15.50
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

#ifndef MAIDSAFE_LIFESTUFF_SHARED_MAIDSAFE_H_
#define MAIDSAFE_LIFESTUFF_SHARED_MAIDSAFE_H_

#include <string>
#include <vector>
#include "boost/cstdint.hpp"
#include "boost/function.hpp"
#include "maidsafe/dht/version.h"
#include "maidsafe/common/utils.h"

#define THIS_MAIDSAFE_DHT_VERSION 29
#if MAIDSAFE_DHT_VERSION < THIS_MAIDSAFE_DHT_VERSION
#error This API is not compatible with the installed library.\
  Please update the maidsafe-dht library.
#elif MAIDSAFE_DHT_VERSION > THIS_MAIDSAFE_DHT_VERSION
#error This API uses a newer version of the maidsafe-dht library.\
  Please update this project.
#endif

#include "maidsafe/lifestuff/shared/returncodes.h"

namespace kad { class Contact; }

namespace maidsafe {

namespace lifestuff {

// system constants
const boost::uint32_t kMinRegularFileSize = 512;
// This is the size in bytes of the NON-HEX format strings used as keys.  When
// encoded to hex the string size is doubled.
const boost::uint32_t kKeySize = 64;

const boost::uint16_t kRsaKeySize = 4096;
// const crypto::hashtype kHashSize(crypto::SHA_512);

const std::string kAnonymousRequestSignature(2 * kKeySize, 'f');

enum DbInitFlag {CONNECT, CREATE, DISCONNECT};

const size_t kMaxPath(300);

const std::string kRoot("/");

const int kRootSubdirSize = 4;
const int kSharesSubdirSize = 1;

const std::string kRootSubdir[kRootSubdirSize][2] = {
  {"/My Stuff", ""},
  {"/Shares", "" },
  {"/Emails", ""},
  {"/Chat", ""}
};

const std::string kSharesSubdir[kSharesSubdirSize][2] = {
  {"/Shares/Private", ""} /*,*/
//  {
//  "/Shares/Public", "a0590baf0f811834de68fec77950c179595f5ecb5dc3c6abac67dc34
//  "9714101e40b44531054196b4616f3314cee94d71babb5fbc7010d7fff958d8c8cc54836c"
//  },
//  {
//    "/Shares/Anonymous",
//    "63ed99cc9f91c7dd568247337fd5b479e2cec00e9054ec4c5797c3"
//    "19a80fe3ab07a01dca8200dfd63142b1ed376970bb3a9acd3fa55e9"
//    "d631d3c0aff42f7660e"
//  }
};

// Chunks which make up an empty dir db
const int kDefaultChunkCount = 3;
const std::string kDefaultChunks[kDefaultChunkCount][2] = {
  {"14933fd0c4630c6ecf39f12bfd00562db926fab95320a5d16d15d640f81886df433dbd8"
    "e29a23a21020f4cf852465b2e2713075402cbde84852f3cca3027ec12",
    "0a05677a69703912a1031f8b0800000000000200ed534d4f0231106d77173c79201cb8"
    "99499408c9260a7edf44c38d8bcabd29db2e34b4bbd816156f1a13ff8dff811fe1bff1"
    "601712584c8c1e3c195e769bcc6b3bf3e6edeccd5547580e71aa15b570800284313a07"
    "4008796815598c733146bfc4a6ff810a788abc57ffdd7fc353af87d6f82b3c6d051b95"
    "52093fef5ada939c29a60a97d7ed56b70dddd645a70d4cd5622139195033803baaa301"
    "d5b5e3c37a088281482c24a97bc752429426c66a9a71f190b85dcd63ae791271032e6d"
    "4d3077c9704d985a243a6a9c66a9465a28aa2730e49365b5ac42bdbe5d28564e4a1889"
    "84f107732bddb8113ab6e92c76a94883a9e2f30edea894cbf8656fd685abe61e6fa58f"
    "b9824c710809551c7a32ed2dc58f13713be621136624e9842c4e846027233ebf662cb5"
    "6621bdb1dfcca45bda5fb16526df8847d783e80fe60eb902212c37647a9fe323cda915"
    "6942ac503cc74b6a2c512913b1e0ec2b4f23e7aac9b14c68e2dc5bfd42795b332fab7e"
    "b17256fece4be71069ba25a87a3f1d6bb8c577d3136c663314ac7fa335d6f8bff804ca"
    "68cb9ee607000018e60f"},
  {"a8807eb9a3f59be307a095ddfe6083fdf7daeac4b25da1dd1aebf22287d1403d446ebe3"
    "b86a6486813da5065c99e01cb4df11c90d25ba8207b4e8bfd1d2d8ddc",
    "0a05677a697039122c1f8b0800000000000200636020007841040bc3281805a3600402"
    "aed1fc3f0a46c18805000fc255e72108000018a110"},
  {"cfcb0bc5a0cb02d62891e33a3578c301eefe8a27b1c7a87a7826732f68d41333dc307f4"
    "76c2af6938a1b5dedc79290baab076d5224b0c0ea4c6eff01d724b63d",
    "0a05677a697039122b1f8b08000000000002006360c005b840040bc3281805a3600482"
    "d1fc3f0a46c1b00500e18d3cddf907000018f90f"}
};

const std::string kAccount("ACCOUNT");

// const std::string default_dir_[] = {
//   "/Documents",
//   "/Backup",
//   "/Groups",
//   "/Library",
//   "/Maidsafe",
//   "/Music",
//   "/Pictures",
//   "/Public",
//   "/Sites",
//   "/Software",
//   "/Web"
// };
//

// config file name
const int kMaxPort = 65535;
const int kMinPort = 5000;

const int kValidityCheckMinTime(1800);  // 30 min
const int kValidityCheckMaxTime(86400);  // 24 hours
// frequency to execute validity check process
const int kValidityCheckInterval(120);  // 2 minutes
// delay to check partner references
const int kCheckPartnerRefDelay(300);  // 5 minutes
// timeout for account amendment transactions in milliseconds
const boost::uint64_t kAccountAmendmentTimeout(120000);
// timeout for account amendment results in seconds
const boost::uint32_t kAccountAmendmentResultTimeout(1800);  // ½ hour
// max. no. of account amendments
const size_t kMaxAccountAmendments(1000);
// max. no. of repeated account amendments (i.e. for same chunk to same PMID)
const size_t kMaxRepeatedAccountAmendments(10);
const int kValidityCheckRetry(2);  // retries for validity check (timeouts)
const boost::uint8_t kMinChunkCopies(4);
const int kMaxChunkLoadRetries(3);  // max number of tries to load a chunk
// max number of tries to store or update a chunk
const int kMaxChunkStoreTries(2);
// max number of peers to try to store a chunk copy on
const boost::uint8_t kMaxStoreFailures(10);
// max number of store retries per peer
const boost::uint8_t kMaxPerPeerStoreFailures(2);
// max number of tries to add to watch list for a chunk
const boost::uint8_t kMaxAddToWatchListTries(3);
// max number of tries to remove from watch list for a chunk
const boost::uint8_t kMaxRemoveFromWatchListFailures(3);
// TODO(Fraser#5#): 2010-01-29 - Move the kMaxSmallChunkSize to be set and held
//                               by session depending on connection speed, etc.
// max size (bytes) of a chunk deemed "small"
const boost::uint64_t kMaxSmallChunkSize(666666);
// max no of dbs in save queue before running save queue
const boost::uint32_t kSaveUpdatesTrigger(10);
const double kMinSuccessfulPecentageOfUpdating(0.9);
const double kMinSuccessfulPecentageStore(0.75);
// port where the service to register a local vault is listening
const boost::uint16_t kLocalPort = 5483;
// additionally paying PMIDs kept in watch lists
const int kMaxReserveWatchListEntries = 250;
// time a watcher is kept in the ChunkInfoHandler's waiting list
const int kChunkInfoWatcherPendingTimeout = 86400;  // 24 hours
// time until a chunk holder is not considered active anymore
const int kChunkInfoRefActiveTimeout = 86400;  // 24 hours
// min. no. of responses required out of k
//  const boost::uint16_t kK(16);
//  const boost::uint16_t kKadUpperThreshold(static_cast<boost::uint16_t>(kK *
//                                         kad::kMinSuccessfulPecentageStore));
//  const boost::uint16_t kKadLowerThreshold(
//      kad::kMinSuccessfulPecentageStore > .25 ?
//      static_cast<boost::uint16_t>(kK * .25) : kKadUpperThreshold);

enum MaidsafeRpcResult {
  kNack, kAck, kNotRemote, kBusy
};

enum DirType {ANONYMOUS, PRIVATE, PRIVATE_SHARE, PUBLIC_SHARE};

enum ValueType {
  SYSTEM_PACKET, BUFFER_PACKET, BUFFER_PACKET_INFO, BUFFER_PACKET_MESSAGE,
  CHUNK_REFERENCE, WATCH_LIST, DATA, PDDIR_SIGNED, PDDIR_NOTSIGNED
};

enum SortingMode { ALPHA, RANK, LAST };

enum ShareFilter { kAll, kRo, kAdmin };

typedef boost::function<void(const ReturnCode&)> VoidFuncOneInt;
typedef boost::function<void(const ReturnCode&, const kad::Contact&)>
        VoidFuncIntContact;
typedef boost::function<void(const ReturnCode&,
    const std::vector<kad::Contact>&)> VoidFuncIntContacts;


inline std::string HexSubstr(const std::string &non_hex) {
  std::string hex(EncodeToHex(non_hex));
  if (hex.size() > 16)
    return (hex.substr(0, 7) + ".." + hex.substr(hex.size() - 7));
  else
    return hex;
}

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_SHARED_MAIDSAFE_H_
