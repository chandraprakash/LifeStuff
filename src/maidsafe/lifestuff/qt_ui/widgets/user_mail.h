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


#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_USER_MAIL_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_USER_MAIL_H_

#include <QWidget>
#include <QString>

// local
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"
#include "maidsafe/lifestuff/qt_ui/widgets/user_inbox.h"
#include "maidsafe/lifestuff/qt_ui/widgets/user_send_mail.h"

#include "ui_user_mail.h"

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

class UserMail : public QDialog {
    Q_OBJECT

 public:
  explicit UserMail(QWidget* parent = 0);
  virtual ~UserMail();

  private slots:
    void onCurrentRowChanged(int);

 private:
  Ui::UserMail ui_;
	UserInbox* userInbox_;
  UserSendMail* userSendMail_;

  void createSettingsMenu();

  enum State {
    SENT,
    INBOX
	};

  void setState(State state);
  State state_;

 protected:
  void changeEvent(QEvent *event);
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_USER_MAIL_H_
