#include "frame_info_dialog.h"
#include "ui_frame_info_dialog.h"

#include <QTimer>

FrameInfoDialog::FrameInfoDialog(SettingsManager * a_pSettingsManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameInfoDialog),
    m_pSettingsManager(a_pSettingsManager)
{
    ui->setupUi(this);
    setWindowGeometry();
}

FrameInfoDialog::~FrameInfoDialog()
{
    if(m_pGeometrySaveTimer->isActive())
    {
        m_pGeometrySaveTimer->stop();
        slotSaveGeometry();
    }
}

void FrameInfoDialog::setColorPickerString(const QString &a_string)
{
    ui->colorPickerLabel->setText(a_string);
}

void FrameInfoDialog::setMousePositionString(const QString &a_string)
{
    ui->mousePositionLabel->setText(a_string);
}

void FrameInfoDialog::moveEvent(QMoveEvent *a_pEvent)
{
    QDialog::moveEvent(a_pEvent);
    saveGeometryDelayed();
}

void FrameInfoDialog::hideEvent(QHideEvent *a_pEvent)
{
    emit signalDialogHidden();
    QDialog::hideEvent(a_pEvent);
    saveGeometryDelayed();
}

void FrameInfoDialog::setWindowGeometry()
{
    m_pGeometrySaveTimer = new QTimer(this);
    m_pGeometrySaveTimer->setInterval(DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY);
    connect(m_pGeometrySaveTimer, &QTimer::timeout,
        this, &FrameInfoDialog::slotSaveGeometry);

    m_windowGeometry = m_pSettingsManager->getFrameInfoDialogGeometry();
    if(!m_windowGeometry.isEmpty())
        restoreGeometry(m_windowGeometry);
}

void FrameInfoDialog::saveGeometryDelayed()
{
    QApplication::processEvents();
    if(!isMaximized())
    {
        m_windowGeometry = saveGeometry();
        m_pGeometrySaveTimer->start();
    }
}

void FrameInfoDialog::slotSaveGeometry()
{
    m_pGeometrySaveTimer->stop();
    m_pSettingsManager->setFrameInfoDialogGeometry(m_windowGeometry);
}
