#include "generic_spinbox.h"

GenericSpinBox::GenericSpinBox(QWidget *parent)
{
    a_pLineEdit = new SpinboxExtendedLineEdit(this);
    setLineEdit(a_pLineEdit);
}

void GenericSpinBox::setFont(const QFont & a_font)
{
    a_pLineEdit->setFont(a_font);
}
