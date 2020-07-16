#ifndef FRAME_INFO_DIALOG_H
#define FRAME_INFO_DIALOG_H

#include "../../common-src/settings/settings_manager.h"

#include <QDialog>

namespace Ui {
class FrameInfoDialog;
}

class FrameInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FrameInfoDialog(SettingsManager * a_pSettingsManager, QWidget *parent = nullptr);
    ~FrameInfoDialog();

    void setColorPickerString(const QString & a_string);
    void setMousePositionString(const QString & a_string);
private:
    Ui::FrameInfoDialog *ui;

    SettingsManager * m_pSettingsManager;

protected:

    void moveEvent(QMoveEvent * a_pEvent) override;
    void hideEvent(QHideEvent * a_pEvent) override;

    void setWindowGeometry();
    void saveGeometryDelayed();

    QTimer * m_pGeometrySaveTimer;
    QByteArray m_windowGeometry;

protected slots:

    void slotSaveGeometry();
};

#endif // FRAME_INFO_DIALOG_H
