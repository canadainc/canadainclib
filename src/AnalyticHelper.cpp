#include "AnalyticHelper.h"
#include "customsqldatasource.h"
#include "Logger.h"

#include <bb/Application>

#define COMMITTING_ANALYTICS -5
#define COMMIT_ANALYTICS -6

namespace canadainc {

AnalyticHelper::AnalyticHelper()
{
}


void AnalyticHelper::clear()
{
    CustomSqlDataSource* sql = initAnalytics();
    sql->setName("analytics2");
    sql->setQuery("DELETE FROM events");
    sql->executeAndWait(QVariant(), COMMITTING_ANALYTICS);
    sql->setQuery("VACUUM");
    sql->executeAndWait(QVariant(), COMMIT_ANALYTICS);
}


bool AnalyticHelper::commitStats(bool termination)
{
    bool success = true;
    bool autoBugReport = !user_triggered && user_notes != ANALYTICS_SIGNATURE && !termination;

    if ( ( !m_counters.isEmpty() || autoBugReport ) && !m_admin.isAdmin )
    {
        LOGGER( bb::Application::instance()->extendTerminationTimeout() );

        QList< QPair<QString, QString> > keys = m_counters.keys();

        if ( !QFile::exists(ANALYTICS_PATH) ) {
            IOUtils::writeFile(ANALYTICS_PATH);
        }

        CustomSqlDataSource* sql = initAnalytics();
        sql->setName("analytics");
        sql->startTransaction(COMMITTING_ANALYTICS);
        sql->setQuery("CREATE TABLE IF NOT EXISTS events (id INTEGER PRIMARY KEY, event TEXT NOT NULL, context TEXT NOT NULL DEFAULT '', count INTEGER DEFAULT 0, UNIQUE(event,context) ON CONFLICT REPLACE CHECK(event <> ''))");
        sql->executeAndWait(QVariant(), COMMITTING_ANALYTICS);

        for (int i = keys.size()-1; i >= 0; i--)
        {
            QPair<QString, QString> pair = keys[i];
            sql->setQuery( QString("INSERT INTO events (event,context,count) VALUES (?,?,COALESCE((SELECT count FROM events WHERE event=? AND context=?)+%1,%1))").arg( m_counters.value(pair) ) );
            sql->executeAndWait( QVariantList() << pair.first << pair.second << pair.first << pair.second, COMMITTING_ANALYTICS );
        }

        if (autoBugReport)
        {
            sql->setQuery("CREATE TABLE IF NOT EXISTS auto_reports (id INTEGER PRIMARY KEY, notes TEXT UNIQUE NOT NULL, creation_time INTEGER)");
            sql->executeAndWait(QVariant(), COMMITTING_ANALYTICS);

            sql->setQuery("SELECT id FROM auto_reports WHERE notes=?");
            success = sql->executeAndWait(QVariantList() << user_notes, COMMITTING_ANALYTICS).result().toList().isEmpty();

            if (success)
            {
                sql->setQuery("INSERT INTO auto_reports (notes,creation_time) VALUES(?,?)");
                sql->executeAndWait(QVariantList() << user_notes << reportID, COMMITTING_ANALYTICS);
            }
        }

        sql->endTransaction(COMMIT_ANALYTICS);
        m_counters.clear();
    }

    return success;
}


CustomSqlDataSource* AnalyticHelper::initAnalytics()
{
    CustomSqlDataSource* sql = new CustomSqlDataSource(this);
    connect( sql, SIGNAL( dataLoaded(int, QVariant const&) ), this, SLOT( onDataLoaded(int, QVariant const&) ) );
    sql->setSource(ANALYTICS_PATH);

    return sql;
}


void AnalyticHelper::onAboutToQuit()
{
    record( "AppClose", QString::number( QDateTime::currentMSecsSinceEpoch() ) );
    commitStats(true);
}


void AnalyticHelper::onDataLoaded(int id, QVariant const& data)
{
    Q_UNUSED(data);

    if (id == COMMIT_ANALYTICS) {
        sender()->deleteLater();
    }
}


void AnalyticHelper::record(QString const& event, QString const& context)
{
    QPair<QString, QString> pair = qMakePair<QString, QString>(event, context);
    int count = m_counters.value(pair);
    ++count;

    m_counters[pair] = count;
}


AnalyticHelper::~AnalyticHelper()
{
}

} /* namespace canadainc */
