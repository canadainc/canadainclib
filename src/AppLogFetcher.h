#ifndef APPLOGFETCHER_H_
#define APPLOGFETCHER_H_

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace bb {
    namespace system {
        class SystemProgressToast;
    }
}

namespace canadainc {

using namespace bb::system;

class AppLogFetcher : public QObject
{
    Q_OBJECT

    QNetworkAccessManager* m_networkManager;
    SystemProgressToast* m_progress;
    QString m_name;

private slots:
    void onNetworkReply(QNetworkReply* reply);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void startCollecting();

public:
    AppLogFetcher(QString const& appName, QObject* parent=NULL);
    virtual ~AppLogFetcher();

    Q_INVOKABLE void submitLogs(bool silent=false);
};

} /* namespace canadainc */

#endif /* APPLOGFETCHER_H_ */
