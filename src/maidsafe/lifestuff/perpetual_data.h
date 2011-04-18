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
 *  Created on: Mar 26, 2009
 *      Author: Team
 */

#ifndef MAIDSAFE_LIFESTUFF_PERPETUAL_DATA_H_
#define MAIDSAFE_LIFESTUFF_PERPETUAL_DATA_H_

// qt
#include <QMainWindow>
#include <QFileDialog>
#include <QTranslator>
#include <QLibraryInfo>
#include <QProcess>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// local
#include "maidsafe/lifestuff/client/client_controller.h"
#include "maidsafe/lifestuff/widgets/personal_messages.h"
#include "maidsafe/lifestuff/widgets/user_settings.h"
#include "maidsafe/lifestuff/widgets/user_inbox.h"
#include "maidsafe/lifestuff/client/save_file_thread.h"

// generated
#include "ui_pd.h"  //  NOLINT

class QLabel;
class Login;
class CreateUser;
class UserPanels;
class MountThread;
class Progress;
class PersonalMessages;
class UserSettings;
class UserInbox;
class PendingOperationsDialog;
class UserCalendar;
class LifeStuffLogin;
class LifeStuffFull;
#ifdef LifeStuff_LIGHT
  class FileBrowser;
#endif

// Main Window for Perpetual Data
/*!
    Structure is:

    Menu Bar
    --------------
    GUI Area


    --------------
    Status Bar

    The GUI area is a stacked widget that swtiches between different panels
    based on the current state.  There are 3 main states (at the moment)
     - logging in       (username, pin, password etc)
     - creating a user  (naming, paying etc)
     - being a user     (messaging, sharing, contacts)
*/
class PerpetualData : public QMainWindow {
  Q_OBJECT

 public:
    explicit PerpetualData(QWidget* parent = 0);
    virtual ~PerpetualData();
    QTranslator* qtTranslator;
    QTranslator* myAppTranslator;

 signals:
    // for updating other parts of the application like sys tray
    void inLoginState();
    void inSetupUserState();
    void inCreateUserState();
    void inLoggedInState();
    void inLoggingOutState();

  public slots:
    void quit();

  private slots:
    void onJoinKademliaCompleted(bool b);
    // Existing user logging in.
    void onLoginExistingUser();

    // New user needs creating
    void onLoginNewUser(const QString& username,
                        const QString& pin,
                        const QString& password);

    void onSetupNewUserComplete();
    void onSetupNewUserCancelled();

    // asyncCreate has completed
    void onUserCreationCompleted(bool success);

    // asyncMount has completed
    void onMountCompleted(bool success);
    void onUnmountCompleted(bool success);

    // Save session completed
    void onSaveSessionCompleted(int result);

    //
    void onFailureAcknowledged();

    void onMyFilesClicked();

    void onLogout();
    void onAbout();
    void onOpsComplete();
    void onLogoutUserCompleted(bool success);
    void onToggleFullScreen(bool b);
    void onApplicationActionTriggered();
    void onQuit();
    void onMyFiles();
    void onPrivateShares();
    void onGoOffline(bool b);
    void onSaveSession();
    void onSettingsTriggered();
    void onOnlineTriggered();
    void onAwayTriggered();
    void onBusyTriggered();
    void onOffline_2Triggered();
    void onEmailTriggered();
    void onCalendarTriggered();
    void onOffTriggered();
    void onSmallTriggered();
    void onFullTriggered();
    void onManualTriggered();
    void onPublicUsernameChosen();
    void onUpdateTriggered();
    void onBlackThemeTriggered();
    void onBlueThemeTriggered();
    void onGreenThemeTriggered();
    void onRedThemeTriggered();
    void onFullViewTriggered();

    void onUpdateChecked(int, QProcess::ExitStatus);

    void showLoggedOutMenu();
    void showLoggedInMenu();

    void onDirectoryEntered(const QString&);

