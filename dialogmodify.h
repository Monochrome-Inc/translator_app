#ifndef DIALOGMODIFY_H
#define DIALOGMODIFY_H

#include <QDialog>
#include "json/json.h"

namespace Ui {
    class DialogModify;
}

class DialogModify : public QDialog
{
    Q_OBJECT

public:
    explicit DialogModify(QWidget *parent = nullptr);
    ~DialogModify();

    void SetIsLangItem(bool state) { bIsLangKey = state; }
    void Setup(Json::Value jdata);

public slots:
    void accept();
    void OnItemChanged(int slot);

private:
    Ui::DialogModify *ui;
    QString OldString;
    bool bIsLangKey;
};

#endif // DIALOGMODIFY_H
