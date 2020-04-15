#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
class Preferences;
}

//! Prefences Window creation and configuration Class
/*!
 * Contains the application preferences which contains system-wide configuration attributes.
 * This will be built over time as more features are introduced, but for now can be an empty
 * or semi-empty window.
 *
 * \inherits QDialog
 * \inmodule SundayBeacon
 * \ingroup ui
 * \since 1
 */
class Preferences : public QDialog
{
    Q_OBJECT

public:
    //! The Preferences::Preferences Class Constructor
    /*!
     * Located under the program menu on Mac OS X, or under Edit menu on Linux or Windows systems
     * (possibly though it may be placed under a Settings menu)
     *
     * \param parent
     * \since 1
     */
    explicit Preferences(QWidget *parent = 0);
    //! The Preferences::Preferences Class Deconstructor
    /*!
     * \since 1
     */
    ~Preferences();

private:
    Ui::Preferences *ui;
};

#endif // PREFERENCES_H
