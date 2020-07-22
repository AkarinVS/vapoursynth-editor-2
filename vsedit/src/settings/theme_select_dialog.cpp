#include "theme_select_dialog.h"
#include "ui_theme_select_dialog.h"

#include <QDebug>

ThemeSelectDialog::ThemeSelectDialog(QStringList a_pThemePresetList,
                                     ThemeSelectionFlag a_flag, QWidget *a_pParent) :
    QDialog(a_pParent),
    ui(new Ui::ThemeSelectDialog),
    m_pThemePresetList(a_pThemePresetList),
    m_flag(a_flag)
{
    ui->setupUi(this);

    for (auto &themeName : m_pThemePresetList) {
        QListWidgetItem *item = new QListWidgetItem(themeName);
        item->setCheckState(Qt::Unchecked);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        ui->themeSelectListWidget->addItem(item);
    }

    QString flag = m_flag == ThemeSelectionFlag::Import? "import" : "export";
    QString title = QString("Select themes to %1").arg(flag);
    ui->titleLabel->setText(title);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ThemeSelectDialog::slotOk);
}

ThemeSelectDialog::~ThemeSelectDialog()
{
    delete ui;
}

void ThemeSelectDialog::slotOk()
{
    QStringList selectedList;
    for(int i = 0; i < ui->themeSelectListWidget->count(); ++i) {
        QListWidgetItem * item = ui->themeSelectListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedList.append(item->text());
        }
    }
    if (m_flag == ThemeSelectionFlag::Import){
        emit signalImportSelectedList(selectedList);
    } else {
        emit signalExportSelectedList(selectedList);
    }

    close();
}
