#include "badoosearchsettingsdialog.h"

#define QML_RANGESLIDER R"(
import QtQuick.Controls
RangeSlider {
    stepSize: 1
    snapMode: RangeSlider.SnapAlways
}
)"

BadooSearchSettingsDialog::BadooSearchSettingsDialog(BadooSettingsContextType bsctNewContext,
                                                     BadooWrapper             *bwParent) {
    QObject              *objRangeSlider;
    BadooSexTypeList     bstlGenders;
    BadooIntRange        birAge,
                         birAgeRange;
    SessionDetails       sdDetails;
    EncountersSettings   esSettings;
    PeopleNearbySettings pnsSettings;

    bsctContext=bsctNewContext;
    bwWrapper=bwParent;

    bwWrapper->getSessionDetails(sdDetails);
    bwWrapper->getEncountersSettings(esSettings);
    bwWrapper->getPeopleNearbySettings(pnsSettings);

    // Configures a fixed-border dialog, with no minimize/maximize buttons and no system menu.
    this->setWindowFlag(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    this->setWindowFlag(Qt::WindowType::CustomizeWindowHint);
    this->setWindowFlag(Qt::WindowType::WindowSystemMenuHint,false);

    if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext) {
        this->setWindowTitle(QStringLiteral("Encounters settings"));
        bstlGenders=esSettings.bstlGenders;
        birAge=esSettings.birAge;
        birAgeRange=esSettings.birAgeRange;
    }
    else if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        this->setWindowTitle(QStringLiteral("People nearby settings"));
        bstlGenders=pnsSettings.bstlGenders;
        birAge=pnsSettings.birAge;
        birAgeRange=pnsSettings.birAgeRange;
    }

    // Section 'Intent'.
    if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        lblIntent.setText(QStringLiteral("I'm here to"));
        // Section contents (a combo-box).
        for(const auto &i:pnsSettings.biksvhOwnIntentHash.keys())
            cboIntent.addItem(pnsSettings.biksvhOwnIntentHash.value(i).sValue,i);
        cboIntent.model()->sort(0);
        cboIntent.setCurrentIndex(cboIntent.findData(pnsSettings.iOwnIntentId));
        // Adds section title to section layout.
        vblIntent.addWidget(&lblIntent);
        // Adds section contents to section layout.
        vblIntent.addWidget(&cboIntent);
    }

    // Section 'Show'.
    lblShow.setText(QStringLiteral("Show"));
    // Section contents (three buttons).
    btnGuys.setText(QStringLiteral("Guys"));
    btnGuys.setAutoExclusive(true);
    btnGuys.setCheckable(true);
    btnGirls.setText(QStringLiteral("Girls"));
    btnGirls.setAutoExclusive(true);
    btnGirls.setCheckable(true);
    btnBoth.setText(QStringLiteral("Both"));
    btnBoth.setAutoExclusive(true);
    btnBoth.setCheckable(true);
    if(-1!=bstlGenders.indexOf(BadooSexType::SEX_TYPE_MALE))
        if(-1!=bstlGenders.indexOf(BadooSexType::SEX_TYPE_FEMALE))
            btnBoth.setChecked(true);
        else
            btnGuys.setChecked(true);
    else
        if(-1!=bstlGenders.indexOf(BadooSexType::SEX_TYPE_FEMALE))
            btnGirls.setChecked(true);
    // Places contents in a horizontal layout.
    hblShow.addWidget(&btnGuys);
    hblShow.addWidget(&btnGirls);
    hblShow.addWidget(&btnBoth);
    // Adds section title to section layout.
    vblShow.addWidget(&lblShow);
    // Adds section contents to section layout.
    vblShow.addLayout(&hblShow);

    // Section 'Age'.
    lblAge.setText(QStringLiteral("Age"));
    lblAgeRange.setText(QStringLiteral("%1-%2").arg(birAge.first).arg(birAge.second));
    // Places titles in a horizontal layout.
    hblAge.addWidget(&lblAge);
    hblAge.addStretch();
    hblAge.addWidget(&lblAgeRange);
    // Section contents (a range slider).
    qkwAge.setSource(QUrl(
        QStringLiteral("data:text/plain;,%1").
        arg(QStringLiteral(QML_RANGESLIDER).toUtf8().toPercentEncoding())
    ));
    qkwAge.setMinimumHeight(qkwAge.height());
    qkwAge.setResizeMode(QQuickWidget::ResizeMode::SizeRootObjectToView);
    // Adds section title to section layout.
    vblAge.addLayout(&hblAge);
    // Adds section contents to section layout.
    vblAge.addWidget(&qkwAge);

    // Section 'Location'.
    if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        BadooAPIError           baeError;
        BadooSearchLocationList bsllList;
        lblLocation.setText(QStringLiteral("Where"));
        // Section contents (a combo-box).
        cboLocation.setEditable(true);
        // Initializes the combo-box with the single location read from the settings.
        // Unfortunately, this request is mandatory because the API does not store ...
        // ... the location 'full name' (country, region, city) but a short name.
        sCurrentLocationCode.clear();
        if(BadooAPI::sendSearchLocations(
            sdDetails.sSessionId,
            pnsSettings.sLocationName,
            bsllList,
            baeError
        ))
            for(const auto &l:bsllList)
                if(pnsSettings.iLocationId==l.iLocationId) {
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
        // Adds section title to section layout.
        vblLocation.addWidget(&lblLocation);
        // Adds section contents to section layout.
        vblLocation.addWidget(&cboLocation);
    }

    // Section 'Distance'.
    if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext) {
        lblDistance.setText(QStringLiteral("Distance"));
        lblDistanceAway.setText(QStringLiteral("%1 km away").arg(esSettings.iDistanceAway));
        // Places titles in a horizontal layout.
        hblDistance.addWidget(&lblDistance);
        hblDistance.addStretch();
        hblDistance.addWidget(&lblDistanceAway);
        // Section contents (a slider).
        sldDistance.setOrientation(Qt::Orientation::Horizontal);
        sldDistance.setMinimum(esSettings.birDistanceAwayRange.first);
        sldDistance.setMaximum(esSettings.birDistanceAwayRange.second);
        sldDistance.setValue(esSettings.iDistanceAway);
        // Adds section title to section layout.
        vblDistance.addLayout(&hblDistance);
        // Adds section contents to section layout.
        vblDistance.addWidget(&sldDistance);
    }
    else if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        lblDistance.setText(QStringLiteral("Distance"));
        // Section contents (a combo-box).
        auto *simModel=qobject_cast<QStandardItemModel *>(cboDistance.model());
        simModel->insertColumn(0);
        for(const auto &i:pnsSettings.bsksvhDistanceHash.keys()) {
            cboDistance.addItem(pnsSettings.bsksvhDistanceHash.value(i).sValue,i);
            simModel->setData(
                simModel->index(cboDistance.count()-1,1),
                pnsSettings.bsksvhDistanceHash.value(i).iIndex,
                Qt::ItemDataRole::UserRole
            );
        }
        simModel->setSortRole(Qt::ItemDataRole::UserRole);
        cboDistance.model()->sort(1);
        cboDistance.setCurrentIndex(cboDistance.findData(pnsSettings.sDistanceCode));
        // Adds section title to section layout.
        vblDistance.addWidget(&lblDistance);
        // Adds section contents to section layout.
        vblDistance.addWidget(&cboDistance);
    }

    // Section - the action buttons.
    btnOK.setText(QStringLiteral("OK"));
    btnOK.setDefault(true);
    btnCancel.setText(QStringLiteral("Cancel"));
    // Places buttons in a horizontal layout.
    hblButtons.addWidget(&btnOK);
    hblButtons.addWidget(&btnCancel);

    // Creates the dialog layout according to the current context.
    if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext) {
        vblMain.addLayout(&vblShow);
        vblMain.addLayout(&vblAge);
        vblMain.addLayout(&vblDistance);
        vblMain.addLayout(&hblButtons);
        this->setLayout(&vblMain);
    }
    else if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        vblMain.addLayout(&vblIntent);
        vblMain.addLayout(&vblShow);
        vblMain.addLayout(&vblAge);
        vblMain.addLayout(&vblLocation);
        vblMain.addLayout(&vblDistance);
        vblMain.addLayout(&hblButtons);
        this->setLayout(&vblMain);
    }

    // Sets the QQuickRangeSlider's background color to the dialog's background color.
    qkwAge.setClearColor(this->palette().color(QPalette::ColorRole::Window));
    // Gets a QObject from the QQuickRangeSlider.
    objRangeSlider=qkwAge.rootObject();
    // Gets a QObject from each one of the range 'handles'.
    objFirstHandle=qvariant_cast<QObject *>(
        objRangeSlider->property(QStringLiteral("first").toUtf8())
    );
    objSecondHandle=qvariant_cast<QObject *>(
        objRangeSlider->property(QStringLiteral("second").toUtf8())
    );
    // Sets the slider min and max values, according to the current settings.
    objRangeSlider->setProperty(
        QStringLiteral("from").toUtf8(),
        birAgeRange.first
    );
    objRangeSlider->setProperty(
        QStringLiteral("to").toUtf8(),
        birAgeRange.second
    );
    // Sets the slider handles position, according to the current settings.
    QMetaObject::invokeMethod(
        objRangeSlider,
        QStringLiteral("setValues").toUtf8(),
        Q_ARG(double,birAge.first),
        Q_ARG(double,birAge.second)
    );

    // Connects each slider handle valueChanged() signal to the same slot ...
    // ... which shows a human-readable range in the 'Age' section title.
    connect(
        objFirstHandle,
        SIGNAL(valueChanged()),
        this,
        SLOT(ageRangeSliderChanged())
    );
    connect(
        objSecondHandle,
        SIGNAL(valueChanged()),
        this,
        SLOT(ageRangeSliderChanged())
    );

    if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext) {
        // Modifies the 'Distance' section title according to its section slider.
        connect(
            &sldDistance,
            &QSlider::valueChanged,
            this,
            &BadooSearchSettingsDialog::distanceSliderChanged
        );
    }
    else if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        // Allows to tab away from the combo-box once an item is selected from the pop-up list.
        connect(
            &cboLocation,
            &QComboBox::activated,
            this,
            &BadooSearchSettingsDialog::locationItemSelected
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
            &BadooSearchSettingsDialog::locationTextEdited
        );
        // Modifies the list of available locations according to what the user types.
        connect(
            &tmrLocation,
            &QTimer::timeout,
            this,
            &BadooSearchSettingsDialog::locationTextEditTimeout
        );
    }

    // Performs just-in-case validations and accepts the dialog, if the OK button is pressed.
    connect(
        &btnOK,
        &QPushButton::clicked,
        this,
        &BadooSearchSettingsDialog::okButtonClicked
    );

    // Rejects the dialog if the Cancel button is pressed.
    connect(
        &btnCancel,
        &QPushButton::clicked,
        this,
        &BadooSearchSettingsDialog::reject
    );
}

