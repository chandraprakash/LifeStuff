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
 *  Created on: March 23 2010
 *      Author: Stephen
 */

#include "maidsafe/lifestuff/qt_ui/client/remove_dir_thread.h"

// qt
#include <QDebug>

// core
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"


RemoveDirThread::RemoveDirThread(const QString& filepath, QObject* parent)
    : WorkerThread(parent), filepath_(filepath) { }

RemoveDirThread::~RemoveDirThread() { }

void RemoveDirThread::run() {
  qDebug() << "SaveFileThread::run" << filepath_;

  int success = ClientController::instance()->rmdir(filepath_);

  emit removeDirCompleted(success, filepath_);
}

