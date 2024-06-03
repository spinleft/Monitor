#include "stdafx.h"
#include "Monitor.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	Monitor w;
	w.show();
	return a.exec();
}