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

#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_CREATE_PAGE_LICENSE_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_CREATE_PAGE_LICENSE_H_

// qt
#include <QWizardPage>

// generated
#include "ui_create_page_license.h"

class QWizardPage;

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

class CreateLicensePage : public QWizardPage {
  Q_OBJECT

 public:
  explicit CreateLicensePage(QWidget* parent = 0);
  virtual ~CreateLicensePage();

  virtual bool isComplete() const;
  virtual void cleanupPage();

 private:
  Ui::license ui_;

  private slots:
    void onAcceptToggled(bool);

 protected:
  void changeEvent(QEvent *event);
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_CREATE_PAGE_LICENSE_H_

