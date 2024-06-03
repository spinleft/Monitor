#pragma once

#include <QWidget>
#include "ui_Connect.h"
#include "MyVi.h"
#include "visa.h"

class Connect : public QWidget
{
	Q_OBJECT

public:
	Connect(MyVi* vi_ptr, QWidget* parent = nullptr);
	~Connect();

private:
	Ui::LinkClass* ui;

	MyVi* vi;
	char instrResourceString[VI_FIND_BUFLEN];

	void updateInstruments();

private slots:
	void on_connect_clicked();
	void on_disconnect_clicked();
};
