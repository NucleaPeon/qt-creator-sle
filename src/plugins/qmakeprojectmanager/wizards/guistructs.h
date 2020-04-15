#ifndef GUISTRUCTS_H
#define GUISTRUCTS_H

#include <QString>

namespace QmakeProjectManager {
namespace Internal {

struct GuiAppParameters
{
    GuiAppParameters();
    QString className;
    QString baseClassName;
    QString sourceFileName;
    QString headerFileName;
    QString formFileName;
    int widgetWidth;
    int widgetHeight;
    bool designerForm;
    bool isMobileApplication;
};

}
}
#endif // GUISTRUCTS_H
