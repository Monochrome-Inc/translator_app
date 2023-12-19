#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QWidget>
#include <QTextCodec>
#include <QTextEncoder>
#include <QCloseEvent>
#include <QDateTime>

#include "KeyValues.h"
#include "translator.h"
#include "qforeach.h"
#include "ui_translator.h"

// Dialogs
#include "dialogadd.h"
#include "dialogaddplain.h"
#include "dialogremove.h"
#include "dialogoptions.h"
#include "dialogabout.h"

Json::Value JsonData;
Json::Value JsonConfig;

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

Translator::Translator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Translator)
{
    ui->setupUi(this);

    // Clear it by default
    ui->listWidget->clear();
    ui->listWidget->setEnabled( false );
    ui->tableWidget->setEnabled( false );

    ui->actionSave->setEnabled( false );
    ui->actionLangAdd->setEnabled( false );
    ui->actionLangRemove->setEnabled( false );
    ui->actionLangExport->setEnabled( false );
    ui->actionLangImport->setEnabled( false );
    ui->actionKeyCreate->setEnabled( false );
    ui->actionKeyRemove->setEnabled( false );

    LoadConfig();

    bIgnoreAddItemError = false;

    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(AutoSave()));
    timer->start(iMinSaveTimer);

    timer_bk = new QTimer(this);
    QObject::connect(timer_bk, SIGNAL(timeout()), this, SLOT(AutoBackup()));
    timer_bk->start(iMinBackupTimer);

    strMainPath = strExportPath = "./translations";
    strBackupLocation = "./translator_auto_backup";

    if ( !JsonConfig["dialog_options"]["main_path"].empty() )
        strMainPath = JsonConfig["dialog_options"]["main_path"].asString();
    if ( !JsonConfig["dialog_options"]["export_path"].empty() )
        strExportPath = JsonConfig["dialog_options"]["export_path"].asString();
    if ( !JsonConfig["dialog_options"]["backup_path"].empty() )
        strBackupLocation = JsonConfig["dialog_options"]["backup_path"].asString();
}

void Translator::ShutDown()
{
    timer->stop();
    SaveConfig(true);
}

Translator::~Translator()
{
    ShutDown();
    delete ui;
}

