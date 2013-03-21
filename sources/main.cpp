#include <QtGui/QApplication>

#include "selectprofilesdlg.hpp"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("On A Profile");

    CSelectProfilesDlg selprofilesdlg;
    return selprofilesdlg.exec();
}
