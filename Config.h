#pragma once
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>

class Config : public QObject
{
	Q_OBJECT

private:
	QWidget* parent;
	QJsonObject obj;
public:
	Config(QWidget* parent = nullptr);
	~Config();

	bool readFile(QString filename);
	void writeFile(QString filename);

	double getMinValue(QString key, double defaultValue);
	double getMaxValue(QString key, double defaultValue);
	double getThreshold(QString key, double defaultValue);
	QString getWebhook(QString defaultValue);

	void setMinValue(QString key, double value);
	void setMaxValue(QString key, double value);
	void setThreshold(QString key, double value);
};