void Translator::closeEvent(QCloseEvent *event)
{
    if ( !bHasModifiedFile )
    {
        event->accept();
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(
                this, "Translator",
                tr("Do you want to save your changes?\n"),
                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                QMessageBox::Yes
    );

    if (resBtn == QMessageBox::No)
        event->accept();
    else if (resBtn == QMessageBox::Yes) {
        SaveFile();
        event->accept();
    } else {
        event->ignore();
    }
}

void Translator::KeySet(QTableWidgetItem *item)
{
    JsonConfig[strCurrentLang]["selected_row"] = item->row();
}

void Translator::KeyModify(QTableWidgetItem *item)
{
    // Grab our stuff
    QTableWidgetItem *widget = ui->tableWidget->verticalHeaderItem( item->row() );

    // Create Dialog
    DialogAdd *pDialog = new DialogAdd(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();

    // Set our string
    std::string strVariable;
    if ( JsonData[ strCurrentLang ][ widget->text().toStdString() ].empty() )
        strVariable = JsonData[ "lang_english" ][ widget->text().toStdString() ].asString();
    else
        strVariable = JsonData[ strCurrentLang ][ widget->text().toStdString() ].asString();

    // Set the values!
    pDialog->SetVariables( item->row(), strCurrentLang, widget->text().toStdString(), strVariable );

    // Resize our window size
    if ( !JsonConfig["dialog_translationvalue"]["width"].empty() && !JsonConfig["dialog_translationvalue"]["height"].empty() )
        pDialog->resize( JsonConfig["dialog_translationvalue"]["width"].asInt(), JsonConfig["dialog_translationvalue"]["height"].asInt() );
}

void Translator::SaveTableValue(int tableid, std::string lang, std::string key, QString value)
{
    // Grab our stuff
    QTableWidgetItem *widget = ui->tableWidget->item( tableid, 0 );
    if ( !widget ) return;

    QTextCodec *codec = QTextCodec::codecForName( "UTF-16" );
    QTextEncoder *encoderWithoutBom = codec->makeEncoder( QTextCodec::IgnoreHeader );
    QByteArray array = encoderWithoutBom->fromUnicode( value );

    std::string output = value.toUtf8().data();
    widget->setText( QString::fromStdString( output ) );
    widget->setForeground( QColor(255, 255, 255) );
    JsonData[ lang ][ key ] = output;
    SetModified(true);
}

void Translator::TabSwitch(QListWidgetItem *item)
{
    std::string strLang = "lang_";
    strLang += item->text().toStdString();
    strCurrentLang = strLang;
    LoadTranslationKeys( JsonData[ strCurrentLang ] );
}

void Translator::ShowAbout()
{
    // Create Dialog
    DialogAbout *pDialog = new DialogAbout(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();
    pDialog->setWindowTitle( "About Translator" );
    pDialog->setSizeGripEnabled( false );
    QSize sizeAboutDialog = pDialog->size();
    pDialog->setFixedSize( sizeAboutDialog );
}

void Translator::ShowQTAbout()
{
    QMessageBox::aboutQt( this, "Translator -- About" );
}

void Translator::Export()
{
    QString filePath = QFileDialog::getSaveFileName(
                this,
                tr("Export Currrent Language"),
                QString::fromStdString(strExportPath),
                tr("Language File (*.jlang)")
               );

    if ( filePath == "" ) return;

    // Setup the export language thingy
    Json::Value JsonTemp;
    // Grab our language groups
    std::vector<std::string> langgroups = JsonData.getMemberNames();
    for ( auto language : langgroups )
    {
        // Skip, since we already added it first
        if (language != strCurrentLang) continue;
        JsonTemp[language] = JsonData[language];
    }

    // Cleanup the stuff
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["commentStyle"] = "None";
    if ( MinimizeSave() )
        wbuilder.settings_["indentation"] = "";
    std::string output = Json::writeString( wbuilder, JsonTemp );
    ReplaceStringInPlace( output, "\\r", "" );

    // Save to file
    std::ofstream file_id;
    file_id.open( filePath.toStdString() );
    file_id << output;
    file_id.close();

    std::string strStatusMsg = strCurrentLang;
    strStatusMsg += " -- Exported succefully!";
    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );
}

void Translator::Import()
{
    QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Import Language"),
                QString::fromStdString(strExportPath),
                tr("Language File (*.jlang)")
               );

    Json::CharReaderBuilder rbuilder;
    Json::Value JSONReaderData;
    std::string strErr;
    bool bResult = false;
    std::ifstream JsonFile( filePath.toStdString(), std::ifstream::binary );
    if ( Json::parseFromStream( rbuilder, JsonFile, &JSONReaderData, &strErr ) )
    {
        // Grab our language groups
        std::vector<std::string> langgroups = JSONReaderData.getMemberNames();
        for ( auto language : langgroups )
        {
            JsonData[language] = JSONReaderData[language];
            std::string strStatusMsg = language;
            strStatusMsg += " -- Imported succefully!";
            ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );
        }
        bResult = true;
    }

    if ( bResult )
    {
        SetModified( true );
        LoadTranslation();
    }
}

void Translator::ClearFile()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(
        this, "Translator",
        tr("Do you want to clear the local data?\n"),
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::Yes
    );
    if (resBtn == QMessageBox::No) return;
    // Clear everything
    bFileLoaded = false;
    JsonData.clear();
    ui->actionClear->setEnabled( false );
    ui->actionSave->setEnabled( false );
    ui->listWidget->setEnabled( false );
    ui->tableWidget->setEnabled( false );
    LoadTranslation();
    JsonData.clear();
    AddKey( "UI_EXAMPLE_VALUE", "", QColor(160, 160, 160) );
    ui->statusBar->showMessage( "Cleared local data.", 10000 );
}

