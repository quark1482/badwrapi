#include "badoolocationdialog.h"

BadooLocationDialog::BadooLocationDialog(BadooWrapper *bwParent,
                                         QWidget      *wgtParent):
QDialog(wgtParent) {
    QString                 sCurrentLocation;
    SessionDetails          sdDetails;
    BadooAPIError           baeError;
    BadooUserProfile        bupUser;
    BadooSearchLocationList bsllList;
    bwWrapper=bwParent;
    bwWrapper->getSessionDetails(sdDetails);
    this->setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    this->setWindowTitle(QStringLiteral("Change location"));
    lblLocation.setText(QStringLiteral("To"));
    cboLocation.setEditable(true);
    sCurrentLocationCode.clear();
    sCurrentLocation.clear();
    if(bwWrapper->getLoggedInProfile(bupUser)) {
        if(!bupUser.sCity.isEmpty()) {
            if(!sCurrentLocation.isEmpty())
                sCurrentLocation.append(QStringLiteral(", "));
            sCurrentLocation.append(bupUser.sCity);
        }
        if(!bupUser.sRegion.isEmpty()) {
            if(!sCurrentLocation.isEmpty())
                sCurrentLocation.append(QStringLiteral(", "));
            sCurrentLocation.append(bupUser.sRegion);
        }
        if(!bupUser.sCountry.isEmpty()) {
            if(!sCurrentLocation.isEmpty())
                sCurrentLocation.append(QStringLiteral(", "));
            sCurrentLocation.append(bupUser.sCountry);
        }
    }
    // This step just verifies that the location the user has logged from, ...
    // ... is actually searchable by the API (SERVER_SEARCH_LOCATIONS). So ...
    // ... the combo-box should never be empty after the dialog is loaded.
    if(BadooAPI::sendSearchLocations(
        sdDetails.sSessionId,
        sCurrentLocation,
        bsllList,
        baeError
    ))
        for(const auto &l:bsllList) {
            if(sCurrentLocation.startsWith(l.sLocationName)) {
                cboLocation.clear();
                cboLocation.addItem(
                    l.sLocationName,
                    QVariant::fromValue(
                        BadooIndexedString({
                            l.iLocationId,
                            l.sLocationCode
                        })
                    )
                );
                cboLocation.setCurrentIndex(0);
                sCurrentLocationCode=l.sLocationCode;
                break;
            }
        }
    vblLocation.addWidget(&lblLocation);
    vblLocation.addWidget(&cboLocation);
    btnOK.setText(QStringLiteral("OK"));
    btnOK.setDefault(true);
    btnCancel.setText(QStringLiteral("Cancel"));
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);
    vblMain.addLayout(&vblLocation);
    vblMain.addLayout(&hblButtons);
    this->setLayout(&vblMain);
    // Allows to tab away from the combo-box once an item is selected from the pop-up list.
    connect(
        &cboLocation,
        &QComboBox::activated,
        this,
        &BadooLocationDialog::locationItemSelected
    );
    // Avoids firing QLineEdit::textEdited again when the text is modified by code.
    disconnect(
        cboLocation.lineEdit(),
        &QLineEdit::editingFinished,
        nullptr,
        nullptr
    );
    // Adds a delay between text edits, to avoid unnecessary API calls.
    connect(
        cboLocation.lineEdit(),
        &QLineEdit::textEdited,
        this,
        &BadooLocationDialog::locationTextEdited
    );
    // Modifies the list of available locations according to what the user types.
    connect(
        &tmrLocation,
        &QTimer::timeout,
        this,
        &BadooLocationDialog::locationTextEditTimeout
    );
    // Performs just-in-case validations and accepts the dialog, if the OK button is pressed.
    connect(
        &btnOK,
        &QPushButton::clicked,
        this,
        &BadooLocationDialog::okButtonClicked
    );
    // Rejects the dialog if the Cancel button is pressed.
    connect(
        &btnCancel,
        &QPushButton::clicked,
        this,
        &BadooLocationDialog::reject
    );
    // Makes the location immediately writable.
    cboLocation.setFocus();
    cboLocation.lineEdit()->selectAll();
}

bool BadooLocationDialog::show() {
    return QDialog::DialogCode::Accepted==this->exec();
}

void BadooLocationDialog::locationItemSelected(int) {
    cboLocation.lineEdit()->releaseKeyboard();
}

void BadooLocationDialog::locationTextEdited(const QString &) {
    // Disables the auto-completer to allow typing from scratch, since the combo-box ...
    // ... could have old items which have not been cleared at this specific point.
    cboLocation.lineEdit()->setCompleter(nullptr);
    tmrLocation.start(500);
}

