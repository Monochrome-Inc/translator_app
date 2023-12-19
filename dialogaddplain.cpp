#include "dialogaddplain.h"
#include "ui_dialogaddplain.h"
#include "translator.h"

DialogAddPlain::DialogAddPlain(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddPlain)
{
    ui->setupUi(this);
    bIsLang = false;
}

DialogAddPlain::~DialogAddPlain()
{
    QSize WindowSize = size();
    JsonConfig["dialog_add"]["width"] = WindowSize.width();
    JsonConfig["dialog_add"]["height"] = WindowSize.height();
    delete ui;
}

void DialogAddPlain::SetText(QString strText)
{
    ui->label->setText( strText );
}

void DialogAddPlain::IsLanguage(bool state)
{
    bIsLang = state;
}

void DialogAddPlain::accept()
{
    Translator *translator = dynamic_cast<Translator *>( parentWidget() );
    if ( !translator ) return;
    translator->AddItem( ui->lineEdit->text().toStdString(), bIsLang );
    close();
}
