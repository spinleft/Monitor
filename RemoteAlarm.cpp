#include "stdafx.h"
#include "RemoteAlarm.h"
#include <QUrl>
#include <QJsonObject>
#include <QSslSocket>

RemoteAlarm::RemoteAlarm(QWidget* parent, Config* configuration)
{
	this->parent = parent;
	this->webhook = configuration->getWebhook(QString("https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=d84c8c27-39e3-40cf-a854-dd93d084f63d"));
	configuration->writeFile("config.json");
	manager = new QNetworkAccessManager(this);
	body.insert("msgtype", QString("text"));
}

RemoteAlarm::~RemoteAlarm()
{
	delete manager;
}

void RemoteAlarm::sendMessage(QString message)
{
	request.setUrl(QUrl(webhook));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	
	sslConfig = QSslConfiguration::defaultConfiguration();
	sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
	sslConfig.setProtocol(QSsl::AnyProtocol);
	request.setSslConfiguration(sslConfig);

	QJsonObject textObj;
	textObj.insert("content", message);
	body.insert("text", textObj);
	QJsonDocument doc;
	doc.setObject(body);

	reply = manager->post(request, doc.toJson());
}

void RemoteAlarm::replyed()
{
	if (reply->error() == QNetworkReply::NoError)
	{
		QString ba = reply->readAll();      // 自行解析接口返回数据
		QMessageBox::warning(parent, "reply", ba);
	}
	else
	{
		QMessageBox::warning(parent, "123", reply->errorString());
	}
}