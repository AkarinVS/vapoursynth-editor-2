#ifndef GENERICTABWIDGET_H
#define GENERICTABWIDGET_H

#include <QObject>
#include <QTabWidget>

class GenericTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    GenericTabWidget(QWidget *parent = nullptr);


private:

    int m_currentSelectedIndex;

signals:

    void signalAboutToChanged(int currentTabIndex, int selectedTabIndex);

private slots:

    void aboutToChangeTab(int index);
};

#endif // GENERICTABWIDGET_H
