// IniFile.cpp:  Implementation of the CIniFile class.
// Written by:   Adam Clauss
// Email: cabadam@tamu.edu
// You may use this class/code as you wish in your programs.  Feel free to distribute it, and
// email suggested changes to me.
//
// Rewritten by: Shane Hill
// Date:         21/08/2001
// Email:        Shane.Hill@dsto.defence.gov.au
// Reason:       Remove dependancy on MFC. Code should compile on any
//               platform. Tested on Windows/Linux/Irix
//
// Rewritten by: Fox (yulefox at gmail.com)
// Date:         2011-09-02
// Reason:       将 string 实例转换为引用, 将区分大小写的功能去掉, 以提高存取效率.
//////////////////////////////////////////////////////////////////////

#ifndef PUBLIC_IO_PARSERS_INIPARSER_H
#define PUBLIC_IO_PARSERS_INIPARSER_H

// C++ Includes
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <strstream>

using namespace std;

// C Includes
#include <stdlib.h>

#define MAX_KEYNAME    128
#define MAX_VALUENAME  128
#define MAX_VALUEDATA 2048

class CIniFile
{
private:
  bool   caseInsensitive;
  string path;
  struct key {
    vector<string> names;
    vector<string> values;
    vector<string> comments;
  };
  vector<key>    keys;
  vector<string> names;
  vector<string> comments;
  const string &CheckCase(string &s) const;

public:
  enum errors{ noID = -1};
  CIniFile(const string &iniPath = "");
  virtual ~CIniFile()                            {}

  // Sets whether or not keynames and valuenames should be case sensitive.
  // The default is case insensitive.
  void CaseSensitive()                           {caseInsensitive = false;}
  void CaseInsensitive()                         {caseInsensitive = true;}

  // Sets path of ini file to read and write from.
  void Path(const string &newPath)                {path = newPath;}
  const string &Path() const                            {return path;}
  void SetPath(const string &newPath)             {Path(newPath);}

  // Reads ini file specified using path.
  // Returns true if successful, false otherwise.
  bool ReadFile();

  // Writes data stored in class to ini file.
  bool WriteFile();

  // Deletes all stored ini data.
  void Erase();
  void Clear()                                   {Erase();}
  void Reset()                                   {Erase();}

  // Returns index of specified key, or noID if not found.
  long FindKey(const string &keyname) const;

  // Returns index of specified value, in the specified key, or noID if not found.
  long FindValue(size_t keyID, const string &valuename) const;

  // Returns number of keys currently in the ini.
  size_t NumKeys() const                       {return names.size();}
  size_t GetNumKeys() const                    {return NumKeys();}

  // Add a key name.
  size_t AddKeyName(const string &keyname);

  // Returns key names by index.
  const string &KeyName(size_t keyID) const;
  const string &GetKeyName(size_t keyID) const {return KeyName(keyID);}

  // Returns number of values stored for specified key.
  size_t NumValues(size_t keyID);
  size_t GetNumValues(size_t keyID)   {return NumValues(keyID);}
  size_t NumValues(const string &keyname);
  size_t GetNumValues(const string &keyname)   {return NumValues(keyname);}

  // Returns value name by index for a given keyname or keyID.
  const string &ValueName(size_t keyID, size_t valueID) const;
  const string &GetValueName(size_t keyID, size_t valueID) const {
    return ValueName(keyID, valueID);
  }
  const string &ValueName(const string &keyname, size_t valueID) const;
  const string &GetValueName(const string &keyname, size_t valueID) const {
    return ValueName(keyname, valueID);
  }

  // Gets value of [keyname] valuename =.
  // Overloaded to return string, int, and double.
  // Returns defValue if key/value not found.
  const string &GetValue(size_t keyID, size_t valueID, const string &defValue = "") const;
  const string &GetValue(const string &keyname, const string &valuename, const string &defValue = "") const;
  int GetValueI(const string &keyname, const string &valuename, int const defValue = 0) const;
 // bool   GetValueB(const string &keyname, const string &valuename, bool const defValue = false) const {
   // return (bool)((bool)GetValueI(keyname, valuename, int(defValue)));
  //}
  double GetValueF(const string &keyname, const string &valuename, double const defValue = 0.0) const;
  // This is a variable length formatted GetValue routine. All these voids
  // are required because there is no vsscanf() like there is a vsprintf().
  // Only a maximum of 8 variable can be read.
  unsigned GetValueV(const string &keyname, const string &valuename, char *format,
		      void *v1 = 0, void *v2 = 0, void *v3 = 0, void *v4 = 0,
  		      void *v5 = 0, void *v6 = 0, void *v7 = 0, void *v8 = 0,
  		      void *v9 = 0, void *v10 = 0, void *v11 = 0, void *v12 = 0,
  		      void *v13 = 0, void *v14 = 0, void *v15 = 0, void *v16 = 0);

  // Sets value of [keyname] valuename =.
  // Specify the optional paramter as false (0) if you do not want it to create
  // the key if it doesn't exist. Returns true if data entered, false otherwise.
  // Overloaded to accept string, int, and double.
  bool SetValue(size_t keyID, size_t valueID, const string &value);
  bool SetValue(const string &keyname, const string &valuename, const string &value, bool const create = true);
  bool SetValueI(const string &keyname, const string &valuename, int const value, bool const create = true);
  bool SetValueB(const string &keyname, const string &valuename, bool const value, bool const create = true) {
    return SetValueI(keyname, valuename, int(value), create);
  }
  bool SetValueF(const string &keyname, const string &valuename, double const value, bool const create = true);
  bool SetValueV(const string &keyname, const string &valuename, char *format, ...);

  // Deletes specified value.
  // Returns true if value existed and deleted, false otherwise.
  bool DeleteValue(const string &keyname, const string &valuename);

  // Deletes specified key and all values contained within.
  // Returns true if key existed and deleted, false otherwise.
  bool DeleteKey(const string &keyname);

  // Header comment functions.
  // Header comments are those comments before the first key.
  //
  // Number of header comments.
  size_t NumHeaderComments() {return comments.size();}
  // Add a header comment.
  void     HeaderComment(const string &comment);
  // Return a header comment.
  const string &  HeaderComment(size_t commentID) const;
  // Delete a header comment.
  bool     DeleteHeaderComment(unsigned commentID);
  // Delete all header comments.
  void     DeleteHeaderComments()               {comments.clear();}

  // Key comment functions.
  // Key comments are those comments within a key. Any comments
  // defined within value names will be added to this list. Therefore,
  // these comments will be moved to the top of the key definition when
  // the CIniFile::WriteFile() is called.
  //
  // Number of key comments.
  size_t NumKeyComments(size_t keyID) const;
  size_t NumKeyComments(const string &keyname) const;
  // Add a key comment.
  bool     KeyComment(size_t keyID, const string &comment);
  bool     KeyComment(const string &keyname, const string &comment);
  // Return a key comment.
  const string &  KeyComment(size_t keyID, size_t commentID) const;
  const string &  KeyComment(const string &keyname, size_t commentID) const;
  // Delete a key comment.
  bool     DeleteKeyComment(size_t keyID, size_t commentID);
  bool     DeleteKeyComment(const string &keyname, size_t commentID);
  // Delete all comments for a key.
  bool     DeleteKeyComments(size_t keyID);
  bool     DeleteKeyComments(const string &keyname);
};

static const string NIL_STR = "";

class CiniFile
{
public:
	CiniFile(void);
	~CiniFile(void);
};

#endif
