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
 *  Created on: Apr 12, 2009
 *      Author: Team
 */

#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_SHARES_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_SHARES_H_

// local
#include "maidsafe/lifestuff/qt_ui/widgets/panel.h"
#include "maidsafe/lifestuff/qt_ui/client/client_controller.h"

// generated
#include "ui_user_shares_panel.h"

// Custom widget that displays shares
/*!
    Displays a list of current shares and lets you create new ones.

    TODO - share creation
    TODO - distinguish types of shares, who's in the share
    TODO - modify existing shares.
*/
class Shares : public Panel {
  Q_OBJECT
 public:
  explicit Shares(QWidget* parent = 0);
  virtual ~Shares();

  // Panel interfaces
  virtual void setActive(bool b);
  virtual void reset();

  int sortType_;
  int filterType_;

  private slots:
    void onCreateShareClicked();
    void onSharesBoxLostFocus();
    void onSharesBoxTextEdited(const QString &value);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onAddedPrivateShare(const QString &name);
    void onCreateShareCompleted(bool b);
    void onShareFilterChanged(int index);

 protected:
  bool eventFilter(QObject *obj, QEvent *ev);
  void changeEvent(QEvent *event);

 private:
  // Initialize the display of a user's shares
  void init();

  // Put a new entry in the list of shares
  void addShare(const QString& shareName);

  Ui::SharesPage ui_;
  bool init_;
  QString shareInProcess_;
  ShareList shares_;

 signals:
  void complete();
};

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_SHARES_H_
