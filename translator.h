#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QMainWindow>
#include <QTimer>
#include <QTableWidgetItem>
#include <QListWidgetItem>

#include "json/writer.h"

namespace Ui
{
    class Translator;
}

extern Json::Value JsonData;
extern Json::Value JsonConfig;

extern bool ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);

class Translator : public QMainWindow
{
    Q_OBJECT

public:
    explicit Translator(QWidget *parent = nullptr);
    ~Translator();

    void AddItem( std::string strItem, bool bLang );
    void AddItem( std::string strItem, std::string strValue, bool bLang );
    void AddItem( std::string strItem, std::string strValue, std::string strLang );
    void RemoveItem( std::string strItem, bool bLang );
    void SaveTableValue(int tableid, std::string lang, std::string key, QString value);
    void closeEvent(QCloseEvent *event);
    void SaveOptions( std::string strMain, std::string strExport, int iTimer, std::string strBackup, int iTimer_Backup );
    bool MinimizeSave();
    void OnNewTranslationCreated( QString strFile );
    void OnItemModified( QString OldString, QString NewString, bool IsLang );

public slots:
    void ClearFile();
    void OpenKeyValueImport();
    void OpenExport();
    void OpenOptions();
    void ShowAbout();
    void ShowQTAbout();
    void NewFile();
    void OpenFile();
    void SaveFile();
    void BackupFile();
    void AutoSave();
    void AutoBackup();
    void ToggleMode(bool state);
    void SetModified(bool state);
    void UpdateTimer();
    void ShutDown();
    void KeySet(QTableWidgetItem *item);
    void KeyModify(QTableWidgetItem *item);
    void TabSwitch(QListWidgetItem *item);
    void Export();
    void Import();
    void LangAdd();
    void LangRemove();
    void KeyAdd();
    void KeyRemove();
    void ModifyItem();
    void ModifyLang();

protected:
    void OpenSpecificFile(std::string strLocation, std::string strFile, bool bConfigClear = true);
    void SaveConfig(bool bSaveWindowSize = false, bool bSaveOptions = false);
    void LoadConfig();
    void LoadTranslation();
    void LoadTranslationKeys(Json::Value jdata);
    void AddKey(std::string strKey, std::string strValue, QColor color);

private:
    Ui::Translator *ui;
    QTimer *timer;
    QTimer *timer_bk;

    int iMinSaveTimer;
    int iMinBackupTimer;
    int iMaxKeys;

    bool bFileLoaded;
    bool bConfigLoaded;
    bool bHasModifiedFile;
    bool bIgnoreAddItemError;

    // START: AS API Docs Support
    /// If enabled, we will check the latest data.json
    /// provided by the SDK AppID, the same exact one used by the API docs website.
    bool bIsAngelscript;
    // END: AS API Docs Support

    std::string strCurrentLang = "lang_english";
    std::string	strFileLocation;
    std::string	strFileLoaded;
    std::string strBackupLocation;
    std::string strMainPath;
    std::string strExportPath;

};

#endif // TRANSLATOR_H
