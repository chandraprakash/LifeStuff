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
 *  Created on: March 15, 2010
 *      Author: Stephen
 */

#ifndef MAIDSAFE_LIFESTUFF_CLIENT_SAVE_PROFILE_SETTINGS_THREAD_H_
#define MAIDSAFE_LIFESTUFF_CLIENT_SAVE_PROFILE_SETTINGS_THREAD_H_

#include <QWidget>
#include <QString>
#include <QHash>

#include "maidsafe/lifestuff/qt_ui/client/worker_thread.h"

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

// Worker thread for saving profile settings
/*!
    Saving Profile Information is blocking and can take a while so we
    use a worker thread to ensure that it doesn't block the main gui.

    Currently intended for single use.
*/
class SaveProfileSettingsThread : public WorkerThread {
  Q_OBJECT
 public:
  SaveProfileSettingsThread(QHash<QString, QString> theHash,
                             QObject* parent = 0);
  virtual ~SaveProfileSettingsThread();

  virtual void run();

 private:
  QHash<QString, QString> theHash_;
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_CLIENT_SAVE_PROFILE_SETTINGS_THREAD_H_
