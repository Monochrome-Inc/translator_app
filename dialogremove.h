#ifndef DIALOGREMOVE_H
#define DIALOGREMOVE_H

#include <QDialog>
#include "json/json.h"

namespace Ui {
class DialogRemove;
}

class DialogRemove : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRemove(QWidget *parent = nullptr);
    ~DialogRemove();
    void SetText( QString strText );
    void SetDialogValues( Json::Value jdata, bool bLang );

public slots:
    void accept();

private:
    Ui::DialogRemove *ui;
    bool bIsLang;
};

#endif // DIALOGREMOVE_H
