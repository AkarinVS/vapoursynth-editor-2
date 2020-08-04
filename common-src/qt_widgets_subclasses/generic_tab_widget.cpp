#include "generic_tab_widget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QTabBar>

GenericTabWidget::GenericTabWidget(QWidget *parent)
{
    m_currentSelectedIndex = 0;
    connect(this, &GenericTabWidget::currentChanged, this, &GenericTabWidget::aboutToChangeTab);
}

void GenericTabWidget::mousePressEvent(QMouseEvent * a_pEvent)
{
    int index = tabBar()->tabAt(a_pEvent->pos());

    if (a_pEvent->button() == Qt::RightButton)
        emit tabBarRightClicked(index); // signal for right click
    else
    if (a_pEvent->button() == Qt::MiddleButton)
        emit tabBarMiddleClicked(index);
    else
        GenericTabWidget::mousePressEvent(a_pEvent);
}

void GenericTabWidget::aboutToChangeTab(int index)
{
    if (m_currentSelectedIndex != index && index >= 0) {
        emit signalAboutToChanged(m_currentSelectedIndex, index);
        m_currentSelectedIndex = index;
    }
    return;
}
