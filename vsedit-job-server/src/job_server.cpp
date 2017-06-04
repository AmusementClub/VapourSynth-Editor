#include "job_server.h"

#include "../../common-src/settings/settings_manager_core.h"
#include "../../common-src/ipc_defines.h"
#include "jobs/jobs_manager.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
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
			for(QWebSocket * pClient : m_clients)
				pClient->sendTextMessage("Closing server.");
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

QString JobServer::jobsInfoMessage()
{
	QJsonArray jsJobs;
	for(const JobProperties & properties : m_pJobsManager->jobsProperties())
	{
		QJsonObject jsJob;
		jsJob["id"] = properties.id.toString();
		jsJob["type"] = (int)properties.type;
		jsJob["jobState"] = (int)properties.jobState;

		QJsonArray jsDependencies;
		for(const QUuid & id : properties.dependsOnJobIds)
			jsDependencies.push_back(QJsonValue(id.toString()));
		jsJob["dependsOnJobIds"] = jsDependencies;

		jsJob["timeStarted"] = properties.timeStarted.toMSecsSinceEpoch();
		jsJob["timeEnded"] = properties.timeEnded.toMSecsSinceEpoch();
		jsJob["scriptName"] = properties.scriptName;
		jsJob["encodingType"] = (int)properties.encodingType;
		jsJob["encodingHeaderType"] = (int)properties.encodingHeaderType;
		jsJob["executablePath"] = properties.executablePath;
		jsJob["arguments"] = properties.arguments;
		jsJob["shellCommand"] = properties.shellCommand;
		jsJob["firstFrame"] = properties.firstFrame;
		jsJob["firstFrameReal"] = properties.firstFrameReal;
		jsJob["lastFrame"] = properties.lastFrame;
		jsJob["lastFrameReal"] = properties.lastFrameReal;
		jsJob["framesProcessed"] = properties.framesProcessed;
		jsJob["fps"] = properties.fps;

		jsJobs.push_back(jsJob);
	}
	QString jobsJson = QString::fromUtf8(QJsonDocument(jsJobs).toJson());
	QString message = QString("%1 %2").arg(SMSG_JOBS_INFO).arg(jobsJson);
	return message;
}

//==============================================================================
