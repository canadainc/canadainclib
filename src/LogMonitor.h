#ifndef LOGMONITOR_H_
#define LOGMONITOR_H_

#include <QFileSystemWatcher>
#include <QSettings>
#include <QTimer>

namespace canadainc {

class LogMonitor : public QObject
{
    Q_OBJECT

    QFileSystemWatcher m_settingsWatcher;
    QString m_logFile;
    QString m_key;
    QTimer m_timer;

    LogMonitor(QString const& key, QString const& logFile, QObject* parent=NULL);

private slots:
    void settingChanged(QString const& file);
    void timeout();

public:
    static void create(QString const& key, QString const& logFile, QObject* parent=NULL);
    virtual ~LogMonitor();
};

} /* namespace canadainc */

#endif /* LOGMONITOR_H_ */
