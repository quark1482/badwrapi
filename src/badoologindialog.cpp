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
    lblIcon.setPixmap(
        QApplication::style()->standardPixmap(
            QStyle::StandardPixmap::SP_MessageBoxWarning
        )
    );
    lblWarn.setText(
        QStringLiteral("<i>Leave the password field blank to<br>"
                       "authenticate with PIN verification.</i>")
    );
    hblUser.addWidget(&lblUser);
    hblUser.addStretch();
    hblUser.addWidget(&ledUser);
    hblPass.addWidget(&lblPass);
    hblPass.addStretch();
    hblPass.addWidget(&ledPass);
    hblButtons.addStretch();
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);
    hblWarn.addWidget(&lblIcon);
    hblWarn.addWidget(&lblWarn);
    hblWarn.addStretch();
    vblMain.addLayout(&hblUser);
    vblMain.addLayout(&hblPass);
    vblMain.addLayout(&hblButtons);
    vblMain.addLayout(&hblWarn);
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
    else {
        if(ledPass.text().isEmpty())
            QMessageBox::warning(
                this,
                this->windowTitle(),
                QStringLiteral("Missing Password.\n\n"
                               "A verification pin will be sent to\n"
                               "%1").arg(ledUser.text())
            );
        ledUser.setEnabled(false);
        ledPass.setEnabled(false);
        btnOK.setEnabled(false);
        btnCancel.setEnabled(false);
        this->setCursor(Qt::CursorShape::BusyCursor);
        QApplication::processEvents();
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
        ledUser.setEnabled(true);
        ledPass.setEnabled(true);
        btnOK.setEnabled(true);
        btnCancel.setEnabled(true);
    }
}
