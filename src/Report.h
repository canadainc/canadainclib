#ifndef REPORT_H_
#define REPORT_H_

#include <QMap>
#include <QStringList>

#define KEY_USER_ID "user_id"

namespace canadainc {

struct AppLaunch
{
    QString appName;
    int launchType;
    qreal processCreated;
    qreal windowPosted;
    qreal fullyVisible;

    AppLaunch(QString const& name, int t, qreal created, qreal posted, qreal visible);
};

struct ReportType
{
    enum Type
    {
        AppVersionDiff,
        BugReportAuto,
        BugReportManual,
        FirstInstall,
        OsVersionDiff,
        Periodic,
        Simulation,
        Attribute
    };
};

struct Report
{
    QStringList attachments;
    QByteArray data;
    bool dumpAll;
    QString id;
    ReportType::Type type;
    QMap<QString, QString> params;
    QMap<QString, QString> removedApps;
    QList<AppLaunch> appLaunches;

    Report(ReportType::Type t);
    Report();
    void applyAddresses(QStringList const& addresses);
};

typedef void (*CompressFiles)(Report&, QString const&, const char*);

} /* namespace canadainc */

#endif /* REPORT_H_ */
