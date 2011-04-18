
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

#include "maidsafe/lifestuff/qt_ui/client/add_contact_thread.h"

// qt
#include <QDebug>

// core
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"

// local

AddContactThread::AddContactThread(const QString& publicUsername,
                                   QObject* parent)
    : WorkerThread(parent), publicUsername_(publicUsername) { }

AddContactThread::~AddContactThread() { }

void AddContactThread::run() {
  qDebug() << "AddContactThread::run";
//  #ifdef DEBUG
//    boost::this_thread::sleep(boost::posix_time::seconds(2));
//    qDebug() << "AddContactThread::run - After SLEEP";
//  #endif
  const int n = ClientController::instance()->addContact(publicUsername_);

  emit completed(n, publicUsername_);

  deleteLater();
}


