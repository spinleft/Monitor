#include "stdafx.h"
#include "GaugePanel.h"
#include <QInputDialog>

GaugePanel::GaugePanel(QWidget* parent) : QWidget(parent)
{
	this->enabled = false;
	this->name = QString("Default");
	this->minValue = 0.0;
	this->maxValue = 1.0;
	this->thresholdValue = 0.3;
	this->value = 0.0;
	
	this->radiusInner = 65.0;
	this->radiusOuter = 80.0;
	this->radiusHalo = 95.0;
	setColorDisabled();

	this->buttonSet = new QPushButton(QStringLiteral("设置"), this);
	this->buttonSet->setFont(QFont("Arial", 10, 0, false));
	this->buttonSet->setGeometry(75, 160, 50, 25);
	this->menuSet = new QMenu(this);
	this->actionMinValue = new QAction(QStringLiteral("最小值"), this);
	this->actionMaxValue = new QAction(QStringLiteral("最大值"), this);
	this->actionThreshold = new QAction(QStringLiteral("报警阈值"), this);
	this->menuSet->addAction(this->actionMinValue);
	this->menuSet->addAction(this->actionMaxValue);
	this->menuSet->addAction(this->actionThreshold);
	this->buttonSet->setMenu(this->menuSet);
	this->configuration = nullptr;

	animation = new QPropertyAnimation(this, "value");
	animation->setDuration(500.0);

	connect(this->actionMinValue, SIGNAL(triggered()), this, SLOT(onDialogMinValueClicked()));
	connect(this->actionMaxValue, SIGNAL(triggered()), this, SLOT(onDialogMaxValueClicked()));
	connect(this->actionThreshold, SIGNAL(triggered()), this, SLOT(onDialogThresholdClicked()));
}

GaugePanel::~GaugePanel()
{
}

void GaugePanel::paintEvent(QPaintEvent*)
{
	int width = this->width();
	int height = this->height();
	int side = qMin(width, height);

	//绘制准备工作,启用反锯齿,平移坐标轴中心,等比例缩放
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.translate(width / 2, height / 2);
	painter.scale(side / 200.0, side / 200.0);

	//内层渐变
	drawInnerGradient(&painter);

	//外层渐变
	drawOuterGradient(&painter);

	//外层光晕
	drawOuterHalo(&painter);

	//刻度线
	drawScale(&painter);
	drawThresholdTriangle(&painter);

	//刻度值
	drawScaleNum(&painter);

	//绘制指针
	drawPointer(&painter);
	drawPointerSector(&painter);
	drawName(&painter);
	drawValue(&painter);
	drawUnit(&painter);
}

void GaugePanel::drawOuterGradient(QPainter* painter)
{
	if (radiusHalo <= radiusOuter)
		return;

	painter->save();

	QRectF rectangle(0 - radiusHalo, 0 - radiusHalo, radiusHalo * 2, radiusHalo * 2);
	QPen framePen(colorOuterFrame);
	framePen.setWidthF(1.5f);
	painter->setPen(framePen);
	painter->drawEllipse(rectangle);

	painter->setPen(Qt::NoPen);

	QPainterPath smallCircle;
	QPainterPath bigCircle;

	float radius = radiusOuter;
	smallCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);
	radius += (radiusHalo - radiusOuter);
	bigCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);

	//大圆抛去小圆部分
	QPainterPath gradientPath = bigCircle - smallCircle;
	QRadialGradient gradient(0, 0, radius, 0, 0);
	//gradient.setSpread(QGradient::ReflectSpread);

	gradient.setColorAt(0.8, colorOuterStart);
	gradient.setColorAt(1, colorOuterEnd);
	painter->setBrush(gradient);
	painter->drawPath(gradientPath);

	painter->restore();
}

void GaugePanel::drawInnerGradient(QPainter* painter)
{
	if (radiusOuter <= radiusInner)
		return;

	painter->save();
	painter->setPen(Qt::NoPen);

	QPainterPath smallCircle;
	QPainterPath bigCircle;

	float radius = radiusInner;
	smallCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);
	radius += (radiusOuter - radiusInner);
	bigCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);

	//大圆抛去小圆部分
	QPainterPath gradientPath = bigCircle - smallCircle;
	QRadialGradient gradient(0, 0, radius, 0, 0);
	//gradient.setSpread(QGradient::ReflectSpread);

	gradient.setColorAt(0.8125, colorInnerStart);
	gradient.setColorAt(1, colorInnerEnd);
	painter->setBrush(gradient);
	painter->drawPath(gradientPath);

	painter->restore();
}