void Translator::OpenKeyValueImport()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Import Keyvalue Translation File"),
        QString::fromStdString(strExportPath),
        tr("KeyValue File (*.txt)")
    );
    KeyValues *pKV = new KeyValues( "lang" );
    KeyValueRead_e ret = pKV->LoadFile( filePath );
    switch (ret)
    {
        case KVRead_OK:
        {
            bool bHaveLanguages = false;
            std::vector<std::string> langgroups = JsonData.getMemberNames();
            if (langgroups.size() > 0)
                bHaveLanguages = true;
            if ( !bHaveLanguages )
            {
                QString savePath = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Translation File"),
                    QString::fromStdString(strMainPath),
                    tr("Translation Files (*.json)")
                );
                if ( savePath == "" ) return;
                QFile f(savePath);
                QFileInfo fileInfo(f.fileName());
                QString filename(fileInfo.fileName());
                f.close();
                if ( filename == "" ) return;
                strFileLoaded = filename.toStdString();
                strFileLocation = savePath.toStdString();
                bFileLoaded = true;
                SaveConfig();
            }
            std::string _lang = pKV->GrabLanguageID().toStdString();
            bIgnoreAddItemError = true;
            AddItem( pKV->GrabLanguageID().toStdString(), true );
            std::list<KeyValueData> _Keys;
            pKV->GrabKeyValues( _Keys );
            for ( auto data : _Keys )
                AddItem( data.Key.toStdString(), data.Value.toStdString(), _lang );
            bIgnoreAddItemError = false;
            SetModified( true );
            LoadTranslation();
            ui->actionClear->setEnabled( true );
            ui->actionSave->setEnabled( true );
            ui->listWidget->setEnabled( true );
            ui->tableWidget->setEnabled( true );

            ui->actionLangAdd->setEnabled( true );
            ui->actionLangRemove->setEnabled( true );
            ui->actionLangExport->setEnabled( true );
            ui->actionLangImport->setEnabled( true );
            ui->actionKeyCreate->setEnabled( true );
            ui->actionKeyRemove->setEnabled( true );

            ui->statusBar->showMessage( "Imported language file.", 10000 );
        }
        break;
        case KVRead_ERR_NOTLANG:
        {
            QMessageBox::warning(
                this,
                "Import Error",
                tr("This is not a translation keyvalue file.\n"),
                QMessageBox::Yes
                );
        }
        break;
        case KVRead_ERR_LANG_NOT_FOUND:
        {
            QMessageBox::warning(
                this,
                "Import Error",
                tr("This language file is missing it's language string!\n"),
                QMessageBox::Yes
                );
        }
        break;
    }
}

void Translator::OpenExport()
{
    QString savePath = QFileDialog::getSaveFileName(
        this,
        tr("Export Translation"),
        QString::fromStdString(strMainPath),
        tr("Translation KeyValue Files (*.txt)")
    );
    if ( savePath == "" ) return;
    QFile f(savePath);
    QFileInfo fileInfo(f.fileName());
    QString filename(fileInfo.fileName());
    f.close();
    if ( filename == "" ) return;
    KeyValues *pKV = new KeyValues( "lang" );
    pKV->Export( savePath, filename, JsonData );
}

