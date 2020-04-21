#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QAction>

#include "about.h"
#include "preferences.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QMenu *mainMenu;
    QMenuBar *mainMenuBar;
    QAction *aboutAction;
    QAction *preferencesAction;
    About *aboutWindow;
    Preferences *preferencesWindow;
    QStatusBar *statusBar;
};

#endif // MAINWINDOW_H
