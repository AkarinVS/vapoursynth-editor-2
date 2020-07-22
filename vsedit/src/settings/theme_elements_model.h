#ifndef THEME_ELEMENTS_MODEL_H_INCLUDED
#define THEME_ELEMENTS_MODEL_H_INCLUDED

#include "../../../common-src/settings/settings_definitions.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <vector>

class SettingsManager;

enum class ThemeElementType
{
    TextCharFormat,
    NonTextCharFormat
};

struct ThemeElementData
{
    QString id;
	ThemeElementType type;	
	QString text;
	QIcon icon;
    QTextCharFormat textCharFormat;
	QColor color;
};

typedef QVector<ThemeElementData> ThemeElementsList;

class ThemeElementsModel : public QAbstractItemModel
{
	Q_OBJECT

public:

    ThemeElementsModel(SettingsManager * a_pSettingsManager, QString a_themeName,
		QObject * a_pParent = nullptr);

    virtual ~ThemeElementsModel() override;

	QModelIndex index(int a_row, int a_column,
		const QModelIndex & a_parent = QModelIndex()) const override;

	QModelIndex parent(const QModelIndex & a_child) const override;

	Qt::ItemFlags flags(const QModelIndex & a_index) const override;

	QVariant data(const QModelIndex & a_index, int a_role = Qt::DisplayRole)
		const override;

	int rowCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	int columnCount(const QModelIndex & a_parent = QModelIndex()) const
		override;

	bool setData(const QModelIndex & a_index, const QVariant & a_value,
		int a_role = Qt::EditRole) override;

	void addThemeElement(const ThemeElementData & a_themeElementData);

    void addTextCharFormat(const QString & a_id, const QString & a_text,
                           const QTextCharFormat & a_charFormat);

    void addNonTextCharFormat(const QString & a_id, const QString & a_text,
                              const QColor & a_color);

    static ThemeElementsList getThemeFromListStringByName (QString &a_themeListString, const QString &a_themeName);

    static QString removeThemeFromListString (QString &a_themeListString, const QString &a_themeName);

    ThemeElementData getThemeElementData(const QString & a_id);

    bool setThemeElementData (const QString & a_id, ThemeElementData & a_themeElementData);

    void fromThemeElementsList (const ThemeElementsList & a_themeElementsList);

    ThemeElementsList toThemeElementsList();

    QString themeName();

    void clear();

public slots:

//	void slotSaveThemeSettings();

private:

	ThemeElementsList m_themeElementsList;

	SettingsManager * m_pSettingsManager;

    QString m_themeName;
};

#endif // THEME_ELEMENTS_MODEL_H_INCLUDED
