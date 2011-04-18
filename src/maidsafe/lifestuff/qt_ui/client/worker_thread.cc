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
 *  Created on: May 20, 2009
 *      Author: Team
 */

#include "maidsafe/lifestuff/qt_ui/client/worker_thread.h"

// qt
#include <QDebug>

WorkerThread::WorkerThread(QObject* parent)
    : QThread(parent) { }

WorkerThread::~WorkerThread() {
  qDebug() << "WorkerThread >> DTOR";

  quit();  // the event loop
  wait();  // until run has exited

  qDebug() << "WorkerThread << DTOR";

  if (isRunning() || !isFinished()) {
    qDebug() << "\nWorkerThread - not shutdown";
  }
}



