#ifndef KEYVALUES_H
#define KEYVALUES_H

#include <QString>
#include <json.h>

struct KeyValueData
{
    QString Key;
    QString Value;
};

enum KeyValueRead_e
{
    KVRead_OK = 0,
    KVRead_ERR_NOTLANG,
    KVRead_ERR_LANG_NOT_FOUND
};

class KeyValues
{
public:
    KeyValues( const char *setName );
    void SetString( const QString szKey, const QString szValue );
    bool SaveFile( const QString szPath, const QString szLanguage );
    KeyValueRead_e LoadFile( const QString szFile );
    void GrabKeyValues( std::list<KeyValueData> &keys );
    QString GrabLanguageID() { return m_Language; };
    void Export( const QString szPath, const QString szFile, Json::Value data );
private:
    // This will be read if we find \r, and everything became 1 single damn line.
    // This happens with very large files, one example being contagion_ui_english.txt
    KeyValueRead_e LoadBloatFile( const QString szFile );
    bool IsLineIgnored( QString szLine );
    KeyValueData ReadLine( QString szLine, bool bTokens );
    QString m_Language;
    std::list<KeyValueData> m_Keys;
    QString m_Name;
};

#endif // KEYVALUES_H
