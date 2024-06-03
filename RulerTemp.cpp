#include "stdafx.h"
#include "RulerTemp.h"

RulerTemp::RulerTemp(QWidget* parent, double updateInterval) : QWidget(parent)
{
	this->enabled = false;
	this->name = QString("Default");

	this->configuration = nullptr;
	this->minValue = 10.0;
	this->maxValue = 40.0;
	this->thresholdValue = 22;
	this->value = this->minValue;

	this->longStep = 2;
	this->shortStep = 1;

	this->thresholdValueColor = QColor(255, 0, 0, 250);
	this->lineColor = QColor(0, 0, 0, 255);
	this->barBgColor = QColor(100, 180, 255, 255);
	setColorDisabled();

	this->barPosition = BarPosition_Center;
	this->tickPosition = TickPosition_Both;

	this->nameHeight = 40;
	this->barWidth = 16;
	this->radius = 24;
	this->rulerSpace = 10;
	this->setHeight = 30;

	this->buttonSet = new QPushButton(QStringLiteral("设置"), this);
	this->buttonSet->setFont(QFont("Arial", 10, 0, false));
	this->buttonSet->setGeometry(45, 270, 50, 25);
	this->menuSet = new QMenu(this);
	this->actionMinValue = new QAction(QStringLiteral("最小值"), this);
	this->actionMaxValue = new QAction(QStringLiteral("最大值"), this);
	this->actionThreshold = new QAction(QStringLiteral("报警阈值"), this);
	this->menuSet->addAction(this->actionMinValue);
	this->menuSet->addAction(this->actionMaxValue);
	this->menuSet->addAction(this->actionThreshold);
	this->buttonSet->setMenu(this->menuSet);

	connect(this->actionMinValue, SIGNAL(triggered()), this, SLOT(onDialogMinValueClicked()));
	connect(this->actionMaxValue, SIGNAL(triggered()), this, SLOT(onDialogMaxValueClicked()));
	connect(this->actionThreshold, SIGNAL(triggered()), this, SLOT(onDialogThresholdClicked()));

	animation = new QPropertyAnimation(this, "value");
	animation->setDuration(updateInterval);
}

RulerTemp::~RulerTemp()
{
}

void RulerTemp::paintEvent(QPaintEvent*)
{
	//绘制准备工作,启用反锯齿
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	//绘制背景
	drawBg(&painter);

	//绘制标尺及刻度尺
	if (tickPosition == TickPosition_Left) {
		drawRuler(&painter, 0);
	}
	else if (tickPosition == TickPosition_Right) {
		drawRuler(&painter, 1);
	}
	else if (tickPosition == TickPosition_Both) {
		drawRuler(&painter, 0);
		drawRuler(&painter, 1);
	}

	//绘制水银柱背景,包含水银柱底部圆
	drawBarBg(&painter);

	drawName(&painter);

	//绘制当前水银柱,包含水银柱底部圆
	drawBar(&painter);

	//绘制当前值
	drawValue(&painter);
}

void RulerTemp::drawBg(QPainter* painter)
{
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(bgColor);
	painter->drawRect(rect());
	painter->restore();
}

void RulerTemp::drawName(QPainter* painter)
{
	painter->save();
	painter->setPen(QColor(0, 0, 0));
	painter->setFont(QFont("Arial", 10, 20, false));
	QRect textRect(0, 0, width(), nameHeight);
	painter->drawText(textRect, Qt::AlignCenter, this->name);
	painter->restore();
}

void RulerTemp::drawBarBg(QPainter* painter)
{
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(barBgColor);

	int barX = width() / 2 - barWidth / 2;
	int barY = nameHeight;
	int barHeight = height() - barY - setHeight - 2 * radius;
	QRectF barRect(barX, barY, barWidth, barHeight);

	int circleX = width() / 2 - radius;
	//偏移 2 个像素,使得看起来边缘完整
	int circleY = height() - radius * 2 - 2 - setHeight;
	int circleWidth = radius * 2;
	QRectF circleRect(circleX, circleY, circleWidth, circleWidth);

	QPainterPath path;
	path.addRect(barRect);
	path.addEllipse(circleRect);
	path.setFillRule(Qt::WindingFill);
	painter->drawPath(path);
	painter->restore();
}

void RulerTemp::drawRuler(QPainter* painter, int type)
{
	painter->save();
	painter->setPen(lineColor);

	int barPercent = barWidth / 8;

	if (barPercent < 2) {
		barPercent = 2;
	}

	//绘制纵向标尺刻度
	double length = height() - nameHeight - setHeight - 2 * radius - rulerSpace;
	//计算每一格移动多少
	double increment = length / (maxValue - minValue);

	//长线条短线条长度
	int longLineLen = 10;
	int shortLineLen = 7;

	//绘制纵向标尺线 偏移 5 像素
	int offset = barWidth / 2 + 5;

	//左侧刻度尺需要重新计算
	if (type == 0) {
		offset = -offset;
		longLineLen = -longLineLen;
		shortLineLen = -shortLineLen;
	}

	double initX = width() / 2 + offset;
	double initY = nameHeight + barPercent;
	QPointF topPot(initX, initY);
	QPointF bottomPot(initX, height() - setHeight - 2 * radius - rulerSpace);
	painter->drawLine(topPot, bottomPot);

	//根据范围值绘制刻度值及刻度值
	int count = 0;
	for (double i = maxValue; i >= minValue; i = i - shortStep) {
		if (qAbs(maxValue - i - longStep * count) < 1e-5) {
			count++;

			//绘制长线条
			QPointF leftPot(initX + longLineLen, initY);
			QPointF rightPot(initX, initY);
			painter->drawLine(leftPot, rightPot);

			//绘制文字
			QString strValue;
			if (qAbs(i - (int)i) < 1e-5)
				strValue = QString("%1").arg((double)i, 0, 'f', 0);
			else
				strValue = QString("%1").arg((double)i, 0, 'f', 1);
			double fontHeight = painter->fontMetrics().height();

			if (type == 0) {
				QRect textRect(initX - 50, initY - fontHeight / 2, 35, 15);
				painter->drawText(textRect, Qt::AlignRight, strValue);
			}
			else if (type == 1) {
				QRect textRect(initX + longLineLen + 5, initY - fontHeight / 2, 35, 15);
				painter->drawText(textRect, Qt::AlignLeft, strValue);
			}
		}
		else {
			//绘制短线条
			QPointF leftPot(initX + shortLineLen, initY);
			QPointF rightPot(initX, initY);
			painter->drawLine(leftPot, rightPot);
		}

		initY += increment * shortStep;
	}

	painter->restore();
}

