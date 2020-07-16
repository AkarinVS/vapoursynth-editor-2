#include "generic_tab_widget.h"
#include <QDebug>

GenericTabWidget::GenericTabWidget(QWidget *parent)
{
    m_currentSelectedIndex = 0;
    connect(this, &GenericTabWidget::currentChanged, this, &GenericTabWidget::aboutToChangeTab);
}

void GenericTabWidget::aboutToChangeTab(int index)
{
    if (m_currentSelectedIndex != index && index >= 0) {
        emit signalAboutToChanged(m_currentSelectedIndex, index);
        m_currentSelectedIndex = index;
    }
    return;
}
