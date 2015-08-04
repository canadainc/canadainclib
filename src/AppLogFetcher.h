#ifndef APPLOGFETCHER_H_
#define APPLOGFETCHER_H_

#include <QFutureWatcher>
#include <QSet>
#include <QStringList>

#include "NetworkProcessor.h"

#define CREATOR_PATH QString("%1/creator_stats.db").arg( QDir::tempPath() )
#define ANALYTICS_PATH QString("%1/analytics.db").arg( QDir::homePath() )
#define DEVICE_INFO_PATH QString("%1/deviceStats.txt").arg( QDir::tempPath() )
#define FS_INFO_PATH "/pps/system/filesystem/local/emmc"
#define KEY_ADMIN_MODE "adminMode"
#define KEY_ADMIN_PASSWORD "adminPassword"
#define KEY_ADMIN_USERNAME "adminUsername"
#define NOTES_PATH QString("%1/notes.txt").arg( QDir::tempPath() )
#define PIDIN_PATH QString("%1/pidin.txt").arg( QDir::tempPath() )
#define SNAPSHOT_PATH QString("%1/snapshot.txt").arg( QDir::tempPath() )
#define SLOG2_PATH QString("%1/slog2.txt").arg( QDir::tempPath() )
#define USER_ID_PATH QString("%1/user.txt").arg( QDir::tempPath() )
#define WHATSAPP_PATH "/pps/accounts/1000/appserv/com.whatsapp.WhatsApp.gYABgD934jlePGCrd74r6jbZ7jk/app_status"
#define ANALYTICS_LOGS QStringList() << QSettings().fileName() << QString("%1/analytics.db").arg( QDir::homePath() ) << DEVICE_INFO_PATH << "/pps/services/progress/status" << "/pps/system/installer/removedapps/applications" << WHATSAPP_PATH << "/base/svnrev" << USER_ID_PATH << "/pps/accounts/1000/appserv/sys.appworld.gYABgNSvaLtte_snIx7wjRsOcyM/service/updates" << "/var/app_launch_data.txt"
#define DEFAULT_LOGS ANALYTICS_LOGS << QSettings(FLAGS_FILE_NAME).fileName() << NOTES_PATH << SLOG2_PATH << PIDIN_PATH << "/var/boottime.txt" << "/pps/services/locale/uom" << "/pps/services/audio/voice_status" << "/pps/services/cellular/radioctrl/status_cell_public" << FS_INFO_PATH << "/pps/services/cellular/sms/options" << "/pps/services/chat/counters" << SNAPSHOT_PATH
#define ZIP_FILE_PATH QString("%1/logs.zip").arg( QDir::tempPath() )

namespace bb {
    namespace cascades {
        class KeyEvent;
    }
}

namespace canadainc {

class CustomSqlDataSource;
class Persistance;

typedef void (*CompressFiles)(QSet<QString>&);

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
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)

    static AppLogFetcher* instance;
    NetworkProcessor m_network;
    QFutureWatcher< QPair<QByteArray,QString> > m_future;
    AdminData m_admin;
    Persistance* m_settings;
    CompressFiles m_compressor;
    QMap< QPair<QString, QString>, int> m_counters;
    bool m_dumpAll;
    QString m_userId;
    bool m_pendingUserId;

    AppLogFetcher(Persistance* settings, CompressFiles func, QObject* parent=NULL, bool dumpAll=true);
    CustomSqlDataSource* initAnalytics();

private slots:
    void onAboutToQuit();
    bool commitStats(bool termination=false);
    void onDataLoaded(int id, QVariant const& data);
    void onFinished();
    void onKeyReleasedHandler(bb::cascades::KeyEvent* event);
    void onRequestComplete(QVariant const& cookie, QByteArray const& data, bool error);
    void onUserIdCaptured();
    void securityCheck();

signals:
    void adminEnabledChanged();
    void onlineChanged();
    void progress(QVariant const& cookie, qint64 bytesSent, qint64 bytesTotal);
    void simulationFilesAvailable(QStringList const& files);
    void submitted(QString const& message);
    void latestAppVersionFound(QString const& version);

public:
    static AppLogFetcher* create(Persistance* settings, CompressFiles func, QObject* parent=NULL, bool dumpAll=true);
    static AppLogFetcher* getInstance();
    virtual ~AppLogFetcher();

    bool adminEnabled() const;
    bool online() const;
    QString getUserId() const;
    Q_SLOT bool performCII();
    Q_INVOKABLE void checkForUpdate(QString const& projectName);
    Q_INVOKABLE void fetchUserId(QObject* caller);
    Q_INVOKABLE void initPage(QObject* page);
    Q_INVOKABLE void submitLogs(QString const& notes=QString(), bool userTriggered=false, bool isSimulation=false, QStringList const& attachments=QStringList());
    Q_INVOKABLE void record(QString const& event, QString const& context="");
    static void removeInvalid(QSet<QString>& input);
    Q_INVOKABLE bool deferredCheck(QString const& key, qint64 diff, bool versionBased=false);
    static void onErrorMessage(const char* msg);
    Q_SLOT void onError(QString const& error);
};

} /* namespace canadainc */

#endif /* APPLOGFETCHER_H_ */
