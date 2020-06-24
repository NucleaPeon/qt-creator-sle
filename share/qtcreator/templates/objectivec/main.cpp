#include "%INCLUDE%"
#include <%QAPP_INCLUDE%>

#ifdef Q_OS_MAC
#include "cocoainitializer.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef Q_OS_MAC
    CocoaInitializer initializer;
#endif
    %CLASS% w;
%SHOWMETHOD%
    return a.exec();
}
