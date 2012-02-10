#include "xmlparser.h"

XMLParser::XMLParser(const char* file, bool sync)
    : m_root(NULL)
    , m_curElem(NULL)
    , m_doc(NULL)
{
    if (sync)
    {
        LOG_WARN("pub.io", "同步加载未实现.");
    }
    else
    {
        m_doc = new TiXmlDocument;
        if (m_doc->LoadFile(file))
        {
            m_curElem = m_root = m_doc->RootElement();
        }
    }
}

XMLParser::XMLParser()
{
}

XMLParser::~XMLParser()
{
	SAFE_DELETE(m_doc);
}

bool XMLParser::SetCursor(int count, const char* attr, ...)
{
	va_list argptr;
	va_start(argptr, attr);

	// 从根节点处查找
	TiXmlElement* elem = m_root;

	while (NULL != elem)
	{
		m_curElem = elem;
		if (--count < 0)
			break;
		elem = elem->FirstChildElement(attr);
		attr = va_arg(argptr, const char*);
	}

	va_end(argptr);

	if (NULL == m_curElem)
	{
		LOG_WARN("pub.io", attr);
		return false;
	}
	return true;
}

bool XMLParser::SetRoot()
{
	if (NULL != m_root)
	{
		m_curElem = m_root;
		return true;
	}
	return false;
}

bool XMLParser::SetParent()
{
	if (NULL == m_curElem)
	{
		LOG_WARN("pub.io", "NULL == m_curElem");
		return false;
	}

	TiXmlNode* node = m_curElem->Parent();
	if (NULL != node)
	{
		m_curElem = node->ToElement();
		return true;
	}
	return false;
}

bool XMLParser::SetPrevSibling(const char* name)
{
	return false;
}

bool XMLParser::SetNextSibling(const char* name)
{
	if (NULL == m_curElem)
	{
		LOG_WARN("pub.io", "NULL == m_curElem");
		return false;
	}

	TiXmlElement* temp = NULL;
	if (NULL == name)
		temp = m_curElem->NextSiblingElement();
	else
		temp = m_curElem->NextSiblingElement(name);

	if (NULL != temp)
	{
		m_curElem = temp;
		return true;
	}
	else
		return false;
}

bool XMLParser::SetFirstChild(const char* name)
{
	if (NULL == m_curElem)
	{
		LOG_WARN("pub.io", "NULL == m_curElem");
		return false;
	}

	TiXmlElement* temp = NULL;
	if (NULL == name)
		temp = m_curElem->FirstChildElement();
	else
		temp = m_curElem->FirstChildElement(name);

	if (NULL != temp)
	{
		m_curElem = temp;
		return true;
	}
	else
		return false;
}

bool XMLParser::SetLastChild(const char* name)
{
	return false;
}

bool XMLParser::SetCurNode(int ind)
{
    if (!SetRoot() || !SetFirstChild())
        return false;

    while (ind-- && SetNextSibling());
    if (ind >= 0) return false;

    return true;
}

const char* XMLParser::GetAttr(const char* attr) const
{
	const char* res = NULL;
	if (NULL != m_curElem)
	{
		res = m_curElem->Attribute(attr);
	}

	if (NULL == res)
	{
		res = "";
	}

	return res;
}

const char* XMLParser::GetChildAttr(const char* tag,
                                      const char* attr) const
{
	const char* res = NULL;
	TiXmlElement* childElem = NULL;
	if (NULL != m_curElem &&
		NULL != (childElem = m_curElem->FirstChildElement(tag)))
	{

		res = childElem->Attribute(attr);
	}

	if (NULL == res)
	{
        LOG_WARN("pub.io", "%s/%s", tag, attr);
	}

	return res;
}

/// 删除当前节点
void XMLParser::RemoveCurNode()
{
    if (NULL == m_curElem)
        return;

    TiXmlElement* parent = dynamic_cast<TiXmlElement*>(m_curElem->Parent());
    if (NULL == parent)
        return;

    TiXmlElement* tmp = m_curElem;
    m_curElem = dynamic_cast<TiXmlElement*>(m_curElem->NextSibling());
    parent->RemoveChild(tmp);
}

/// 插入当前节点的一个副本到当前节点位置
void XMLParser::CopyAndInsertCurNode()
{
    if (NULL == m_curElem)
        return;

    TiXmlElement* parent = dynamic_cast<TiXmlElement*>(m_curElem->Parent());
    if (NULL == parent)
        return;

    parent->InsertBeforeChild(m_curElem, *m_curElem);
}
