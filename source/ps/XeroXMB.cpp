// $Id: XeroXMB.cpp,v 1.3 2004/07/10 20:55:55 philip Exp $

#include "precompiled.h"

#include "Xeromyces.h"

#include "ps/utf16string.h"

#include <assert.h>

const int HeaderMagic = 0x30424D58; // = "XMB0" (little-endian)
const char* HeaderMagicStr = "XMB0";

typedef std::utf16string::value_type char16;

// Warning: May contain traces of pointer abuse

void XMBFile::Initialise(char* FileData)
{
	m_Pointer = FileData;
	int Header = *(int*)m_Pointer; m_Pointer += 4;
	assert(Header == HeaderMagic && "Invalid XMB header!");

	int Checksum = *(int*)m_Pointer; m_Pointer += 4;

	int i;

#ifdef XERO_USEMAP
	// Build a std::map of all the names->ids
	int ElementNameCount = *(int*)m_Pointer; m_Pointer += 4;
	for (i = 0; i < ElementNameCount; ++i)
		m_ElementNames[ReadZStrA()] = i;

	int AttributeNameCount = *(int*)m_Pointer; m_Pointer += 4;
	for (i = 0; i < AttributeNameCount; ++i)
		m_AttributeNames[ReadZStrA()] = i;
#else
	// Ignore all the names for now, and skip over them
	// (remembering the position of the first)
	m_ElementNameCount = *(int*)m_Pointer; m_Pointer += 4;
	m_ElementPointer = m_Pointer;
	for (i = 0; i < m_ElementNameCount; ++i)
		m_Pointer += 4 + *(int*)m_Pointer; // skip over the string

	m_AttributeNameCount = *(int*)m_Pointer; m_Pointer += 4;
	m_AttributePointer = m_Pointer;
	for (i = 0; i < m_AttributeNameCount; ++i)
		m_Pointer += 4 + *(int*)m_Pointer; // skip over the string
#endif

}

std::string XMBFile::ReadZStrA()
{
	int Length = *(int*)m_Pointer;
	m_Pointer += 4;
	std::string String (m_Pointer); // reads up until the first NULL
	m_Pointer += Length;
	return String;
}

XMBElement XMBFile::getRoot()
{
	return XMBElement(m_Pointer);
}


#ifdef XERO_USEMAP

int XMBFile::getElementID(const char* Name)
{
	return m_ElementNames[Name];
}

int XMBFile::getAttributeID(const char* Name)
{
	return m_AttributeNames[Name];
}

#else // #ifdef XERO_USEMAP

int XMBFile::getElementID(const char* Name)
{
	char* Pos = m_ElementPointer;

	int len = (int)strlen(Name)+1; // count bytes, including null terminator

	// Loop through each string to find a match
	for (int i = 0; i < m_ElementNameCount; ++i)
	{
		// See if this could be the right string, checking its
		// length and then its contents
		if (*(int*)Pos == len && memcmp(Pos+4, Name, len) == 0)
			return i;
		// If not, jump to the next string
		Pos += 4 + *(int*)Pos;
	}
	// Failed
	return -1;
}

int XMBFile::getAttributeID(const char* Name)
{
	char* Pos = m_AttributePointer;

	int len = (int)strlen(Name)+1; // count bytes, including null terminator

	// Loop through each string to find a match
	for (int i = 0; i < m_AttributeNameCount; ++i)
	{
		// See if this could be the right string, checking its
		// length and then its contents
		if (*(int*)Pos == len && memcmp(Pos+4, Name, len) == 0)
			return i;
		// If not, jump to the next string
		Pos += 4 + *(int*)Pos;
	}
	// Failed
	return -1;
}
#endif // #ifdef XERO_USEMAP / #else


// Relatively inefficient, so only use when
// laziness overcomes the need for speed
std::string XMBFile::getElementString(const int ID)
{
	char* Pos = m_ElementPointer;
	for (int i = 0; i < ID; ++i)
		Pos += 4 + *(int*)Pos;
	return std::string(Pos+4);
}

std::string XMBFile::getAttributeString(const int ID)
{
	char* Pos = m_AttributePointer;
	for (int i = 0; i < ID; ++i)
		Pos += 4 + *(int*)Pos;
	return std::string(Pos+4);
}



int XMBElement::getNodeName()
{
	return *(int*)(m_Pointer + 4); // == ElementName
}

XMBElementList XMBElement::getChildNodes()
{
	return XMBElementList(
		m_Pointer + 20 + *(int*)(m_Pointer + 16), // == Children[]
		*(int*)(m_Pointer + 12) // == ChildCount
	);
}

XMBAttributeList XMBElement::getAttributes()
{
	return XMBAttributeList(
		m_Pointer + 24 + *(int*)(m_Pointer + 20), // == Attributes[]
		*(int*)(m_Pointer + 8) // == AttributeCount
	);
}

std::utf16string XMBElement::getText()
{
	return std::utf16string((char16*)(m_Pointer + 24));
}


XMBElement XMBElementList::item(const int id)
{
	assert(id >= 0 && id < Count && "Element ID out of range");
	char* Pos;

	// If access is sequential, don't bother scanning
	// through all the nodes to find the next one
	if (id == m_LastItemID+1)
	{
		Pos = m_LastPointer;
		Pos += *(int*)Pos; // skip over the last node
	}
	else
	{
		Pos = m_Pointer;
		// Skip over each preceding node
		for (int i=0; i<id; ++i)
			Pos += *(int*)Pos;
	}
	// Cache information about this node
	m_LastItemID = id;
	m_LastPointer = Pos;

	return XMBElement(Pos);
}

std::utf16string XMBAttributeList::getNamedItem(const int AttributeName)
{
	char* Pos = m_Pointer;

	// Maybe not the cleverest algorithm, but it should be
	// fast enough with half a dozen attributes:
	for (int i = 0; i < Count; ++i)
	{
		if (*(int*)Pos == AttributeName)
			return std::utf16string((char16*)(Pos+8));
		Pos += 8 + *(int*)(Pos+4); // Skip over the string
	}

	// Can't find attribute
	return std::utf16string();
}

XMBAttribute XMBAttributeList::item(const int id)
{
	assert(id >= 0 && id < Count && "Attribute ID out of range");
	char* Pos;

	// If access is sequential, don't bother scanning through
	// all the nodes to find the right one
	if (id == m_LastItemID+1)
	{
		Pos = m_LastPointer;
		// Skip over the last attribute
		Pos += 8 + *(int*)(Pos+4);
	}
	else
	{
		Pos = m_Pointer;
		// Skip over each preceding attribute
		for (int i=0; i<id; ++i)
			Pos += 8 + *(int*)(Pos+4); // skip ID, length, and string data
	}
	// Cache information about this attribute
	m_LastItemID = id;
	m_LastPointer = Pos;

	return XMBAttribute(*(int*)Pos, std::utf16string( (char16*)(Pos+8) ));
}
