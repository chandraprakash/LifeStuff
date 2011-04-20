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
 *  Created on: May 9, 2009
 *      Author: Team
 */

#include "maidsafe/lifestuff/qt_ui/client/make_directory_thread.h"

// qt
#include <QDebug>

// core
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"


namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

MakeDirectoryThread::MakeDirectoryThread(const QString& filepath,
                                         QObject* parent)
    : WorkerThread(parent), filepath_(filepath) { }

MakeDirectoryThread::~MakeDirectoryThread() { }

void MakeDirectoryThread::run() {
  qDebug() << "MakeDirectoryThread::run" << filepath_;

  int success = ClientController::instance()->mkdir(filepath_);

  emit makeDirectoryCompleted(success, filepath_);
}


}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe
