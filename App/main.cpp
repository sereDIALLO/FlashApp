#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include  "mainwindow.h"
#include <QSharedMemory>
#include <QScreen>
// Function for execute app only one time.
bool isOnlyInstance()
{
    static QSharedMemory shareMemory;
    shareMemory.setKey("UniqueApplicationKey");
    if(shareMemory.attach())
    {
        // une autre instance est déjà en cours d'exécution
        return false;
    }
    if(!shareMemory.create(1))
    {
        // Impossible de créer le segment de mémoire partagé
        //Une meme zone de mémoire est acceder par plusieurs processus.
        return false;
    }
    return true;
}
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    if(!isOnlyInstance())
    {
        qWarning() << "Another instance of this application is already running;";

        return 0;
    }
    MainWindow flash;

    flash.setWindowTitle("FLASH BIOCORP");
    flash.setWindowIcon(QIcon("../logo.png"));
    // To set my application to start in full screen.
    //flash.setWindowState(Qt::WindowFullScreen);
    //centrer la fenetre.
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - flash.width()) / 2;
    int y = (screenGeometry.height() - flash.height()) / 2;
    flash.setGeometry(x,y, flash.width(), flash.height());

    flash.show();
    return QApplication::exec();
}
