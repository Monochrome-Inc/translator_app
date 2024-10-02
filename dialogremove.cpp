#include "dialogremove.h"
#include "ui_dialogremove.h"
#include "translator.h"

DialogRemove::DialogRemove(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRemove)
{
    ui->setupUi(this);
    bIsLang = false;
}

DialogRemove::~DialogRemove()
{
    QSize WindowSize = size();
    JsonConfig["dialog_remove"]["width"] = WindowSize.width();
    JsonConfig["dialog_remove"]["height"] = WindowSize.height();
    delete ui;
}

void DialogRemove::SetText(QString strText)
{
    ui->label->setText( strText );
}

void DialogRemove::SetDialogValues(Json::Value jdata, bool bLang)
{
    bIsLang = bLang;
    ui->comboBox->clear();
    std::vector<std::string> jdatagroups = jdata.getMemberNames();
    for (const auto &jvalue : jdatagroups)
    {
        std::string str = jvalue;
        if ( bLang )
        {
            str.replace( 0, 5, "" );
            if ( str == "english" ) continue;
        }
        ui->comboBox->addItem( QString::fromStdString( str ) );
    }
}

void DialogRemove::accept()
{
    Translator *translator = (Translator *)parentWidget();
    if ( !translator ) return;
    translator->RemoveItem( ui->comboBox->itemText( ui->comboBox->currentIndex() ).toStdString(), bIsLang );
    close();
}
