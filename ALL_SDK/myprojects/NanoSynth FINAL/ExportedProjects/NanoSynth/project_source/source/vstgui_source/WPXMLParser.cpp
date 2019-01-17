#include "WPXMLParser.h"

/* ------------------------------------------------------
     CWPXMLParser
     Custom VSTGUI Object by Will Pirkle
     Created with RackAFX(TM) Plugin Development Software
     www.willpirkle.com
 -------------------------------------------------------*/

CWPXMLParser::CWPXMLParser(void)
{
}

CWPXMLParser::~CWPXMLParser(void)
{
}
	
bool CWPXMLParser::saveXMLFile(const char* pFilePath)
{	
	bool result = m_Doc.save_file(pFilePath);
	return result;
}
	
bool CWPXMLParser::loadXMLResource(const void* pXMLResource, DWORD size)
{
	pugi::xml_parse_result result = m_Doc.load_buffer(pXMLResource, size);
	if(result.status == 0)
	{
		return true;
	}

	return false;

}

bool CWPXMLParser::loadXMLFile(const char* pFilePath)
{
	pugi::xml_parse_result result = m_Doc.load_file(pFilePath);
	if(result.status == 0)
	{
		bFileLoaded = true;
		return true;
	}

	return false;
}
	
int CWPXMLParser::getSubViewCount(pugi::xml_node node)
{
	int nViewCount = 0;
	for(pugi::xml_node views = node.child("view"); views; views = views.next_sibling("view"))
		nViewCount++;
	return nViewCount;
}

xml_node CWPXMLParser::getViewSubViewControllerNode(pugi::xml_node node, int nStartIndex)
{
	xml_node emptyNode;

	int m = 0;
	for(pugi::xml_node views = node.child("view"); views; views = views.next_sibling("view"))
	{
		if(m >= nStartIndex)
		{
			const pugi::char_t* testname = views.attribute("class").value();
			if(strcmp("CViewContainer", testname) == 0)
				return views;
		}

		m++;
	}

	return emptyNode;
}

xml_node CWPXMLParser::getViewSubViewNode(pugi::xml_node node, int nIndex)
{
	xml_node emptyNode;

	int m = 0;
	for(pugi::xml_node views = node.child("view"); views; views = views.next_sibling("view"))
	{
		if(m == nIndex)
			return views;

		m++;
	}

	return emptyNode;
}

xml_node CWPXMLParser::getTemplateInfo(const pugi::char_t* name, int& nViewCount)
{
	nViewCount = 0;
	bool bRet = false;
	for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(pugi::xml_node Template = uidesc.child("template"); Template; Template = Template.next_sibling("template"))
		{
			const pugi::char_t* testname = Template.attribute("name").value();
			if(strcmp(name, testname) == 0)
			{
				bRet = true;
				// count views
				for(pugi::xml_node views = Template.child("view"); views; views = views.next_sibling("view"))
					nViewCount++;

				return Template;
			}
		}
	}

	xml_node emptyTemplate;
	return emptyTemplate;
}
	
const pugi::char_t* CWPXMLParser::getTemplateAttribute(const pugi::char_t* templateName, const pugi::char_t* attributeName)
{

	for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(pugi::xml_node Template = uidesc.child("template"); Template; Template = Template.next_sibling("template"))
		{
			const pugi::char_t* testname = Template.attribute("name").value();
			if(strcmp(templateName, testname) == 0)
			{
				const pugi::char_t* att = Template.attribute(attributeName).value();
				return att;
			}
		}
	}

	return "";
}
	
const char_t* CWPXMLParser::getBitmapAttribute(const char_t* bitmapName, const char_t* attributeName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapname = bitmap.attribute("name").value();
				string sName(bitmapname);
				string sTargetBitmap(bitmapName);
				
				// --- found name
				if(strcmp(sName.c_str(), sTargetBitmap.c_str()) == 0)
				{
					// --- get NPT
					const char_t* att = bitmap.attribute(attributeName).value();
					return att;
				}
			}
		}
	}
	return "";
}

xml_node CWPXMLParser::getVSTGUIDescNode()
{
	xml_node uidesc = m_Doc.child("vstgui-ui-description"); //uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"));

	return uidesc;
}

