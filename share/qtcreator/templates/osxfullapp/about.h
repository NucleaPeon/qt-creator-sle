#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

/*!
 * \brief The About class builds and configures the About window
 *
 * The About window contains things such as copyright, version and contributor information.
 * On Mac OS X it is contained beneath the program menu in the top bar, whereas on Windows
 * and Linux it should be located under the Help menu.
 *
 * \inherits QDialog
 * \inmodule SundayBeacon
 * \ingroup ui
 * \since 1
 */
class About : public QDialog
{
    Q_OBJECT

public:\
    //! Constructor for the About::About Window, no parent by default
    /*!
     * \param parent
     * \since 1
     */
    explicit About(QWidget *parent = 0);
    //! Deconstructor for the About::About Window
    /*!
     * \since 1
     */
    ~About();

private:
    //! About page ui
    Ui::About *ui;
};

#endif // ABOUT_H
