
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


#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_FILE_TRANSFER_SETTINGS_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_FILE_TRANSFER_SETTINGS_H_

#include <QWidget>
#include <QString>

// local
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"

#include "ui_file_transfer_settings.h"

class FileTransferSettings : public QWidget {
    Q_OBJECT
 public:
  explicit FileTransferSettings(QWidget* parent = 0);
  virtual ~FileTransferSettings();

  QHash<QString, QString> changedValues_;

 private:
  Ui::FileTransferPage ui_;

 protected:
  void changeEvent(QEvent *event);
};

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_FILE_TRANSFER_SETTINGS_H_
