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
 *  Created on: May 19, 2010
 *      Author: Stephen
 */

#include "maidsafe/lifestuff/qt_ui/client/get_email_thread.h"

// qt
#include <QDebug>

// core
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"


namespace maidsafe {

namespace lifestuff {

namespace qt_ui {


GetEmailThread::GetEmailThread(const QString& subject, QObject* parent)
    : WorkerThread(parent), subject_(subject) {}

GetEmailThread::~GetEmailThread() { }

void GetEmailThread::run() {
//  qDebug() << "SendEmailThread::run" << text_;
//
//  int success = ClientController::instance()->newEmailSendMessage();
//
//  emit sendEmailCompleted(success, text_);
}

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe
