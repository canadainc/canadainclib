#include "customsqldatasource.h"
#include "IOUtils.h"
#include "Logger.h"

#include <bb/data/SqlConnection>
#include <QFile>

//#define SHOW_STATS 1

namespace canadainc {

using namespace bb::data;

CustomSqlDataSource::CustomSqlDataSource(QObject *parent) : QObject(parent),
        m_name("connect"), m_sqlConnector(NULL), m_verbose(false)
{
}

CustomSqlDataSource::~CustomSqlDataSource()
{
	if (m_sqlConnector) {
		m_sqlConnector->stop();
	}
}


void CustomSqlDataSource::setVerbose(bool verbose) {
    m_verbose = verbose;
}


void CustomSqlDataSource::setQuery(QString const& query)
{
    if (m_query.compare(query) != 0) {
        m_query = query;
        emit queryChanged(m_query);
    }
}

QString CustomSqlDataSource::query() const {
    return m_query;
}

void CustomSqlDataSource::setName(QString const& name)
{
    if (m_name.compare(name) != 0) {
    	m_name = name;
        emit nameChanged(m_name);
    }
}

QString CustomSqlDataSource::name() const {
    return m_name;
}

bool CustomSqlDataSource::checkConnection()
{
    if (m_sqlConnector) {
        return true;
    } else {
        QFile newFile(m_source);

        if (newFile.exists()) {

            // Remove the old connection if it exists
            if(m_sqlConnector){
                disconnect( m_sqlConnector, SIGNAL( reply(bb::data::DataAccessReply const&) ), this, SLOT( onLoadAsyncResultData(bb::data::DataAccessReply const&) ) );
                m_sqlConnector->setParent(NULL);
                delete m_sqlConnector;
            }
            m_sqlConnector = new SqlConnection(m_source, m_name, this);

            connect(m_sqlConnector, SIGNAL( reply(bb::data::DataAccessReply const&) ), this, SLOT( onLoadAsyncResultData(bb::data::DataAccessReply const&) ) );

            return true;

        } else {
            LOGGER("Failed to load data base, file does not exist." << m_source);
            emit setupError( "Failed to load data base, file does not exist: "+m_source+". If this is in your SD Card, make sure your USB Mass Storage Mode is set to Off." );
        }
    }

    return false;
}

DataAccessReply CustomSqlDataSource::executeAndWait(QVariant const& criteria, int id)
{
    DataAccessReply reply;

    if ( checkConnection() )
    {
        reply = m_sqlConnector->executeAndWait(m_query, criteria.toList(), id);

        if ( reply.hasError() ) {
            LOGGER("error " << reply);
        }
    }

    return reply;
}

void CustomSqlDataSource::execute(QVariant const& criteria, int id)
{
    if ( checkConnection() )
    {
        if (m_verbose) {
            LOGGER( id << criteria.toString() );
        } else {
            LOGGER(id);
        }

#ifdef SHOW_STATS
        m_elapsed.restart();
#endif
        m_sqlConnector->execute(criteria, id);
    }
}


void CustomSqlDataSource::executePrepared(QVariantList const& values, int id)
{
    if ( checkConnection() )
    {
        if (m_verbose) {
            LOGGER( m_query << values << id );
        } else {
            LOGGER(id);
        }

#ifdef SHOW_STATS
        m_elapsed.restart();
#endif
        m_sqlConnector->execute(m_query, values, id);
    }
}


void CustomSqlDataSource::startTransaction(int id)
{
    if ( checkConnection() ) {
        m_sqlConnector->beginTransaction(id);
    }
}


void CustomSqlDataSource::endTransaction(int id)
{
    if ( checkConnection() ) {
        m_sqlConnector->endTransaction(id);
    }
}


void CustomSqlDataSource::load(int id)
{
    if ( !m_query.isEmpty() ) {
        execute(m_query, id);
    }
}

void CustomSqlDataSource::onLoadAsyncResultData(bb::data::DataAccessReply const& replyData)
{
    if ( replyData.hasError() ) {
        LOGGER( replyData.id() << ", SQLError: " << replyData );
        emit error( replyData.errorMessage() );
    } else {
        QVariantList resultList = replyData.result().toList();
        LOGGER( replyData.id() << "=" << resultList.size() );

#ifdef SHOW_STATS
        LOGGER( "ExecTime:" << m_elapsed.elapsed() );
#endif

        emit dataLoaded( replyData.id(), resultList );
    }
}


void CustomSqlDataSource::setSource(QString const& source) {
	m_source = source;
}


QString CustomSqlDataSource::source() const {
	return m_source;
}

}
