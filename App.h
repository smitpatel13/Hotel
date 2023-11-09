#ifndef APP_H
#define APP_H

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <sqlite3.h>


class App : public Wt::WApplication {
public:
    App(const Wt::WEnvironment& env);

private:
    Wt::WLineEdit *nameEdit_;
    Wt::WLineEdit *emailEdit_;
    Wt::WLineEdit *passwordEdit_;
    Wt::WPushButton *registerButton_;
    Wt::WText *responseText_;

    Wt::WLineEdit *loginEmailEdit_;
    Wt::WLineEdit *loginPasswordEdit_;
    Wt::WPushButton *loginButton_;
    Wt::WText *loginMessage_;
    Wt::WContainerWidget *loginContainer_;

    bool isLoggedIn_;



    void onLogout();
    void showLogin();
    Wt::WContainerWidget *usersListContainer_;


    void onRegister();
    void onLogin();
    void createUserTable();
    void showUsers();
    void insertUser(const std::string& name, const std::string& email, const std::string& password);
    bool verifyCredentials(const std::string& email, const std::string& password);
};

#endif // APP_H