bool CWPXMLParser::isBitmapTiled(const char_t* bitmapName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapname = bitmap.attribute("name").value();
				string sName(bitmapname);
				string sTargetBitmap(bitmapName);
				
				// --- found name
				if(strcmp(sName.c_str(), sTargetBitmap.c_str()) == 0)
				{
					// --- get NPT
					const char_t* tiled = bitmap.attribute("nineparttiled-offsets").value();
					string sTiled(tiled);
					if(sTiled.length() <= 0)
						return false;
					else
						return true;
				}
			}
		}
	}
	return false;
}

const char* CWPXMLParser::getRAFXBitmapType(const char_t* bitmapName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapname = bitmap.attribute("name").value();
				string sName(bitmapname);
				string sTargetBitmap(bitmapName);
				
				// --- found name
				if(strcmp(sName.c_str(), sTargetBitmap.c_str()) == 0)
				{
					// --- get NPT
					const char_t* type = bitmap.attribute("rafxtype").value();
					string sType(type);
					if(sType.length() <= 0)
						return "n/a";
					else
						return type;
				}
			}
		}
	}
	return "n/a";
}

bool CWPXMLParser::setRAFXBitmapType(const char* bitmapName, const char* bitmapType)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapname = bitmap.attribute("name").value();
				string sName(bitmapname);
				string sTargetBitmap(bitmapName);
				
				// --- found name
				if(strcmp(sName.c_str(), sTargetBitmap.c_str()) == 0)
				{
					// --- get NPT
					const char_t* type = bitmap.attribute("rafxtype").value();
					string sType(type);
					if(sType.length() <= 0)
						bitmap.append_attribute("rafxtype") = bitmapType;
					else
						this->setAttribute(bitmap, "rafxtype", bitmapType);

					return true;
				}
			}
		}
	}
	return false;

}
	
const char_t* CWPXMLParser::getNextBitmapNameWithType(int& nStartIndex, const char_t* rafxType)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				if(m >= nStartIndex)
				{
					const char_t* type = bitmap.attribute("rafxtype").value();
					if(type)
					{
						if(strcmp(type, rafxType) == 0)
						{
							const char_t* bitmapname = bitmap.attribute("name").value();
							nStartIndex = m+1;
							return bitmapname;
						}
					}

					m++;
				}
				else
					m++;
			}
		}
	}
	return NULL;

}

const char_t* CWPXMLParser::getBitmapName(int nIndex)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				if(m == nIndex)
				{
					const char_t* bitmapname = bitmap.attribute("name").value();
					return bitmapname;
				}
				else
					m++;
			}
		}
	}
	return NULL;
}

const char_t* CWPXMLParser::getBitmapNameWithFileName(const char_t* filename)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapfilename = bitmap.attribute("path").value();
				if(strcmp(bitmapfilename, filename) == 0)
				{
					const char_t* bitmapname = bitmap.attribute("name").value();
					return bitmapname;
				}
			}
		}
	}
	return NULL;
}

bool CWPXMLParser::setBitmapTiled(const char_t* bitmapName, bool bTiled, const char_t* tiledOffsetString )
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- 
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapname = bitmap.attribute("name").value();
				if(strcmp(bitmapName, bitmapname) == 0)
				{
					if(!bTiled)
						bitmap.remove_attribute("nineparttiled-offsets");
					else
						bitmap.append_attribute("nineparttiled-offsets") = tiledOffsetString;

					return true;
				}
			}
		}
	}

	return false;
}

bool CWPXMLParser::addBitmap(const char_t* bitmapName, const char_t* bitmapFileName)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			pugi::xml_node param = n1.append_child("bitmap");
			param.append_attribute("name") = bitmapName;
			param.append_attribute("path") = bitmapFileName;
			return true;
		}
	}
	return false;
}


bool CWPXMLParser::hasBitmap(const char_t* bitmapName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("bitmaps"); n1; n1 = n1.next_sibling("bitmaps"))
		{
			// --- these go to CFontDesc's
			for(xml_node bitmap = n1.child("bitmap"); bitmap; bitmap = bitmap.next_sibling("bitmap"))
			{
				const char_t* bitmapname = bitmap.attribute("name").value();
				string sName(bitmapname);
				string sTargetBitmap(bitmapName);

				if(strcmp(sName.c_str(), sTargetBitmap.c_str()) == 0)
					return true;
			}
		}
	}
	return false;
}
		
