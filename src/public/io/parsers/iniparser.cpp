// IniFile.cpp:  Implementation of the CIniFile class.
// Written by:   Adam Clauss
// Email: cabadam@houston.rr.com
// You may use this class/code as you wish in your programs.  Feel free to distribute it, and
// email suggested changes to me.
//
// Rewritten by: Shane Hill
// Date:         21/08/2001
// Email:        Shane.Hill@dsto.defence.gov.au
// Reason:       Remove dependancy on MFC. Code should compile on any
//               platform.
//////////////////////////////////////////////////////////////////////

// C Includes
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

// Local Includes
#include "iniparser.h"

#if defined(WIN32)
#define iniEOL endl
#else
#define iniEOL '\r' << endl
#endif

CIniFile::CIniFile(const string &iniPath)
{
    Path(iniPath);
    caseInsensitive = true;
    //std::locale::global(std::locale(""));
}

bool CIniFile::ReadFile()
{
    // Normally you would use ifstream, but the SGI CC compiler has
    // a few bugs with ifstream. So ... fstream used.
    fstream f;
    string   line;
    string   keyname, valuename, value;
    string::size_type pLeft, pRight;

    f.open(path.c_str(), ios::in);
    if (f.fail())
        return false;

    while(getline(f, line)) {
        // To be compatible with Win32, check for existence of '\r'.
        // Win32 files have the '\r' and Unix files don't at the end of a line.
        // Note that the '\r' will be written to INI files from
        // Unix so that the created INI file can be read under Win32
        // without change.
        size_t nLen = line.length();
        if(nLen==0 ) continue;  // + Charles 2008-4-30
        if (line[nLen - 1] == '\r')
            line = line.substr(0, line.length() - 1);

        if (line.length()) {
            // Check that the user hasn't openned a binary file by checking the first
            // character of each line!
            if (!isprint(line[0])) {
                printf("Failing on char %d\n", line[0]);
                f.close();
                return false;
            }
            if ((pLeft = line.find_first_of(";#[=")) != string::npos) {
                switch (line[pLeft]) {
    case '[':
        if ((pRight = line.find_last_of("]")) != string::npos &&
            pRight > pLeft) {
                keyname = line.substr(pLeft + 1, pRight - pLeft - 1);
                AddKeyName(keyname);
        }
        break;

    case '=':
        valuename = line.substr(0, pLeft);
        value = line.substr(pLeft + 1);
        SetValue(keyname, valuename, value);
        break;

    case ';':
    case '#':
        if (!names.size())
            HeaderComment(line.substr(pLeft + 1));
        else
            KeyComment(keyname, line.substr(pLeft + 1));
        break;
                }
            }
        }
    }

    f.close();
    if (names.size())
        return true;
    return false;
}

bool CIniFile::WriteFile()
{
    unsigned commentID, keyID, valueID;
    // Normally you would use ofstream, but the SGI CC compiler has
    // a few bugs with ofstream. So ... fstream used.
    fstream f;

    f.open(path.c_str(), ios::out);
    if (f.fail())
        return false;

    // Write header comments.
    for (commentID = 0; commentID < comments.size(); ++commentID)
        f << ';' << comments[commentID] << iniEOL;
    if (comments.size())
        f << iniEOL;

    // Write keys and values.
    for (keyID = 0; keyID < keys.size(); ++keyID) {
        f << '[' << names[keyID] << ']' << iniEOL;
        // Comments.
        for (commentID = 0; commentID < keys[keyID].comments.size(); ++commentID)
            f << ';' << keys[keyID].comments[commentID] << iniEOL;
        // Values.
        for (valueID = 0; valueID < keys[keyID].names.size(); ++valueID)
            f << keys[keyID].names[valueID] << '=' << keys[keyID].values[valueID] << iniEOL;
        f << iniEOL;
    }
    f.close();

    return true;
}

long CIniFile::FindKey(const string &keyname) const
{
    string lstr, rstr;

    rstr = keyname;
    for (unsigned keyID = 0; keyID < names.size(); ++keyID)
    {
        lstr = names[keyID];
        if (CheckCase(lstr) == CheckCase(rstr))
        {
            return long(keyID);
        }
    }
    return noID;
}

long CIniFile::FindValue(size_t keyID, const string &valuename) const
{
    if (!keys.size() || keyID >= keys.size())
        return noID;

    string lstr, rstr;

    rstr = valuename;
    for (unsigned valueID = 0; valueID < keys[keyID].names.size(); ++valueID)
    {
        lstr = keys[keyID].names[valueID];
        if (CheckCase(lstr) == CheckCase(rstr))
        {
            return long(valueID);
        }
    }
    return noID;
}

