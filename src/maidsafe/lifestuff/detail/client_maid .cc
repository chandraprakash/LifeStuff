/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#include "maidsafe/lifestuff/detail/client_maid.h"

namespace maidsafe {
namespace lifestuff {

ClientMaid::ClientMaid(ClientNfs& client_nfs)
  : client_nfs_(client_nfs) {}

void ClientMaid::CreateUser(const Keyword& /*keyword*/, const Pin& /*pin*/, const Password& /*password*/) {

}

void ClientMaid::MountDrive() {

}

}  // lifestuff
}  // maidsafe