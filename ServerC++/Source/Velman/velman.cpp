//**********************************************************************************************//
//* Programme	:	velman.cpp																	//
//*																	Date : 11/01/2022			//
//*																	Last update : 16/01/2022	//
//*---------------------------------------------------------------------------------------------//
//* Developper	:	CAUET Clément																//
//*---------------------------------------------------------------------------------------------//
//*																								//
//* Goal		:	Receive values of the Velman and send them to database						//
//*																								//
//**********************************************************************************************//

#include "velman.h"

/* Constructeur */
velman::velman(quint16 port)
{
	card->OpenDevice(0);

	this->webSocketServer = new QWebSocketServer(QStringLiteral("Server WebSocket"), QWebSocketServer::NonSecureMode);

	if (this->webSocketServer->listen(QHostAddress::AnyIPv4, port))
	{
		qDebug() << "Server WebSocket: Nouvelle connexion sur le port " << port << "\n";

		QObject::connect(webSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	}
	else
	{
		qDebug() << "Server WebSocket: Erreur d'ecoute sur le port " << port << "\n";
	}
}

/* Destructeur */
velman::~velman()
{
	webSocketServer->close();
}

/* New WebSocket Client Connection */
void velman::onNewConnection()
{
	socket = this->webSocketServer->nextPendingConnection();

	QObject::connect(socket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(processTextMessage(const QString&)));

	QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

	this->wsclients.push_back(socket);
}

/* Receive message of WebSocket Client */
void velman::processTextMessage(const QString & message)
{
	QString hostName = message.section(";", 0, 0);
	QString userName = message.section(";", 1, 1);
	QString password = message.section(";", 2, 2);
	QString dbName = message.section(";", 3, 3);

	dbInit(hostName, userName, password, dbName);
}

/* Initialisation database */
void velman::dbInit(QString hostName, QString userName, QString password, QString dbName)
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(hostName);
	db.setUserName(userName);
	db.setPassword(password);
	db.setDatabaseName(dbName);

	if (db.open()) {
		qDebug() << "BDD : New Connection\n";
		selectValue();
		timerOn();
	}
	else {
		qDebug() << db.lastError();
	}

}

/* Start timer of the sampling */
void velman::timerOn()
{
	timer = new QTimer(this);
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(insertValue()));

	timer->start(10000);
}

/* Recuperation of Velman values and send them to database */
void velman::insertValue()
{
	float bruteValue = card->ReadAnalogChannel(1);
	float voltageValue = (bruteValue / 255) * 5;
	float tempValue = (voltageValue * 90 / 5) - 30;

	QSqlQuery query;
	query.prepare("INSERT INTO `meteo`(`bruteValue`, `tension`, `temperature`) VALUES(:bruteValue, :voltageValue, :tempValue)");
	query.bindValue(":bruteValue", bruteValue);
	query.bindValue(":voltageValue", voltageValue);
	query.bindValue(":tempValue", tempValue);
	query.exec();

	query.exec("SELECT `bruteValue`, `tension`, `temperature`, DATE_FORMAT(`date`, '%H:%i:%s %d/%m/%Y') FROM `meteo` ORDER BY `date` DESC");

	if (query.next())
	{
		QString bruteValue = query.value(0).toString();
		QString voltageValue = query.value(1).toString();
		QString tempValue = query.value(2).toString();
		QString date = query.value(3).toString();

		QString sendText = bruteValue + ";" + voltageValue + ";" + tempValue + ";" + date;

		for (QList<QWebSocket*>::iterator it = wsclients.begin(); it != wsclients.end(); it++) {
			(*it)->sendTextMessage(sendText);
		}
	}
}

/* Send	values at WebSocket Client */
void velman::selectValue()
{
	QSqlQuery query;
	query.exec("SELECT `bruteValue`, `tension`, `temperature`, DATE_FORMAT(`date`, '%H:%i:%s %d/%m/%Y') FROM `meteo` ORDER BY `date`");

	while (query.next()) 
	{
		QString bruteValue = query.value(0).toString();
		QString voltageValue = query.value(1).toString();
		QString tempValue = query.value(2).toString();
		QString date = query.value(3).toString();

		QString sendText = bruteValue + ";" + voltageValue + ";" + tempValue + ";" + date;

		socket->sendTextMessage(sendText);
	}
	
}

/* Websocket Client Disconnection */
void velman::socketDisconnected()
{
	timer->stop();

	qDebug() << "Server WebSocket: Deconnexion\n";
}
