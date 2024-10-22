
#include "dialogmodify.h"
#include "ui_dialogmodify.h"
#include "translator.h"

DialogModify::DialogModify(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogModify)
{
    bIsLangKey = false;
    ui->setupUi(this);
    ui->lineEdit->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
}

DialogModify::~DialogModify()
{
    QSize WindowSize = size();
    JsonConfig["dialog_modify"]["width"] = WindowSize.width();
    JsonConfig["dialog_modify"]["height"] = WindowSize.height();
    delete ui;
}

void DialogModify::Setup(Json::Value jdata)
{
    ui->comboBox->clear();
    std::vector<std::string> jdatagroups = jdata.getMemberNames();
    for (const auto &jvalue : jdatagroups)
    {
        std::string str = jvalue;
        if ( bIsLangKey )
            str.replace( 0, 5, "" );
        ui->comboBox->addItem( QString::fromStdString( str ) );
    }

    // Set the string
    OldString = ui->comboBox->itemText( ui->comboBox->currentIndex() );
    ui->lineEdit->setText( OldString );
}

void DialogModify::accept()
{
    Translator *translator = dynamic_cast<Translator *>( parentWidget() );
    if ( !translator ) return;
    translator->OnItemModified( OldString, ui->lineEdit->text(), bIsLangKey ),
    close();
}

void DialogModify::OnItemChanged( int slot )
{
    // Set the string
    OldString = ui->comboBox->itemText( slot );
    ui->lineEdit->setText( OldString );
}
