#include "job_server.h"

#include "../../common-src/ipc_defines.h"
#include "jobs/jobs_manager.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <cassert>

//==============================================================================

JobServer::JobServer(QObject * a_pParent) : QObject(a_pParent)
	, m_pSettingsManager(nullptr)
	, m_pJobsManager(nullptr)
	, m_pWebSocketServer(nullptr)
{
	m_pSettingsManager = new SettingsManagerCore(this);

	m_pJobsManager = new JobsManager(m_pSettingsManager, this);
	m_pJobsManager->loadJobs();
	connect(m_pJobsManager, &JobsManager::signalLogMessage,
		this, &JobServer::slotLogMessage);
	connect(m_pJobsManager, &JobsManager::signalJobCreated,
		this, &JobServer::slotJobCreated);
	connect(m_pJobsManager, &JobsManager::signalJobChanged,
		this, &JobServer::slotJobChanged);
	connect(m_pJobsManager, &JobsManager::signalJobStateChanged,
		this, &JobServer::slotJobStateChanged);
	connect(m_pJobsManager, &JobsManager::signalJobProgressChanged,
		this, &JobServer::slotJobProgressChanged);
	connect(m_pJobsManager, &JobsManager::signalJobsSwapped,
		this, &JobServer::slotJobsSwapped);
	connect(m_pJobsManager, &JobsManager::signalJobsDeleted,
		this, &JobServer::slotJobsDeleted);

	m_pWebSocketServer = new QWebSocketServer(JOB_SERVER_NAME,
		QWebSocketServer::NonSecureMode, this);
	connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
		this, &JobServer::slotNewConnection);
}

//==============================================================================

JobServer::~JobServer()
{
	for(QWebSocket * pClient : m_clients)
		delete pClient;
	m_clients.clear();
	m_subscribers.clear();
	m_pWebSocketServer->close();
}

//==============================================================================

bool JobServer::start()
{
	assert(m_pWebSocketServer);
	return m_pWebSocketServer->listen(QHostAddress::Any, JOB_SERVER_PORT);
}

//==============================================================================

void JobServer::slotNewConnection()
{
	QWebSocket * pSocket = m_pWebSocketServer->nextPendingConnection();
	m_clients.push_back(pSocket);

	connect(pSocket, &QWebSocket::binaryMessageReceived,
		this, &JobServer::slotBinaryMessageReceived);
	connect(pSocket, &QWebSocket::textMessageReceived,
		this, &JobServer::slotTextMessageReceived);
	connect(pSocket, &QWebSocket::disconnected,
		this, &JobServer::slotSocketDisconnected);
}

//==============================================================================

void JobServer::slotBinaryMessageReceived(
	const QByteArray & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	QString messageString = trUtf8(a_message);
	processMessage(pClient, messageString);
}

//==============================================================================

void JobServer::slotTextMessageReceived(const QString & a_message)
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	processMessage(pClient, a_message);
}

//==============================================================================

void JobServer::slotSocketDisconnected()
{
	QWebSocket * pClient = qobject_cast<QWebSocket *>(sender());
	if(!pClient)
		return;
	m_clients.remove(pClient);
	m_subscribers.remove(pClient);
	pClient->deleteLater();
}

//==============================================================================

void JobServer::slotLogMessage(const QString & a_message,
	const QString & a_style)
{
	LogEntry entry(a_message, a_style);
	m_logEntries.push_back(entry);
	broadcastMessage(jsonMessage(SMSG_LOG_MESSAGE, entry.toJson()));
}

//==============================================================================

void JobServer::slotJobCreated(const JobProperties & a_properties)
{
	broadcastMessage(jsonMessage(SMSG_JOB_CREATED, a_properties.toJson()));
}

//==============================================================================

void JobServer::slotJobChanged(const JobProperties & a_properties)
{
	broadcastMessage(jsonMessage(SMSG_JOB_UPDATE, a_properties.toJson()));
}

//==============================================================================

