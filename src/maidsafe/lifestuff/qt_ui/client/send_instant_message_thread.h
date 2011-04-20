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
 *  Created on: March 02, 2010
 *      Author: Stephen Alexander
 */

#ifndef MAIDSAFE_LIFESTUFF_CLIENT_SEND_INSTANT_MESSAGE_THREAD_H_
#define MAIDSAFE_LIFESTUFF_CLIENT_SEND_INSTANT_MESSAGE_THREAD_H_

#include "maidsafe/lifestuff/qt_ui/client/worker_thread.h"

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

// Worker thread for sending instant messages
/*!
    sending an instant message over the network is blocking and can take
    a while so we use a worker thread to ensure that it doesn't block the main
    gui.
    Currently intended for single use.
*/
class SendInstantMessageThread : public WorkerThread {
  Q_OBJECT
 public:
  SendInstantMessageThread(const QString& text, const QString& convName,
                           QList<QString> conts, QObject* parent = 0);
  virtual ~SendInstantMessageThread();

  virtual void run();

 private:
  QString text_;
  QList<QString> conts_;
  QString convName_;

  signals:
    void sendMessageCompleted(bool, const QString&);
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  // MAIDSAFE_LIFESTUFF_CLIENT_SEND_INSTANT_MESSAGE_THREAD_H_
