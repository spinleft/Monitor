#include "stdafx.h"
#include "MyVi.h"
#include <QMessageBox>

MyVi::MyVi(QWidget* parent)
	: QObject(parent)
	, m_isConnected(false)
{
	this->parent = parent;
	writeBuf = new ViByte[512];
	readBuf = new ViByte[512];
	connect(this, SIGNAL(isConnectedChanged(bool)), this, SLOT(initInstr(bool)));
}

MyVi::~MyVi()
{
	viClose(defaultRM);
	delete writeBuf;
	delete readBuf;
}

bool MyVi::isConnected() {
	return m_isConnected;
};

void MyVi::setIsConnected(bool isConnected) {
	m_isConnected = isConnected;
	emit isConnectedChanged(isConnected);
}

void MyVi::openDefaultRM()
{
	if (viOpenDefaultRM(&defaultRM) < VI_SUCCESS)
	{
		QMessageBox::critical(parent, tr("Error"), tr("Could not open a session to the VISA Resource Manager!"), QMessageBox::Ok);
		exit(EXIT_FAILURE);
	}
}

void MyVi::setResourceType(ResourceType rsrcType)
{
	this->resourceType = rsrcType;
}

void MyVi::closeFindList()
{
	viClose(findList);
}

void MyVi::findRsrc(ViChar _VI_FAR desc[])
{
	ViStatus status = VI_ERROR_RSRC_NFOUND;
	if (resourceType == USB) {
		status = viFindRsrc(defaultRM, "USB?*INSTR", &findList, &numInstr, desc);
	}
	else if (resourceType == ASRL) {
		status = viFindRsrc(defaultRM, "ASRL?*INSTR", &findList, &numInstr, desc);
	}
	else {
		QMessageBox::critical(parent, tr("Error"), tr("Not supported type of resources.\n"), QMessageBox::Ok);
	}

	if (status == VI_ERROR_RSRC_NFOUND) {
		desc[0] = '\0';
	}
	else if (status < VI_SUCCESS) {
		QMessageBox::critical(parent, tr("Error"), tr("An error occurred while finding resources.\n"), QMessageBox::Ok);
		desc[0] = '\0';
	}
}

void MyVi::findNext(ViChar* desc)
{
	ViStatus status = viFindNext(findList, desc);

	if (status == VI_ERROR_RSRC_NFOUND) {
		desc[0] = '\0';
	}
	else if (status < VI_SUCCESS) {
		QMessageBox::critical(parent, tr("Error"), tr("An error occurred while finding next resources.\n"), QMessageBox::Ok);
		desc[0] = '\0';
	}
}

ViStatus MyVi::open(ViConstRsrc name, ViAccessMode mode, ViUInt32 timeout)
{
	ViStatus status = viOpen(defaultRM, name, mode, timeout, &currentInstrSession);

	if (status < VI_SUCCESS) {
		QMessageBox::critical(parent, tr("Error"), tr("An error occurred while open this resources.\n"), QMessageBox::Ok);
	}
	else {
		setIsConnected(true);
	}

	return status;
}

void MyVi::closeInstr()
{
	viClose(currentInstrSession);
	setIsConnected(false);
}

ViUInt32 MyVi::write(const char* buf)
{
	strcpy(reinterpret_cast<char*>(writeBuf), buf);
	ViStatus status = viWrite(currentInstrSession, writeBuf, (ViUInt32)strlen(reinterpret_cast<const char*>(writeBuf)), &writeCount);
	if (status < VI_SUCCESS)
	{
		QMessageBox::critical(parent, tr("Error"), tr("Error writing to the device."), QMessageBox::Ok);
		return 0;
	}
	return writeCount;
}

ViUInt32 MyVi::read(ViPBuf buf, ViUInt32 cnt)
{
	ViStatus status = viRead(currentInstrSession, buf, cnt, &retCount);
	if (status < VI_SUCCESS)
	{
		QMessageBox::critical(parent, tr("Error"), tr("Error reading a response from the device."), QMessageBox::Ok);
		return 0;
	}
	else
	{
		buf[retCount] = '\0';
		return retCount;
	}
}

void MyVi::setAttribute(ViAttr attrName, ViAttrState attrValue)
{
	ViStatus status = viSetAttribute(currentInstrSession, attrName, attrValue);
	if (status < VI_SUCCESS)
	{
		QMessageBox::critical(parent, tr("Error"), tr("Error setting attribute to the device."), QMessageBox::Ok);
		return;
	}
}

void MyVi::getAttribute(ViAttr attrName, ViPBuf buf)
{
	ViStatus status = viGetAttribute(currentInstrSession, attrName, buf);
	if (status < VI_SUCCESS)
	{
		QMessageBox::critical(parent, tr("Error"), tr("Error getting attribute from the device."), QMessageBox::Ok);
		return;
	}
}

void MyVi::initInstr(bool isConnected)
{
	if (isConnected) {
		if (resourceType == USB) {
			write("*IDN?\n");
			read(readBuf, 100);
			QMessageBox::information(parent, tr("Information"), QString::fromLocal8Bit(reinterpret_cast<const char*>(readBuf)), QMessageBox::Ok);
			write("*RST\n");
			write("VOLT:RANG 10, (@120:126,117,118,149,150,110:116,142:148,102:109)\n");
			write("VOLT:STYP DIFF, (@120:131,102:109)\n");
			write("VOLT:STYP NRS, (@117,118,149,150,110:116,142:148)\n");
			write("ACQ:SRAT 100000\n");
		}
		else if (resourceType == ASRL) {
			getAttribute(VI_ATTR_INTF_INST_NAME, readBuf);
			QMessageBox::information(parent, tr("Information"), QString::fromLocal8Bit(reinterpret_cast<const char*>(readBuf)), QMessageBox::Ok);
			setAttribute(VI_ATTR_TMO_VALUE, 10000);
			setAttribute(VI_ATTR_ASRL_BAUD, 9600);
			setAttribute(VI_ATTR_ASRL_DATA_BITS, 8);
			setAttribute(VI_ATTR_ASRL_STOP_BITS, VI_ASRL_STOP_ONE);
			setAttribute(VI_ATTR_ASRL_PARITY, VI_ASRL_PAR_NONE);
			setAttribute(VI_ATTR_TERMCHAR_EN, VI_TRUE);
			setAttribute(VI_ATTR_TERMCHAR, 0xA);
			setAttribute(VI_ATTR_ASRL_FLOW_CNTRL, VI_ASRL_FLOW_NONE);
			setAttribute(VI_ATTR_ASRL_END_IN, VI_ASRL_END_TERMCHAR);
		}
	}
}