void JobServer::slotJobStateChanged(const QUuid & a_jobID, JobState a_state)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_JOB_STATE] = (int)a_state;
	broadcastMessage(jsonMessage(SMSG_JOB_STATE_UPDATE, jsJob));
}

//==============================================================================

void JobServer::slotJobProgressChanged(const QUuid & a_jobID, int a_progress)
{
	QJsonObject jsJob;
	jsJob[JP_ID] = a_jobID.toString();
	jsJob[JP_FRAMES_PROCESSED] = a_progress;
	broadcastMessage(jsonMessage(SMSG_JOB_PROGRESS_UPDATE, jsJob));
}

//==============================================================================

void JobServer::slotJobsSwapped(const QUuid & a_jobID1, const QUuid & a_jobID2)
{
	QJsonObject jsSwap;
	jsSwap[JOBS_SWAPPED_ID1] = a_jobID1.toString();
	jsSwap[JOBS_SWAPPED_ID2] = a_jobID2.toString();
	broadcastMessage(jsonMessage(SMSG_JOBS_SWAPPED, jsSwap));
}

//==============================================================================

void JobServer::slotJobsDeleted(const std::vector<QUuid> & a_ids)
{
	QJsonArray jsIdsArray;
	for(const QUuid & id : a_ids)
		jsIdsArray.push_back(id.toString());
	broadcastMessage(jsonMessage(SMSG_JOBS_DELETED, jsIdsArray));
}

//==============================================================================

void JobServer::processMessage(QWebSocket * a_pClient,
	const QString & a_message)
{
	bool local = a_pClient->peerAddress().isLoopback();

	if(a_message == QString(MSG_GET_JOBS_INFO))
	{
		a_pClient->sendTextMessage(jobsInfoMessage());
		return;
	}

	if(a_message == QString(MSG_CLOSE_SERVER))
	{
		if(local)
		{
			broadcastMessage(SMSG_CLOSING_SERVER, true);
			emit finish();
		}
		else
			a_pClient->sendTextMessage("Can not close server remotely.");
		return;
	}

	if(a_message == QString(MSG_SUBSCRIBE))
	{
		m_subscribers.push_back(a_pClient);
		a_pClient->sendTextMessage("Subscribed to jobs updates.");
		return;
	}

	if(a_message == QString(MSG_UNSUBSCRIBE))
	{
		m_subscribers.remove(a_pClient);
		a_pClient->sendTextMessage("Unsubscribed from jobs updates.");
		return;
	}

	a_pClient->sendTextMessage(QString("Received an unknown command: %1")
		.arg(a_message));
}

//==============================================================================

QString JobServer::jobsInfoMessage() const
{
	QJsonArray jsJobs;
	for(const JobProperties & properties : m_pJobsManager->jobsProperties())
		jsJobs.push_back(properties.toJson());
	QString message = jsonMessage(SMSG_JOBS_INFO, jsJobs);
	return message;
}

//==============================================================================

QString JobServer::jsonMessage(const QString & a_command,
	const QJsonObject & a_jsonObject) const
{
	return jsonMessage(a_command, QJsonDocument(a_jsonObject));
}

//==============================================================================

QString JobServer::jsonMessage(const QString & a_command,
	const QJsonArray & a_jsonArray) const
{
	return jsonMessage(a_command, QJsonDocument(a_jsonArray));
}

//==============================================================================

QString JobServer::jsonMessage(const QString & a_command,
	const QJsonDocument & a_jsonDocument) const
{
	QString jobsJson = QString::fromUtf8(a_jsonDocument.toJson());
	QString message = QString("%1 %2").arg(a_command).arg(jobsJson);
	return message;
}

//==============================================================================

void JobServer::broadcastMessage(const QString & a_message,
	bool a_includeNonSubscribers)
{
	std::list<QWebSocket *> & clients =
		a_includeNonSubscribers ? m_clients : m_subscribers;
	for(QWebSocket * pClient : clients)
		pClient->sendTextMessage(a_message);
}

//==============================================================================