void Translator::OpenOptions()
{
    // Create Dialog
    DialogOptions *pDialog = new DialogOptions(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();
    pDialog->SetDefault( "./translations", strBackupLocation, 5 );
    pDialog->ReadConfig();
    // Resize our window size
    if ( !JsonConfig["dialog_options"]["width"].empty() && !JsonConfig["dialog_options"]["height"].empty() )
        pDialog->resize( JsonConfig["dialog_options"]["width"].asInt(), JsonConfig["dialog_options"]["height"].asInt() );
}

void Translator::LangAdd()
{
    // Create Dialog
    DialogAddPlain *pDialog = new DialogAddPlain(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();
    pDialog->setWindowTitle( "Language" );
    pDialog->SetText( "Add Language" );
    pDialog->IsLanguage( true );
    // Resize our window size
    if ( !JsonConfig["dialog_add"]["width"].empty() && !JsonConfig["dialog_add"]["height"].empty() )
        pDialog->resize( JsonConfig["dialog_add"]["width"].asInt(), JsonConfig["dialog_add"]["height"].asInt() );
}

void Translator::LangRemove()
{
    // Create Dialog
    DialogRemove *pDialog = new DialogRemove(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();
    pDialog->setWindowTitle( "Language" );
    pDialog->SetText( "Remove Language" );
    pDialog->SetDialogValues( JsonData, true );
    // Resize our window size
    if ( !JsonConfig["dialog_remove"]["width"].empty() && !JsonConfig["dialog_remove"]["height"].empty() )
        pDialog->resize( JsonConfig["dialog_remove"]["width"].asInt(), JsonConfig["dialog_remove"]["height"].asInt() );
}

void Translator::KeyAdd()
{
    // Create Dialog
    DialogAddPlain *pDialog = new DialogAddPlain(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();
    pDialog->setWindowTitle( "Translation Key" );
    pDialog->SetText( "Add Translation Key" );
    // Resize our window size
    if ( !JsonConfig["dialog_add"]["width"].empty() && !JsonConfig["dialog_add"]["height"].empty() )
        pDialog->resize( JsonConfig["dialog_add"]["width"].asInt(), JsonConfig["dialog_add"]["height"].asInt() );
}

void Translator::KeyRemove()
{
    // Create Dialog
    DialogRemove *pDialog = new DialogRemove(this);
    pDialog->setAttribute(Qt::WA_DeleteOnClose);
    pDialog->show();
    pDialog->setWindowTitle( "Translation Key" );
    pDialog->SetText( "Remove Translation Key" );
    pDialog->SetDialogValues( JsonData["lang_english"], false );
    // Resize our window size
    if ( !JsonConfig["dialog_remove"]["width"].empty() && !JsonConfig["dialog_remove"]["height"].empty() )
        pDialog->resize( JsonConfig["dialog_remove"]["width"].asInt(), JsonConfig["dialog_remove"]["height"].asInt() );
}

bool HasLanguage(std::string strLang)
{
    std::vector<std::string> langgroups = JsonData.getMemberNames();
    for (auto language : langgroups)
    {
        if ( language == strLang ) return true;
    }
    return false;
}

bool HasKey(std::string strKey)
{
    std::vector<std::string> keygroups = JsonData["lang_english"].getMemberNames();
    for (auto key : keygroups)
    {
        if ( key == strKey ) return true;
    }
    return false;
}

void Translator::AddItem(std::string strItem, bool bLang)
{
    AddItem(strItem, "", bLang);
}

void Translator::AddItem(std::string strItem, std::string strValue, bool bLang)
{
    std::string strInput = strItem;
    if ( bLang )
        ReplaceStringInPlace( strInput, "lang_", "" );
    if ( strInput.length() <= 1 )
    {
        QMessageBox::warning(
                    this,
                    "Error",
                    tr("The string is invalid, and cannot be used!\n"),
                    QMessageBox::Yes
        );
        return;
    }

    if ( bLang )
    {
        std::string strLang = "lang_";
        strLang += strInput;
        // Check if it already exist
        if ( HasLanguage( strLang ) )
        {
            if (!bIgnoreAddItemError)
                QMessageBox::warning(
                            this,
                            "Error",
                            tr("The language already exist!\n"),
                            QMessageBox::Yes
                );
            return;
        }
        Json::Value jTemp;
        JsonData[strLang] = jTemp;
    }
    else
    {
        // Check if it already exist
        if ( HasKey( strInput ) )
        {
            if (!bIgnoreAddItemError)
                QMessageBox::warning(
                            this,
                            "Error",
                            tr("The translation key already exist!\n"),
                            QMessageBox::Yes
                );
            return;
        }
        JsonData["lang_english"][strInput] = strValue;
    }
    LoadTranslation();

    std::string strStatusMsg = strInput;
    strStatusMsg += " -- has been added!";
    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );

    SetModified( true );
}

void Translator::AddItem(std::string strItem, std::string strValue, std::string strLang)
{
    JsonData["lang_" + strLang][strItem] = strValue;
    ui->statusBar->showMessage( "Imported translation file", 10000 );
}

void Translator::RemoveItem(std::string strItem, bool bLang)
{
    std::string strValue = bLang ? "lang_" + strItem : strItem;

    // Don't make this possible
    if (strValue == "lang_english" && bLang)
    {
        QMessageBox::warning(
                    this,
                    "Error",
                    tr("You cannot remove the english language!\n"),
                    QMessageBox::Yes
        );
        return;
    }

    Json::Value jTempData;
    int iTemp = 0;
    if ( bLang )
    {
        std::vector<std::string> keygroups = JsonData.getMemberNames();
        for (auto key : keygroups)
        {
            // Skip
            if (key == strValue) continue;
            jTempData[key] = JsonData[key];
            iTemp++;
        }
    }
    else
    {
        std::vector<std::string> langgroups = JsonData.getMemberNames();
        for (auto language : langgroups)
        {
            std::vector<std::string> keygroups = JsonData[language].getMemberNames();
            for (auto key : keygroups)
            {
                // Skip
                if (key == strValue) continue;
                jTempData[language][key] = JsonData[language][key];
                if (language == "lang_english")
                    iTemp++;
            }
        }
    }

    JsonData = jTempData;
    jTempData.clear();

    // Check the count
    if ( iTemp <= 1 )
    {
        if ( bLang )
            ui->actionLangRemove->setEnabled( false );
        else
            ui->actionKeyRemove->setEnabled( false );
    }
    LoadTranslation();

    std::string strStatusMsg = strValue;
    strStatusMsg += " -- has been removed!";
    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );

    SetModified( true );
}

void Translator::OpenFile()
{
    QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Open Translation File"),
                QString::fromStdString(strMainPath),
                tr("Translation Files (*.json)")
               );
    QFile f(filePath);
    QFileInfo fileInfo(f.fileName());
    QString filename(fileInfo.fileName());
    f.close();

    if ( !bHasModifiedFile )
    {
        OpenSpecificFile( filePath.toStdString(), filename.toStdString() );
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(
                this, "Translator",
                tr("Do you want to save your changes?\n"),
                QMessageBox::No | QMessageBox::Yes,
                QMessageBox::Yes
    );

    if (resBtn == QMessageBox::No)
        OpenSpecificFile( filePath.toStdString(), filename.toStdString() );
    else if (resBtn == QMessageBox::Yes) {
        SaveFile();
        OpenSpecificFile( filePath.toStdString(), filename.toStdString() );
    }
}

void Translator::AutoSave()
{
    // Don't autosave when we don't have anything to save...
    if ( !bFileLoaded ) return;
    SaveFile();
    std::string strStatusMsg = "Autosaving: ";
    strStatusMsg += strFileLoaded;
    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );
}

void Translator::AutoBackup()
{
    // Don't backup when we don't have anything to backup...
    if ( !bFileLoaded ) return;
    BackupFile();
    std::string strStatusMsg = "Creating backup: ";
    strStatusMsg += strFileLoaded;
    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );
}