void GaugePanel::drawOuterHalo(QPainter* painter)
{
	painter->save();
	painter->setPen(Qt::NoPen);

	QPainterPath smallCircle;
	QPainterPath bigCircle;

	float radius = radiusHalo;
	smallCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);
	radius += (100.0 - radiusHalo);
	bigCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);

	//大圆抛去小圆部分
	QPainterPath gradientPath = bigCircle - smallCircle;
	QRadialGradient gradient(0, 0, 100, 0, 0);
	gradient.setSpread(QGradient::ReflectSpread);

	gradient.setColorAt(radiusHalo / 100, colorHaloStart);
	gradient.setColorAt(1, colorHaloEnd);
	painter->setBrush(gradient);
	painter->drawPath(gradientPath);

	painter->restore();
}

void GaugePanel::drawScale(QPainter* painter)
{
	float radius = 92.5;
	painter->save();
	painter->setPen(QColor(255, 255, 255));

	painter->rotate(30);
	int steps = (30);
	double angleStep = (360.0 - 60) / steps;
	QPen pen = painter->pen();
	pen.setCapStyle(Qt::RoundCap);

	for (int i = 0; i <= steps; i++) {
		if (i % 3 == 0) {
			pen.setWidthF(1.5);
			painter->setPen(pen);
			QLineF line(0.0f, radius - 10.0f, 0.0f, radius);
			painter->drawLine(line);
		}
		else {
			pen.setWidthF(0.5);
			painter->setPen(pen);
			QLineF line(0.0f, radius - 4.0f, 0.0f, radius);
			painter->drawLine(line);
		}

		painter->rotate(angleStep);
	}

	painter->restore();
}

void GaugePanel::drawThresholdTriangle(QPainter* painter)
{
	float radius = 92.5;
	QPolygon pts;
	painter->save();
	painter->setPen(QColor(255, 255, 255));

	painter->rotate(-60 + (thresholdValue - minValue) / (maxValue - minValue) * 300);
	QPen pen = painter->pen();
	pen.setCapStyle(Qt::RoundCap);

	pts.append(QPoint(-radius, 0));
	pts.append(QPoint(-radius - 7.5, -4));
	pts.append(QPoint(-radius - 7.5, 4));

	painter->setBrush(QColor(255, 0, 0, 250));
	painter->drawPolygon(pts);

	painter->restore();
}

void GaugePanel::drawScaleNum(QPainter* painter)
{
	float radius = 65.0f;
	painter->save();
	painter->setPen(QColor(0, 0, 0));

	double startRad = (330 - 90) * (M_PI / 180);
	double deltaRad = (300) * (M_PI / 180) / 10;

	for (int i = 0; i <= 10; i++) {
		double sina = sin(startRad - i * deltaRad);
		double cosa = cos(startRad - i * deltaRad);
		double value = minValue + i * (maxValue - minValue) / 10.0;//刻度值范围

		QString strValue = QString("%1").arg((double)value, -1, 'f', 1);
		double textWidth = fontMetrics().width(strValue);
		double textHeight = fontMetrics().height();
		int x = radius * cosa - textWidth / 2;
		int y = -radius * sina + textHeight / 4;
		painter->drawText(x, y, strValue);
	}

	painter->restore();
}

void GaugePanel::drawPointer(QPainter* painter)
{
	painter->save();

	float radius = 83.0;
	double showValue = qMax(qMin(value, maxValue), minValue);
	painter->rotate(30 + (showValue - minValue) / (maxValue - minValue) * 300);
	QPen pen = painter->pen();
	pen.setWidthF(1.0);
	pen.setColor(this->colorPointer);
	painter->setPen(pen);
	QLineF line(0.0f, 0.0f, 0.0f, radius);
	painter->drawLine(line);

	painter->restore();
}

