#ifndef SELECTION_TOOLS_DIALOG_H
#define SELECTION_TOOLS_DIALOG_H

//#include "canvas.h"
#include <QDialog>
#include <QGraphicsScene>

namespace Ui {
class SelectionToolsDialog;
}

enum SelectionTools {
    Rectangle,
    LazyRectangle
};

class SelectionToolsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectionToolsDialog(QWidget *a_pParent = nullptr);
    ~SelectionToolsDialog() override;

    void setFramePixmap(const QPixmap&);

protected:
    void hideEvent(QHideEvent * a_pEvent) override;

private:
    Ui::SelectionToolsDialog *ui;

    SelectionTools m_selectionTool;


signals:
    void signalLoadPixmapRequested();
    void signalDialogHidden();
    void signalPasteSelectionPointsString(const QString &);

public slots:

    void slotCloseDialog();
    void slotSendSignalPastePointsToScript();

};

#endif // SELECTION_TOOLS_DIALOG_H
