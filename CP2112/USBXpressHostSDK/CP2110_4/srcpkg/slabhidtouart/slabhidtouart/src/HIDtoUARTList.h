#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "HIDDevice.h"
#include "ReportQueue.h"
#include "CriticalSectionLock.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CHIDtoUART Class
/////////////////////////////////////////////////////////////////////////////

class CHIDtoUART
{
public:
	CHIDDevice		hid;
	CReportQueue	queue;
	DWORD			readTimeout;
	DWORD			writeTimeout;
	BYTE			partNumber;

	CHIDtoUART()
	{
		readTimeout		= 1000;
		writeTimeout	= 1000;
		partNumber		= 0x00;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CHIDtoUARTList Class
/////////////////////////////////////////////////////////////////////////////

class CHIDtoUARTList
{
// Constructor/Destructor
public:
	CHIDtoUARTList();
	~CHIDtoUARTList();

// Public Methods
public:
	BOOL Validate(CHIDtoUART* object);
	CHIDtoUART* Construct();
	void Destruct(CHIDtoUART* object);

// Protected Members
protected:
	std::vector<CHIDtoUART*>	m_list;
	CCriticalSectionLock		m_lock;
};
