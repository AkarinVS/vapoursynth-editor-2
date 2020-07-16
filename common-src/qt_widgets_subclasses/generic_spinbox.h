#ifndef GENERICSPINBOX_H
#define GENERICSPINBOX_H

#include <QSpinBox>
#include "spinbox_extended_lineedit.h"


class GenericSpinBox : public QSpinBox
{   
    Q_OBJECT
public:
    GenericSpinBox(QWidget *parent = nullptr);

    void setFont(const QFont & a_font);

private:
    SpinboxExtendedLineEdit * a_pLineEdit;
};

#endif // GENERICSPINBOX_H