bool Translator::MinimizeSave()
{
     if ( JsonConfig["dialog_options"]["minimize"].empty() ) return true;
     return JsonConfig["dialog_options"]["minimize"].asBool();
}

void Translator::BackupFile()
{
    // Setup the export language thingy
    Json::Value JsonTemp;

    if ( !JsonConfig["dialog_options"]["backup_option"].empty() && JsonConfig["dialog_options"]["backup_option"].asBool() )
    {
        // Grab our language groups
        std::vector<std::string> langgroups = JsonData.getMemberNames();
        for ( auto language : langgroups )
        {
            // Skip, since we already added it first
            if (language != strCurrentLang) continue;
            JsonTemp[language] = JsonData[language];
        }
    }
    else
        JsonTemp = JsonData;

    // Cleanup the stuff
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["commentStyle"] = "None";
    if ( MinimizeSave() )
        wbuilder.settings_["indentation"] = "";
    std::string output = Json::writeString( wbuilder, JsonTemp );
    ReplaceStringInPlace( output, "\\r", "" );

    // Create folder, if it doesn't exist
    if ( !QDir( QString::fromStdString( strBackupLocation ) ).exists() )
        QDir().mkdir( QString::fromStdString( strBackupLocation ) );

    // Get our date
    QDateTime qTime = QDateTime::currentDateTime();

    // Our filename (<name>_<date>)
    std::string strBackupFile = strBackupLocation;
    strBackupFile += "/";
    strBackupFile += strFileLoaded;
    strBackupFile += "_";
    strBackupFile += qTime.toString( "yyyy-MM-dd_HH-mm-ss" ).toStdString();

    if ( !JsonConfig["dialog_options"]["backup_option"].empty() )
        strBackupFile += JsonConfig["dialog_options"]["backup_option"].asBool() ? ".jlang" : ".json";
    else
        strBackupFile += ".js";

    // Save backup
    std::ofstream file_id;
    file_id.open( strBackupFile );
    file_id << output;
    file_id.close();
}

