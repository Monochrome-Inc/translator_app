#include "dialogaddplain.h"
#include "ui_dialogaddplain.h"
#include "translator.h"

DialogAddPlain::DialogAddPlain(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddPlain)
{
    ui->setupUi(this);
    ui->lineEdit->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
    bIsLang = false;
    bIsNewTranslationFile = false;
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

void DialogAddPlain::SetDefaultTextLine(QString strText)
{
    ui->lineEdit->setText( strText );
}

void DialogAddPlain::IsLanguage(bool state)
{
    bIsLang = state;
}

void DialogAddPlain::accept()
{
    Translator *translator = dynamic_cast<Translator *>( parentWidget() );
    if ( !translator ) return;
    if ( bIsNewTranslationFile )
        translator->OnNewTranslationCreated( ui->lineEdit->text() );
    else
        translator->AddItem( ui->lineEdit->text().toStdString(), bIsLang );
    close();
}
