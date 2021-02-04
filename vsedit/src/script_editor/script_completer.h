#ifndef SCRIPTCOMPLETER_H
#define SCRIPTCOMPLETER_H

#include <QCompleter>

class ScriptCompleter : public QCompleter
{
	Q_OBJECT

public:

	ScriptCompleter(QAbstractItemModel * a_pModel,
		QObject * a_pParent = nullptr);

    virtual ~ScriptCompleter() override;

    QString pathFromIndex(const QModelIndex & a_index) const override;

    QStringList splitPath(const QString & a_path) const override;
};

#endif // SCRIPTCOMPLETER_H
