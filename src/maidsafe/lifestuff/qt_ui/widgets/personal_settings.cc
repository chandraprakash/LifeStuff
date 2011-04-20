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
#include <QDebug>

#include "maidsafe/lifestuff/qt_ui/widgets/personal_settings.h"
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

PersonalSettings::PersonalSettings(QWidget* parent)
    : QWidget(parent), init_(false) {
  ui_.setupUi(this);

    connect(ui_.usernameEdit, SIGNAL(textEdited(const QString&)),
          this,           SLOT(onUsernameTextEdit(const QString&)));

    connect(ui_.messageEdit, SIGNAL(textEdited(const QString&)),
          this,           SLOT(onMessageTextEdit(const QString&)));

    connect(ui_.pushButtonPicture, SIGNAL(clicked(bool)),
          this,           SLOT(onPicChangeClicked(bool)));
}

PersonalSettings::~PersonalSettings() { }

void PersonalSettings::setActive(bool b) {
  if (b && !init_) {
    init_ = true;
    ui_.usernameEdit->setText(ClientController::instance()->publicUsername());
    ui_.messageEdit->setText("Hello, this is my message!");

    QDir dir(":/translations/");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    QStringList theList = dir.entryList();
    foreach(QString tFile, theList) {
      tFile.remove("pd_translation_", Qt::CaseInsensitive);
      tFile.remove(".qm", Qt::CaseInsensitive);
      QLocale locale(tFile);
      ui_.langListWidget->addItem(locale.languageToString(locale.language())
                                + "(" + tFile + ")");
    }

    connect(ui_.langListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            this,               SLOT(onLangSelect(QListWidgetItem*)));
  }
}

void PersonalSettings::reset() { }

void PersonalSettings::onUsernameTextEdit(const QString& text) {
  changedValues_.insert("username", text);
}

void PersonalSettings::onMessageTextEdit(const QString& text) {
  changedValues_.insert("message", text);
}

void PersonalSettings::onPicChangeClicked(bool) {
  changedValues_.insert("changepic", "");
}

void PersonalSettings::onLangSelect(QListWidgetItem* item) {
  changedValues_.insert("language", item->text());
}

void PersonalSettings::changeEvent(QEvent *event) {
  if (event->type() == QEvent::LanguageChange) {
    ui_.retranslateUi(this);
  } else {
    QWidget::changeEvent(event);
  }
}

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe
