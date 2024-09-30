#include "KeyValues.h"
#include "translator.h"
#include "qforeach.h"

#include <QFile>
#include <QTextStream>
#include <QChar>

#include <fstream>

static bool s_bMultiLineValue = false;
static KeyValueData s_MultiLineValue;

KeyValues::KeyValues( const char *setName )
{
    m_Language = "";
    m_Name = setName;
    m_Keys.clear();
}

void KeyValues::SetString( const QString szKey, const QString szValue )
{
    KeyValueData data;
    data.Key = szKey;
    data.Value = szValue;
    m_Keys.push_back( data );
}

bool KeyValues::SaveFile( const QString szPath, const QString szLangauge )
{
    if ( szPath.isEmpty() ) return false;
    std::ofstream file_id;
    std::string szSTDLangFile = szPath.toStdString();
    std::string szLanguageStripped = szLangauge.toStdString();
    ReplaceStringInPlace( szLanguageStripped, "lang_", "" );
    std::string szReplaceString = "_" + szLanguageStripped + ".txt";
    if ( !ReplaceStringInPlace( szSTDLangFile, ".txt", szReplaceString ) )
        szSTDLangFile += szReplaceString;

    file_id.open( szSTDLangFile );
    //Output will be this:
/*
 * "lang"
 * {
 *    "Language" "english"
 *    "Tokens"
 *    {
 *       "UI_EXAMPLE"   "My Sample Text"
 *       ...
 *    }
 * }
*/
    file_id << "\"lang\"\n";
    file_id << "{\n";
        file_id << "\t\"Language\"\t\"" << szLanguageStripped << "\"\n";
        file_id << "\t\"Tokens\"\n";
        file_id << "\t{\n";
        foreach(KeyValueData data, m_Keys)
        {
            file_id << "\t\t\"" << data.Key.toStdString() << "\"\t\"" << data.Value.toStdString() << "\"\n";
        }
        file_id << "\t}\n";
    file_id << "}\n";
    file_id.close();
    return true;
}

KeyValueRead_e KeyValues::LoadFile( const QString szFile )
{
    QFile f( szFile );
    if ( f.open(QIODevice::ReadOnly) )
    {
        s_bMultiLineValue = false;
        enum TranslationReading
        {
            ReadNone = 0,
            ReadLang,
            ReadTokens
        };

        TranslationReading eTranslationKV = ReadNone;
        QTextStream in(&f);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            KeyValueData data = ReadLine( line, (eTranslationKV == ReadTokens) );
            switch (eTranslationKV)
            {
                case ReadNone:
                {
                    if ( data.Key == "lang" )
                        eTranslationKV = ReadLang;
                }
                break;
                case ReadLang:
                {
                    if ( data.Key == "Language" )
                        m_Language = data.Value.toLower();
                    else if ( data.Key == "Tokens" && m_Language != "" )
                        eTranslationKV = ReadTokens;
                }
                break;
                case ReadTokens:
                {
                    // Did the previous data not have an end qoute?
                    if ( s_bMultiLineValue ) continue;
                    // Ignore comments etc.
                    if ( IsLineIgnored( line ) ) continue;
                    // We can't add nothing
                    if ( data.Key == "" ) continue;
                    m_Keys.push_back( data );
                }
                break;
            }
        }
        f.close();
        if ( eTranslationKV == ReadNone )
            return KVRead_ERR_NOTLANG;
        return (m_Keys.size() > 0) ? KVRead_OK : KVRead_ERR_LANG_NOT_FOUND;
    }
    return KVRead_ERR_NOTLANG;
}

void KeyValues::Export( const QString szPath, const QString szFile, Json::Value JsonData )
{
    m_Keys.clear();
    // Exports the json data to KeyValues
    std::vector<std::string> langgroups = JsonData.getMemberNames();
    foreach(std::string language, langgroups)
    {
        std::vector<std::string> keygroups = JsonData[language].getMemberNames();
        foreach(std::string key, keygroups)
        {
            SetString( QString::fromStdString(key), QString::fromStdString(JsonData[language][key].asString()) );
        }
        SaveFile( szPath, QString::fromStdString(language) );
        m_Keys.clear();
    }
}

bool KeyValues::IsLineIgnored( QString szLine )
{
    if ( szLine.indexOf("//") != -1 ) return true;
    if ( szLine.indexOf("{") != -1 ) return true;
    if ( szLine.indexOf("}") != -1 ) return true;
    return false;
}

KeyValueData KeyValues::ReadLine( QString szLine, bool bTokens )
{
    KeyValueData data;

    enum QouteState
    {
        BEGIN = 0,
        KEY_START,
        KEY_END,
        VALUE_START,
        VALUE_END
    };
    QouteState eState = BEGIN;

    bool bFoundAValidToken = false;
    bool bSlashQoute = false;
    bool bMultilineReading = s_bMultiLineValue;
    if ( !bTokens )
        bMultilineReading = false;

    // If not multiline
    if ( !bMultilineReading )
    {
        // If the line is just straight up empty.
        //if ( szLine == "" ) return data;

        foreach( QChar var, szLine )
        {
            // We don't want to include the qoute
            // But if we had a backslash before this qoute, then we simply add it.
            if ( '"' == var && !bSlashQoute )
            {
                switch( eState )
                {
                    case BEGIN: eState = KEY_START; bFoundAValidToken = true; break;
                    case KEY_START: eState = KEY_END; break;
                    case KEY_END: eState = VALUE_START; break;
                    case VALUE_START: eState = VALUE_END; break;
                    default: break;
                }
                continue;
            }
            // Grab the Key
            if ( eState == KEY_START )
                data.Key += var;
            // Grab the Value
            else if ( eState == VALUE_START )
                data.Value += var;
            // Do this last, since we want to check if this current var has a slash
            if ( '\\' == var )
                bSlashQoute = true;
            else
                bSlashQoute = false;
        }
    }
    else
    {
        foreach( QChar var, szLine )
        {
            // We don't want to include the qoute
            // But if we had a backslash before this qoute, then we simply add it.
            if ( '"' == var && !bSlashQoute )
            {
                eState = VALUE_END;
                break;
            }
            else
                s_MultiLineValue.Value += var;
            // Do this last, since we want to check if this current var has a slash
            if ( '\\' == var )
                bSlashQoute = true;
            else
                bSlashQoute = false;
        }
    }

    if (bTokens)
    {
        if ( !bFoundAValidToken )
        {
            data.Key.clear();
            data.Value.clear();
            return data;
        }

        // If eState is VALUE_END, then we ain't a multiline value
        if ( eState == VALUE_END )
        {
            if ( s_bMultiLineValue )
            {
                s_bMultiLineValue = false;
                data = s_MultiLineValue;
            }
        }
        else
        {
            if ( !s_bMultiLineValue )
            {
                data.Value += "\n";
                s_MultiLineValue = data;
                s_bMultiLineValue = true;
            }
        }
    }
    return data;
}

void KeyValues::GrabKeyValues( std::list<KeyValueData> &keys )
{
    keys = m_Keys;
}
