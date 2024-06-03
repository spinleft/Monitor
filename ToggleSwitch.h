#pragma once
#include <QWidget>
#include <QPropertyAnimation>

class ToggleSwitch :
    public QWidget
{
    Q_OBJECT
        Q_PROPERTY(int sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderPositionChanged)

public:
    ToggleSwitch(QWidget* parent = nullptr);

    bool isOn() const {
        return _isOn;
    };

    int sliderPosition() const {
        return _sliderPosition;
    };

    void setText(QString onText, QString offText)
    {
        _onText = onText;
        _offText = offText;
    };

    void setSliderPosition(int pos) {
        _sliderPosition = pos;
        update();
    };

    void toggleEvent();

protected:
    void paintEvent(QPaintEvent* event) override;
    void drawBg(QPainter* painter);
    void drawSlider(QPainter* painter);
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void toggled(bool isOn);
    void sliderPositionChanged(int);

private:
    bool _isOn;
    int _sliderPosition;
    QString _onText;
    QString _offText;
    QPropertyAnimation* _animation;
};

