#pragma once
#include "pugixml.hpp"
#include <math.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace pugi;


typedef unsigned int        UINT;
typedef unsigned long       DWORD;
typedef unsigned char		UCHAR;
typedef unsigned char       BYTE;

/*
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
*/
#ifndef itoa
#define itoa(value,string,radix)  sprintf(string, "%d", value)
#endif

#ifndef ltoa
#define ltoa(value,string,radix)  sprintf(string, "%u", value)
#endif


// --- a partial descritpion of a view
typedef struct{
	string sName;		
	string sClass;		
	string sTemplateName;
	string sOrigin;
	string sSize;
	bool bTransparemt;
	bool bTemplate;
	pugi::xml_node viewNode;
} VIEW_DESC;

/* ------------------------------------------------------
     CWPXMLParser
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

class CWPXMLParser
{
public:
	CWPXMLParser(void);
	~CWPXMLParser(void);

	// --- THE DOC
	xml_document m_Doc;

	inline bool isFileLoaded(){return bFileLoaded;}
	bool bFileLoaded;

	bool loadXMLResource(const void* pXMLResource, DWORD size);
	bool loadXMLFile(const char* pFilePath);
	bool saveXMLFile(const char* pFilePath);

	bool hasBitmap(const char_t* bitmapName);
	bool isBitmapTiled(const char_t* bitmapName);
	const char* getRAFXBitmapType(const char_t* bitmapName);
	bool setRAFXBitmapType(const char* bitmapName, const char* bitmapType);

	bool hasColor(const char_t* colorName);
	bool hasRafxTemplate(const char_t* templateName);
	bool addColor(const char_t* colorName, const char_t* rgba);

   	const char_t* getGradientStartRGBA(const char_t* name);
    const char_t* getGradientStopRGBA(const char_t* name);
 
	xml_node hasFont(const char_t* fontName);
	xml_node addFont(const char_t* fontName, const char_t* weight, const char_t* bold, const char_t* italic);

	xml_node getVSTGUIDescNode();

	// --- get a template's subview node
	xml_node getTemplateSubViewNode(xml_node templateNode, const char_t* viewClass, const char_t* customViewName = NULL, const char_t* subControllerName = NULL);
	
	// --- get a view's subview node
	xml_node getViewSubViewNode(pugi::xml_node node, int nIndex);

	// --- get a view's subview-controller node (for layered vcs0)
	xml_node getViewSubViewControllerNode(pugi::xml_node node, int nStartIndex = 0);

	// --- only partly used; gets attribs about a template;s subview
	VIEW_DESC* getTemplateSubViewDesc(const char_t* templateName, int nIndex);

	// --- get template node and subview count
	xml_node getTemplateInfo(const char_t* name, int& nViewCount);
    xml_node getTemplateNode(const char_t* name);

	// --- get subview count of a node
	int getSubViewCount(pugi::xml_node node);

    const char_t* getBitmapNameWithFileName(const char_t* filename);

	const char_t* getBitmapName(int nIndex);
	const char_t* getNextBitmapNameWithType(int& nStartIndex, const char_t* rafxType);
	bool addBitmap(const char_t* bitmapName, const char_t* bitmapFileName);
	bool setBitmapTiled(const char_t* bitmapName, bool bTiled, const char_t* tiledOffsetString = "0,0,0,0");

	const char_t* getNextFontName(int nIndex);
	const char_t* getNextColorName(int nIndex);
	const char_t* getNextControlTagName(int nIndex);

	const char_t* getTemplateAttribute(const char_t* templateName, const char_t* attributeName);
	const char_t* getBitmapAttribute(const char_t* bitmapName, const char_t* attributeName);
	const char_t* getColorAttribute(const char_t* colorName, const char_t* attributeName);
	const char_t* getFontAttribute(const char_t* fontName, const char_t* attributeName);
	const char_t* getControlTagAttribute(const char_t* tagName, const char_t* attributeName);
	const char_t* getCustomAttribute(const char_t* attributeName, xml_node& node);
	bool setCustomAttribute(const char_t* attributeName, const char_t* attValue);

	xml_node getCustomNode();

	int getControlTypeCount(const char_t* className);
	int getControlTypeCount(const char_t* className, const pugi::xml_node node);
	int getTagCount();

	// --- index is zero-based to match our pControlMap
	int getTagIndex(const char_t* tagName);
	int getTagIndex(int nTagValue);
	//const char_t* getTagValue(const char_t* tagName);

	// --- for synchronizing tags
	void clearBaseTags();
	bool hasTag(const char_t* tagName);

//	bool addTag(string name, string controlName, string tag);
	bool addTag(string name, string tag);

	// --- supported objects here:
	const char_t* getAttribute(pugi::xml_node node, const char_t* attributeName);
    int getAttributeCount(pugi::xml_node node);
	bool getAttributeValueNamePair(pugi::xml_node node, int index, char_t* attributeName, char_t* attributeValue);
	const char_t* getAttributeValue(pugi::xml_node node, int index);
	const char_t* getAttributeName(pugi::xml_node node, int index);

	bool setAttribute(pugi::xml_node node, const char_t* attributeName, const char_t* attributeValue);

	inline xml_node addSubNode(pugi::xml_node node, const char_t* nodeName)
	{
		pugi::xml_node child = node.append_child(nodeName);
		return child;
	}

	inline bool removeSubNode(pugi::xml_node node, pugi::xml_node subNode)
	{
		return node.remove_child(subNode);
	}

	//inline COLORREF transformHTML(string color)
	//{
	//	string s1(color, 1,2);
	//	string s2(color, 3,2);
	//	string s3(color, 5,2);

	//	long r = strtol(s1.c_str(), NULL, 16);
	//	long g = strtol(s2.c_str(), NULL, 16);
	//	long b = strtol(s3.c_str(), NULL, 16);

	//	return RGB (r,g,b);
	//}

};
