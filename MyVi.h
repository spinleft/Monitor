#pragma once

#include "visa.h"
#include <QWidget>

struct ViInfo {
	ViSession defaultRM;
	ViSession instr;
	bool connected;
};

class MyVi : public QObject
{
	Q_OBJECT
		Q_PROPERTY(bool isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)

public:
	MyVi(QWidget* parent = nullptr);
	~MyVi();

	enum ResourceType {
		USB = 0,
		ASRL = 1
	};

	bool isConnected();
	void setIsConnected(bool);

	void openDefaultRM();
	void setResourceType(ResourceType rsrcType = USB);
	void closeFindList();
	void findRsrc(ViChar _VI_FAR desc[]);
	void findNext(ViChar* desc);
	ViStatus open(ViConstRsrc name, ViAccessMode mode, ViUInt32 timeout);
	void closeInstr();
	ViUInt32 write(const char* buf);
	ViUInt32 read(ViPBuf buf, ViUInt32 cnt);
	void setAttribute(ViAttr attrName, ViAttrState attrValue);
	void getAttribute(ViAttr attrName, ViPBuf buf);

private:
	QWidget* parent;
	bool m_isConnected;
	ViSession defaultRM;
	ViFindList findList;
	ViUInt32 numInstr;
	ViChar instrResourceString[VI_FIND_BUFLEN];
	ResourceType resourceType;
	ViSession currentInstrSession;
	ViBuf writeBuf;
	ViUInt32 writeCount;
	ViPBuf readBuf;
	ViUInt32 retCount;

private slots:
	void initInstr(bool);

Q_SIGNALS:
	void isConnectedChanged(bool);
};