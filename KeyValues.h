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
    KeyValueRead_e LoadFile( const QString szFile );
    void GrabKeyValues( std::list<KeyValueData> &keys );
    QString GrabLanguageID() { return m_Language; };
    void Export( const QString szPath, const QString szFile, Json::Value data );
private:
    bool IsLineIgnored( QString szLine );
    KeyValueData ReadLine( QString szLine, bool bTokens );
    QString m_Language;
    std::list<KeyValueData> m_Keys;
    QString m_Name;
};

#endif // KEYVALUES_H