xml_node CWPXMLParser::addFont(const char_t* fontName, const char_t* weight, const char_t* bold, const char_t* italic)
{
	xml_node empty;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("fonts"); n1; n1 = n1.next_sibling("fonts"))
		{
			pugi::xml_node param = n1.append_child("font");
			param.append_attribute("bold") = bold;
			param.append_attribute("font-name") = fontName;
			param.append_attribute("italic") = italic;
			param.append_attribute("name") = fontName;
			param.append_attribute("size") = weight;
			return param;
		}
	}
	return empty;
}

bool CWPXMLParser::addColor(const char_t* colorName, const char_t* rgba)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("colors"); n1; n1 = n1.next_sibling("colors"))
		{
			pugi::xml_node param = n1.append_child("color");
			param.append_attribute("name") = colorName;
			param.append_attribute("rgba") = rgba;
			return true;
		}
	}
	return false;
}
		
xml_node CWPXMLParser::hasFont(const char_t* fontName)
{
	xml_node empty;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("fonts"); n1; n1 = n1.next_sibling("fonts"))
		{
			// --- these go to CFontDesc's
			for(xml_node node = n1.child("font"); node; node = node.next_sibling("font"))
			{
				const char_t* fontname = node.attribute("name").value();
			
				if(strcmp(fontName, fontname) == 0)
					return node;
			}
		}
	}
	return empty;
}

bool CWPXMLParser::hasColor(const char_t* colorName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("colors"); n1; n1 = n1.next_sibling("colors"))
		{
			// --- these go to CFontDesc's
			for(xml_node color = n1.child("color"); color; color = color.next_sibling("color"))
			{
				const char_t* colorname = color.attribute("name").value();
				string sName(colorname);
				string sTargetColor(colorName);

				if(strcmp(sName.c_str(), sTargetColor.c_str()) == 0)
					return true;
			}
		}
	}
	return false;
}
const char_t* CWPXMLParser::getGradientStartRGBA(const char_t* name)
{
    const char_t* empty = "";
    for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
    {
        for(pugi::xml_node Template = uidesc.child("gradients"); Template; Template = Template.next_sibling("gradients"))
        {
            for(pugi::xml_node views = Template.child("gradient"); views; views = views.next_sibling("gradient"))
            {
                const pugi::char_t* testname = views.attribute("name").value();
                if(strcmp(name, testname) == 0)
                {
                    for(pugi::xml_node colorStops = views.child("color-stop"); colorStops; colorStops = colorStops.next_sibling("color-stop"))
                    {
                        const pugi::char_t* testname = colorStops.attribute("start").value();
                        if(strcmp("0", testname) == 0)
                        {
                            const pugi::char_t* rgba = colorStops.attribute("rgba").value();
                            return rgba;
                        }
                    }
                }
            }
        }
    }
    
    return empty;
}

const char_t* CWPXMLParser::getGradientStopRGBA(const char_t* name)
{
    const char_t* empty = "";
    for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
    {
        for(pugi::xml_node Template = uidesc.child("gradients"); Template; Template = Template.next_sibling("gradients"))
        {
            for(pugi::xml_node views = Template.child("gradient"); views; views = views.next_sibling("gradient"))
            {
                const pugi::char_t* testname = views.attribute("name").value();
                if(strcmp(name, testname) == 0)
                {
                    for(pugi::xml_node colorStops = views.child("color-stop"); colorStops; colorStops = colorStops.next_sibling("color-stop"))
                    {
                        const pugi::char_t* testname = colorStops.attribute("start").value();
                        if(strcmp("1", testname) == 0)
                        {
                            const pugi::char_t* rgba = colorStops.attribute("rgba").value();
                            return rgba;
                        }
                    }
                }
            }
        }
    }
    
    return empty;
}

int CWPXMLParser::getTagIndex(int nTagValue)
{
	//return nTagValue;

	int m = 0;
	char* tagStr = new char[33];
	itoa(nTagValue, tagStr, 10);

	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
			{
				const char_t* thename = tag.attribute("tag").value();
				string sName(thename);

				if(strcmp(thename, tagStr) == 0)
				{
					delete [] tagStr;
					return m;
				}

				m++;
			}
		}
	}

	delete [] tagStr;
	return -1;
}
	
