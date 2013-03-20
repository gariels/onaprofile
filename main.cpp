#include <QtGui/QApplication>
#include "onaprofile.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    onaprofile foo;
    foo.show();
    return app.exec();
}
