
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

#include "maidsafe/lifestuff/qt_ui/client/check_user_thread.h"

// qt
#include <QDebug>

// std
#include <string>

// core
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"

// local

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {


CheckUserThread::CheckUserThread(const QString& username,
                                   const QString& pin,
                                   QObject* parent)
    : WorkerThread(parent), username_(username), pin_(pin) { }

CheckUserThread::~CheckUserThread() { }

void CheckUserThread::run() {
  qDebug() << "CheckUserThread::run";
  const std::string username = username_.toStdString();
  const std::string pin = pin_.toStdString();

  bool result = ClientController::instance()->
    CheckUserExists(username, pin, ClientController::kDefCon3);

  if (result) {
    emit completed(false);
  } else {
    emit completed(true);
  }

  deleteLater();
}

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe
