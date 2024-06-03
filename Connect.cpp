#include "stdafx.h"
#include "Connect.h"
#include <QPushButton>

Connect::Connect(MyVi* vi, QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::LinkClass())
{
	ui->setupUi(this);
	ui->buttonDisconnect->setEnabled(false);
	this->vi = vi;
	updateInstruments();

	connect(ui->buttonConnect, SIGNAL(clicked()), this, SLOT(on_connect_clicked()));
	connect(ui->buttonDisconnect, SIGNAL(clicked()), this, SLOT(on_disconnect_clicked()));
}

Connect::~Connect()
{}

void Connect::updateInstruments()
{
	ui->comboBoxInstrument->clear();
	vi->findRsrc(instrResourceString);

	while (instrResourceString[0] != '\0') {
		ui->comboBoxInstrument->addItem(QString(instrResourceString));
		vi->findNext(instrResourceString);
	}
}

void Connect::on_connect_clicked()
{
	ViStatus status = vi->open(ui->comboBoxInstrument->currentText().toLatin1().data(), VI_NULL, VI_NULL);

	if (status == VI_SUCCESS)
	{
		ui->comboBoxInstrument->setEnabled(false);
		ui->buttonConnect->setEnabled(false);
		ui->buttonDisconnect->setEnabled(true);
	}
}

void Connect::on_disconnect_clicked()
{
	vi->closeInstr();
	vi->closeFindList();
	ui->buttonDisconnect->setEnabled(false);
	ui->buttonConnect->setEnabled(true);
	ui->comboBoxInstrument->setEnabled(true);
	updateInstruments();
}