void Translator::SaveFile()
{
    // Save config file
    SaveConfig();

    // Cleanup the stuff
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["commentStyle"] = "None";
    if ( MinimizeSave() )
        wbuilder.settings_["indentation"] = "";
    std::string output = Json::writeString( wbuilder, JsonData );
    ReplaceStringInPlace( output, "\\r", "" );

    // Save to file
    std::ofstream file_id;
    file_id.open( strFileLocation );
    file_id << output;
    file_id.close();

    std::string strStatusMsg = "Saving: ";
    strStatusMsg += strFileLoaded;
    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );

    SetModified( false );
}

void Translator::SetModified(bool state)
{
    // Our title output
    std::string strValue;

    // We have the file loaded?
    if ( bFileLoaded )
    {
        strValue = strFileLoaded;
        strValue += " - Translator";
    }
    else
        strValue = "Translator";

    // Modified? then apply the star
    if ( state )
        strValue += "*";

    // Set our modified state
    bHasModifiedFile = state;

    // Set our window title
    setWindowTitle( QString::fromStdString( strValue ) );

    // Save our config
    if ( state )
        SaveConfig();
}

void Translator::OpenSpecificFile(std::string strLocation, std::string strFile, bool bConfigClear)
{
    Json::CharReaderBuilder rbuilder;
    Json::Value JSONReaderData;
    std::string strErr;
    std::string strStatusMsg = strFile;

    std::ifstream JsonFile( strLocation, std::ifstream::binary );
    if ( Json::parseFromStream( rbuilder, JsonFile, &JSONReaderData, &strErr ) )
    {
        if ( bConfigClear )
        {
            std::vector<std::string> langgroups = JSONReaderData.getMemberNames();
            for ( auto language : langgroups )
            {
                if ( !JsonConfig[language].empty() )
                    JsonConfig.removeMember( language );
            }
            JsonConfig.removeMember( "file_name" );
            JsonConfig.removeMember( "file_path" );
        }
        JsonData.clear();
        JsonData = JSONReaderData;
        bFileLoaded = true;

        strFileLoaded = strFile;
        strFileLocation = strLocation;

        ui->actionClear->setEnabled( true );
        ui->actionSave->setEnabled( true );
        ui->actionLangAdd->setEnabled( true );
        ui->actionKeyCreate->setEnabled( true );
        ui->actionLangExport->setEnabled( true );
        ui->actionLangImport->setEnabled( true );

        LoadTranslation();

        // Still false? Then it means we don't exist, then enable it
        if ( !bConfigLoaded )
            bConfigLoaded = true;

        SetModified( false );
        SaveConfig();

        // Enable it
        ui->listWidget->setEnabled( true );
        ui->tableWidget->setEnabled( true );

        strStatusMsg += " loaded succefully!";
    }
    else
    {
        if ( strFile != "" )
            strStatusMsg += " failed to load!";
    }


    ui->statusBar->showMessage( QString::fromStdString( strStatusMsg ), 10000 );
}

