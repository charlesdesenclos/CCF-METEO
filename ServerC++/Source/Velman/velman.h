#pragma once

#include <QObject>
#include <qdebug.h>

#include <QSqlDatabase>
#include <QtSql/QtSql>
#include <QSqlQuery>
#include <QtSql>

#include <windows.h>
#include "K8055Adapter.h"


#include <QWebSocket>
#include <QWebSocketServer>

class velman : public QObject
{
	Q_OBJECT

		QWebSocketServer *webSocketServer;
		QWebSocket * socket;
		QList<QWebSocket*> wsclients;

public:

	/* Constructeur */
	velman(quint16 port);

	/* Destructeur */
	~velman();

	/* Initialisation database */
	void dbInit(QString hostName, QString userName, QString password, QString dbName);

private slots:

	/* New WebSocket Client Connection */
	void onNewConnection();

	/* Receive message of WebSocket Client */
	void processTextMessage(const QString& message);

	/* Start timer of the sampling */
	void timerOn();

	/* Recuperation of Velman values and send them to database */
	void insertValue();

	/* Send	values at WebSocket Client */
	void selectValue();

	/* Websocket Client Disconnection */
	void socketDisconnected();

private:

	QSqlDatabase db;

	K8055Adapter * card = K8055Adapter::getInstance();

	QTimer *timer;
};

