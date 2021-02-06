#ifndef SPINBOXEXTENDEDLINEEDIT_H
#define SPINBOXEXTENDEDLINEEDIT_H

#include <QLineEdit>
#include <QMouseEvent>

class SpinboxExtendedLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit SpinboxExtendedLineEdit(QWidget *parent = nullptr);

protected:    
    virtual void mousePressEvent(QMouseEvent *a_pEvent) override;
};

#endif // SPINBOXEXTENDEDLINEEDIT_H
