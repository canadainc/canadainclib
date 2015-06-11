#include "LocaleUtil.h"
#include "Logger.h"

#include <QDateTime>

#include <bb/utility/i18n/Formatting>
#include <bb/system/LocaleHandler>

namespace canadainc {

LocaleUtil::LocaleUtil(QString const& appName, QObject* parent) : QObject(parent), m_appName(appName)
{
    LOGGER("********* X");
    if ( !connect( &m_pLocaleHandler, SIGNAL( systemLanguageChanged() ), this, SLOT( onSystemLanguageChanged() ) ) ) {
        LOGGER("Recovering from a failed connect()");
    } else {
        onSystemLanguageChanged();
    }
}


void LocaleUtil::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(&m_pTranslator);

    // Initiate, load and install the application translation files.
    QString localeString = QLocale().name();

    if ( m_appName.isNull() ) {
    	m_appName = QCoreApplication::applicationName();
    }

    QString fileName = QString("%1_%2").arg(m_appName).arg(localeString);

    LOGGER("LocaleFileName: " << fileName);

    if ( m_pTranslator.load(fileName, "app/native/qm") ) {
        QCoreApplication::instance()->installTranslator(&m_pTranslator);
    } else {
        LOGGER("LoadFailed" << fileName);
    }

    fileName = QString("canadainc_%2").arg(localeString);

    LOGGER("LocaleFileName: " << fileName);

    if ( m_libTranslator.load(fileName, "app/native/qm") ) {
        QCoreApplication::instance()->installTranslator(&m_libTranslator);
    } else {
        LOGGER("LoadFailed" << fileName);
    }
}


LocaleUtil::~LocaleUtil()
{
}


} /* namespace canadainc */
