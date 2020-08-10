#ifndef PREVIEW_FILTERS_DIALOG_H
#define PREVIEW_FILTERS_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QButtonGroup>

class SettingsManager;

namespace Ui {
class PreviewFiltersDialog;
}

class PreviewFiltersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewFiltersDialog(SettingsManager * a_pSettingsManager,
                                  QWidget *parent = nullptr);
    ~PreviewFiltersDialog() override;

private:
    Ui::PreviewFiltersDialog *ui;

    SettingsManager * m_pSettingsManager;

    QMap<QString, int> m_previewFiltersMap;

    QButtonGroup * m_pChannelsButtonGroup;

    void setFilterButtons();

protected:

    void moveEvent(QMoveEvent * a_pEvent) override;
    void hideEvent(QHideEvent * a_pEvent) override;

    void saveGeometryDelayed();
    void setWindowGeometry();

    QTimer * m_pGeometrySaveTimer;
    QByteArray m_windowGeometry;

signals:

    void signalChannelsButtonClicked(int);
    void signalDialogHidden();
    void signalPreviewFiltersChanged(const QMap<QString, int> a_previewFiltersMap);

public slots:

    void slotSendPreviewFiltersMapSignal();
    void slotUpdateDisplay(const QMap<QString,int>&);

private slots:

    void slotUpdateChannels(int);

protected slots:

    void slotSaveGeometry();
};

#endif // PREVIEW_FILTERS_DIALOG_H
