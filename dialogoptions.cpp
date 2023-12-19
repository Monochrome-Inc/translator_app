#include "dialogoptions.h"
#include "ui_dialogoptions.h"
#include "translator.h"

#include <QFileDialog>

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);
}

DialogOptions::~DialogOptions()
{
    // Save our size
    QSize WindowSize = size();
    JsonConfig["dialog_options"]["width"] = WindowSize.width();
    JsonConfig["dialog_options"]["height"] = WindowSize.height();
    delete ui;
}

void DialogOptions::SetDefault(std::string gamepath, std::string backuppath, int timer)
{
    strDefault_MainPath = gamepath;
    strDefault_ExportPath = gamepath;
    strDefault_BackupPath = backuppath;
    iDefault_SaveTimer = timer;
    bDefault_Minimize = true;
    bDefault_BackupOption = false;
}

void DialogOptions::ResetToDefult()
{
    ui->Option1_MainPath->setText( QString::fromStdString( strDefault_MainPath ) );
    ui->Option2_ExportPath->setText( QString::fromStdString( strDefault_ExportPath ) );
    ui->Option3_SaveTimer->setSliderPosition( iDefault_SaveTimer );
    ui->Option4_Backup_Timer->setSliderPosition( iDefault_SaveTimer );
    ui->Option4_BackupOption->setChecked( bDefault_BackupOption );
    ui->Option_MinimizeSave->setChecked( bDefault_Minimize );
}

void DialogOptions::ReadConfig()
{
    // Default
    QString qstrMainPath = QString::fromStdString( strDefault_MainPath );
    QString qstrExportPath = QString::fromStdString( strDefault_ExportPath );
    QString qstrBackupPath = QString::fromStdString( strDefault_BackupPath );
    int iTimer = iDefault_SaveTimer;
    int iTimer_Backup = iTimer;
    bool bOptionState = true;
    bool bMinimizwState = true;
    // Read from config
    if ( !JsonConfig["dialog_options"]["main_path"].empty() )
        qstrMainPath = QString::fromStdString( JsonConfig["dialog_options"]["main_path"].asString() );
    if ( !JsonConfig["dialog_options"]["export_path"].empty() )
        qstrExportPath = QString::fromStdString( JsonConfig["dialog_options"]["export_path"].asString() );
    if ( !JsonConfig["dialog_options"]["backup_path"].empty() )
        qstrBackupPath = QString::fromStdString( JsonConfig["dialog_options"]["backup_path"].asString() );
    if ( !JsonConfig["dialog_options"]["auto_save_pos"].empty() )
        iTimer = JsonConfig["dialog_options"]["auto_save_pos"].asInt();
    if ( !JsonConfig["dialog_options"]["auto_backup_pos"].empty() )
        iTimer_Backup = JsonConfig["dialog_options"]["auto_backup_pos"].asInt();
    if ( !JsonConfig["dialog_options"]["backup_option"].empty() )
        bOptionState = JsonConfig["dialog_options"]["backup_option"].asBool();
    if ( !JsonConfig["dialog_options"]["minimize"].empty() )
        bMinimizwState = JsonConfig["dialog_options"]["minimize"].asBool();
    // Set our stuff
    ui->Option1_MainPath->setText( qstrMainPath );
    ui->Option2_ExportPath->setText( qstrExportPath );
    ui->Option4_BackupPath->setText( qstrBackupPath );
    ui->Option3_SaveTimer->setSliderPosition( iTimer );
    ui->Option4_Backup_Timer->setSliderPosition( iTimer_Backup );
    ui->Option4_BackupOption->setChecked( bOptionState );
    ui->Option_MinimizeSave->setChecked( bMinimizwState );
    // Save the stuff
    SaveConfig();
}

void DialogOptions::ChooseMain()
{
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Choose Main Path"),
                ui->Option1_MainPath->text(),
                QFileDialog::ShowDirsOnly
               );

    if ( dirPath == "" ) return;
    ui->Option1_MainPath->setText( dirPath );
}

void DialogOptions::ChooseExport()
{
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Choose Export Path"),
                ui->Option2_ExportPath->text(),
                QFileDialog::ShowDirsOnly
               );

    if ( dirPath == "" ) return;
    ui->Option2_ExportPath->setText( dirPath );
}

void DialogOptions::ChooseBackup()
{
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Choose Backup Path"),
                ui->Option4_BackupPath->text(),
                QFileDialog::ShowDirsOnly
               );

    if ( dirPath == "" ) return;
    ui->Option4_BackupPath->setText( dirPath );
}

void DialogOptions::UpdateBackupOption( bool state )
{
    JsonConfig["dialog_options"]["backup_option"] = state;
}

void DialogOptions::UpdateMinimize( bool state )
{
    JsonConfig["dialog_options"]["minimize"] = state;
}

void DialogOptions::SaveConfig()
{
    Translator *translator = dynamic_cast<Translator *>( parentWidget() );
    if ( !translator ) return;

    // Update the values
    UpdateBackupOption( ui->Option4_BackupOption->isChecked() );
    UpdateMinimize( ui->Option_MinimizeSave->isChecked() );

    translator->SaveOptions( ui->Option1_MainPath->text().toStdString(), ui->Option2_ExportPath->text().toStdString(), ui->Option3_SaveTimer->sliderPosition(), ui->Option4_BackupPath->text().toStdString(), ui->Option4_Backup_Timer->sliderPosition() );
}

void DialogOptions::accept()
{
    SaveConfig();
    close();
}