void Translator::SaveOptions(std::string strMain, std::string strExport, int iTimer, std::string strBackup, int iTimer_Backup)
{
    // Save the tick pos
    JsonConfig["dialog_options"]["auto_save_pos"] = iTimer;
    JsonConfig["dialog_options"]["auto_backup_pos"] = iTimer_Backup;
    strMainPath = strMain;
    strExportPath = strExport;
    strBackupLocation = strBackup;
    iMinSaveTimer = iTimer * 60000;
    iMinBackupTimer = iTimer_Backup * 60000;
    SaveConfig( false, true );
}

void Translator::SaveConfig(bool bSaveWindowSize, bool bSaveOptions)
{
    if ( !bConfigLoaded ) return;
    // Save our info
    JsonConfig["auto_save"] = iMinSaveTimer;
    JsonConfig["backup_save"] = iMinBackupTimer;
    JsonConfig["file_path"] = strFileLocation;
    JsonConfig["file_name"] = strFileLoaded;

    if ( bSaveOptions )
    {
        JsonConfig["dialog_options"]["main_path"] = strMainPath;
        JsonConfig["dialog_options"]["export_path"] = strExportPath;
        JsonConfig["dialog_options"]["backup_path"] = strBackupLocation;
    }

    if ( bSaveWindowSize )
    {
        QSize MainWindowSize = size();
        JsonConfig["mainwindow"]["width"] = MainWindowSize.width();
        JsonConfig["mainwindow"]["height"] = MainWindowSize.height();
    }

    // Cleanup the stuff
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["commentStyle"] = "None";
    //wbuilder.settings_["indentation"] = "";
    std::string output = Json::writeString( wbuilder, JsonConfig );
    ReplaceStringInPlace( output, "\\r", "" );

    // Save to file
    std::ofstream file_id;
    file_id.open( "config.data" );
    file_id << output;
    file_id.close();
}

void Translator::LoadConfig()
{
    // if config.data doesn't exist, default save timer will be 5min
    iMinSaveTimer = iMinBackupTimer = 5 * 60000;

    Json::CharReaderBuilder rbuilder;
    Json::Value JSONReaderData;
    std::string strFile = "config.data";
    std::string strErr;
    std::ifstream JsonFile( strFile, std::ifstream::binary );
    if ( Json::parseFromStream( rbuilder, JsonFile, &JSONReaderData, &strErr ) )
    {
        JsonConfig = JSONReaderData;

        if ( !JsonConfig["auto_save"].empty() )
            iMinSaveTimer = JsonConfig["auto_save"].asInt();
        if ( !JsonConfig["backup_save"].empty() )
            iMinBackupTimer = JsonConfig["backup_save"].asInt();

        // Load the file
        OpenSpecificFile( JsonConfig["file_path"].asString(), JsonConfig["file_name"].asString(), false );

        // Resize our window size
        if ( !JsonConfig["mainwindow"]["width"].empty() && !JsonConfig["mainwindow"]["height"].empty() )
            resize( JsonConfig["mainwindow"]["width"].asInt(), JsonConfig["mainwindow"]["height"].asInt() );

        bConfigLoaded = true;
    }
}