size_t CIniFile::AddKeyName(const string &keyname)
{
    names.resize(names.size() + 1, keyname);
    keys.resize(keys.size() + 1);
    return names.size() - 1;
}

const string &CIniFile::KeyName(size_t keyID) const
{
    if (keyID < names.size())
        return names[keyID];
    else
        return NIL_STR;
}

size_t CIniFile::NumValues(size_t keyID)
{
    if (keyID < keys.size())
        return keys[keyID].names.size();
    return 0;
}

size_t CIniFile::NumValues(const string &keyname)
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return 0;
    return keys[keyID].names.size();
}

const string &CIniFile::ValueName(size_t keyID, size_t valueID) const
{
    if (keyID < keys.size() && valueID < keys[keyID].names.size())
        return keys[keyID].names[valueID];
    return NIL_STR;
}

const string &CIniFile::ValueName(const string &keyname, size_t valueID) const
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return NIL_STR;
    return ValueName(keyID, valueID);
}

bool CIniFile::SetValue(size_t keyID, size_t valueID, const string &value)
{
    if (keyID < keys.size() && valueID < keys[keyID].names.size())
        keys[keyID].values[valueID] = value;

    return false;
}

bool CIniFile::SetValue(const string &keyname, const string &valuename, const string &value, bool const create)
{
    long keyID = FindKey(keyname);
    if (keyID == noID) {
        if (create)
            keyID = long(AddKeyName(keyname));
        else
            return false;
    }

    long valueID = FindValue(unsigned(keyID), valuename);
    if (valueID == noID) {
        if (!create)
            return false;
        keys[keyID].names.resize(keys[keyID].names.size() + 1, valuename);
        keys[keyID].values.resize(keys[keyID].values.size() + 1, value);
    } else
        keys[keyID].values[valueID] = value;

    return true;
}

bool CIniFile::SetValueI(const string &keyname, const string &valuename, int const value, bool const create)
{
    char svalue[MAX_VALUEDATA];

    sprintf(svalue, "%d", value);
    return SetValue(keyname, valuename, svalue);
}

bool CIniFile::SetValueF(const string &keyname, const string &valuename, double const value, bool const create)
{
    char svalue[MAX_VALUEDATA];

    sprintf(svalue, "%f", value);
    return SetValue(keyname, valuename, svalue);
}

bool CIniFile::SetValueV(const string &keyname, const string &valuename, char *format, ...)
{
    va_list args;
    char value[MAX_VALUEDATA];

    va_start(args, format);
    vsprintf(value, format, args);
    va_end(args);
    return SetValue(keyname, valuename, value);
}

const string &CIniFile::GetValue(size_t keyID, size_t valueID, const string &defValue) const
{
    if (keyID < keys.size() && valueID < keys[keyID].names.size())
        return keys[keyID].values[valueID];
    return defValue;
}

const string &CIniFile::GetValue(const string &keyname, const string &valuename, const string &defValue) const
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return defValue;

    long valueID = FindValue(unsigned(keyID), valuename);
    if (valueID == noID)
        return defValue;

    return keys[keyID].values[valueID];
}

int CIniFile::GetValueI(const string &keyname, const string &valuename, int const defValue) const
{
    char svalue[MAX_VALUEDATA];

    sprintf(svalue, "%d", defValue);
    return atoi(GetValue(keyname, valuename, svalue).c_str());
}

double CIniFile::GetValueF(const string &keyname, const string &valuename, double const defValue) const
{
    char svalue[MAX_VALUEDATA];

    sprintf(svalue, "%f", defValue);
    return atof(GetValue(keyname, valuename, svalue).c_str());
}

// 16 variables may be a bit of over kill, but hey, it's only code.
unsigned CIniFile::GetValueV(const string &keyname, const string &valuename, char *format,
                             void *v1, void *v2, void *v3, void *v4,
                             void *v5, void *v6, void *v7, void *v8,
                             void *v9, void *v10, void *v11, void *v12,
                             void *v13, void *v14, void *v15, void *v16)
{
    string   value;
    // va_list  args;
    unsigned nVals;


    value = GetValue(keyname, valuename);
    if (!value.length())
        return false;
    // Why is there not vsscanf() function. Linux man pages say that there is
    // but no compiler I've seen has it defined. Bummer!
    //
    // va_start(args, format);
    // nVals = vsscanf(value.c_str(), format, args);
    // va_end(args);

    nVals = sscanf(value.c_str(), format,
        v1, v2, v3, v4, v5, v6, v7, v8,
        v9, v10, v11, v12, v13, v14, v15, v16);

    return nVals;
}

