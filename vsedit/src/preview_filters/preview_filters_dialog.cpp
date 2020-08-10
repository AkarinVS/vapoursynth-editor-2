#include "preview_filters_dialog.h"
#include "ui_preview_filters_dialog.h"
#include "../../common-src/settings/settings_manager.h"

#include <QTimer>
#include <QDebug>

PreviewFiltersDialog::PreviewFiltersDialog(SettingsManager * a_pSettingsManager,
                                           QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewFiltersDialog),
    m_pSettingsManager(a_pSettingsManager),
    m_previewFiltersMap({{"channels", -2}})
{
    ui->setupUi(this);

    setWindowGeometry();

    m_pChannelsButtonGroup = new QButtonGroup(this);
    m_pChannelsButtonGroup->addButton(ui->offChannelButton, -2);
    m_pChannelsButtonGroup->addButton(ui->YChannelButton, 0);
    m_pChannelsButtonGroup->addButton(ui->CbChannelButton, 1);
    m_pChannelsButtonGroup->addButton(ui->CrChannelButton, 2);

    connect(m_pChannelsButtonGroup, &QButtonGroup::idClicked,
            this, &PreviewFiltersDialog::slotUpdateChannels);
}

PreviewFiltersDialog::~PreviewFiltersDialog()
{
    delete ui;
}

void PreviewFiltersDialog::setFilterButtons()
{
    /* slot for setting button only, don't emit signal */
    QAbstractButton * channelsButton = m_pChannelsButtonGroup->button(m_previewFiltersMap["channels"]);

    if (channelsButton != nullptr) {
        blockSignals(true);
        channelsButton->setChecked(true);
        blockSignals(false);
    }
}

void PreviewFiltersDialog::moveEvent(QMoveEvent *a_pEvent)
{
    QDialog::moveEvent(a_pEvent);
    saveGeometryDelayed();
}

void PreviewFiltersDialog::hideEvent(QHideEvent *a_pEvent)
{
    emit signalDialogHidden();
    QDialog::hideEvent(a_pEvent);        
    saveGeometryDelayed();
}

void PreviewFiltersDialog::saveGeometryDelayed()
{
    QApplication::processEvents();
    if(!isMaximized())
    {
        m_windowGeometry = saveGeometry();
        m_pGeometrySaveTimer->start();
    }
}

void PreviewFiltersDialog::setWindowGeometry()
{
    m_pGeometrySaveTimer = new QTimer(this);
    m_pGeometrySaveTimer->setInterval(DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY);
    connect(m_pGeometrySaveTimer, &QTimer::timeout,
        this, &PreviewFiltersDialog::slotSaveGeometry);

    m_windowGeometry = m_pSettingsManager->getPreviewFiltersDialogGeometry();
    if(!m_windowGeometry.isEmpty())
        restoreGeometry(m_windowGeometry);
}

void PreviewFiltersDialog::slotSendPreviewFiltersMapSignal()
{
    emit signalPreviewFiltersChanged(m_previewFiltersMap);
}

void PreviewFiltersDialog::slotUpdateDisplay(const QMap<QString, int> & a_previewFiltersMap)
{
    if (a_previewFiltersMap != m_previewFiltersMap) {
        m_previewFiltersMap = a_previewFiltersMap;
        setFilterButtons();
    }
}

void PreviewFiltersDialog::slotUpdateChannels(int a_id)
{
    QAbstractButton * channelsBtn = m_pChannelsButtonGroup->button(a_id);
    channelsBtn->setChecked(true);

    m_previewFiltersMap["channels"] = a_id;
    slotSendPreviewFiltersMapSignal();
}

void PreviewFiltersDialog::slotSaveGeometry()
{
    m_pGeometrySaveTimer->stop();
    m_pSettingsManager->setPreviewFiltersDialogGeometry(m_windowGeometry);
}
