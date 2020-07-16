#ifndef COLLAPSEEXPANDWIDGET_H
#define COLLAPSEEXPANDWIDGET_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>

/*
 * code reference:
 * https://stackoverflow.com/questions/32476006/how-to-make-an-expandable-collapsable-section-widget-in-qt
 *
 */

class CollapseExpandWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CollapseExpandWidget(QWidget *parent = nullptr);

private:
    QGridLayout mainLayout;
    QToolButton toggleButton;
    QFrame headerLine;
    QParallelAnimationGroup toggleAnimation;
    QScrollArea contentArea;
    int animationDuration{300};

    QString title;

public:
    void setContentLayout(QLayout & contentLayout);

    void setTitle(const QString & a_title);
    void setAnimationDuration(const int a_animationDuration);


signals:

};

#endif // COLLAPSEEXPANDWIDGET_H