bool CIniFile::DeleteValue(const string &keyname, const string &valuename)
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return false;

    long valueID = FindValue(unsigned(keyID), valuename);
    if (valueID == noID)
        return false;

    // This looks strange, but is neccessary.
    vector<string>::iterator npos = keys[keyID].names.begin() + valueID;
    vector<string>::iterator vpos = keys[keyID].values.begin() + valueID;
    keys[keyID].names.erase(npos, npos + 1);
    keys[keyID].values.erase(vpos, vpos + 1);

    return true;
}

bool CIniFile::DeleteKey(const string &keyname)
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return false;

    // Now hopefully this destroys the vector lists within keys.
    // Looking at <vector> source, this should be the case using the destructor.
    // If not, I may have to do it explicitly. Memory leak check should tell.
    // memleak_test.cpp shows that the following not required.
    //keys[keyID].names.clear();
    //keys[keyID].values.clear();

    vector<string>::iterator npos = names.begin() + keyID;
    vector<key>::iterator    kpos = keys.begin() + keyID;
    names.erase(npos, npos + 1);
    keys.erase(kpos, kpos + 1);

    return true;
}

void CIniFile::Erase()
{
    // This loop not needed. The vector<> destructor seems to do
    // all the work itself. memleak_test.cpp shows this.
    //for (unsigned i = 0; i < keys.size(); ++i) {
    //  keys[i].names.clear();
    //  keys[i].values.clear();
    //}
    names.clear();
    keys.clear();
    comments.clear();
}

void CIniFile::HeaderComment(const string &comment)
{
    comments.resize(comments.size() + 1, comment);
}

const string &CIniFile::HeaderComment(size_t commentID) const
{
    if (commentID < comments.size())
        return comments[commentID];
    return NIL_STR;
}

bool CIniFile::DeleteHeaderComment(unsigned commentID)
{
    if (commentID < comments.size()) {
        vector<string>::iterator cpos = comments.begin() + commentID;
        comments.erase(cpos, cpos + 1);
        return true;
    }
    return false;
}

size_t CIniFile::NumKeyComments(size_t keyID) const
{
    if (keyID < keys.size())
        return keys[keyID].comments.size();
    return 0;
}

size_t CIniFile::NumKeyComments(const string &keyname) const
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return 0;
    return keys[keyID].comments.size();
}

bool CIniFile::KeyComment(size_t keyID, const string &comment)
{
    if (keyID < keys.size()) {
        keys[keyID].comments.resize(keys[keyID].comments.size() + 1, comment);
        return true;
    }
    return false;
}

bool CIniFile::KeyComment(const string &keyname, const string &comment)
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return false;
    return KeyComment(unsigned(keyID), comment);
}

const string &CIniFile::KeyComment(size_t keyID, size_t commentID) const
{
    if (keyID < keys.size() && commentID < keys[keyID].comments.size())
        return keys[keyID].comments[commentID];
    return NIL_STR;
}

const string &CIniFile::KeyComment(const string &keyname, size_t commentID) const
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return NIL_STR;
    return KeyComment(unsigned(keyID), commentID);
}

bool CIniFile::DeleteKeyComment(size_t keyID, size_t commentID)
{
    if (keyID < keys.size() && commentID < keys[keyID].comments.size()) {
        vector<string>::iterator cpos = keys[keyID].comments.begin() + commentID;
        keys[keyID].comments.erase(cpos, cpos + 1);
        return true;
    }
    return false;
}

bool CIniFile::DeleteKeyComment(const string &keyname, size_t commentID)
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return false;
    return DeleteKeyComment(unsigned(keyID), commentID);
}

bool CIniFile::DeleteKeyComments(size_t keyID)
{
    if (keyID < keys.size()) {
        keys[keyID].comments.clear();
        return true;
    }
    return false;
}

bool CIniFile::DeleteKeyComments(const string &keyname)
{
    long keyID = FindKey(keyname);
    if (keyID == noID)
        return false;
    return DeleteKeyComments(unsigned(keyID));
}

const string &CIniFile::CheckCase(string &s) const
{
    if (caseInsensitive)
        for (string::size_type i = 0; i < s.length(); ++i)
            s[i] = tolower(s[i]);
    return s;
}

CiniFile::CiniFile(void)
{
}

CiniFile::~CiniFile(void)
{
}
