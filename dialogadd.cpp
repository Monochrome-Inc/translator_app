
#include "dialogadd.h"
#include "ui_dialogadd.h"
#include "translator.h"

DialogAdd::DialogAdd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAdd)
{
    ui->setupUi(this);
}

DialogAdd::~DialogAdd()
{
    QSize WindowSize = size();
    JsonConfig["dialog_translationvalue"]["width"] = WindowSize.width();
    JsonConfig["dialog_translationvalue"]["height"] = WindowSize.height();
    delete ui;
}

void DialogAdd::SetVariables(int tableid, std::string lang, std::string key, std::string value)
{
    // Set the values
    iTableID = tableid;
    strKey = key;
    strLang = lang;
    strValue = value;

    // Set the string
    ui->plainTextEdit->setPlainText( QString::fromStdString( strValue ) );
}

void DialogAdd::accept()
{
    Translator *translator = dynamic_cast<Translator *>( parentWidget() );
    if ( !translator ) return;
    translator->SaveTableValue( iTableID, strLang, strKey, ui->plainTextEdit->toPlainText() );
    close();
}
