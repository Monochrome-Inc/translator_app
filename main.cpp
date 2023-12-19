#include "translator.h"
#include <QApplication>
#include <QMessageBox>

#include "steam/steam_api.h"

int main(int argc, char *argv[])
{
    if ( SteamAPI_Init() )
    {
        QApplication a(argc, argv);
        Translator w;
        w.show();

        return a.exec();
    }

    int iErr = QMessageBox::warning( nullptr, "SteamAPI Error", "Steam API couldn't initialize!\nMake sure Steam is running!", QMessageBox::Ok, QMessageBox::Ok);
    return iErr;
}
