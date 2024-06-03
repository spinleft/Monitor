#include "stdafx.h"
#include "SwitchButton.h"

SwitchButton::SwitchButton(QWidget* parent)
    : QWidget(parent)
{
    int space = 10;
    int rectRadius = 10;
    bool checked = false;
    bool showText = true;
    bool showCircle = false;
    bool animation = true;

    ButtonStyle buttonStyle = ButtonStyle_Rect;

    QColor bgColorOff = QColor(169, 169, 169, 255);
    QColor bgColorOn = QColor(124,252,0,255);
    QColor sliderColorOff = QColor(255, 255, 240, 255);
    QColor sliderColorOn = QColor(255, 255, 240, 255);
    QColor textColorOff = QColor(255, 255, 240, 255);
    QColor textColorOn = QColor(255, 255, 240, 255);

    QString textOff = QString::fromUtf8("停止");
    QString textOn = QString::fromUtf8("运行");

    int step = 1;                       //每次移动的步长
    int startX = 0;                     //滑块开始X轴坐标
    int endX = 50;                       //滑块结束X轴坐标
    QTimer* timer = new QTimer(this);    //定时器绘制

    connect(this, SIGNAL(mousePressEvent()), this, SLOT(change()));
}

SwitchButton::~SwitchButton() {
}

void SwitchButton::paintEvent(QPaintEvent*)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //绘制背景
    drawBg(&painter);
    //绘制滑块
    drawSlider(&painter);
}

void SwitchButton::drawBg(QPainter* painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor bgColor = checked ? bgColorOn : bgColorOff;
    if (!isEnabled()) {
        bgColor.setAlpha(60);
    }

    painter->setBrush(bgColor);

    if (buttonStyle == ButtonStyle_Rect) {
        painter->drawRoundedRect(rect(), rectRadius, rectRadius);
    }
    else if (buttonStyle == ButtonStyle_CircleIn) {
        QRect rect(0, 0, width(), height());
        //半径为高度的一半
        int side = qMin(rect.width(), rect.height());

        //左侧圆
        QPainterPath path1;
        path1.addEllipse(rect.x(), rect.y(), side, side);
        //右侧圆
        QPainterPath path2;
        path2.addEllipse(rect.width() - side, rect.y(), side, side);
        //中间矩形
        QPainterPath path3;
        path3.addRect(rect.x() + side / 2, rect.y(), rect.width() - side, rect.height());

        QPainterPath path;
        path = path3 + path1 + path2;
        painter->drawPath(path);
    }
    else if (buttonStyle == ButtonStyle_CircleOut) {
        QRect rect(height() / 2, space, width() - height(), height() - space * 2);
        painter->drawRoundedRect(rect, rectRadius, rectRadius);
    }

    if (buttonStyle == ButtonStyle_Rect || buttonStyle == ButtonStyle_CircleIn) {
        //绘制文本和小圆,互斥
        if (showText) {
            int sliderWidth = qMin(width(), height()) - space * 2;
            if (buttonStyle == ButtonStyle_Rect) {
                sliderWidth = width() / 2 - 5;
            }
            else if (buttonStyle == ButtonStyle_CircleIn) {
                sliderWidth -= 5;
            }

            if (checked) {
                QRect textRect(0, 0, width() - sliderWidth, height());
                painter->setPen(textColorOn);
                painter->drawText(textRect, Qt::AlignCenter, textOn);
            }
            else {
                QRect textRect(sliderWidth, 0, width() - sliderWidth, height());
                painter->setPen(textColorOff);
                painter->drawText(textRect, Qt::AlignCenter, textOff);
            }
        }
        else if (showCircle) {
            int side = qMin(width(), height()) / 2;
            int y = (height() - side) / 2;

            if (checked) {
                QRect circleRect(side / 2, y, side, side);
                QPen pen(textColorOn, 2);
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(circleRect);
            }
            else {
                QRect circleRect(width() - (side * 1.5), y, side, side);
                QPen pen(textColorOff, 2);
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(circleRect);
            }
        }
    }

    painter->restore();
}

void SwitchButton::drawSlider(QPainter* painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    if (!checked) {
        painter->setBrush(sliderColorOff);
    }
    else {
        painter->setBrush(sliderColorOn);
    }

    if (buttonStyle == ButtonStyle_Rect) {
        int sliderWidth = width() / 2 - space * 2;
        int sliderHeight = height() - space * 2;
        QRect sliderRect(startX + space, space, sliderWidth, sliderHeight);
        painter->drawRoundedRect(sliderRect, rectRadius, rectRadius);
    }
    else if (buttonStyle == ButtonStyle_CircleIn) {
        QRect rect(0, 0, width(), height());
        int sliderWidth = qMin(rect.width(), rect.height()) - space * 2;
        QRect sliderRect(startX + space, space, sliderWidth, sliderWidth);
        painter->drawEllipse(sliderRect);
    }
    else if (buttonStyle == ButtonStyle_CircleOut) {
        int sliderWidth = this->height();
        QRect sliderRect(startX, 0, sliderWidth, sliderWidth);

        QColor color1 = (checked ? Qt::white : bgColorOff);
        QColor color2 = (checked ? sliderColorOn : sliderColorOff);

        QRadialGradient radialGradient(sliderRect.center(), sliderWidth / 2);
        radialGradient.setColorAt(0, checked ? color1 : color2);
        radialGradient.setColorAt(0.5, checked ? color1 : color2);
        radialGradient.setColorAt(0.6, checked ? color2 : color1);
        radialGradient.setColorAt(1.0, checked ? color2 : color1);
        painter->setBrush(radialGradient);

        painter->drawEllipse(sliderRect);
    }

    painter->restore();
}

bool SwitchButton::getChecked() const {
    return checked;
}

void SwitchButton::setChecked(bool checked) {
    this->checked = checked;
}

void SwitchButton::change() {
    checked = !checked;
    updateValue();
}

void SwitchButton::updateValue()
{
    this->update();
    emit checkedChanged(checked);
}