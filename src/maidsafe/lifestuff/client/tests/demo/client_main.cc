/*
* ============================================================================
*
* Copyright [2011] maidsafe.net limited
*
* Version:      1.0
* Created:      2011-04-19
* Author:       Team
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

#include <iostream>  //NOLINT
#include <memory>
#include <string>

#include "maidsafe/lifestuff/log.h"
#include "maidsafe/lifestuff/client/clientcontroller.h"
#include "maidsafe/lifestuff/client/tests/demo/commands.h"
#include "maidsafe/lifestuff/client/user_credentials_api.h"


int main(int /*argc*/, char *argv[]) {
  // Initialising logging
  google::InitGoogleLogging(argv[0]);
  // Choose to direct output to stderr or not.
  FLAGS_logtostderr = true;
  // If Google logging is linked in, log messages at or above this level.
  // Severity levels are INFO, WARNING, ERROR, and FATAL (0 to 3 respectively).
  FLAGS_minloglevel = google::INFO;
  FLAGS_ms_logging_common = false;
  FLAGS_ms_logging_pki = false;
  FLAGS_ms_logging_passport = false;
  std::cout << "LifeStuff Demo" << std::endl;

  std::shared_ptr<maidsafe::lifestuff::ClientController> cc(
      new maidsafe::lifestuff::ClientController);
  cc->Init(0);
  std::shared_ptr<maidsafe::lifestuff::UserCredentials> user_credentials = cc;
  maidsafe::lifestuff::commandline_demo::Commands commands(user_credentials);

  commands.Run();
  cc->CloseConnection(false);
}
