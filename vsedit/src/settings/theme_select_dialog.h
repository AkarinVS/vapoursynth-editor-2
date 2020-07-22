#ifndef THEME_SELECT_DIALOG_H
#define THEME_SELECT_DIALOG_H

#include "../../common-src/settings/settings_definitions.h"

#include <QDialog>

namespace Ui {
class ThemeSelectDialog;
}

class ThemeSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThemeSelectDialog(QStringList a_pThemePresetList,
                               ThemeSelectionFlag a_flag, QWidget *parent = nullptr);
    ~ThemeSelectDialog();

signals:

    void signalImportSelectedList(QStringList & a_selectedlist);

    void signalExportSelectedList(QStringList & a_selectedlist);

private:
    Ui::ThemeSelectDialog *ui;

    QStringList m_pThemePresetList;

    ThemeSelectionFlag m_flag;

private slots:

    void slotOk();
};

#endif // THEME_SELECT_DIALOG_H