//bool CWPXMLParser::addTag(string name, string controlName, string tag)
bool CWPXMLParser::addTag(string name, string tag)
{
	if(hasTag(name.c_str())) return true;
	
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		xml_node n1 = uidesc.child("control-tags");
		xml_node child = n1.append_child("control-tag");
		child.append_attribute("name") = name.c_str();
		child.append_attribute("tag") = tag.c_str();
	//	child.append_attribute("control-name") = controlName.c_str();
	}

	return true;
}
	
bool CWPXMLParser::hasTag(const char_t* tagName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
			{
				const char_t* theTag = tag.attribute("name").value();
				if(strcmp(theTag, tagName) == 0)
					return true;
			}
		}
	}
	return false;
}
	
void CWPXMLParser::clearBaseTags()
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
			{
				const char_t* theTag = tag.attribute("tag").value();
				
                std::stringstream strValue;
                strValue << theTag;
                
                int nTag = -1;
                strValue >> nTag;
              
				//if(nTag < 32768)
				if(true)
				{
					// --- remove eeet
					n1.remove_child(tag);
				}
			}
		}
	}
}

int CWPXMLParser::getTagIndex(const char_t* tagName)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
			{
				const char_t* thename = tag.attribute("name").value();
				string sName(thename);
				string sTargetTag(tagName);

				if(strcmp(sName.c_str(), sTargetTag.c_str()) == 0)
				{
					const char_t* theTag = tag.attribute("tag").value();
					int n = atoi(theTag);
					//return n;

					return m;
				}

				m++;
			}
		}
	}
	return -1;
}

//const char_t* CWPXMLParser::getTagValue(const char_t* tagName)
//{
//	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
//	{
//		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
//		{
//			// --- 
//			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
//			{
//				const char_t* thename = tag.attribute("name").value();
//				string sName(thename);
//				string sTargetTag(tagName);
//
//				if(strcmp(sName.c_str(), sTargetTag.c_str()) == 0)
//				{
//					
//				}
//			}
//		}
//	}
//}



int CWPXMLParser::getTagCount()
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node color = n1.child("control-tag"); color; color = color.next_sibling("control-tag"))
			{
				m++;
			}
		}
	}
	return m;
}

const char_t* CWPXMLParser::getControlTagAttribute(const char_t* tagName, const char_t* attributeName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
			{
				const char_t* thename = tag.attribute("name").value();
				string sName(thename);
				string sTargetTag(tagName);

				if(strcmp(sName.c_str(), sTargetTag.c_str()) == 0)
				{
					const char_t* att = tag.attribute(attributeName).value();
					return att;
				}
			}
		}
	}
	return "";
}

const char_t* CWPXMLParser::getNextControlTagName(int nIndex)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("control-tags"); n1; n1 = n1.next_sibling("control-tags"))
		{
			// --- 
			for(xml_node tag = n1.child("control-tag"); tag; tag = tag.next_sibling("control-tag"))
			{
				if(m == nIndex)
				{
					const char_t* tagname = tag.attribute("name").value();
					return tagname;
				}
				m++;
			}
		}
	}

	return NULL;
}
		
const char_t* CWPXMLParser::getCustomAttribute(const char_t* attributeName, xml_node& node)
{
	xml_node empty;
	node = empty;

	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("custom"); n1; n1 = n1.next_sibling("custom"))
		{
			// --- find the attribute
			for(xml_node tag = n1.child("attributes"); tag; tag = tag.next_sibling("attributes"))
			{
				const char_t* att = tag.attribute(attributeName).value();
				if(strlen(att) > 0)
				{
					node = tag;
					return att;
				}
			}
		}
	}
	return NULL;
}

bool CWPXMLParser::setCustomAttribute(const char_t* attributeName, const char_t* attValue)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("custom"); n1; n1 = n1.next_sibling("custom"))
		{
			// --- find the attribute
			for(xml_node tag = n1.child("attributes"); tag; tag = tag.next_sibling("attributes"))
			{
				const char_t* att = tag.attribute(attributeName).value();
				if(strlen(att) > 0)
				{
					this->setAttribute(tag, attributeName, attValue);
					return true;
				}
			}

			// --- if we get here didn't find it so add
			n1.append_attribute(attributeName) = attValue;
			return true;
		}
	}

	return false;
}


