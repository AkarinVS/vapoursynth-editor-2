#include "generic_stringlist_model.h"

GenericStringListModel::GenericStringListModel()
{

}

void GenericStringListModel::append(const QString &a_string)
{
    if (insertRow(rowCount())) {
        QModelIndex index = this->index(rowCount() - 1, 0);
        setData(index, a_string);
    }
}

void GenericStringListModel::removeOne(QString &a_string)
{
    int index = stringList().indexOf(a_string);
    removeRow(index);
}
