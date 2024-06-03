#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Monitor.h"
#include "Connect.h"
#include "Config.h"
#include "MyVi.h"
#include "RemoteAlarm.h"
#include "visa.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MonitorClass; };
QT_END_NAMESPACE

class Monitor : public QMainWindow
{
	Q_OBJECT

public:
	Monitor(QWidget* parent = nullptr);
	~Monitor();

private:
	Ui::MonitorClass* ui;
	Connect* widgetConnectDA;
	Connect* widgetConnectVaccum;

	double thresholdScienceCell;
	double threshold3DMOT;
	double threshold2DMOT;
	double thresholdZeemanSlower;
	double latestScienceCellAlarmValue;
	double latest3DMOTAlarmValue;
	double latest2DMOTAlarmValue;
	double latestZeemanSlowerAlarmValue;


	Config* configuration;
	MyVi* vi_da;
	MyVi* vi_vaccum;
	ViPBuf readBuf;
	ViUInt32 retCount;

	double* monitor_value;
	int monitor_count;

	QTimer sample_timer;
	RemoteAlarm* remoteAlarm;
	bool alarmed;

	void configVaule(GaugePanel* panel, double defaultMinValue, double defaultMaxValue, double defaultThreshold);
	void configVaule(RulerTemp* panel, double defaultMinValue, double defaultMaxValue, double defaultThreshold);

private slots:
	void switchToggled(bool isOn);
	void onTimeout();
	void setThresholdScienceCell();
	void setThreshold3DMOT();
	void setThreshold2DMOT();
	void setThresholdZeemanSlower();
};
