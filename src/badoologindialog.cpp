#include "badoologindialog.h"

BadooLoginDialog::BadooLoginDialog(BadooWrapper *bwParent,
                                   QWidget      *wgtParent):
QDialog(wgtParent) {
    bwWrapper=bwParent;
    this->setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    this->setWindowTitle(QStringLiteral("Login"));
    lblUser.setText(QStringLiteral("Email / Phone:"));
    lblPass.setText(QStringLiteral("Password:"));
    ledPass.setEchoMode(QLineEdit::EchoMode::Password);
    btnOK.setText(QStringLiteral("OK"));
    btnOK.setDefault(true);
    btnCancel.setText(QStringLiteral("Cancel"));
    hblUser.addWidget(&lblUser);
    hblUser.addStretch();
    hblUser.addWidget(&ledUser);
    hblPass.addWidget(&lblPass);
    hblPass.addStretch();
    hblPass.addWidget(&ledPass);
    hblButtons.addStretch();
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);
    vblMain.addLayout(&hblUser);
    vblMain.addLayout(&hblPass);
    vblMain.addLayout(&hblButtons);
    this->setLayout(&vblMain);
    // Performs validations and accepts the dialog, if the OK button is pressed.
    connect(
        &btnOK,
        &QPushButton::clicked,
        this,
        &BadooLoginDialog::okButtonClicked
    );
    // Rejects the dialog if the Cancel button is pressed.
    connect(
        &btnCancel,
        &QPushButton::clicked,
        this,
        &BadooLoginDialog::reject
    );
}

bool BadooLoginDialog::show() {
    return QDialog::DialogCode::Accepted==this->exec();
}

void BadooLoginDialog::okButtonClicked(bool) {
    ledUser.setText(ledUser.text().trimmed());
    if(ledUser.text().isEmpty()) {
        QMessageBox::critical(
            this,
            this->windowTitle(),
            QStringLiteral("Missing Email / Phone")
        );
        ledUser.setFocus();
    }
    else if(ledPass.text().isEmpty()) {
        QMessageBox::critical(
            this,
            this->windowTitle(),
            QStringLiteral("Missing Password")
        );
        ledPass.setFocus();
    }
    else {
        this->setCursor(Qt::CursorShape::BusyCursor);
        bwWrapper->logout();
        if(bwWrapper->login(ledUser.text(),ledPass.text()))
            this->accept();
        else
            QMessageBox::critical(
                this,
                this->windowTitle(),
                bwWrapper->getLastError()
            );
        this->unsetCursor();
    }
}
