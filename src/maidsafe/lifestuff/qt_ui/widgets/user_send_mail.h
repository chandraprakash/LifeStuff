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
 *      Author: Stephen Alexander
 */


#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_USER_SEND_MAIL_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_USER_SEND_MAIL_H_

#include <QWidget>
#include <QString>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// local
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"
#include "maidsafe/lifestuff/qt_ui/client/send_email_thread.h"
#include "maidsafe/lifestuff/qt_ui/client/save_file_thread.h"

#include "ui_user_send_mail.h"

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

class UserSendMail : public QDialog {
    Q_OBJECT

 public:
  explicit UserSendMail(QWidget* parent = 0);
  virtual ~UserSendMail();
	void addToRecipients(const QList<QString>&);
	void addSingleRecipient(const QString&);

 private:
  Ui::UserSendMail ui_;

 private slots:
  void onSendClicked(bool);
	void onSendEmailCompleted(int, const QString&);
  void onSaveFileCompleted(int, const QString&);

 signals:
  void sendEmailCompleted(int, const QString&);

 protected:
  void changeEvent(QEvent *event);
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_USER_SEND_MAIL_H_