void GaugePanel::drawPointerSector(QPainter* painter)
{
	float radius = 80.0f;
	painter->save();
	painter->setPen(Qt::NoPen);

	double showValue = qMax(qMin(value, maxValue), minValue);
	QRectF rect(-radius, -radius, radius * 2, radius * 2);
	painter->setBrush(this->colorPointerSector);
	painter->drawPie(rect, -120 * 16, -(showValue - minValue) / (maxValue - minValue) * 300 * 16);

	painter->restore();
}

void GaugePanel::drawName(QPainter* painter)
{
	int radius = 100;
	painter->save();
	painter->setPen(QColor(0, 0, 0));
	painter->setFont(QFont("Arial", 10, 20, false));

	QRectF textRect(-radius, -radius - 35, radius * 2, radius * 2);
	painter->drawText(textRect, Qt::AlignCenter, this->name);

	painter->restore();
}

void GaugePanel::drawValue(QPainter* painter)
{
	int radius = 100;
	painter->save();
	painter->setPen(QColor(0, 0, 0));
	painter->setFont(QFont("Arial", 20, 20, true));

	QRectF textRect(-radius, -radius, radius * 2, radius * 2);
	QString strValue = QString("%1").arg((double)value, 0, 'f', 2);
	painter->drawText(textRect, Qt::AlignCenter, strValue);

	painter->restore();
}

void GaugePanel::drawUnit(QPainter* painter)
{
	int radius = 100;
	painter->save();
	painter->setPen(QColor(0, 0, 0));
	painter->setFont(QFont("Arial", 7, -1, true));

	QRectF textRect(-radius, -radius + 20, radius * 2, radius * 2);
	painter->drawText(textRect, Qt::AlignCenter, "LPM");

	painter->restore();
}

void GaugePanel::setColorNormal()
{
	colorOuterFrame = QColor(50, 154, 255, 250);
	colorInnerStart = QColor(50, 154, 255, 0);
	colorInnerEnd = QColor(50, 154, 255, 100);
	colorOuterStart = QColor(50, 154, 255, 150);
	colorOuterEnd = QColor(50, 154, 255, 200);
	colorHaloStart = QColor(100, 180, 255, 80);
	colorHaloEnd = QColor(30, 80, 120, 20);
	colorPointer = QColor(50, 154, 255, 200);
	colorPointerSector = QColor(50, 154, 255, 50);
}

void GaugePanel::setColorWarning()
{
	colorOuterFrame = QColor(255, 154, 50, 250);
	colorInnerStart = QColor(255, 154, 50, 0);
	colorInnerEnd = QColor(255, 154, 50, 100);
	colorOuterStart = QColor(255, 154, 50, 150);
	colorOuterEnd = QColor(255, 154, 50, 200);
	colorHaloStart = QColor(255, 180, 100, 80);
	colorHaloEnd = QColor(120, 80, 30, 20);
	colorPointer = QColor(50, 154, 255, 200);
	colorPointerSector = QColor(50, 154, 255, 50);
}

void GaugePanel::setColorError()
{
	colorOuterFrame = QColor(50, 154, 255, 250);
	colorInnerStart = QColor(50, 154, 255, 0);
	colorInnerEnd = QColor(50, 154, 255, 100);
	colorOuterStart = QColor(50, 154, 255, 150);
	colorOuterEnd = QColor(50, 154, 255, 200);
	colorHaloStart = QColor(100, 180, 255, 80);
	colorHaloEnd = QColor(30, 80, 120, 20);
	colorPointer = QColor(255, 0, 0, 200);
	colorPointerSector = QColor(255, 0, 0, 50);
}

void GaugePanel::setColorDisabled()
{
	colorOuterFrame = QColor(0, 0, 0, 120);
	colorInnerStart = QColor(0, 0, 0, 0);
	colorInnerEnd = QColor(0, 0, 0, 50);
	colorOuterStart = QColor(0, 0, 0, 80);
	colorOuterEnd = QColor(0, 0, 0, 120);
	colorHaloStart = QColor(0, 0, 0, 40);
	colorHaloEnd = QColor(0, 0, 0, 20);
	colorPointer = QColor(0, 0, 0, 200);
	colorPointerSector = QColor(0, 0, 0, 50);
}

void GaugePanel::onDialogMinValueClicked()
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

void GaugePanel::onDialogMaxValueClicked()
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

void GaugePanel::onDialogThresholdClicked()
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