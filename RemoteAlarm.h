#pragma once
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "Config.h"

class RemoteAlarm :
    public QObject
{
    Q_OBJECT

public:
    RemoteAlarm(QWidget* parent, Config* configuration);
    ~RemoteAlarm();
    void setWebhook(QString url) { this->webhook = url; };
    void sendMessage(QString message);

private:
    QWidget* parent;
    QString webhook;
    QNetworkAccessManager* manager;
    QNetworkRequest request;
    QNetworkReply* reply;
    QSslConfiguration sslConfig;

    QJsonObject body;

private slots:
    void replyed();
};

