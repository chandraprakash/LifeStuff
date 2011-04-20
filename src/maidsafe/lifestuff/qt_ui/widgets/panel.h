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
 *  Created on: Apr 11, 2009
 *      Author: Team
 */

#ifndef MAIDSAFE_LIFESTUFF_WIDGETS_PANEL_H_
#define MAIDSAFE_LIFESTUFF_WIDGETS_PANEL_H_

// qt
#include <QWidget>

namespace maidsafe {

namespace lifestuff {

namespace qt_ui {

// Base class for UI Panels
/*!
    Panels are used to display user information.
    The application uses a series of panels to display information
    and switches between them.
*/
class Panel : public QWidget {
  Q_OBJECT

 public:
  explicit Panel(QWidget* parent = 0)
           : QWidget(parent ) {}
  virtual ~Panel() {}

  virtual void reset() {}
  virtual void setActive(bool) {}
};

}  // namespace qt_ui

}  // namespace lifestuff

}  // namespace maidsafe

#endif  //  MAIDSAFE_LIFESTUFF_WIDGETS_PANEL_H_