    void onMessageReceived(int,
                           const QDateTime& time,
                           const QString& sender,
                           const QString& message,
                           const QString& conversation);

    void onShareReceived(const QString&, const QString&);
    void onFileReceived(const QString &sender, const QString &filename,
                        const QString &tag, int sizeLow, int sizeHigh,
                        const ClientController::ItemType &type);

    void onEmailReceived(const QString &subject, const QString &conversation,
                         const QString &message, const QString &sender,
                         const QString &date);
    void onSaveFileCompleted(int, const QString&);
    void onConnectionStatusChanged(int status);

    void onUnreadMessagesChanged(int count);

    void onLangChanged(const QString &lang);

    // slot invoked when login or logout is in progress
    // and we do not want to process any actions like
    // button press or menu selection
    void enableInputs(bool);  //NOLINT
    void positionLSWinInCenter();
    void positionWidgetInScreenCenter(QWidget *);  //NOLINT
    void onShowWindowRequest();   // example when user click on sys tray icon

 public:
    void showLoginWindow();

 private:
  Ui::PerpetualData ui_;
  QFileDialog *qfd_;


  // Actions
  void createActions();
  enum Action {
    LOGOUT,
    FULLSCREEN,
    QUIT,
    ABOUT,
    MY_FILES,
    PRIVATE_SHARES,
    GO_OFFLINE,
    SAVE_SESSION,
    SETTINGS,
    ONLINE,
    BUSY,
    AWAY,
    OFFLINE_2,
    EMAIL,
    CALENDAR,
    OFF,
    SMALL,
    FULL,
    MANUAL,
    UPDATE,
    THEME_GREEN,
    THEME_BLUE,
    THEME_BLACK,
    THEME_RED
    };
  typedef QMap<Action, QAction*> ActionMap;
  ActionMap actions_;

  // Adds any dyncamic actions to the application menu
  void createMenus();

  // Application state
  /*!
      Typical progression:
      LOGIN -> (SETUP -> CREATE ->) MOUNT -> LOGGED_IN

      SETUP/CREATE are only required for new users

  */
  enum State {
    LOGIN,          //  < Gathering user credentials
    SETUP_USER,     //  < Setup a new user
    CREATE_USER,    //  < Create a new user
    MOUNT_USER,     //  < Mount user space file system
    LOGGED_IN,      //  < User logged in
    LOGGING_OUT,    //  < Logging user out
    FAILURE         //  < Something critical failed.  Showing message before
                    //  < returning to login
  };


  // flag set to true when application is quitting
  bool quitting_;

  // Login screen
  Login* login_;

  // Create user wizard
  CreateUser* create_;

  // General purpose progress page
  /*!
      Used to show progress of:
       - creating user
       - mounting user
       - logging users out
  */
  Progress* progressPage_;

  // User level pages - shown once logged in
  UserPanels* userPanels_;

  // Status bar label for message count
  QLabel* message_status_;

  // User Settings Window
  UserSettings* settings_;

  // Email Inbox
  UserInbox* inbox_;

  // Pending Ops
  PendingOperationsDialog* pendingOps_;

  // User Calendar
  UserCalendar* userCal_;

  LifeStuffLogin* lsLogin_;

  LifeStuffFull* lifeStuffFull_;

#ifdef LifeStuff_LIGHT
  FileBrowser* browser_;
#endif

  // Switch between different application states
  void setState(State state);
  State state_;

  // Uses MountThread to perform a non-blocking mount
  /*!
      Success or failure is indicated via onMountComplete
  */
  void asyncMount();
  void asyncCreateUser();
  void asyncUnmount();
  void hideOpButtons();
  void showOpButtons();

  private:
      void initSignals();
      void updateTitlebar(const QString&);
  protected:
  void changeEvent(QEvent *event);
};

#endif  // MAIDSAFE_LIFESTUFF_PERPETUAL_DATA_H_

