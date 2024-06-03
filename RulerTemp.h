#pragma once
#include <QWidget>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QPropertyAnimation>
#include <QtMath>
#include <QPainter>
#include "Config.h"

class RulerTemp : public QWidget
{
	Q_OBJECT
		Q_ENUMS(BarPosition)
		Q_ENUMS(TickPosition)
		Q_PROPERTY(double value READ getValue WRITE setValue)

public:
	enum BarPosition {
		BarPosition_Left = 0,       //�����ʾ
		BarPosition_Right = 1,      //�Ҳ���ʾ
		BarPosition_Center = 2      //������ʾ
	};

	enum TickPosition {
		TickPosition_Null = 0,      //����ʾ
		TickPosition_Left = 1,      //�����ʾ
		TickPosition_Right = 2,     //�Ҳ���ʾ
		TickPosition_Both = 3       //������ʾ
	};

private:
	bool enabled;
	QString name;

	Config* configuration;
	double minValue;
	double maxValue;
	double thresholdValue;
	double value;
	double latestAlarmValue;

	double longStep;                //�������ȷֲ���
	double shortStep;               //�������ȷֲ���

	QColor thresholdValueColor;     //�û��趨ֵ��ɫ

	QColor bgColor;					//������ɫ
	QColor lineColor;               //������ɫ
	QColor barBgColor;              //��״����ɫ
	QColor barColor;                //��״��ɫ

	BarPosition barPosition;        //��״��λ��
	TickPosition tickPosition;      //�̶ȳ�λ��

	int nameHeight;
	int barWidth;                   //ˮ�������
	int radius;                     //ˮ�����ײ�Բ�뾶
	int rulerSpace;
	int setHeight;
	QRectF barRect;                 //��״����
	QRectF circleRect;              //�ײ�Բ����

	QPushButton* buttonSet;
	QMenu* menuSet;
	QAction* actionMinValue;
	QAction* actionMaxValue;
	QAction* actionThreshold;

	QPropertyAnimation* animation;

	void setColorNormal() {
		bgColor = QColor(255, 255, 255, 255);
		barColor = QColor(80, 80, 255, 255);
	};
	void setColorWarning() {
		bgColor = QColor(255, 255, 255, 255);
		barColor = QColor(255, 80, 80, 255);
	};
	void setColorError() {
		bgColor = QColor(255, 0, 0, 150);
	}
	void setColorDisabled() {
		bgColor = QColor(200, 200, 200, 255);
		barColor = QColor(100, 100, 100, 255);
	};
	void drawBg(QPainter* painter);
	void drawBarBg(QPainter* painter);
	void drawName(QPainter* painter);
	void drawRuler(QPainter* painter, int type);
	void drawBar(QPainter* painter);
	void drawValue(QPainter* painter);

public:
	explicit RulerTemp(QWidget* parent = 0, double updateInterval = 500.0);
	~RulerTemp();

	QString getName()			const { return this->name; };
	double	getMinValue()        const { return this->minValue; };
	double	getMaxValue()		const { return this->maxValue; };
	double	getThresholdValue()	const { return this->thresholdValue; };
	double	getValue()			const { return this->value; };
	bool	isAlarming() { return this->value > this->thresholdValue; };
	bool	needRemoteAlarm() {
		if (this->value > this->latestAlarmValue)
		{
			this->latestAlarmValue = this->value;
			return true;
		}
		else
			return false;
	}

	void setEnable(bool enable) {
		this->enabled = enable;
		if (!enable) {
			this->value = this->minValue;
			setColorDisabled();
			update();
		}
	};
	void setName(QString name) { this->name = name; };

	void setConfig(Config* configuration) { this->configuration = configuration; };
	void setMinValue(double minValue) {
		if (minValue >= this->maxValue)
			this->maxValue = minValue + 1.0;
		this->minValue = minValue;
		this->longStep = (this->maxValue - this->minValue) / 4.0;
		this->shortStep = (this->maxValue - this->minValue) / 8.0;
		update();
	};
	void setMaxValue(double maxValue) {
		if (maxValue <= this->minValue)
			this->minValue = maxValue - 1.0;
		this->maxValue = maxValue;
		this->longStep = (this->maxValue - this->minValue) / 4.0;
		this->shortStep = (this->maxValue - this->minValue) / 8.0;
		update();
	};
	void setThresholdValue(double thresholdValue) {
		this->thresholdValue = qMax(qMin(thresholdValue, this->maxValue), this->minValue);
		this->latestAlarmValue = this->thresholdValue;
		update();
	};
	void setValue(double value) {
		if (this->enabled) {
			if (value > this->maxValue || value < this->minValue)
				setColorError();
			else if (value > thresholdValue)
				setColorWarning();
			else
				setColorNormal();
			this->value = value;
			update();
		}
	};
	void setTargetValue(double value) {
		animation->setEndValue(value);
		animation->start();
	};
	void resetRemoteAlarm()
	{
		this->latestAlarmValue = this->thresholdValue;
	}

protected:
	void paintEvent(QPaintEvent*);

private slots:
	void onDialogMinValueClicked();
	void onDialogMaxValueClicked();
	void onDialogThresholdClicked();
};
