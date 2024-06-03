#include "stdafx.h"
#include "Monitor.h"
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <stdlib.h>
#include <windows.h>

Monitor::Monitor(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MonitorClass())
{
	ui->setupUi(this);

	configuration = new Config(this);
	configuration->readFile("config.json");
	readBuf = new ViByte[512];
	monitor_value = new double[64];

	vi_da = new MyVi(this);
	vi_da->openDefaultRM();
	vi_da->setResourceType(MyVi::USB);
	vi_vaccum = new MyVi(this);
	vi_vaccum->openDefaultRM();
	vi_vaccum->setResourceType(MyVi::ASRL);

	sample_timer.stop();

	widgetConnectDA = new Connect(vi_da);
	widgetConnectDA->setWindowModality(Qt::ApplicationModal);
	widgetConnectVaccum = new Connect(vi_vaccum);
	widgetConnectVaccum->setWindowModality(Qt::ApplicationModal);

	remoteAlarm = new RemoteAlarm(this, configuration);
	alarmed = false;

	ui->panelMagTrans16->setName(QStringLiteral("磁传输16号线圈"));
	ui->panelFast->setName(QStringLiteral("FAST"));
	ui->panelMagTrans13->setName(QStringLiteral("磁传输13号线圈"));
	ui->panelMagTrans14->setName(QStringLiteral("磁传输14号线圈"));
	ui->panelFeshbach->setName(QStringLiteral("FESHBACH"));
	ui->panelLevitation->setName(QStringLiteral("LEVITATION"));
	ui->panelQp->setName(QStringLiteral("QP"));
	ui->panelZeemanSlower->setName(QStringLiteral("塞曼减速"));
	ui->panelMagTransDown->setName(QStringLiteral("磁传输 下"));
	ui->panelMagTransUp->setName(QStringLiteral("磁传输 上"));
	ui->panel2dPush->setName(QStringLiteral("2D Push/\nZeeman 补偿"));
	ui->tempMagTrans1->setName(QStringLiteral("磁传输1号线圈"));
	ui->tempMagTrans2->setName(QStringLiteral("磁传输2号线圈"));
	ui->tempMagTrans3->setName(QStringLiteral("磁传输3号线圈"));
	ui->tempMagTransUp->setName(QStringLiteral("磁传输 上 水温"));

	
	configVaule(ui->panelMagTrans16, 0.0, 1.0, 0.4);
	configVaule(ui->panelFast, 0.0, 2.0, 1.0);
	configVaule(ui->panelMagTrans13, 0.0, 2.0, 0.9);
	configVaule(ui->panelMagTrans14, 0.0, 1.0, 0.5);
	configVaule(ui->panelFeshbach, 0.0, 2.0, 1.6);
	configVaule(ui->panelLevitation, 0.0, 1.0, 0.4);
	configVaule(ui->panelQp, 0.0, 2.0, 1.6);
	configVaule(ui->panelZeemanSlower, 0.0, 2.0, 0.4);
	configVaule(ui->panelMagTransDown, 0.0, 10.0, 0.0);
	configVaule(ui->panelMagTransUp, 0.0, 10.0, 5.0);
	configVaule(ui->panel2dPush, 0.0, 2.0, 1.0);
	configVaule(ui->tempMagTrans1, 10.0, 40.0, 30.0);
	configVaule(ui->tempMagTrans2, 10.0, 40.0, 22.0);
	configVaule(ui->tempMagTrans3, 10.0, 40.0, 22.0);
	configVaule(ui->tempMagTransUp, 20.0, 30.0, 25.0);
	thresholdScienceCell = configuration->getThreshold("Science Cell", 2.5e-11);
	threshold3DMOT = configuration->getThreshold("3D MOT", 1.0);
	threshold2DMOT = configuration->getThreshold("2D+ MOT", 6.0e-4);
	thresholdZeemanSlower = configuration->getThreshold("Zeeman Slower", 2e-10);
	configuration->writeFile("config.json");
	latestScienceCellAlarmValue = thresholdScienceCell;
	latest3DMOTAlarmValue = threshold3DMOT;
	latest2DMOTAlarmValue = threshold2DMOT;
	latestZeemanSlowerAlarmValue = thresholdZeemanSlower;

	ui->toggleSwitchRun->setText(QStringLiteral("运行"), QStringLiteral("停止"));
	ui->toggleSwitchAlarm->setText(QStringLiteral("启用报警"), QStringLiteral("停用报警"));
	ui->toggleSwitchRemoteAlarm->setText(QStringLiteral("远程报警开"), QStringLiteral("远程报警关"));

	connect(ui->actionConnectDA, &QAction::triggered, widgetConnectDA, &Connect::show);
	connect(ui->actionConnectVaccum, &QAction::triggered, widgetConnectVaccum, &Connect::show);
	connect(ui->toggleSwitchRun, SIGNAL(toggled(bool)), this, SLOT(switchToggled(bool)));
	connect(&sample_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	connect(ui->buttonSetScienceCell, SIGNAL(clicked()), this, SLOT(setThresholdScienceCell()));
	connect(ui->buttonSet3DMOT, SIGNAL(clicked()), this, SLOT(setThreshold3DMOT()));
	connect(ui->buttonSet2DMOT, SIGNAL(clicked()), this, SLOT(setThreshold2DMOT()));
	connect(ui->buttonSetZeemanSlower, SIGNAL(clicked()), this, SLOT(setThresholdZeemanSlower()));
}

Monitor::~Monitor()
{
	delete ui;
	delete configuration;
	delete monitor_value;
	delete readBuf;
	delete vi_da;
	delete vi_vaccum;
	delete widgetConnectDA;
	delete widgetConnectVaccum;
}

void Monitor::configVaule(GaugePanel* panel, double defaultMinValue, double defaultMaxValue, double defaultThreshold)
{
	double minValue, maxValue, threshold;
	minValue = configuration->getMinValue(panel->getName(), defaultMinValue);
	maxValue = configuration->getMaxValue(panel->getName(), defaultMaxValue);
	threshold = configuration->getThreshold(panel->getName(), defaultThreshold);
	panel->setConfig(this->configuration);
	panel->setMinValue(minValue);
	panel->setMaxValue(maxValue);
	panel->setThresholdValue(threshold);
}

void Monitor::configVaule(RulerTemp* temp, double defaultMinValue, double defaultMaxValue, double defaultThreshold)
{
	double minValue, maxValue, threshold;
	minValue = configuration->getMinValue(temp->getName(), defaultMinValue);
	maxValue = configuration->getMaxValue(temp->getName(), defaultMaxValue);
	threshold = configuration->getThreshold(temp->getName(), defaultThreshold);
	temp->setConfig(this->configuration);
	temp->setMinValue(minValue);
	temp->setMaxValue(maxValue);
	temp->setThresholdValue(threshold);
}

void Monitor::switchToggled(bool isOn)
{
	if (isOn) {
		if (vi_da->isConnected()) {
			ui->panelMagTrans16->setEnable(true);
			ui->panelFast->setEnable(true);
			ui->panelMagTrans13->setEnable(true);
			ui->panelMagTrans14->setEnable(true);
			ui->panelFeshbach->setEnable(true);
			ui->panelLevitation->setEnable(true);
			ui->panelQp->setEnable(true);
			ui->panelZeemanSlower->setEnable(true);
			ui->panelMagTransDown->setEnable(true);
			ui->panelMagTransUp->setEnable(true);
			ui->panel2dPush->setEnable(true);
			ui->tempMagTrans1->setEnable(true);
			ui->tempMagTrans2->setEnable(true);
			ui->tempMagTrans3->setEnable(true);
			ui->tempMagTransUp->setEnable(true);
			onTimeout();
			sample_timer.start(1000);
		}
		else {
			ui->toggleSwitchRun->toggleEvent();
			QMessageBox::critical(this, tr("Error"), tr("Device not connected."), QMessageBox::Ok);
		}
	}
	else {
		sample_timer.stop();
		ui->panelMagTrans16->setEnable(false);
		ui->panelFast->setEnable(false);
		ui->panelMagTrans13->setEnable(false);
		ui->panelMagTrans14->setEnable(false);
		ui->panelFeshbach->setEnable(false);
		ui->panelLevitation->setEnable(false);
		ui->panelQp->setEnable(false);
		ui->panelZeemanSlower->setEnable(false);
		ui->panelMagTransDown->setEnable(false);
		ui->panelMagTransUp->setEnable(false);
		ui->panel2dPush->setEnable(false);

		ui->tempMagTrans1->setEnable(false);
		ui->tempMagTrans2->setEnable(false);
		ui->tempMagTrans3->setEnable(false);
		ui->tempMagTransUp->setEnable(false);

		if (ui->toggleSwitchAlarm->isOn())
		{
			ui->toggleSwitchAlarm->toggleEvent();
			vi_da->write("SOUR:VOLT 0, (@201)\n");
		}
		if (ui->toggleSwitchRemoteAlarm->isOn())
		{
			ui->toggleSwitchRemoteAlarm->toggleEvent();
		}
		alarmed = false;
	}
}

void parse_floats(const char* input, int* count, double* num) {
	if (input == NULL || count == NULL) return;

	// 首先计算输入字符串中浮点数的数量
	*count = 1; // 至少有一个数
	for (const char* p = input; *p; ++p) {
		if (*p == ',') (*count)++;
	}

	// 逐个解析浮点数
	const char* start = input;
	for (int i = 0; i < *count; ++i) {
		char* end;
		num[i] = strtod(start, &end);
		start = end + 1; // 移动到下一个数的起始位置
	}
}

void Monitor::onTimeout()
{
	bool alarm = false;
	bool needRemoteAlarm = false;
	if (vi_da->isConnected()) {
		vi_da->write("MEAS? (@120:126,102,103,105,106,143:148,112:116,117,118,149:150,142,110,111,127:131,107:109,104)\n");
		retCount = vi_da->read(readBuf, 380);
		if (retCount)
		{
			parse_floats(reinterpret_cast<const char*>(readBuf), &monitor_count, monitor_value);
			if (monitor_count == 38) {
				ui->panelMagTrans16->setTargetValue(monitor_value[0] * 0.227);
				ui->panelFast->setTargetValue(monitor_value[1] * 0.227);
				ui->panelMagTrans13->setTargetValue(monitor_value[2] * 0.227);
				ui->panelMagTrans14->setTargetValue(monitor_value[3] * 0.227);
				ui->panelFeshbach->setTargetValue(monitor_value[4] * 0.227);
				ui->panelLevitation->setTargetValue(monitor_value[5] * 0.227);
				ui->panelQp->setTargetValue(monitor_value[6] * 0.227);
				ui->panelZeemanSlower->setTargetValue(monitor_value[7] * 0.227);
				ui->panelMagTransDown->setTargetValue(monitor_value[8] * 1.5);
				ui->panelMagTransUp->setTargetValue(monitor_value[9] * 1.5);
				ui->panel2dPush->setTargetValue(monitor_value[10] * 1.5);

				ui->tempMagTrans1->setTargetValue(-200.0 * monitor_value[34]);
				ui->tempMagTrans2->setTargetValue(-200.0 * monitor_value[35]);
				ui->tempMagTrans3->setTargetValue(-200.0 * monitor_value[36]);
				ui->tempMagTransUp->setTargetValue(15.0 * monitor_value[37] - 20.0);
			}
		}
		alarm = alarm || ui->panelMagTrans16->isAlarming();
		alarm = alarm || ui->panelFast->isAlarming();
		alarm = alarm || ui->panelMagTrans13->isAlarming();
		alarm = alarm || ui->panelMagTrans14->isAlarming();
		alarm = alarm || ui->panelFeshbach->isAlarming();
		alarm = alarm || ui->panelLevitation->isAlarming();
		alarm = alarm || ui->panelQp->isAlarming();
		alarm = alarm || ui->panelZeemanSlower->isAlarming();
		alarm = alarm || ui->panelMagTransDown->isAlarming();
		alarm = alarm || ui->panelMagTransUp->isAlarming();
		alarm = alarm || ui->panel2dPush->isAlarming();
		alarm = alarm || ui->tempMagTrans1->isAlarming();
		alarm = alarm || ui->tempMagTrans2->isAlarming();
		alarm = alarm || ui->tempMagTrans3->isAlarming();
		alarm = alarm || ui->tempMagTransUp->isAlarming();

		if (ui->toggleSwitchAlarm->isOn()) {
			needRemoteAlarm = needRemoteAlarm || ui->panelMagTrans16->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelFast->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelMagTrans13->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelMagTrans14->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelFeshbach->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelLevitation->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelQp->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelZeemanSlower->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelMagTransDown->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panelMagTransUp->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->panel2dPush->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->tempMagTrans1->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->tempMagTrans2->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->tempMagTrans3->needRemoteAlarm();
			needRemoteAlarm = needRemoteAlarm || ui->tempMagTransUp->needRemoteAlarm();
		}
	}

	if (vi_vaccum->isConnected()) {
		vi_vaccum->write("#000F\r");
		Sleep(100);
		retCount = vi_vaccum->read(readBuf, 41);
		if (retCount == 41) {
			readBuf[10] = '\0';
			ui->labelScienceCell->setText(QString((const char*)readBuf + 1));
			readBuf[20] = '\0';
			ui->label3DMOT->setText(QString((const char*)readBuf + 11));
			readBuf[30] = '\0';
			ui->label2DMOT->setText(QString((const char*)readBuf + 21));
			readBuf[40] = '\0';
			ui->labelZeemanSlower->setText(QString((const char*)readBuf + 31));
		}
		alarm = alarm || ui->labelScienceCell->text().toDouble() > thresholdScienceCell;
		alarm = alarm || ui->label3DMOT->text().toDouble() > threshold3DMOT;
		alarm = alarm || ui->label2DMOT->text().toDouble() > threshold2DMOT;
		alarm = alarm || ui->labelZeemanSlower->text().toDouble() > thresholdZeemanSlower;
		if (ui->labelScienceCell->text().toDouble() > latestScienceCellAlarmValue) {
			latestScienceCellAlarmValue = ui->labelScienceCell->text().toDouble();
			needRemoteAlarm = true;
		}
		if (ui->label3DMOT->text().toDouble() > latest3DMOTAlarmValue) {
			latest3DMOTAlarmValue = ui->label3DMOT->text().toDouble();
			needRemoteAlarm = true;
		}
		if (ui->label2DMOT->text().toDouble() > latest2DMOTAlarmValue) {
			latest2DMOTAlarmValue = ui->label2DMOT->text().toDouble();
			needRemoteAlarm = true;
		}
		if (ui->labelZeemanSlower->text().toDouble() > latestZeemanSlowerAlarmValue) {
			latestZeemanSlowerAlarmValue = ui->labelZeemanSlower->text().toDouble();
			needRemoteAlarm = true;
		}
	}

	if (vi_da->isConnected() && ui->toggleSwitchAlarm->isOn() && alarm)
	{
		vi_da->write("SOUR:VOLT 10,(@201)\n");
	}
	else
	{
		vi_da->write("SOUR:VOLT 0,(@201)\n");
	}

	if (ui->toggleSwitchRemoteAlarm->isOn() && needRemoteAlarm)
	{
		alarmed = true;
		QString text;
		text += QStringLiteral("监控系统报警: \n\n水冷流量: ");
		if (ui->panelMagTrans16->isAlarming()) {
			text += QString("\n") + ui->panelMagTrans16->getName() + QString(": ");
			text += QString::number(ui->panelMagTrans16->getValue()) + QString(" / ") + QString::number(ui->panelMagTrans16->getThresholdValue());
		}
		if (ui->panelFast->isAlarming()) {
			text += QString("\n") + ui->panelFast->getName() + QString(": ");
			text += QString::number(ui->panelFast->getValue()) + QString(" / ") + QString::number(ui->panelFast->getThresholdValue());
		}
		if (ui->panelMagTrans13->isAlarming()) {
			text += QString("\n") + ui->panelMagTrans13->getName() + QString(": ");
			text += QString::number(ui->panelMagTrans13->getValue()) + QString(" / ") + QString::number(ui->panelMagTrans13->getThresholdValue());
		}
		if (ui->panelMagTrans14->isAlarming()) {
			text += QString("\n") + ui->panelMagTrans14->getName() + QString(": ");
			text += QString::number(ui->panelMagTrans14->getValue()) + QString(" / ") + QString::number(ui->panelMagTrans14->getThresholdValue());
		}
		if (ui->panelFeshbach->isAlarming()) {
			text += QString("\n") + ui->panelFeshbach->getName() + QString(": ");
			text += QString::number(ui->panelFeshbach->getValue()) + QString(" / ") + QString::number(ui->panelFeshbach->getThresholdValue());
		}
		if (ui->panelLevitation->isAlarming()) {
			text += QString("\n") + ui->panelLevitation->getName() + QString(": ");
			text += QString::number(ui->panelLevitation->getValue()) + QString(" / ") + QString::number(ui->panelLevitation->getThresholdValue());
		}
		if (ui->panelQp->isAlarming()) {
			text += QString("\n") + ui->panelQp->getName() + QString(": ");
			text += QString::number(ui->panelQp->getValue()) + QString(" / ") + QString::number(ui->panelQp->getThresholdValue());
		}
		if (ui->panelZeemanSlower->isAlarming()) {
			text += QString("\n") + ui->panelZeemanSlower->getName() + QString(": ");
			text += QString::number(ui->panelZeemanSlower->getValue()) + QString(" / ") + QString::number(ui->panelZeemanSlower->getThresholdValue());
		}
		if (ui->panelMagTransDown->isAlarming()) {
			text += QString("\n") + ui->panelMagTransDown->getName() + QString(": ");
			text += QString::number(ui->panelMagTransDown->getValue()) + QString(" / ") + QString::number(ui->panelMagTransDown->getThresholdValue());
		}
		if (ui->panelMagTransUp->isAlarming()) {
			text += QString("\n") + ui->panelMagTransUp->getName() + QString(": ");
			text += QString::number(ui->panelMagTransUp->getValue()) + QString(" / ") + QString::number(ui->panelMagTransUp->getThresholdValue());
		}
		if (ui->panel2dPush->isAlarming()) {
			text += QString("\n") + ui->panel2dPush->getName() + QString(": ");
			text += QString::number(ui->panel2dPush->getValue()) + QString(" / ") + QString::number(ui->panel2dPush->getThresholdValue());
		}
		text += QStringLiteral("\n\n水温: ");
		if (ui->tempMagTrans1->isAlarming()) {
			text += QString("\n") + ui->tempMagTrans1->getName() + QString(": ");
			text += QString::number(ui->tempMagTrans1->getValue()) + QString(" / ") + QString::number(ui->tempMagTrans1->getThresholdValue());
		}
		if (ui->tempMagTrans2->isAlarming()) {
			text += QString("\n") + ui->tempMagTrans2->getName() + QString(": ");
			text += QString::number(ui->tempMagTrans2->getValue()) + QString(" / ") + QString::number(ui->tempMagTrans2->getThresholdValue());
		}
		if (ui->tempMagTrans3->isAlarming()) {
			text += QString("\n") + ui->tempMagTrans3->getName() + QString(": ");
			text += QString::number(ui->tempMagTrans3->getValue()) + QString(" / ") + QString::number(ui->tempMagTrans3->getThresholdValue());
		}
		if (ui->tempMagTransUp->isAlarming()) {
			text += QString("\n") + ui->tempMagTransUp->getName() + QString(": ");
			text += QString::number(ui->tempMagTransUp->getValue()) + QString(" / ") + QString::number(ui->tempMagTransUp->getThresholdValue());
		}
		text += QStringLiteral("\n\n真空: ");
		if (vi_vaccum->isConnected()) {
			if (ui->labelScienceCell->text().toDouble() > thresholdScienceCell) {
				text += QString("\n") + ui->labelForScienceCell->text() + QString(": ");
				text += ui->labelScienceCell->text() + QString(" / ") + QString::number(thresholdScienceCell, 'e', 3);
			}
			if (ui->label3DMOT->text().toDouble() > threshold3DMOT) {
				text += QString("\n") + ui->labelFor3DMOT->text() + QString(": ");
				text += ui->label3DMOT->text() + QString(" / ") + QString::number(threshold3DMOT, 'e', 3);
			}
			if (ui->label2DMOT->text().toDouble() > threshold2DMOT) {
				text += QString("\n") + ui->labelFor2DMOT->text() + QString(": ");
				text += ui->label2DMOT->text() + QString(" / ") + QString::number(threshold2DMOT, 'e', 3);
			}
			if (ui->labelZeemanSlower->text().toDouble() > thresholdZeemanSlower) {
				text += QString("\n") + ui->labelForZeemanSlower->text() + QString(": ");
				text += ui->labelZeemanSlower->text() + QString(" / ") + QString::number(thresholdZeemanSlower, 'e', 3);
			}
		}
		remoteAlarm->sendMessage(text);
	}
	else if (ui->toggleSwitchRemoteAlarm->isOn() && alarmed && !alarm) {
		alarmed = false;
		ui->panelMagTrans16->resetRemoteAlarm();
		ui->panelFast->resetRemoteAlarm();
		ui->panelMagTrans13->resetRemoteAlarm();
		ui->panelMagTrans14->resetRemoteAlarm();
		ui->panelFeshbach->resetRemoteAlarm();
		ui->panelLevitation->resetRemoteAlarm();
		ui->panelQp->resetRemoteAlarm();
		ui->panelZeemanSlower->resetRemoteAlarm();
		ui->panelMagTransDown->resetRemoteAlarm();
		ui->panelMagTransUp->resetRemoteAlarm();
		ui->panel2dPush->resetRemoteAlarm();
		ui->tempMagTrans1->resetRemoteAlarm();
		ui->tempMagTrans2->resetRemoteAlarm();
		ui->tempMagTrans3->resetRemoteAlarm();
		ui->tempMagTransUp->resetRemoteAlarm();
		latestScienceCellAlarmValue = thresholdScienceCell;
		latest3DMOTAlarmValue = threshold3DMOT;
		latest2DMOTAlarmValue = threshold2DMOT;
		latestZeemanSlowerAlarmValue = thresholdZeemanSlower;
		remoteAlarm->sendMessage(QStringLiteral("所有状态恢复正常"));
	}
	else if (!ui->toggleSwitchRemoteAlarm->isOn() && alarmed) {
		alarmed = false;
		ui->panelMagTrans16->resetRemoteAlarm();
		ui->panelFast->resetRemoteAlarm();
		ui->panelMagTrans13->resetRemoteAlarm();
		ui->panelMagTrans14->resetRemoteAlarm();
		ui->panelFeshbach->resetRemoteAlarm();
		ui->panelLevitation->resetRemoteAlarm();
		ui->panelQp->resetRemoteAlarm();
		ui->panelZeemanSlower->resetRemoteAlarm();
		ui->panelMagTransDown->resetRemoteAlarm();
		ui->panelMagTransUp->resetRemoteAlarm();
		ui->panel2dPush->resetRemoteAlarm();
		ui->tempMagTrans1->resetRemoteAlarm();
		ui->tempMagTrans2->resetRemoteAlarm();
		ui->tempMagTrans3->resetRemoteAlarm();
		ui->tempMagTransUp->resetRemoteAlarm();
		latestScienceCellAlarmValue = thresholdScienceCell;
		latest3DMOTAlarmValue = threshold3DMOT;
		latest2DMOTAlarmValue = threshold2DMOT;
		latestZeemanSlowerAlarmValue = thresholdZeemanSlower;
		remoteAlarm->sendMessage(QStringLiteral("远程报警关闭"));
	}
}

void Monitor::setThresholdScienceCell()
{
	bool ok = false;
	QString setValue = QInputDialog::getText(this, QStringLiteral("设置"), QStringLiteral("设置报警阈值"), QLineEdit::Normal, QString::number(this->thresholdScienceCell, 'e', 3), &ok);
	if (ok) {
		this->thresholdScienceCell = setValue.toDouble();
		if (this->configuration) {
			this->configuration->setThreshold("Science Cell", setValue.toDouble());
			this->configuration->writeFile("config.json");
		}
	}
}

void Monitor::setThreshold3DMOT()
{
	bool ok = false;
	QString setValue = QInputDialog::getText(this, QStringLiteral("设置"), QStringLiteral("设置报警阈值"), QLineEdit::Normal, QString::number(this->threshold3DMOT, 'e', 3), &ok);
	if (ok) {
		this->threshold3DMOT = setValue.toDouble();
		if (this->configuration) {
			this->configuration->setThreshold("3D MOT", setValue.toDouble());
			this->configuration->writeFile("config.json");
		}
	}
}

void Monitor::setThreshold2DMOT()
{
	bool ok = false;
	QString setValue = QInputDialog::getText(this, QStringLiteral("设置"), QStringLiteral("设置报警阈值"), QLineEdit::Normal, QString::number(this->threshold2DMOT, 'e', 3), &ok);
	if (ok) {
		this->threshold2DMOT = setValue.toDouble();
		if (this->configuration) {
			this->configuration->setThreshold("2D+ MOT", setValue.toDouble());
			this->configuration->writeFile("config.json");
		}
	}
}

void Monitor::setThresholdZeemanSlower()
{
	bool ok = false;
	QString setValue = QInputDialog::getText(this, QStringLiteral("设置"), QStringLiteral("设置报警阈值"), QLineEdit::Normal, QString::number(this->thresholdZeemanSlower, 'e', 3), &ok);
	if (ok) {
		this->thresholdZeemanSlower = setValue.toDouble();
		if (this->configuration) {
			this->configuration->setThreshold("Zeeman Slower", setValue.toDouble());
			this->configuration->writeFile("config.json");
		}
	}
}