xml_node CWPXMLParser::getCustomNode()
{
	xml_node empty;

	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("custom"); n1; n1 = n1.next_sibling("custom"))
		{
			return n1;
		}
	}

	return empty;
}


const char_t* CWPXMLParser::getNextColorName(int nIndex)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("colors"); n1; n1 = n1.next_sibling("colors"))
		{
			// --- 
			for(xml_node color = n1.child("color"); color; color = color.next_sibling("color"))
			{
				if(m == nIndex)
				{
					const char_t* colorname = color.attribute("name").value();
					return colorname;
				}
				m++;
			}
		}
	}

	return NULL;
}

const char_t* CWPXMLParser::getNextFontName(int nIndex)
{
	int m = 0;
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("fonts"); n1; n1 = n1.next_sibling("fonts"))
		{
			// --- 
			for(xml_node font = n1.child("font"); font; font = font.next_sibling("font"))
			{
				if(m == nIndex)
				{
					const char_t* fontname = font.attribute("name").value();
					return fontname;
				}
				m++;
			}
		}
	}

	return NULL;
}


const char_t* CWPXMLParser::getColorAttribute(const char_t* colorName, const char_t* attributeName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("colors"); n1; n1 = n1.next_sibling("colors"))
		{
			// --- t
			for(xml_node color = n1.child("color"); color; color = color.next_sibling("color"))
			{
				const char_t* colorname = color.attribute("name").value();
				string sName(colorname);
				string sTargetColor(colorName);

				if(strcmp(sName.c_str(), sTargetColor.c_str()) == 0)
				{
					const char_t* att = color.attribute(attributeName).value();
					return att;
				}
			}
		}
	}
	return "";
}	

const char_t* CWPXMLParser::getFontAttribute(const char_t* fontName, const char_t* attributeName)
{
	for(xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(xml_node n1 = uidesc.child("fonts"); n1; n1 = n1.next_sibling("fonts"))
		{
			// --- these go to CFontDesc's
			for(xml_node font = n1.child("font"); font; font = font.next_sibling("font"))
			{
				const char_t* name = font.attribute("name").value();
				string sName(name);
				string sTargetName(fontName);

				if(strcmp(sName.c_str(), sTargetName.c_str()) == 0)
				{
					const char_t* att = font.attribute(attributeName).value();
					return att;
				}
			}
		}
	}
	return "";
}




const char_t* CWPXMLParser::getAttribute(pugi::xml_node node, const char_t* attributeName)
{
	const char_t* att = node.attribute(attributeName).value();
	return att;
}
	
bool CWPXMLParser::setAttribute(pugi::xml_node node, const char_t* attributeName, const char_t* attributeValue)
{
	xml_attribute attr = node.attribute(attributeName);
	if(attr)
	{
		attr.set_value(attributeValue);
		return true;
	}
	// else create it
	node.append_attribute(attributeName) = attributeValue;

	return true;
}
	
int CWPXMLParser::getControlTypeCount(const char_t* className, pugi::xml_node node)
{
	int nCount = 0;
	// --- find sub-views
	for(pugi::xml_node view = node.child("view"); view; view = view.next_sibling("view"))
	{
		const pugi::char_t* classname = view.attribute("class").value();

		// -- will need to add more of these if we add more container support!
		if(strcmp(classname, "CViewContainer") == 0)
		{
			nCount += getControlTypeCount(className, view);
		}
		if(strcmp(className, classname) == 0)
			nCount++;
	}

	return nCount;
}


int CWPXMLParser::getControlTypeCount(const char_t* className)
{
	int nCount = 0;
	for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(pugi::xml_node Template = uidesc.child("template"); Template; Template = Template.next_sibling("template"))
		{
			// --- find sub-views
			for(pugi::xml_node view = Template.child("view"); view; view = view.next_sibling("view"))
			{
				const pugi::char_t* classname = view.attribute("class").value();

				// -- will need to add more of these if we add more container support!
				if(strcmp(classname, "CViewContainer") == 0)
				{
					nCount += getControlTypeCount(className, view);
				}

				if(strcmp(className, classname) == 0)
					nCount++;
			}
		}
	}
	return nCount;
}
		
