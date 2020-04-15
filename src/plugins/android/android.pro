include(../../qtcreatorplugin.pri)

isEmpty(ANDROID_ENABLE):ANDROID_EXPERIMENTAL_STR="true"
else:ANDROID_EXPERIMENTAL_STR="false"

QT += xml network

HEADERS += \
    androidconstants.h \
    androidconfigurations.h \
    androidmanager.h \
    androidrunconfiguration.h \
    androidruncontrol.h \
    androidrunfactories.h \
    androidsettingspage.h \
    androidsettingswidget.h \
    androidtoolchain.h \
    androidpackageinstallationstep.h \
    androidpackageinstallationfactory.h \
    androidpackagecreationstep.h \
    androidpackagecreationfactory.h \
    androidpackagecreationwidget.h \
    androiddeploystep.h \
    androiddeploystepwidget.h \
    androiddeploystepfactory.h \
    androiderrormessage.h \
    androidglobal.h \
    androidrunner.h \
    androiddebugsupport.h \
    androidqtversionfactory.h \
    androidqtversion.h \
    androiddeployconfiguration.h \
    androidcreatekeystorecertificate.h \
    javaparser.h \
    androidplugin.h \
    androiddevicefactory.h \
    androiddevice.h \
    androidgdbserverkitinformation.h \
    androidanalyzesupport.h \
    androidrunsupport.h \
    androidmanifesteditorfactory.h \
    androidmanifesteditor.h \
    androidmanifesteditorwidget.h \
    androidmanifestdocument.h \
    androiddevicedialog.h \
    androiddeployqtstep.h \
    certificatesmodel.h \
    androiddeployqtwidget.h \
    createandroidmanifestwizard.h \
    androidpotentialkit.h \
    androidextralibrarylistmodel.h \
    androidsignaloperation.h \
    javaeditor.h \
    javaeditorfactory.h \
    javaindenter.h \
    javaautocompleter.h \
    javacompletionassistprovider.h \
    javafilewizard.h

SOURCES += \
    androidconfigurations.cpp \
    androidmanager.cpp \
    androidrunconfiguration.cpp \
    androidruncontrol.cpp \
    androidrunfactories.cpp \
    androidsettingspage.cpp \
    androidsettingswidget.cpp \
    androidtoolchain.cpp \
    androidpackageinstallationstep.cpp \
    androidpackageinstallationfactory.cpp \
    androidpackagecreationstep.cpp \
    androidpackagecreationfactory.cpp \
    androidpackagecreationwidget.cpp \
    androiddeploystep.cpp \
    androiddeploystepwidget.cpp \
    androiddeploystepfactory.cpp \
    androiderrormessage.cpp \
    androidrunner.cpp \
    androiddebugsupport.cpp \
    androidqtversionfactory.cpp \
    androidqtversion.cpp \
    androiddeployconfiguration.cpp \
    androidcreatekeystorecertificate.cpp \
    javaparser.cpp \
    androidplugin.cpp \
    androiddevicefactory.cpp \
    androiddevice.cpp \
    androidgdbserverkitinformation.cpp \
    androidanalyzesupport.cpp \
    androidrunsupport.cpp \
    androidmanifesteditorfactory.cpp \
    androidmanifesteditor.cpp \
    androidmanifesteditorwidget.cpp \
    androidmanifestdocument.cpp \
    androiddevicedialog.cpp \
    androiddeployqtstep.cpp \
    certificatesmodel.cpp \
    androiddeployqtwidget.cpp \
    createandroidmanifestwizard.cpp \
    androidpotentialkit.cpp \
    androidextralibrarylistmodel.cpp \
    androidsignaloperation.cpp \
    javaeditor.cpp \
    javaeditorfactory.cpp \
    javaindenter.cpp \
    javaautocompleter.cpp \
    javacompletionassistprovider.cpp \
    javafilewizard.cpp

FORMS += \
    androidsettingswidget.ui \
    androidpackagecreationwidget.ui \
    androiddeploystepwidget.ui \
    addnewavddialog.ui \
    androidcreatekeystorecertificate.ui \
    androiddevicedialog.ui \
    androiddeployqtwidget.ui

RESOURCES = android.qrc
DEFINES += ANDROID_LIBRARY
