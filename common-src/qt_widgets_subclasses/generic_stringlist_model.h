#ifndef GENERICSTRINGLISTMODEL_H
#define GENERICSTRINGLISTMODEL_H

#include <QObject>
#include <QWidget>
#include <QStringListModel>

class GenericStringListModel : public QStringListModel
{
    Q_OBJECT
public:
    GenericStringListModel();

    void append(const QString &a_string);

    void removeOne(QString & a_string);
};

#endif // GENERICSTRINGLISTMODEL_H
