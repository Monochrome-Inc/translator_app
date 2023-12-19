#ifndef DIALOGADDPLAIN_H
#define DIALOGADDPLAIN_H

#include <QDialog>

namespace Ui {
class DialogAddPlain;
}

class DialogAddPlain : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddPlain(QWidget *parent = nullptr);
    ~DialogAddPlain();
    void SetText( QString strText );
    void IsLanguage( bool state );

public slots:
    void accept();

private:
    Ui::DialogAddPlain *ui;
    bool bIsLang;
};

#endif // DIALOGADDPLAIN_H
