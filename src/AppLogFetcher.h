#ifndef APPLOGFETCHER_H_
#define APPLOGFETCHER_H_

#include <QFutureWatcher>
#include <QStringList>

#include "NetworkProcessor.h"

#define START_LOGGING_KEY "startLogging"
#define STOP_LOGGING_KEY "stopLogging"
#define UI_KEY "logUI"
#define REMOVED_APPS_PATH QString("%1/removedapps").arg( QDir::tempPath() )
#define DEVICE_INFO_PATH QString("%1/deviceInfo.txt").arg( QDir::tempPath() )
#define UI_LOG_FILE QString("%1/logs/ui.log").arg( QDir::currentPath() )
#define DEFAULT_LOGS QStringList() << QSettings().fileName() << DEVICE_INFO_PATH << UI_LOG_FILE << REMOVED_APPS_PATH << "/var/boottime.txt"
#define ZIP_FILE_PATH QString("%1/logs.zip").arg( QDir::tempPath() )
#define COLLECT_ANALYTICS if ( !m_persistance.contains("analytics_collected") ) { m_persistance.saveValueFor("analytics_collected", true, false); AppLogFetcher::getInstance()->submitLogs("[canadainc_collect_analytics]");}

namespace bb {
    namespace cascades {
        class KeyEvent;
    }
}

namespace canadainc {

class LogCollector
{
public:
    LogCollector();
    virtual QByteArray compressFiles() = 0;
    virtual ~LogCollector();
};

struct AdminData
{
    QStringList buffer;
    QString expected;
    bool isAdmin;
};

class AppLogFetcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAdmin READ adminEnabled NOTIFY adminEnabledChanged)

    static AppLogFetcher* instance;
    NetworkProcessor m_network;
    QFutureWatcher<QByteArray> m_future;
    LogCollector* m_collector;
    AdminData m_admin;

    AppLogFetcher(LogCollector* collector, QObject* parent=NULL);
    void cleanUp();

private slots:
    void onFinished();
    void onKeyReleasedHandler(bb::cascades::KeyEvent* event);
    void onRequestComplete(QVariant const& cookie, QByteArray const& data);
    void onReplyError();
    void startCollection();

signals:
    void adminEnabledChanged();
    void progress(QVariant const& cookie, qint64 bytesSent, qint64 bytesTotal);
    void submitted(QString const& message);

public:
    static AppLogFetcher* create(LogCollector* collector, QObject* parent=NULL);
    static AppLogFetcher* getInstance();
    virtual ~AppLogFetcher();

    static void dumpDeviceInfo(QString const& additional=QString());
    void submitLogsLegacy();
    Q_INVOKABLE void submitLogs(QString const& notes=QString(), bool userTriggered=false);
    Q_INVOKABLE void initPage(QObject* page);
    bool adminEnabled() const;
};

} /* namespace canadainc */

#endif /* APPLOGFETCHER_H_ */
