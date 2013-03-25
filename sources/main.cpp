#include <QtGui/QApplication>

#include "selectprofilesdlg.hpp"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QObject::trUtf8("On A Profile"));
    app.setWindowIcon(QIcon("://onaprofile.png"));

    CSelectProfilesDlg selprofilesdlg;
    return selprofilesdlg.exec();
}
