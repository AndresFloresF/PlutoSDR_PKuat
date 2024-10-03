/////////////////////////////////////////////////////////////////////////////
// HIDtoUARTList.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "HIDtoUARTList.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CHIDtoUARTList Class - Constructor/Destructor
/////////////////////////////////////////////////////////////////////////////

CHIDtoUARTList::CHIDtoUARTList()
{

}

CHIDtoUARTList::~CHIDtoUARTList()
{
	// Enter critical section
	m_lock.Lock();

	// Deallocate all device objects
	// (Destructor closes the devices)
	for (DWORD i = 0; i < m_list.size(); i++)
	{
		delete m_list[i];
	}

	// Remove all device references
	m_list.clear();

	// Leave critical section
	m_lock.Unlock();
}

/////////////////////////////////////////////////////////////////////////////
// CHIDtoUARTList Class - Public Methods
/////////////////////////////////////////////////////////////////////////////

// Make sure that a CHIDtoUART pointer is valid
BOOL CHIDtoUARTList::Validate(CHIDtoUART* object)
{
	BOOL retVal = FALSE;

	// Enter critical section
	m_lock.Lock();

	if (find(m_list.begin(), m_list.end(), object) != m_list.end())
	{
		retVal = TRUE;
	}

	// Unlock critical section
	m_lock.Unlock();

	return retVal;
}

// Create a new CHIDtoUART object on the heap
// and call the CHIDtoUART constructor
// and track memory usage in m_list
CHIDtoUART* CHIDtoUARTList::Construct()
{
	// Create the object memory on the heap
	// Call the CHIDtoUART constructor
	CHIDtoUART* object = new CHIDtoUART();

	// Enter critical section
	m_lock.Lock();

	m_list.push_back(object);

	// Leave critical section
	m_lock.Unlock();

	return object;
}

// Remove the object pointer from the m_list
// vector and call the CHIDtoUART destructor by
// deallocating the object
void CHIDtoUARTList::Destruct(CHIDtoUART* object)
{
	// Enter critical section
	m_lock.Lock();

	if (Validate(object))
	{
		CHIDtoUART* hidUart = (CHIDtoUART*)object;

		// Find the object pointer in the vector and return an iterator
		std::vector<CHIDtoUART*>::iterator iter = find(m_list.begin(), m_list.end(), object);

		// Remove the pointer from the vector if it exists
		if (iter != m_list.end())
		{
			m_list.erase(iter);
		}

		// Call the CHIDtoUART destructor
		// Free the object memory on the heap
		delete hidUart;
	}

	// Leave critical section
	m_lock.Unlock();
}
