
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
 *  Created on: May 5, 2009
 *      Author: Team
 */

#ifndef MAIDSAFE_LIFESTUFF_CLIENT_MOUNT_THREAD_H_
#define MAIDSAFE_LIFESTUFF_CLIENT_MOUNT_THREAD_H_

#include "maidsafe/lifestuff/qt_ui/client/worker_thread.h"


// Worker thread for mounting
/*!
    Mounting is blocking and can take a while so we use a worker thread
    to ensure that it doesn't block the main gui.

    Currently intended for single use.
*/
class MountThread : public WorkerThread {
  Q_OBJECT
 public:
  enum MountAction {
    MOUNT,
    UNMOUNT
  };

  MountThread(MountAction action, QObject* parent = 0);
  virtual ~MountThread();

  virtual void run();

 private:
  MountAction action_;
};

#endif  // MAIDSAFE_LIFESTUFF_CLIENT_MOUNT_THREAD_H_

