#include "InvocationUtils.h"
#include "Logger.h"
#include "Persistance.h"

namespace canadainc {

void InvocationUtils::launchAppPermissionSettings() {
	launchSettingsApp("permissions");
}


void InvocationUtils::launchSettingsApp(QString const& key)
{
	bb::system::InvokeRequest request;
	request.setTarget("sys.settings.target");
	request.setAction("bb.action.OPEN");
	request.setMimeType("settings/view");
	request.setUri( QUrl("settings://"+key) );

	bb::system::InvokeManager().invoke(request);
}


void InvocationUtils::replyToMessage(qint64 accountId, QString const& messageId, InvokeManager& invokeManager)
{
	LOGGER(accountId << messageId);

	bb::system::InvokeRequest request;
	request.setAction("bb.action.REPLY");
	request.setMimeType("message/rfc822");
	request.setUri( QString("pim:message/rfc822:%1:%2").arg(accountId).arg(messageId) );

	invokeManager.invoke(request);
}


bool InvocationUtils::validateSharedFolderAccess(QString const& message, bool launchAppPermissions)
{
	QString sdDirectory = QString("/accounts/1000/shared/downloads");

	if ( !QDir(sdDirectory).exists() )
	{
		LOGGER(sdDirectory << "Did not exist!");
		Persistance::showBlockingToast( message, tr("OK"), "asset:///images/ic_folder_warning.png" );

		if (launchAppPermissions) {
			InvocationUtils::launchAppPermissionSettings();
		}

		return false;
	}

	return true;
}


bool InvocationUtils::validateEmailSMSAccess(QString const& message, bool launchAppPermissions)
{
	if ( !QFile("/var/db/text_messaging/messages.db").exists() )
	{
		LOGGER("messages.db did not exist!");
		Persistance::showBlockingToast( message, tr("OK") );

		if (launchAppPermissions) {
			InvocationUtils::launchAppPermissionSettings();
		}

		return false;
	}

	return true;
}


bool InvocationUtils::validateLocationAccess(QString const& message, bool launchAppPermissions)
{
	QFile target("/pps/services/geolocation/control");

	if ( !target.open(QIODevice::ReadOnly) )
	{
		Persistance::showBlockingToast( message, tr("OK"), "asset:///images/ic_location_failed.png" );

		if (launchAppPermissions) {
			InvocationUtils::launchAppPermissionSettings();
		}

		return false;
	}

	target.close();

	return true;
}


void InvocationUtils::launchLocationServices() {
	launchSettingsApp("location");
}


void InvocationUtils::launchBBM()
{
	bb::system::InvokeRequest request;
	request.setTarget("sys.bbm");
	request.setAction("bb.action.OPEN");

	bb::system::InvokeManager().invoke(request);
}


void InvocationUtils::launchSMSComposer(QString const& number, InvokeManager& invokeManager)
{
	LOGGER(number);

	bb::system::InvokeRequest request;
	request.setTarget("sys.pim.text_messaging.composer");
	request.setAction("bb.action.SENDTEXT");
	request.setUri( QString("tel:%1").arg(number) );
	invokeManager.invoke(request);
}


void InvocationUtils::launchBBMCall(QString const& pin, bool videoEnabled)
{
	LOGGER(pin << videoEnabled);

	bb::system::InvokeRequest request;
	request.setTarget("sys.service.videochat");
	request.setAction("bb.action.OPEN");
	request.setPerimeter(bb::system::SecurityPerimeter::Personal);
	request.setData( QString("dest=%1&video=%2").arg(pin).arg( videoEnabled ? 1 : 0 ).toUtf8() );
	bb::system::InvokeManager().invoke(request);
}


void InvocationUtils::launchBBMChat(QString const& pin, InvokeManager& invokeManager)
{
	LOGGER(pin);

	bb::system::InvokeRequest request;
	request.setTarget("sys.bbm.sharehandler");
	request.setAction("bb.action.BBMCHAT");
	request.setUri( QString("pin:%1").arg(pin) );
	invokeManager.invoke(request);
}


void InvocationUtils::launchEmailComposer(QString const& address, InvokeManager& invokeManager)
{
	LOGGER(address);

	bb::system::InvokeRequest request;
	request.setTarget("sys.pim.uib.email.hybridcomposer");
	request.setAction("bb.action.OPEN, bb.action.SENDEMAIL");
	request.setUri( QString("mailto:%1").arg(address) );
	invokeManager.invoke(request);
}


void InvocationUtils::launchPhoto(QString const& uri, InvokeManager* invokeManager)
{
	LOGGER(uri);

	bb::system::InvokeRequest request;
	request.setTarget("sys.pictures.card.previewer");
	request.setAction("bb.action.VIEW");
	request.setUri( QString("file://%1").arg(uri) );
	invokeManager->invoke(request);
}


void InvocationUtils::launchDoc(QString const& uri, InvokeManager& invokeManager)
{
	LOGGER(uri);

	int periodIndex = uri.lastIndexOf(".");

	if (periodIndex != -1)
	{
		QString extension = uri.mid(periodIndex);

		QString target;
		QString mimeType;

		if (extension == "ppt" || extension == "pot" || extension == "pps" || extension == "pptx" || extension == "potx" || extension == "ppsx" || extension == "pptm" || extension == "potm" || extension == "ppsm") {
			//request.insert("mime", "application/vnd.ms-powerpoint");
			target = "sys.slideshowtogo.previewer";
		} else if (extension == "xls" || extension == "xlt" || extension == "xlsx" || extension == "xltx" || extension == "xlsm" || extension == "xltm") {
			//mimeType = "application/vnd.ms-excel, application/vnd.openxmlformats-officedocument.spreadsheetml.sheet, application/vnd.openxmlformats-officedocument.spreadsheetml.template, application/vnd.ms-excel.sheet.macroEnabled.12, application/vnd.ms-excel.template.macroEnabled.12";
			target = "sys.sheettogo.previewer";
		} else if (extension == "doc" || extension == "dot" || extension == "txt" || extension == "docx" || extension == "dotx" || extension == "docm" || extension == "dotm") {
			//mimeType = "application/msword";
			target = "sys.wordtogo.previewer";
		} else if (extension == "pdf") {
			target = "com.rim.bb.app.adobeReader.viewer";
		}

		bb::system::InvokeRequest request;
		request.setTarget(target);
		request.setAction("bb.action.VIEW");
		request.setUri( QString("file://%1").arg(uri) );
		invokeManager.invoke(request);
	}
}


void InvocationUtils::launchAudio(QString const& uri)
{
	bb::system::InvokeManager invokeManager;

	bb::system::InvokeRequest request;
	request.setTarget("sys.mediaplayer.previewer");
	request.setAction("bb.action.OPEN");
	request.setMimeType("audio/m4a");
	request.setUri( QString("file://%1").arg(uri) );

	invokeManager.invoke(request);
}


QVariantMap InvocationUtils::parseArgs(QString const& requestUri)
{
	QStringList args = requestUri.split("&");
	QVariantMap argMap;

	for (int i = args.size()-1; i >= 0; i--)
	{
		QStringList token = args[i].split("=");

		if ( token.size() > 1 ) {
			argMap[ token.first() ] = token.last();
		}
	}

	return argMap;
}


QString InvocationUtils::encodeArgs(QVariantMap const& map)
{
	QString result;
	QStringList keys = map.keys();

	for (int i = keys.size()-1; i >= 0; i--)
	{
		QString key = keys[i];
		result += key+"="+map.value(key).toString();

		if (i > 0) {
			result += "&";
		}
	}

	return result;
}

} /* namespace canadainc */
