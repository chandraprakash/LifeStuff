/*
 * copyright maidsafe.net limited 2008
 * The following source code is property of maidsafe.net limited and
 * is not meant for external use. The use of this code is governed
 * by the license file LICENSE.TXT found in the root of this directory and also
 * on www.maidsafe.net.
 *
 * You are not free to copy, amend or otherwise use this source code without
 * explicit written permission of the board of directors of maidsafe.net
 *
 *  Created on: Mar 26, 2009
 *      Author: Team
 */

#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_CREATE_PAGE_LOCALVAULT_SETUP_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_CREATE_PAGE_LOCALVAULT_SETUP_H_

// qt
#include <QWizardPage>

#include <string>

// generated
#include "ui_create_page_localvault_setup.h"


class QWizardPage;

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

class CreateLocalVaultPage : public QWizardPage {
  Q_OBJECT

 public:
  explicit CreateLocalVaultPage(QWidget* parent = 0);
  virtual ~CreateLocalVaultPage();
  virtual void cleanupPage();
  virtual bool isComplete() const;
  QString SpaceOffered() const;
  QString PortChosen() const;
  QString DirectoryChosen() const;

 private:
  Ui::localVaultSetup ui_;
  bool spaceReady_, dirReady_;
  std::string availableSpace_;

  private slots:
    void onBrowseClicked();
    void onSpaceEdited(const QString& text);
    void onPortModified();
    void onDirectoryEdited(const QString& text);

 protected:
  void changeEvent(QEvent *event);
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_CREATE_PAGE_LOCALVAULT_SETUP_H_