void BadooLocationDialog::locationTextEditTimeout() {
    int                     iMaxLocLen=QFontMetrics(cboLocation.font()).boundingRect(
                                QStringLiteral("City not found")
                            ).width(),
                            iDialogMargins=this->width()-cboLocation.width(),
                            iTextMargins=cboLocation.width()-cboLocation.lineEdit()->width(),
                            iScrollBarWidth=cboLocation.style()->pixelMetric(
                                QStyle::PixelMetric::PM_ScrollBarExtent
                            ),
                            iNewDialogWidth=iDialogMargins+iTextMargins;
    QString                 sText;
    QFontMetrics            ftmLocation(cboLocation.font());
    BadooAPIError           baeError;
    BadooSearchLocationList bsllList;
    SessionDetails          sdDetails;
    bwWrapper->getSessionDetails(sdDetails);
    sText=cboLocation.currentText();
    // Clears the previous list of available locations.
    cboLocation.clear();
    // Restores the text immediately for a better visual response.
    cboLocation.setEditText(sText);
    // Hides the possibly empty pop-up.
    cboLocation.hidePopup();
    if(sText.length()) {
        // Avoids calling the API for just a single-letter.
        if(sText.length()>1) {
            cboLocation.setEnabled(false);
            if(BadooAPI::sendSearchLocations(sdDetails.sSessionId,sText,bsllList,baeError))
                for(const auto &l:bsllList) {
                    // Calculates the width (in pixels) of the longest location in the ...
                    // ... drop-down list, the way no item is shown with an ellipsis.
                    int iThisLocLen=ftmLocation.boundingRect(l.sLocationName).width();
                    if(iMaxLocLen<iThisLocLen)
                        iMaxLocLen=iThisLocLen;
                    cboLocation.addItem(
                        l.sLocationName,
                        QVariant::fromValue(
                            BadooIndexedString({
                                l.iLocationId,
                                l.sLocationCode
                            })
                        )
                    );
                }
            if(!cboLocation.count()) {
                cboLocation.addItem(QStringLiteral("City not found"));
                auto *simModel=qobject_cast<QStandardItemModel *>(cboLocation.model());
                simModel->item(0,0)->setFlags(Qt::ItemFlag::NoItemFlags);
            }
            // Calculates the new width of the dialog, the way the location combo-box ...
            // ... line-edit is resized with the exact space to hold the longest item ...
            // ... available, so it can be fully visible without truncation.
            iNewDialogWidth+=iMaxLocLen;
            if(cboLocation.count()>cboLocation.maxVisibleItems())
                iNewDialogWidth+=iScrollBarWidth;
            if(iNewDialogWidth<this->sizeHint().width())
                iNewDialogWidth=this->sizeHint().width();
            if(iNewDialogWidth!=this->width()) {
                this->setFixedWidth(iNewDialogWidth);
                // Centers the now-resized dialog in its container.
                if(this->parentWidget()!=nullptr) {
                    QRect recParent=this->parentWidget()->geometry();
                    QRect recDialog=this->geometry();
                    recDialog.setLeft(recParent.left()+recParent.width()/2-recDialog.width()/2);
                    this->setGeometry(recDialog);
                }
            }
            cboLocation.setEnabled(true);
            cboLocation.showPopup();
        }
        if(cboLocation.currentText().startsWith(
            sText,
            Qt::CaseSensitivity::CaseInsensitive
        ))
            // Selects the auto-completed text, if possible.
            cboLocation.lineEdit()->setSelection(
                sText.length(),
                cboLocation.currentText().length()-sText.length()
            );
        else
            // Writes back what the user typed if no matching location was found.
            cboLocation.setEditText(sText);
        cboLocation.lineEdit()->grabKeyboard();
    }
    tmrLocation.stop();
}

void BadooLocationDialog::okButtonClicked(bool) {
    if(-1==cboLocation.currentIndex()||!cboLocation.currentData().isValid()) {
        // Shows a critical error if the SERVER_SEARCH_LOCATIONS API request failed ...
        // ... or the search did not find a single matching location result.
        QMessageBox::critical(
            this,
            this->windowTitle(),
            QStringLiteral("Missing location")
        );
        cboLocation.setFocus();
    }
    else {
        this->setCursor(Qt::CursorShape::BusyCursor);
        if(bwWrapper->setLocation(cboLocation.currentText()))
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
