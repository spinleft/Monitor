#include "stdafx.h"
#include "ToggleSwitch.h"

ToggleSwitch::ToggleSwitch(QWidget* parent)
	: QWidget(parent)
	, _isOn(false)
	, _sliderPosition(0)
{
	_animation = new QPropertyAnimation(this, "sliderPosition");
	_animation->setDuration(100); // 100ms µÄ¶¯»­Ê±¼ä
}

void ToggleSwitch::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	// ±³¾°
	drawBg(&painter);

	// »¬¿é
	drawSlider(&painter);
}

void ToggleSwitch::drawBg(QPainter* painter)
{
	painter->save();
	painter->setPen(Qt::NoPen);

	QColor bgColor = _isOn ? QColor("#32CD32") : QColor("#808080");
	painter->setBrush(bgColor);
	painter->drawRoundedRect(rect(), 10, 10);

	int sliderWidth = width() / 2 - 5;

	if (_isOn) {
		QRect textRect(0, 0, width() - sliderWidth, height());
		painter->setPen(QColor("#FFFFFF"));
		painter->drawText(textRect, Qt::AlignCenter, _onText);
	}
	else {
		QRect textRect(sliderWidth, 0, width() - sliderWidth, height());
		painter->setPen(QColor("#FFFFFF"));
		painter->drawText(textRect, Qt::AlignCenter, _offText);
	}

	painter->restore();
}

void ToggleSwitch::drawSlider(QPainter* painter)
{
	painter->save();
	painter->setPen(Qt::NoPen);

	painter->setBrush(QColor("#FFFFFF"));
	int sliderWidth = width() / 2 - 5 * 2;
	int sliderHeight = height() - 5 * 2;
	QRect sliderRect(_sliderPosition + 5, 5, sliderWidth, sliderHeight);
	painter->drawRoundedRect(sliderRect, 10, 10);

	painter->restore();
}

void ToggleSwitch::mousePressEvent(QMouseEvent* event)
{
	_isOn = !_isOn;
	_animation->setStartValue(_isOn ? 0 : width() / 2);
	_animation->setEndValue(_isOn ? width() / 2 : 0);
	_animation->start();
	emit toggled(_isOn);
}

void ToggleSwitch::toggleEvent()
{
	_isOn = !_isOn;
	_animation->setStartValue(_isOn ? 0 : width() / 2);
	_animation->setEndValue(_isOn ? width() / 2 : 0);
	_animation->start();
}