xml_node CWPXMLParser::getTemplateSubViewNode(xml_node templateNode, const char_t* viewClass, 
											  const char_t* customViewName, const char_t* subControllerName)
{
	pugi::xml_node blank;
	
	// --- find sub-view of this template at nIndex
	for(pugi::xml_node view = templateNode.child("view"); view; view = view.next_sibling("view"))
	{
		const pugi::char_t* classname = view.attribute("class").value();
		const pugi::char_t* customview = view.attribute("custom-view-name").value();
		const pugi::char_t* subcontrollername = view.attribute("sub-controller").value();
		
		if(strcmp(viewClass, classname) == 0)
		{
			return view;
		}
	}

	return blank;
}
	
bool CWPXMLParser::hasRafxTemplate(const char_t* templateName)
{
	for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(pugi::xml_node Template = uidesc.child("template"); Template; Template = Template.next_sibling("template"))
		{
			const pugi::char_t* testname = Template.attribute("name").value();

			if(strcmp(templateName, testname) == 0)
				return true;
		}
	}

	return false;
}

VIEW_DESC* CWPXMLParser::getTemplateSubViewDesc(const char_t* templateName, int nIndex)
{
	VIEW_DESC* pDesc = NULL;

	int m = 0;
	for(pugi::xml_node uidesc = m_Doc.child("vstgui-ui-description"); uidesc; uidesc = uidesc.next_sibling("vstgui-ui-description"))
	{
		for(pugi::xml_node Template = uidesc.child("template"); Template; Template = Template.next_sibling("template"))
		{
			const pugi::char_t* testname = Template.attribute("name").value();

			if(strcmp(templateName, testname) == 0)
			{
				// --- find sub-view of this template at nIndex
				for(pugi::xml_node view = Template.child("view"); view; view = view.next_sibling("view"))
				{
					// --- find it
					if(m == nIndex)
					{
						pDesc = new VIEW_DESC;
						pDesc->viewNode = view;
						
						// --- get attributes
						const pugi::char_t* name = Template.attribute("name").value();
						pDesc->sName = string(name);

						const pugi::char_t* classname = Template.attribute("class").value();
						pDesc->sClass = string(classname);
						
						const pugi::char_t* templatename = Template.attribute("template").value();
						pDesc->sTemplateName = string(templatename);
						if(strlen(templatename) > 0)
							pDesc->bTemplate = true;
						else
							pDesc->bTemplate = false;

						const pugi::char_t* origin = Template.attribute("origin").value();
						pDesc->sOrigin = string(origin);

						const pugi::char_t* size = Template.attribute("size").value();
						pDesc->sSize = string(size);

						const pugi::char_t* transparent = Template.attribute("transparent").value();
						if(strcmp(transparent, "true") == 0 || strcmp(transparent, "TRUE") == 0)
							pDesc->bTransparemt = true;
						else
							pDesc->bTransparemt = false;

						return pDesc;
					}
					m++;
				}
			}
		}
	}
	return NULL;
}

int CWPXMLParser::getAttributeCount(pugi::xml_node node)
{
	int m = 0;
	for(xml_attribute a = node.first_attribute(); a; a = a.next_attribute())
		m++;
	
	return m;
}

bool CWPXMLParser::getAttributeValueNamePair(pugi::xml_node node, int index, char_t* attributeName, char_t* attributeValue)
{
	int m = 0;
	for(xml_attribute a = node.first_attribute(); a; a = a.next_attribute())
	{
		if(m == index)
		{
			const char* n = a.name();
			attributeName = new char[strlen(a.name())];
			strcpy((char*)attributeName, n);
            
			const char* v = a.value();
			attributeValue = new char[strlen(a.value())];
			strcpy((char*)attributeValue,v);
            
			return true;
		}
		m++;
	}
	return false;
}
const char_t* CWPXMLParser::getAttributeValue(pugi::xml_node node, int index)
{
	int m = 0;
	for(xml_attribute a = node.first_attribute(); a; a = a.next_attribute())
	{
		if(m == index)
		{
			return a.value();
		}
		m++;
	}
	return NULL;
}

const char_t* CWPXMLParser::getAttributeName(pugi::xml_node node, int index)
{
	int m = 0;
	for(xml_attribute a = node.first_attribute(); a; a = a.next_attribute())
	{
		if(m == index)
		{
			return a.name();
		}
		m++;
	}
	return NULL;
}

xml_node CWPXMLParser::getTemplateNode(const char_t* name)
{
	int nViewCount = 0;
	return getTemplateInfo(name, nViewCount);
}

