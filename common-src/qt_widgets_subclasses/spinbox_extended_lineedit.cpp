#include "spinbox_extended_lineedit.h"

SpinboxExtendedLineEdit::SpinboxExtendedLineEdit(QWidget *parent)
{

}

void SpinboxExtendedLineEdit::mousePressEvent(QMouseEvent *a_pEvent)
{
    if (a_pEvent->button() == Qt::LeftButton) {
        selectAll();
        a_pEvent->accept();
        return;
    }
    QLineEdit::mousePressEvent(a_pEvent);
}
