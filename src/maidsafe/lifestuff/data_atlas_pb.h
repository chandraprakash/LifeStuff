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


#ifndef MAIDSAFE_LIFESTUFF_DATA_ATLAS_PB_H_
#define MAIDSAFE_LIFESTUFF_DATA_ATLAS_PB_H_

#ifdef __MSVC__
#  pragma warning(push)
#  pragma warning(disable: 4127 4244 4267)
#endif
#include "maidsafe/lifestuff/data_atlas.pb.h"
#ifdef __MSVC__
#  pragma warning(pop)
#endif

#include "maidsafe/lifestuff/version.h"

#if MAIDSAFE_LIFESTUFF_VERSION != 400
#  error This API is not compatible with the installed library.\
    Please update the maidsafe-lifestuff library.
#endif

#endif  // MAIDSAFE_LIFESTUFF_DATA_ATLAS_PB_H_
