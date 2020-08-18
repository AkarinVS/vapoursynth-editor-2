#include "selection_tools_dialog.h"
#include "ui_selection_tools_dialog.h"
//#include "canvas.h"

#include <QDebug>

SelectionToolsDialog::SelectionToolsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectionToolsDialog)
{
    ui->setupUi(this);

    connect(ui->loadPixmapButton, &QPushButton::clicked,
        [=]() { emit signalLoadPixmapRequested(); });

    connect(ui->lazyRectSelectButton, &QToolButton::clicked,
        [=]() { ui->canvas->slotSetSelectionTool(SelectionTools::LazyRectangle); });

    connect(ui->closeButton, &QPushButton::clicked,
            this, &SelectionToolsDialog::slotCloseDialog);

    connect(ui->pasteButton, &QPushButton::clicked,
        this, &SelectionToolsDialog::slotSendSignalPastePointsToScript);
}

SelectionToolsDialog::~SelectionToolsDialog()
{
    delete ui;
}

void SelectionToolsDialog::setFramePixmap(const QPixmap &a_framePixmap)
{
    ui->canvas->drawFrame(a_framePixmap);
}

void SelectionToolsDialog::hideEvent(QHideEvent *a_pEvent)
{
    emit signalDialogHidden();
    QDialog::hideEvent(a_pEvent);
}

void SelectionToolsDialog::slotCloseDialog()
{
    this->hide();
}

void SelectionToolsDialog::slotSendSignalPastePointsToScript()
{
    QString pointsString = ui->canvas->selectionXYToString();
    emit signalPasteSelectionPointsString(pointsString);
}