void RulerTemp::drawBar(QPainter* painter)
{
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(barColor);

	//计算在背景宽度的基础上缩小的百分比, 至少为 2
	int barPercent = barWidth / 8;
	int circlePercent = radius / 6;

	if (barPercent < 2) {
		barPercent = 2;
	}

	if (circlePercent < 2) {
		circlePercent = 2;
	}

	//标尺长度
	double length = height() - nameHeight - setHeight - 2 * radius - rulerSpace;
	//计算每一格移动多少
	double increment = length / (maxValue - minValue);
	//计算标尺的高度
	int rulerHeight = nameHeight + barPercent;

	double showValue = qMax(qMin(value, maxValue), minValue);
	int barX = width() / 2 - barWidth / 2;
	int barY = rulerHeight + (maxValue - showValue) * increment;
	int barHeight = height() - barY - setHeight - radius;
	barRect = QRectF(barX + barPercent, barY, barWidth - barPercent * 2, barHeight);

	int circleX = width() / 2 - radius;
	//偏移 2 个像素,使得看起来边缘完整
	int circleY = height() - setHeight - radius * 2 - 2;
	int circleWidth = radius * 2 - circlePercent * 2;
	circleRect = QRectF(circleX + circlePercent, circleY + circlePercent, circleWidth, circleWidth);

	QPainterPath path;
	path.addRect(barRect);
	path.addEllipse(circleRect);
	path.setFillRule(Qt::WindingFill);
	painter->drawPath(path);

	//绘制用户设定值三角号
	if (tickPosition == TickPosition_Left || tickPosition == TickPosition_Both) {
		QPolygon pts;
		int offset = 7;
		double initX = width() / 2 - (barWidth / 2 + 5);
		double initY = rulerHeight + (maxValue - thresholdValue) * increment;
		pts.append(QPoint(initX, initY));
		pts.append(QPoint(initX - offset, initY - offset / 2));
		pts.append(QPoint(initX - offset, initY + offset / 2));
		painter->setBrush(thresholdValueColor);
		painter->drawPolygon(pts);
	}

	if (tickPosition == TickPosition_Right || tickPosition == TickPosition_Both) {
		QPolygon pts;
		int offset = 7;
		double initX = width() / 2 + (barWidth / 2 + 5);
		double initY = rulerHeight + (maxValue - thresholdValue) * increment;
		pts.append(QPoint(initX, initY));
		pts.append(QPoint(initX + offset, initY - offset / 2));
		pts.append(QPoint(initX + offset, initY + offset / 2));
		painter->setBrush(thresholdValueColor);
		painter->drawPolygon(pts);
	}

	painter->restore();
}

void RulerTemp::drawValue(QPainter* painter)
{
	painter->save();

	QFont font;
	font.setPixelSize(circleRect.width() * 0.4);
	painter->setFont(font);
	painter->setPen(Qt::white);
	painter->drawText(circleRect, Qt::AlignCenter, QString("%1").arg(value, 0, 'f', 1));

	painter->restore();
}

void RulerTemp::onDialogMinValueClicked()
{
	bool ok = false;
	double setValue = QInputDialog::getDouble(this, QStringLiteral("设置"), QStringLiteral("设置最小值"), this->getMinValue(), 0.0, 100.0, 1, &ok);
	if (ok) {
		this->setMinValue(setValue);
		if (this->configuration) {
			this->configuration->setMinValue(this->getName(), setValue);
			this->configuration->writeFile("config.json");
		}
	}
}

void RulerTemp::onDialogMaxValueClicked()
{
	bool ok = false;
	double setValue = QInputDialog::getDouble(this, QStringLiteral("设置"), QStringLiteral("设置最大值"), this->getMaxValue(), 0.0, 100.0, 1, &ok);
	if (ok) {
		this->setMaxValue(setValue);
		if (this->configuration) {
			this->configuration->setMaxValue(this->getName(), setValue);
			this->configuration->writeFile("config.json");
		}
	}
}

void RulerTemp::onDialogThresholdClicked()
{
	bool ok = false;
	double setValue = QInputDialog::getDouble(this, QStringLiteral("设置"), QStringLiteral("设置报警阈值"), this->getThresholdValue(), this->getMinValue(), this->getMaxValue(), 1, &ok);
	if (ok) {
		this->setThresholdValue(setValue);
		if (this->configuration) {
			this->configuration->setThreshold(this->getName(), setValue);
			this->configuration->writeFile("config.json");
		}
	}
}