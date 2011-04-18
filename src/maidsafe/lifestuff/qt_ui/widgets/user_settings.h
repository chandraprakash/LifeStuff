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
 *  Created on: Jan 23, 2010
 *      Author: Stephen Alexander
 */


#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_USER_SETTINGS_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_USER_SETTINGS_H_

#include <QWidget>
#include <QString>

// local
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"
#include "maidsafe/lifestuff/qt_ui/widgets/personal_settings.h"
#include "maidsafe/lifestuff/qt_ui/widgets/vault_info.h"
#include "maidsafe/lifestuff/qt_ui/widgets/connection_settings.h"
#include "maidsafe/lifestuff/qt_ui/widgets/file_transfer_settings.h"
#include "maidsafe/lifestuff/qt_ui/widgets/security_settings.h"
#include "maidsafe/lifestuff/qt_ui/widgets/profile_settings.h"
#include "ui_user_settings.h"

class SecuritySettings;
class FileTransferSettings;
class ConnectionSettings;
class PersonalSettings;
class VaultInfo;
class ProfileSettings;

class UserSettings : public QDialog {
    Q_OBJECT

 public:
  explicit UserSettings(QWidget* parent = 0);
  virtual ~UserSettings();

  private slots:
    void onCurrentRowChanged(int);
    void HandleOK();
    void HandleApply();
    void HandleCancel();
    void onSaveProfileSettingsCompleted(bool);
    void onSaveSecuritySettingsCompleted(bool);

  signals:
    void langChanged(const QString &lang);

 private:
  Ui::UserSettingsPage ui_;
  QStringList Pages;

  void createSettingsMenu();

  enum State {
    PERSONAL,
    CONNECTION,
    FILE_TRANSFER,
    SECURITY,
    VAULT_INFO,
    PROFILE
    };

  void setState(State state);
  State state_;

  PersonalSettings* personal_;
  VaultInfo* vault_;
  ConnectionSettings* connection_;
  FileTransferSettings* fileTransfer_;
  SecuritySettings* security_;
  ProfileSettings* profile_;

 protected:
  void changeEvent(QEvent *event);
};

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_USER_SETTINGS_H_