bool BadooSearchSettingsDialog::show() {
    return QDialog::DialogCode::Accepted==this->exec();
}

void BadooSearchSettingsDialog::ageRangeSliderChanged() {
    lblAgeRange.setText(
        QStringLiteral("%1-%2").
        arg(objFirstHandle->property(QStringLiteral("value").toUtf8()).toInt()).
        arg(objSecondHandle->property(QStringLiteral("value").toUtf8()).toInt())
    );
}

void BadooSearchSettingsDialog::distanceSliderChanged(int iValue) {
    lblDistanceAway.setText(QStringLiteral("%1 km away").arg(iValue));
}

void BadooSearchSettingsDialog::locationItemSelected(int) {
    cboLocation.lineEdit()->releaseKeyboard();
}

void BadooSearchSettingsDialog::locationTextEdited(const QString &) {
    // Disables the auto-completer to allow typing from scratch, since the combo-box ...
    // ... could have old items which have not been cleared at this specific point.
    cboLocation.lineEdit()->setCompleter(nullptr);
    tmrLocation.start(500);
}

void BadooSearchSettingsDialog::locationTextEditTimeout() {
    QString                 sText;
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
                for(const auto &l:bsllList)
                    cboLocation.addItem(
                        l.sLocationName,
                        QVariant::fromValue(
                            BadooIndexedString({
                                l.iLocationId,
                                l.sLocationCode
                            })
                        )
                    );
            if(!cboLocation.count()) {
                cboLocation.addItem(QStringLiteral("City not found"));
                auto *simModel=qobject_cast<QStandardItemModel *>(cboLocation.model());
                simModel->item(0,0)->setFlags(Qt::ItemFlag::NoItemFlags);
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

void BadooSearchSettingsDialog::okButtonClicked(bool) {
    bool bOK=false;
    if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext)
        bOK=true;
    else if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
        if(-1==cboIntent.currentIndex()) {
            QMessageBox::critical(
                this,
                this->windowTitle(),
                QStringLiteral("Missing intent")
            );
            cboIntent.setFocus();
        }
        else if(-1==cboLocation.currentIndex()||!cboLocation.currentData().isValid()) {
            // Shows a critical error if the SERVER_SEARCH_LOCATIONS API request failed ...
            // ... or the search did not find a single matching location result.
            QMessageBox::critical(
                this,
                this->windowTitle(),
                QStringLiteral("Missing location")
            );
            cboLocation.setFocus();
        }
        else if(-1==cboDistance.currentIndex()) {
            QMessageBox::critical(
                this,
                this->windowTitle(),
                QStringLiteral("Missing distance")
            );
            cboDistance.setFocus();
        }
        else
            bOK=true;
    }
    if(bOK) {
        EncountersSettings   esSettings;
        PeopleNearbySettings pnsSettings;
        bwWrapper->getEncountersSettings(esSettings);
        bwWrapper->getPeopleNearbySettings(pnsSettings);
        BadooSexTypeList bstlEdit;
        if(btnGuys.isChecked())
            bstlEdit.append(BadooSexType::SEX_TYPE_MALE);
        else if(btnGirls.isChecked())
            bstlEdit.append(BadooSexType::SEX_TYPE_FEMALE);
        else if(btnBoth.isChecked()) {
            bstlEdit.append(BadooSexType::SEX_TYPE_MALE);
            bstlEdit.append(BadooSexType::SEX_TYPE_FEMALE);
        }
        BadooIntRange birEdit={
            objFirstHandle->property(QStringLiteral("value").toUtf8()).toInt(),
            objSecondHandle->property(QStringLiteral("value").toUtf8()).toInt()
        };
        if(SETTINGS_CONTEXT_TYPE_ENCOUNTERS==bsctContext) {
            // Modifies the search settings for Encounters, in memory.
            esSettings.bstlGenders=bstlEdit;
            esSettings.birAge=birEdit;
            esSettings.iDistanceAway=sldDistance.value();
            // Modifies the search settings shared with PeopleNearby, also in memory.
            pnsSettings.bstlGenders=bstlEdit;
            pnsSettings.birAge=birEdit;
        }
        else if(SETTINGS_CONTEXT_TYPE_PEOPLE_NEARBY==bsctContext) {
            BadooIndexedString bisLocation=qvariant_cast<BadooIndexedString>(
                cboLocation.currentData()
            );
            // Modifies the search settings for People Nearby, in memory.
            pnsSettings.bstlGenders=bstlEdit;
            pnsSettings.birAge=birEdit;
            pnsSettings.iLocationId=bisLocation.iIndex;
            pnsSettings.sLocationName=cboLocation.currentText();
            pnsSettings.sDistanceCode=(sCurrentLocationCode==bisLocation.sValue)?
                cboDistance.currentData().toString():
                bisLocation.sValue; // Picks a default distance, if the location changes.
            pnsSettings.iOwnIntentId=cboIntent.currentData().toInt();
            // Modifies the search settings shared with Encounters, also in memory.
            esSettings.bstlGenders=bstlEdit;
            esSettings.birAge=birEdit;
        }
        bwWrapper->setEncountersSettings(esSettings);
        bwWrapper->setPeopleNearbySettings(pnsSettings);
        this->setCursor(Qt::CursorShape::BusyCursor);
        if(bwWrapper->saveSearchSettings())
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
