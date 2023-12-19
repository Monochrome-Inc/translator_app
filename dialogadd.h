#ifndef DIALOGADD_H
#define DIALOGADD_H

#include <QDialog>

namespace Ui {
    class DialogAdd;
}

class DialogAdd : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAdd(QWidget *parent = nullptr);
    ~DialogAdd();

    void SetVariables(int tableid, std::string lang, std::string key, std::string value);

public slots:
    void accept();

private:
    Ui::DialogAdd *ui;
    int iTableID;
    std::string strKey;
    std::string strValue;
    std::string strLang;
};

#endif // DIALOGADD_H
