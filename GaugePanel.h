#pragma once
#include <QWidget>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QPropertyAnimation>
#include <QtMath>
#include <QPainter>
#include <QtMath>
#include "Config.h"

class GaugePanel : public QWidget
{
	Q_OBJECT
		Q_PROPERTY(double value READ getValue WRITE setValue)

private:
	bool enabled;
	QString name;

	double minValue;
	double maxValue;
	double thresholdValue;
	double value;
	double latestAlarmValue;
	
	double radiusInner;             //渐变内圈内半径
	double radiusOuter;             //渐变外圈内半径
	double radiusHalo;              //光晕内半径
	QColor colorOuterFrame;         //表盘外边框颜色
	QColor colorInnerStart;         //渐变内圈起始颜色
	QColor colorInnerEnd;           //渐变内圈结束颜色
	QColor colorOuterStart;         //渐变外圈起始颜色
	QColor colorOuterEnd;           //渐变外圈结束颜色
	QColor colorHaloStart;          //光晕起始颜色
	QColor colorHaloEnd;            //光晕结束颜色
	QColor colorPointer;			//指针颜色
	QColor colorPointerSector;		//指针扇区颜色

	QPushButton* buttonSet;
	QMenu* menuSet;
	Config* configuration;
	QAction* actionMinValue;
	QAction* actionMaxValue;
	QAction* actionThreshold;
	
	QPropertyAnimation* animation;

	void drawOuterGradient(QPainter* painter);
	void drawInnerGradient(QPainter* painter);
	void drawOuterHalo(QPainter* painter);
	void drawScale(QPainter* painter);
	void drawThresholdTriangle(QPainter* painter);
	void drawScaleNum(QPainter* painter);
	void drawPointer(QPainter* painter);
	void drawPointerSector(QPainter* painter);
	void drawName(QPainter* painter);
	void drawValue(QPainter* painter);
	void drawUnit(QPainter* painter);

	void setColorNormal();
	void setColorWarning();
	void setColorError();
	void setColorDisabled();

public:
	explicit GaugePanel(QWidget* parent = nullptr);
	~GaugePanel();

	QString getName()			const { return this->name; };
	double	getMinValue()		const { return this->minValue; };
	double	getMaxValue()		const { return this->maxValue; };
	double	getThresholdValue()	const { return this->thresholdValue; };
	double	getValue()			const { return this->value; };
	bool	isAlarming() { return this->value < this->thresholdValue; };
	bool	needRemoteAlarm() {
		if (this->value < this->latestAlarmValue)
		{
			this->latestAlarmValue = this->value;
			return true;
		}
		else
			return false;
	}

	void setName(QString name) {
		this->name = name;
	};
	void setConfig(Config* configuration) { this->configuration = configuration; };
	void setMinValue(double value) {
		if (value >= this->maxValue)
			this->maxValue = value + 1.0;
		this->minValue = qMax(value, 0.0);
		update();
	};
	void setMaxValue(double value) {
		if (value <= this->minValue)
			this->minValue = qMax(value - 1.0, 0.0);
		this->maxValue = qMax(value, 1.0);
		update();
	};
	void setThresholdValue(double value) {
		this->thresholdValue = qMax(qMin(value, this->maxValue), this->minValue);
		this->latestAlarmValue = this->thresholdValue;
		update();
	};
	void setValue(double value) {
		if (this->enabled) {
			if (value > this->maxValue || value < this->minValue)
				setColorError();
			else if (value < thresholdValue)
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
	}
	void setEnable(bool enable) {
		this->enabled = enable;
		if (!enable) {
			this->value = this->minValue;
			setColorDisabled();
			update();
		}
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
