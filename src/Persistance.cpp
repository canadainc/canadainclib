#include <bb/system/Clipboard>
#include <bb/system/SystemDialog>
#include <bb/system/SystemPrompt>
#include <bb/system/SystemToast>

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>

#include <bb/PpsObject>

#include <sys/utsname.h>

#include "Persistance.h"
#include "InvocationUtils.h"
#include "IOUtils.h"
#include "Logger.h"

#include "bbndk.h"

#if BBNDK_VERSION_AT_LEAST(10,3,1)
#include <bb/cascades/Control>
#include <bb/cascades/DeviceShortcut>
#endif

#define KEY_PROMOTED "promoted"
#define KEY_TOAST_SHOWING "showing"
#define KEY_SUPPRESS_TUTORIAL "suppressTutorials"
#define KEY_ARGS "args"
#define METHOD_NAME "onFinished"

namespace {

bool isNowBlocked = false;

}

namespace canadainc {

using namespace bb::cascades;
using namespace bb::system;

Persistance::Persistance(QObject* parent) :
        QObject(parent), m_toast(NULL), m_dialog(NULL),
        m_suppress( getValueFor(KEY_SUPPRESS_TUTORIAL).toInt() == 1 ), m_flags("flags.conf")
{
    QDeclarativeContext* rootContext = QmlDocument::defaultDeclarativeEngine()->rootContext();
    rootContext->setContextProperty("persist", this);

    connect( Application::instance(), SIGNAL( aboutToQuit() ), this, SLOT( commit() ) );

    isNowBlocked = false;
}


void Persistance::commit()
{
    QStringList keys = m_pending.keys();

    foreach ( QString const& key, m_pending.keys() ) {
        m_settings.setValue( key, m_pending.value(key) );
    }
}


void Persistance::showToast(QString const& text, QString const& icon, bb::system::SystemUiPosition::Type pos)
{
	if (m_toast == NULL) {
		m_toast = new SystemToast(this);
        connect( m_toast, SIGNAL( finished(bb::system::SystemUiResult::Type) ), this, SLOT( finished(bb::system::SystemUiResult::Type) ) );
	}

	m_toast->setBody(text);
	m_toast->setIcon( icon.startsWith("asset:///") || icon.startsWith("file:///") ? icon : "asset:///"+icon );
	m_toast->setProperty(KEY_TOAST_SHOWING, true);
	m_toast->setPosition(pos);
	m_toast->show();
}


void Persistance::finished(bb::system::SystemUiResult::Type value)
{
    Q_UNUSED(value);
    sender()->setProperty(KEY_TOAST_SHOWING, false);
}


bool Persistance::showBlockingToast(QString const& text, QString const& icon)
{
    isNowBlocked = true;
	SystemToast toast;
	toast.setBody(text);
	toast.setIcon(icon);
	bool result = toast.exec() == SystemUiResult::ButtonSelection;
	isNowBlocked = false;

	return result;
}


bool Persistance::showBlockingDialog(QString const& title, QString const& text, QString const& okButton, QString const& cancelButton, bool okEnabled)
{
	bool remember = false;
	return showBlockingDialog(title, text, QString(), remember, okButton, cancelButton, okEnabled);
}


void Persistance::onDestroyed(QObject* obj)
{
    if (obj == m_dialog) {
        m_dialog = NULL;
    }
}


void Persistance::showDialog(QString const& title, QString const& text, QString okButton) {
    showDialog(NULL, QVariant(), title, text, okButton, "");
}


void Persistance::showDialog(QObject* caller, QString const& title, QString const& text, QString const& okButton, QString const& cancelButton, QString const& rememberMeText, bool rememberMeValue) {
    showDialog(caller, QVariant(), title, text, okButton, cancelButton, true, rememberMeText, rememberMeValue);
}


void Persistance::showDialog(QObject* caller, QVariant const& data, QString const& title, QString const& text, QString const& okButton, QString const& cancelButton, bool okEnabled, QString const& rememberMeText, bool rememberMeValue)
{
    isNowBlocked = true;

    if (m_dialog == NULL)
    {
        m_dialog = new SystemDialog(caller);
        connect( m_dialog, SIGNAL( finished(bb::system::SystemUiResult::Type) ), this, SLOT( dialogFinished(bb::system::SystemUiResult::Type) ) );
        connect( m_dialog, SIGNAL( destroyed(QObject*) ), this, SLOT( onDestroyed(QObject*) ) );
    }

    bool showRememberMe = !rememberMeText.isEmpty();

    if (showRememberMe)
    {
        m_dialog->setIncludeRememberMe(true);
        m_dialog->setRememberMeChecked(rememberMeValue);
        m_dialog->setRememberMeText(rememberMeText);
    }

    m_dialog->setParent(caller);
    m_dialog->setBody(text);
    m_dialog->setTitle(title);
    m_dialog->cancelButton()->setLabel(cancelButton);
    m_dialog->confirmButton()->setLabel(okButton);
    m_dialog->confirmButton()->setEnabled(okEnabled);
    m_dialog->setProperty(KEY_ARGS, data);
    m_dialog->show();
}


void Persistance::dialogFinished(bb::system::SystemUiResult::Type value)
{
    isNowBlocked = false;

    QObject* caller = m_dialog->parent();

    if (caller != NULL)
    {
        bool result = value == SystemUiResult::ConfirmButtonSelection;
        QVariant data = m_dialog->property(KEY_ARGS);
        m_dialog->setParent(this);

        if ( m_dialog->includeRememberMe() )
        {
            bool rememberMe = m_dialog->rememberMeSelection();

            if ( data.isValid() ) {
                QMetaObject::invokeMethod( caller, "onFinished", Qt::QueuedConnection, Q_ARG(QVariant, result), Q_ARG(QVariant, rememberMe), Q_ARG(QVariant, data) );
            } else {
                QMetaObject::invokeMethod( caller, "onFinished", Qt::QueuedConnection, Q_ARG(QVariant, result), Q_ARG(QVariant, rememberMe) );
            }
        } else {
            if ( data.isValid() ) {
                QMetaObject::invokeMethod( caller, "onFinished", Qt::QueuedConnection, Q_ARG(QVariant, result), Q_ARG(QVariant, data) );
            } else {
                QMetaObject::invokeMethod( caller, "onFinished", Qt::QueuedConnection, Q_ARG(QVariant, result) );
            }
        }

        m_dialog->setProperty(KEY_ARGS, QVariant());
    }
}


bool Persistance::showBlockingDialog(QString const& title, QString const& text, QString const& rememberMeText, bool &rememberMeValue, QString const& okButton, QString const& cancelButton, bool okEnabled)
{
    isNowBlocked = true;

    SystemDialog dialog;
    dialog.setBody(text);
    dialog.setTitle(title);
    dialog.confirmButton()->setLabel(okButton);
    dialog.cancelButton()->setLabel(cancelButton);
    dialog.confirmButton()->setEnabled(okEnabled);

    bool showRememberMe = !rememberMeText.isNull();

    if (showRememberMe)
    {
        dialog.setIncludeRememberMe(true);
        dialog.setRememberMeChecked(rememberMeValue);
        dialog.setRememberMeText(rememberMeText);
    }

    bool result = dialog.exec() == SystemUiResult::ConfirmButtonSelection;
    rememberMeValue = dialog.rememberMeSelection();

    isNowBlocked = false;

    return result;
}


QString Persistance::showBlockingPrompt(QString const& title, QString const& body, QString const& defaultText, QString const& hintText, int maxLength, bool autoCapitalize, QString const& okButton, QString const& cancelButton, int inputMode)
{
    isNowBlocked = true;

    SystemUiInputMode::Type m = (SystemUiInputMode::Type)inputMode;

    SystemPrompt dialog;
    dialog.setBody(body);
    dialog.setTitle(title);
    dialog.inputField()->setDefaultText(defaultText);
    dialog.inputField()->setEmptyText(hintText);
    dialog.inputField()->setMaximumLength(maxLength);
    dialog.inputField()->setInputMode(m);
    dialog.setInputOptions(autoCapitalize ? SystemUiInputOption::AutoCapitalize : SystemUiInputOption::None);
    dialog.confirmButton()->setLabel(okButton);
    dialog.cancelButton()->setLabel(cancelButton);

    bool result = dialog.exec() == SystemUiResult::ConfirmButtonSelection;

    isNowBlocked = false;

    return result ? dialog.inputFieldTextEntry() : QString();
}


void Persistance::copyToClipboard(QString const& text, bool showToastMessage)
{
	Clipboard clipboard;
	clipboard.clear();
	clipboard.insert( "text/plain", convertToUtf8(text) );

	if (showToastMessage) {
		showToast( tr("Copied: %1 to clipboard").arg(text), "asset:///images/menu/ic_copy.png" );
	}
}


QString Persistance::getClipboardText() const
{
    Clipboard clipboard;
    QByteArray qba = clipboard.value("text/plain");
    return QString::fromUtf8( qba.data() );
}


QByteArray Persistance::convertToUtf8(QString const& text) {
	return text.toUtf8();
}


void Persistance::forceSync() {
    m_settings.sync();
}


QVariant Persistance::getValueFor(QString const& objectName)
{
    QVariant value = m_pending.contains(objectName) ? m_pending.value(objectName) : m_settings.value(objectName);

    if ( !m_logMap.contains(objectName) )
    {
        m_logMap.insert(objectName, true);

#if defined(QT_NO_DEBUG)
        int type = value.type();

        if ( type < QMetaType::Double || (type == QMetaType::QChar) || (type >= QMetaType::QString && type <= QMetaType::QStringList) ) { // no need to log that info already found in settings file in release mode
            return value;
        }
#endif

        LOGGER(objectName << value);
    }

    return value;
}


bool Persistance::contains(QString const& key) const {
	return m_pending.contains(key) || m_settings.contains(key);
}


bool Persistance::saveValueFor(const QString &objectName, const QVariant &inputValue, bool fireEvent)
{
	if ( m_settings.value(objectName) != inputValue )
	{
	    LOGGER(objectName << inputValue);

        m_settings.setValue(objectName, inputValue);
        m_logMap.remove(objectName);

	    if (fireEvent) {
	        m_settings.setValue(objectName, inputValue);
	        emit settingChanged(objectName);
	    } else {
	        m_pending[objectName] = inputValue;
	    }

		return true;
	} else {
		return false;
	}
}


void Persistance::remove(QString const& key, bool fireEvent)
{
	if ( contains(key) )
	{
	    LOGGER(key);
		m_settings.remove(key);
		m_logMap.remove(key);

		if (fireEvent) {
	        emit settingChanged(key);
		}
	}
}


bool Persistance::containsFlag(QString const& key)
{
    return m_flags.contains(key);
}


QVariant Persistance::getFlag(QString const& key)
{
    return m_flags.value(key);
}


void Persistance::setFlag(QString const& key, QVariant const& value) {
    m_flags.setValue(key, value);
}


void Persistance::donate(QString const& uri) {
    InvocationUtils::launchBrowser(uri);
}


void Persistance::reviewApp()
{
    InvokeRequest request;
    request.setTarget("sys.appworld.review");
    request.setAction("bb.action.OPEN");
    request.setMimeType("text/html");
    request.setUri("appworld://review");

    InvokeManager().invoke(request);
}


void Persistance::openBlackBerryWorld(QString const& appID)
{
    InvokeRequest request;
    request.setTarget("sys.appworld");
    request.setAction("bb.action.OPEN");
    request.setMimeType("text/html");
    request.setUri("appworld://content/"+appID);

    InvokeManager().invoke(request);
}


void Persistance::openChannel(bool promote)
{
    if (promote)
    {
        if ( contains(KEY_PROMOTED) ) {
            return;
        }

        saveValueFor(KEY_PROMOTED, 1, false);
    }

    InvokeRequest request;
    request.setTarget("sys.bbm.channels.card.previewer");
    request.setAction("bb.action.OPENBBMCHANNEL");
    request.setUri("bbmc:C0034D28B");

    InvokeManager().invoke(request);
}


bool Persistance::clearCache()
{
    bool clear = showBlockingDialog( tr("Confirmation"), tr("Are you sure you want to clear the cache?") );

    if (clear) {
        QFutureWatcher<void>* qfw = new QFutureWatcher<void>(this);
        connect( qfw, SIGNAL( finished() ), this, SLOT( cacheCleared() ) );

        QFuture<void> future = QtConcurrent::run(&IOUtils::clearAllCache);
        qfw->setFuture(future);
    }

    return clear;
}


void Persistance::cacheCleared() {
    showToast( tr("Cache was successfully cleared!"), "file:///usr/share/icons/bb_action_delete.png" );
}


void Persistance::clear() {
	m_settings.clear();
}


void Persistance::launchAppPermissionSettings() {
    InvocationUtils::launchAppPermissionSettings();
}


bool Persistance::hasLocationAccess()
{
    QFile target("/pps/services/geolocation/status");

    if ( !target.open(QIODevice::ReadOnly) ) {
        return false;
    }

    target.close();

    return true;
}


bool Persistance::hasSharedFolderAccess()
{
    QString sdDirectory = QString("/accounts/1000/shared/downloads");

    if ( !QDir(sdDirectory).exists() )
    {
        LOGGER(sdDirectory << "DidNotExist!");
        return false;
    }

    return true;
}


bool Persistance::hasEmailSmsAccess()
{
    if ( QFile("/var/db/text_messaging/messages.db").exists() || QFile("/accounts/1000/_startup_data/sysdata/text_messaging/messages.db").exists() ) {
        return true;
    } else {
        struct utsname udata;
        uname(&udata);

        if ( QString(udata.machine).startsWith("x86") ) { // simulator build doesn't have it
            return true;
        }
    }

    LOGGER("NoEmailSmsAccess");
    return false;
}


bool Persistance::hasPhoneControlAccess()
{
    if ( QFile("/pps/services/phone/protected/status").exists() ) {
        return true;
    }

    LOGGER("NoPhoneControlAccess");
    return false;
}


void Persistance::attachBackKeyToClickedSignal(QObject* abstractButton, QObject* rootControl)
{
#if BBNDK_VERSION_AT_LEAST(10,3,1)
    Control* c = static_cast<Control*>(rootControl);
    DeviceShortcut* ds = DeviceShortcut::create(DeviceShortcuts::BackTap).onTriggered( abstractButton, SIGNAL( clicked() ) );
    c->addShortcut(ds);
#else
    Q_UNUSED(abstractButton);
    Q_UNUSED(rootControl);
#endif
}


void Persistance::invoke(QString const& targetId, QString const& action, QString const& mime, QString const& uri, QString const& data)
{
    InvokeRequest request;
    request.setTarget(targetId);
    request.setAction(action);
    request.setUri(uri);
    request.setMimeType(mime);
    request.setData( data.toUtf8() );

    InvokeManager().invoke(request);
}


bool Persistance::isBlocked() const {
    return isNowBlocked;
}


bool Persistance::suppressTutorials() const {
    return m_suppress;
}


void Persistance::setSuppressTutorials(bool value)
{
    if (m_suppress != value)
    {
        m_suppress = value;
        saveValueFor(KEY_SUPPRESS_TUTORIAL, value ? 1 : 0, false);
    }
}


bool Persistance::isUpdateNeeded(QString const& key, int diffDaysMin)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastUpdateCheck = QDateTime::fromMSecsSinceEpoch( getValueFor(key).toLongLong() );
    int diff = lastUpdateCheck.daysTo(now);

    LOGGER("diffLastUpdateCheck" << diff);
    return diff > diffDaysMin;
}


void Persistance::call(QString const& number)
{
    LOGGER(number);
    QVariantMap map;
    map.insert("number", number);
    QByteArray requestData = bb::PpsObject::encode(map, NULL);

    bb::system::InvokeRequest request;
    request.setAction("bb.action.DIAL");
    request.setMimeType("application/vnd.blackberry.phone.startcall");
    request.setData(requestData);

    InvokeManager().invoke(request);
}


Persistance::~Persistance()
{
}


} /* namespace canadainc */
