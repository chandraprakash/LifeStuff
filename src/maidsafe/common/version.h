/*
* ============================================================================
*
* Copyright [2011] maidsafe.net limited
*
* Description:  Local version definition and DHT version check
* Created:      2011-01-27
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

#ifndef MAIDSAFE_COMMON_VERSION_H_
#define MAIDSAFE_COMMON_VERSION_H_

#define MAIDSAFE_LifeStuff_VERSION 1
#define MAIDSAFE_COMMON_VERSION 1

#include "maidsafe-dht/version.h"
#define THIS_MAIDSAFE_DHT_VERSION 26
#if MAIDSAFE_DHT_VERSION < THIS_MAIDSAFE_DHT_VERSION
#error This API is not compatible with the installed library.\
  Please update the maidsafe-dht library.
#elif MAIDSAFE_DHT_VERSION > THIS_MAIDSAFE_DHT_VERSION
#error This API uses a newer version of the maidsafe-dht library.\
  Please update this project.
#endif

#endif  // MAIDSAFE_COMMON_VERSION_H_
