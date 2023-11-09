#include "App.h"
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <string>
#include <sqlite3.h>


App::App(const Wt::WEnvironment& env) : Wt::WApplication(env), isLoggedIn_(false) {
    setTitle("User Registration");

    createUserTable();

    // Name field
    nameEdit_ = root()->addWidget(std::make_unique<Wt::WLineEdit>());
    nameEdit_->setPlaceholderText("Enter your name");

    // Email field
    emailEdit_ = root()->addWidget(std::make_unique<Wt::WLineEdit>());
    emailEdit_->setPlaceholderText("Enter your email");

    // Password field
    passwordEdit_ = root()->addWidget(std::make_unique<Wt::WLineEdit>());
    passwordEdit_->setPlaceholderText("Enter your password");
    passwordEdit_->setEchoMode(Wt::EchoMode::Password); // Hide password input

    // Register button
    registerButton_ = root()->addWidget(std::make_unique<Wt::WPushButton>("Register"));
    registerButton_->clicked().connect(this, &App::onRegister);

    // Response text
    responseText_ = root()->addWidget(std::make_unique<Wt::WText>());

    root()->addWidget(std::make_unique<Wt::WText>("<h2>Login</h2>"));

    showLogin();


    // User List Container
    usersListContainer_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    showUsers();

}

void App::onRegister() {
    insertUser(nameEdit_->text().toUTF8(), emailEdit_->text().toUTF8(), passwordEdit_->text().toUTF8());
}

void App::createUserTable() {
    sqlite3 *db;
    char *errMsg = nullptr;

    int rc = sqlite3_open("userdb.sqlite", &db);
    if (rc) {
        responseText_->setText("Cannot open database.");
        return;
    }

    const char *sql = 
        "CREATE TABLE IF NOT EXISTS users (" \
        "id INTEGER PRIMARY KEY AUTOINCREMENT," \
        "name TEXT NOT NULL," \
        "email TEXT UNIQUE NOT NULL," \
        "password TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        responseText_->setText("Failed to create table.");
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
}

void App::insertUser(const std::string& name, const std::string& email, const std::string& password) {
    sqlite3 *db;
    char *errMsg = nullptr;

    int rc = sqlite3_open("userdb.sqlite", &db);
    if (rc) {
        responseText_->setText("Cannot open database.");
        return;
    }

    std::string sql = "INSERT INTO users (name, email, password) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {
        // Bind the parameters
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT);

        // Execute the statement
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            responseText_->setText("User registered successfully!");
        } else {
            responseText_->setText("Registration failed: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_finalize(stmt);
    } else {
        responseText_->setText("Preparation failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_close(db);
}


void App::showUsers() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int rc = sqlite3_open("userdb.sqlite", &db);
    if (rc) {
        responseText_->setText("Cannot open database.");
        return;
    }

    const char *sql = "SELECT name, email FROM users";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        responseText_->setText("Failed to fetch users.");
        sqlite3_close(db);
        return;
    }

    usersListContainer_->clear(); // Clear the container before displaying new results

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *name = sqlite3_column_text(stmt, 0);
        const unsigned char *email = sqlite3_column_text(stmt, 1);
        usersListContainer_->addWidget(std::make_unique<Wt::WText>(
            std::string(reinterpret_cast<const char*>(name)) + " - " +
            std::string(reinterpret_cast<const char*>(email))
        ));
        usersListContainer_->addWidget(std::make_unique<Wt::WBreak>()); // Line break between users
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

}


void App::showLogin() {
    loginContainer_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    loginEmailEdit_ = loginContainer_->addWidget(std::make_unique<Wt::WLineEdit>());
    loginEmailEdit_->setPlaceholderText("Email");

    loginPasswordEdit_ = loginContainer_->addWidget(std::make_unique<Wt::WLineEdit>());
    loginPasswordEdit_->setPlaceholderText("Password");
    loginPasswordEdit_->setEchoMode(Wt::EchoMode::Password);

    loginButton_ = loginContainer_->addWidget(std::make_unique<Wt::WPushButton>("Login"));
    loginButton_->clicked().connect(this, &App::onLogin);

    loginMessage_ = loginContainer_->addWidget(std::make_unique<Wt::WText>());
}


void App::onLogin() {
    const std::string email = loginEmailEdit_->text().toUTF8();
    const std::string password = loginPasswordEdit_->text().toUTF8();
    
    sqlite3 *db;
    int rc = sqlite3_open("userdb.sqlite", &db);
    if (rc != SQLITE_OK) {
        loginMessage_->setText("Cannot open database.");
        return;
    }

    const char *sql = "SELECT password FROM users WHERE email = ?;";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *dbPassword = sqlite3_column_text(stmt, 0);
            if (password == reinterpret_cast<const char*>(dbPassword)) {
                isLoggedIn_ = true;
                loginMessage_->setText("Login successful.");
                // Hide login widgets and show logout button
                loginContainer_->hide();
                // Add other application functionality for logged-in users here
            } else {
                loginMessage_->setText("Login failed: Incorrect password.");
            }
        } else {
            loginMessage_->setText("Login failed: User not found.");
        }
        sqlite3_finalize(stmt);
    } else {
        loginMessage_->setText("Login failed: Unable to query database.");
    }
    sqlite3_close(db);
}

void App::onLogout() {
    // Clear the session and show the login form again
    isLoggedIn_ = false;
    loginContainer_->show();
    loginMessage_->setText("You have been Logout");
    // Hide or clear other application content related to the user session
}