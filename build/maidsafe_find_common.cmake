#==============================================================================#
#                                                                              #
# Copyright [2011] maidsafe.net limited                                        #
#                                                                              #
# Description:  See below.                                                     #
# Version:      1.0                                                            #
# Created:      2011-03-20-23.22.31                                            #
# Revision:     none                                                           #
# Company:      maidsafe.net limited                                           #
#                                                                              #
# The following source code is property of maidsafe.net limited and is not     #
# meant for external use.  The use of this code is governed by the license     #
# file LICENSE.TXT found in the root of this directory and also on             #
# www.maidsafe.net.                                                            #
#                                                                              #
# You are not free to copy, amend or otherwise use this source code without    #
# the explicit written permission of the board of directors of maidsafe.net.   #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Written by maidsafe.net team                                                #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Module used to locate MaidSafe-Common tools, cmake modules and the          #
#    maidsafe_common libs and headers.                                         #
#                                                                              #
#  Settable variables to aid with finding MaidSafe-Common are:                 #
#    MAIDSAFE_COMMON_INSTALL_DIR                                               #
#                                                                              #
#  If found, a target named maidsafe_common_static is imported.                #
#                                                                              #
#  Variables set and cached by this module are:                                #
#    MaidSafeCommon_INCLUDE_DIR, MaidSafeCommon_MODULES_DIR,                   #
#    MaidSafeCommon_TOOLS_DIR and MAIDSAFE_COMMON_VERSION.                     #
#                                                                              #
#==============================================================================#

UNSET(MaidSafeCommon_INCLUDE_DIR CACHE)
UNSET(MaidSafeCommon_MODULES_DIR CACHE)
UNSET(MaidSafeCommon_TOOLS_DIR CACHE)

IF(NOT MAIDSAFE_COMMON_INSTALL_DIR)
  IF(DEFAULT_THIRD_PARTY_ROOT)
    SET(MAIDSAFE_COMMON_INSTALL_DIR ${DEFAULT_THIRD_PARTY_ROOT})
  ELSE()
    SET(MAIDSAFE_COMMON_INSTALL_DIR ${PROJECT_SOURCE_DIR}/../MaidSafe-Common/installed)
  ENDIF()
ENDIF()

SET(MAIDSAFE_PATH_SUFFIX share/maidsafe)
FIND_FILE(MAIDSAFE_THIRD_PARTY_CMAKE maidsafe_third_party.cmake PATHS ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_FILE(BOOST_LIBS_CMAKE boost_libs.cmake PATHS ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
FIND_FILE(MAIDSAFE_COMMON_CMAKE maidsafe_common.cmake PATHS ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
IF(MAIDSAFE_THIRD_PARTY_CMAKE AND BOOST_LIBS_CMAKE AND MAIDSAFE_COMMON_CMAKE)
  INCLUDE(${MAIDSAFE_THIRD_PARTY_CMAKE})
  INCLUDE(${BOOST_LIBS_CMAKE})
  INCLUDE(${MAIDSAFE_COMMON_CMAKE})
ENDIF()

SET(MAIDSAFE_PATH_SUFFIX include)
FIND_PATH(MaidSafeCommon_INCLUDE_DIR maidsafe/common/version.h PATHS ${MAIDSAFE_COMMON_INC_DIR} ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

SET(MAIDSAFE_PATH_SUFFIX share/maidsafe/cmake_modules)
FIND_PATH(MaidSafeCommon_MODULES_DIR maidsafe_run_protoc.cmake PATHS ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

SET(MAIDSAFE_PATH_SUFFIX share/maidsafe/tools)
FIND_PATH(MaidSafeCommon_TOOLS_DIR cpplint.py PATHS ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MaidSafeCommon_MODULES_DIR})

SET(MAIDSAFE_PATH_SUFFIX ../../..)
FIND_PATH(DEFAULT_THIRD_PARTY_ROOT README PATHS ${MAIDSAFE_COMMON_INSTALL_DIR} PATH_SUFFIXES ${MAIDSAFE_PATH_SUFFIX} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

IF(NOT MaidSafeCommon_INCLUDE_DIR OR NOT MaidSafeCommon_MODULES_DIR OR NOT MaidSafeCommon_TOOLS_DIR)
  SET(ERROR_MESSAGE "${MaidSafeCommon_INCLUDE_DIR}\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${MaidSafeCommon_MODULES_DIR}\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}${MaidSafeCommon_TOOLS_DIR}\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\nCould not find MaidSafe Common.\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can clone it at git@github.com:maidsafe/MaidSafe-Common.git\n\n")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}If MaidSafe Common is already installed, run:")
  SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DMAIDSAFE_COMMON_INSTALL_DIR=<Path to MaidSafe Common install directory>\n\n")
  MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
ENDIF()

MESSAGE("-- Found MaidSafe Common library (version ${MAIDSAFE_COMMON_VERSION})")
MESSAGE("-- Found MaidSafe Common Debug library (version ${MAIDSAFE_COMMON_VERSION})")