#include "stdafx.h"
#include "Config.h"
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QMessageBox>

Config::Config(QWidget* parent) : QObject(parent)
{
	this->parent = parent;
}

Config::~Config()
{

}

bool Config::readFile(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::information(this->parent, QStringLiteral("Information"), QStringLiteral("Could not read config file, using default."), QMessageBox::Ok);
		return false;
	}
	QByteArray data = file.readAll();
	file.close();
	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (!doc.isObject())
	{
		QMessageBox::critical(this->parent, QStringLiteral("Error"), QStringLiteral("Config file format error, using default."), QMessageBox::Ok);
		return false;
	}
	this->obj = doc.object();
}

void Config::writeFile(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(this->parent, QStringLiteral("Error"), QStringLiteral("Could not write config file!"), QMessageBox::Ok);
		return;
	}
	QJsonDocument doc;
	doc.setObject(this->obj);
	file.write(doc.toJson());
	file.close();
}

double Config::getMinValue(QString key, double defaultValue)
{
	if (this->obj.contains(key) && this->obj[key].isObject())
	{
		QJsonObject subObj = obj[key].toObject();
		if (subObj.contains("min_value"))
		{
			QJsonValue minValue = subObj.value("min_value");
			if (minValue.isDouble())
				return minValue.toDouble();
			else
			{
				subObj["min_value"] = defaultValue;
				this->obj[key] = subObj;
			}
		}
		else
		{
			subObj.insert("min_value", defaultValue);
			this->obj[key] = subObj;
		}
	}
	else
	{
		QJsonObject subObj;
		subObj.insert("min_value", defaultValue);
		this->obj[key] = subObj;
	}
	return defaultValue;
}

double Config::getMaxValue(QString key, double defaultValue)
{
	if (this->obj.contains(key) && this->obj[key].isObject())
	{
		QJsonObject subObj = obj[key].toObject();
		if (subObj.contains("max_value"))
		{
			QJsonValue minValue = subObj.value("max_value");
			if (minValue.isDouble())
				return minValue.toDouble();
			else
			{
				subObj["max_value"] = defaultValue;
				this->obj[key] = subObj;
			}
		}
		else
		{
			subObj.insert("max_value", defaultValue);
			this->obj[key] = subObj;
		}
	}
	else
	{
		QJsonObject subObj;
		subObj.insert("max_value", defaultValue);
		this->obj[key] = subObj;
	}
	return defaultValue;
}

double Config::getThreshold(QString key, double defaultValue)
{
	if (this->obj.contains(key) && this->obj[key].isObject())
	{
		QJsonObject subObj = obj[key].toObject();
		if (subObj.contains("threshold"))
		{
			QJsonValue minValue = subObj.value("threshold");
			if (minValue.isDouble())
				return minValue.toDouble();
			else
			{
				subObj["threshold"] = defaultValue;
				this->obj[key] = subObj;
			}
		}
		else
		{
			subObj.insert("threshold", defaultValue);
			this->obj[key] = subObj;
		}
	}
	else
	{
		QJsonObject subObj;
		subObj.insert("threshold", defaultValue);
		this->obj[key] = subObj;
	}
	return defaultValue;
}

QString Config::getWebhook(QString defaultValue)
{
	if (this->obj.contains("webhook"))
	{
		QJsonValue webhook = this->obj.value("webhook");
		if (webhook.isString())
			return webhook.toString();
		else
			this->obj["webhook"] = defaultValue;
	}
	else
	{
		this->obj["webhook"] = defaultValue;
	}
	return defaultValue;
}

void Config::setMinValue(QString key, double value)
{
	QJsonObject subObj = obj[key].toObject();
	subObj.insert("min_value", value);
	this->obj[key] = subObj;
}

void Config::setMaxValue(QString key, double value)
{
	QJsonObject subObj = obj[key].toObject();
	subObj.insert("max_value", value);
	this->obj[key] = subObj;
}

void Config::setThreshold(QString key, double value)
{
	QJsonObject subObj = obj[key].toObject();
	subObj.insert("threshold", value);
	this->obj[key] = subObj;
}