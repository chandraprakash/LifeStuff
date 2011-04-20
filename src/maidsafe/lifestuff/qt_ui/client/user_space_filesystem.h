/*
 * copyright maidsafe.net limited 2009
 * The following source code is property of maidsafe.net limited and
 * is not meant for external use. The use of this code is governed
 * by the license file LICENSE.TXT found in the root of this directory and also
 * on www.maidsafe.net.
 *
 * You are not free to copy, amend or otherwise use this source code without
 * explicit written permission of the board of directors of maidsafe.net
 *
 *  Created on: May 19, 2009
 *      Author: Team
 */

#ifndef MAIDSAFE_LIFESTUFF_CLIENT_USER_SPACE_FILESYSTEM_H_
#define MAIDSAFE_LIFESTUFF_CLIENT_USER_SPACE_FILESYSTEM_H_

// qt
#include <QObject>
#include <QString>

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

// Manages access and control of the user space file systems
/*!

*/
class UserSpaceFileSystem : public QObject {
    Q_OBJECT
 public:
  static UserSpaceFileSystem* instance();

  // Mount the user space filesystem
  /*!
      This is blocking and takes a while.  Should normally only ever be
      called by MountThread
  */
  bool mount();

  // Unmount the user space filesystem
  /*!
      This is blocking and takes a while.  Should normally only ever be
      called by MountThread
  */
  bool unmount();

  enum Location {
      MY_FILES,
      PRIVATE_SHARES
  };

  // Explore a filesystem location
  /*!

  */
  void explore(Location l, QString subDir = QString());

 private:
  explicit UserSpaceFileSystem(QObject* parent = 0);
  virtual ~UserSpaceFileSystem();

  class UserSpaceFileSystemImpl;
  UserSpaceFileSystemImpl* impl_;
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_CLIENT_USER_SPACE_FILESYSTEM_H_