void Translator::UpdateTimer()
{
    // Our new value
    iMinSaveTimer = JsonConfig["auto_save"].asInt();
    iMinBackupTimer = JsonConfig["backup_save"].asInt();

    // Stop and resume
    timer->stop();
    timer->start(iMinSaveTimer);

    timer_bk->stop();
    timer_bk->start(iMinBackupTimer);

    SaveConfig( false );
}

void Translator::LoadTranslation()
{
    // Clear it
    ui->listWidget->clear();

    // Grab our language groups
    std::vector<std::string> langgroups = JsonData.getMemberNames();

    // Load our language
    int iLang = 0;
    for ( auto language : langgroups )
    {
        std::string str = language;
        str.replace( 0, 5, "" );

        QListWidgetItem *widget = new QListWidgetItem( ui->listWidget );
        widget->setTextAlignment( Qt::AlignCenter );
        widget->setText( QString::fromStdString( str ) );
        iLang++;
    }

    ui->actionLangRemove->setEnabled( iLang > 1 ? true : false );

    // Load our stuff into this
    strCurrentLang = "lang_english";
    LoadTranslationKeys( JsonData[strCurrentLang] );
}

void Translator::LoadTranslationKeys(Json::Value jdata)
{
    std::vector<std::string> translationkeys = jdata.getMemberNames();
    std::vector<std::string> FoundKeys;
    std::vector<std::string>::iterator it;

    iMaxKeys = 0;

    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount( 1 );

    QTableWidgetItem *item_table = new QTableWidgetItem();
    ui->tableWidget->setHorizontalHeaderItem(0, item_table);
    item_table->setText( QApplication::translate("Translator", "Translation Value", nullptr) );

    int iKeys = 0;
    for (auto translatekey : translationkeys)
    {
        AddKey( translatekey, jdata[translatekey].asString(), QColor(255, 255, 255) );
        // Add to foundkeys list
        it = FoundKeys.begin();
        it = FoundKeys.insert( it, translatekey );
        iKeys++;
    }

    // Add english values, if we aren't in english translation mode
    if ( strCurrentLang != "lang_english" )
    {
        translationkeys = JsonData["lang_english"].getMemberNames();
        for (auto translatekey : translationkeys)
        {
            bool bCanAddKey = true;
            for (it = FoundKeys.begin(); it < FoundKeys.end(); it++)
            {
                if (*it == translatekey)
                {
                    bCanAddKey = false;
                    break;
                }
            }

            if (bCanAddKey == false) continue;

            AddKey( translatekey, JsonData["lang_english"][translatekey].asString(), QColor(160, 160, 160) );
            iKeys++;
        }
    }

    ui->actionKeyRemove->setEnabled( iKeys > 0 ? true : false );

    // Set the size for the vertical header
    ui->tableWidget->verticalHeader()->setFixedWidth( 350 );

    // Select our row
    ui->tableWidget->selectRow( JsonConfig[strCurrentLang]["selected_row"].asInt() );
    QTableWidgetItem *pItem = ui->tableWidget->item( JsonConfig[strCurrentLang]["selected_row"].asInt(), 0 );
    if ( pItem )
        ui->tableWidget->scrollToItem( pItem );
}

void Translator::AddKey(std::string strKey, std::string strValue, QColor color)
{
    ui->tableWidget->setRowCount(iMaxKeys + 1);

    QTableWidgetItem *item_A = new QTableWidgetItem();
    ui->tableWidget->setVerticalHeaderItem(iMaxKeys, item_A);
    QTableWidgetItem *item_B = new QTableWidgetItem();
    item_B->setTextAlignment(Qt::AlignCenter);
    item_B->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
    ui->tableWidget->setItem(iMaxKeys, 0, item_B);

    item_A->setText( QString::fromStdString( strKey ) );

    const bool __sortingEnabled = ui->tableWidget->isSortingEnabled();
    ui->tableWidget->setSortingEnabled(false);
    item_B->setText( QString::fromStdString( strValue ) );
    item_B->setForeground( color );
    ui->tableWidget->setSortingEnabled(__sortingEnabled);

    iMaxKeys++;
}
