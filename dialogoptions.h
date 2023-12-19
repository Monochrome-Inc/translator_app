#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptions(QWidget *parent = nullptr);
    ~DialogOptions();
    void SetDefault( std::string gamepath, std::string backuppath, int timer );
    void ReadConfig();
    void SaveConfig();

public slots:
    void accept();
    void ResetToDefult();
    void ChooseMain();
    void ChooseExport();
    void ChooseBackup();
    void UpdateBackupOption( bool state );
    void UpdateMinimize( bool state );

private:
    Ui::DialogOptions *ui;
    std::string strDefault_MainPath;
    std::string strDefault_ExportPath;
    std::string strDefault_BackupPath;
    int iDefault_SaveTimer;
    bool bDefault_Minimize;
    bool bDefault_BackupOption;
};

#endif // DIALOGOPTIONS_H
