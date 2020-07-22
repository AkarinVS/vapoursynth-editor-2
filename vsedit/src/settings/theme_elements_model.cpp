#include "theme_elements_model.h"

#include "../../../common-src/settings/settings_manager.h"

//==============================================================================

ThemeElementsModel::ThemeElementsModel(SettingsManager * a_pSettingsManager, QString a_themeName,
	QObject * a_pParent) : QAbstractItemModel(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
    , m_themeName(a_themeName)
{

}

// END OF ThemeElementsModel::ThemeElementsModel(
//		SettingsManager * a_pSettingsManager, QObject * a_pParent)
//==============================================================================

ThemeElementsModel::~ThemeElementsModel()
{

}

// END OF ThemeElementsModel::~ThemeElementsModel()
//==============================================================================

QModelIndex ThemeElementsModel::index(int a_row, int a_column,
	const QModelIndex & a_parent) const
{
	(void)a_parent;
	return createIndex(a_row, a_column);
}

// END OF QModelIndex ThemeElementsModel::index(int a_row, int a_column,
//		const QModelIndex & a_parent) const
//==============================================================================

QModelIndex ThemeElementsModel::parent(const QModelIndex & a_child) const
{
	(void)a_child;
	return QModelIndex();
}

// END OF QModelIndex ThemeElementsModel::parent(const QModelIndex & a_child)
//		const
//==============================================================================

Qt::ItemFlags ThemeElementsModel::flags(const QModelIndex & a_index) const
{
	if (!a_index.isValid())
	{
		return Qt::NoItemFlags;
	}

	Qt::ItemFlags cellFlags = Qt::NoItemFlags
		| Qt::ItemIsEnabled
		| Qt::ItemIsSelectable
	;

	return cellFlags;
}

// END OF Qt::ItemFlags ThemeElementsModel::flags(const QModelIndex & a_index)
//		const
//==============================================================================

QVariant ThemeElementsModel::data(const QModelIndex & a_index, int a_role) const
{
	if(!a_index.isValid())
		return QVariant();

    if((a_index.row() >= int(m_themeElementsList.size())) ||
		(a_index.column() >= 1))
		return QVariant();

	if(a_role == Qt::DecorationRole)
		return m_themeElementsList[a_index.row()].icon;
	else if((a_role == Qt::DisplayRole) ||
		(a_role == Qt::ToolTipRole))
		return m_themeElementsList[a_index.row()].text;
	else if(a_role == Qt::UserRole)
		return m_themeElementsList[a_index.row()].id;

	return QVariant();
}

// END OF QVariant ThemeElementsModel::data(const QModelIndex & a_index,
//		int a_role) const
//==============================================================================

int ThemeElementsModel::rowCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
    return int(m_themeElementsList.size());
}

// END OF int ThemeElementsModel::rowCount(const QModelIndex & a_parent) const
//==============================================================================

int ThemeElementsModel::columnCount(const QModelIndex & a_parent) const
{
	(void)a_parent;
	return 2;
}

// END OF int ThemeElementsModel::columnCount(const QModelIndex & a_parent)
//		const
//==============================================================================

bool ThemeElementsModel::setData(const QModelIndex & a_index,
	const QVariant & a_value, int a_role)
{
	(void)a_index;
	(void)a_value;
	(void)a_role;
	return false;
}

// END OF bool ThemeElementsModel::setData(const QModelIndex & a_index,
//		const QVariant & a_value, int a_role)
//==============================================================================

void ThemeElementsModel::addThemeElement(
	const ThemeElementData & a_themeElementData)
{
    for(int i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].id == a_themeElementData.id)
			return;
	}

	m_themeElementsList.push_back(a_themeElementData);
	emit layoutChanged();
}

// END OF void ThemeElementsModel::addThemeElement(
//		const ThemeElementData & a_themeElementData)
//==============================================================================

void ThemeElementsModel::addTextCharFormat(const QString & a_id,
    const QString & a_text, const QTextCharFormat & a_charFormat)
{
    ThemeElementData newThemeElementData;
    newThemeElementData.id = a_id;
    newThemeElementData.type = ThemeElementType::TextCharFormat;
    newThemeElementData.text = a_text;
    newThemeElementData.icon = QIcon(QString(":font.png"));
    newThemeElementData.textCharFormat = a_charFormat;

    addThemeElement(newThemeElementData);
}

// END OF void ThemeElementsModel::addTextCharFormat(const QString & a_id,
//		const QString & a_text)
//==============================================================================

void ThemeElementsModel::addNonTextCharFormat(const QString & a_id,
    const QString & a_text, const QColor & a_color)
{
    ThemeElementData newThemeElementData;
    newThemeElementData.id = a_id;
    newThemeElementData.type = ThemeElementType::NonTextCharFormat;
    newThemeElementData.text = a_text;
    newThemeElementData.icon = QIcon(QString(":color_swatch.png"));
    newThemeElementData.color = a_color;
    addThemeElement(newThemeElementData);
}

ThemeElementsList ThemeElementsModel::getThemeFromListStringByName (QString &a_themeListString, const QString &a_themeName)
{
    QTextStream in(&a_themeListString);
    in.seek(0);
    ThemeElementsList elementlist;
    ThemeElementData themeElement;
    QString capturedThemeName("");
    bool matchOne = false;
    bool endRead = false;

    QRegularExpression reThemeHeader("^\\[(.+)\\]"); // match for [preset name]

    /* match for element with textCharFormat string  [a; 0; c; textChar] */
    QRegularExpression reTextCharElements("^([\\w\\d\\s]+)\\s*;\\s*([0]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*([\\w\\d\\s,-|#]+)$");

    /* match for element with nonTextCharFormat string  [a; 1; c; #ffffff] */
    QRegularExpression reNonTextElements("^([\\w\\d\\s]+)\\s*;\\s*([1]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*(#[a-z0-9]{6})\\s*$");

    while (!in.atEnd()) {
        if (endRead) break;

        QString line = in.readLine();
        QRegularExpressionMatch matchThemeHeader = reThemeHeader.match(line);
        QRegularExpressionMatch matchTextCharElements = reTextCharElements.match(line);
        QRegularExpressionMatch matchNonTextCharElements = reNonTextElements.match(line);

        if (matchTextCharElements.hasMatch()) {
            if (a_themeName == capturedThemeName) {
                // trim the captures
                QStringList capturedElement = matchTextCharElements.capturedTexts()
                        .replaceInStrings(QRegExp("^\\s+|\\s+$"), "");
                themeElement.id = capturedElement[1];
                themeElement.type = ThemeElementType(capturedElement[2].toInt());
                themeElement.text = capturedElement[3];

                // split charformat string
                QStringList textCharStringsList = (capturedElement[4].split("|"))
                        .replaceInStrings(QRegExp("^\\s+|\\s+$"), "");

                QTextCharFormat charFormat;
                QFont font;
                font.fromString(textCharStringsList[0]);
                charFormat.setFont(font);
                charFormat.setForeground(QColor(textCharStringsList[1]));
                charFormat.setFontWeight(textCharStringsList[2].toInt());

                themeElement.textCharFormat = charFormat;
                themeElement.icon = QIcon(QString(":font.png"));

                elementlist.append(themeElement);
            }
        }

        if (matchNonTextCharElements.hasMatch()) {
            if (a_themeName == capturedThemeName) {
                QStringList capturedNonTextElement = matchNonTextCharElements.capturedTexts()
                        .replaceInStrings(QRegExp("^\\s+|\\s+$"), "");

                themeElement.id = capturedNonTextElement[1];
                themeElement.type = ThemeElementType(capturedNonTextElement[2].toInt());
                themeElement.text = capturedNonTextElement[3];
                themeElement.color = capturedNonTextElement[4];
                themeElement.icon = QIcon(QString(":color_swatch.png"));

                elementlist.append(themeElement);
            }
        }

        if (matchThemeHeader.hasMatch()) {
            capturedThemeName = matchThemeHeader.captured(1).trimmed();
            if (a_themeName == capturedThemeName) {
                matchOne = true; // first match
            } else {
                if (matchOne) { // end reading when second match hits, don't need it
                    endRead = true;
                }
            }
        }
    }
    return elementlist;
}

QString ThemeElementsModel::removeThemeFromListString (QString &a_themeListString, const QString &a_themeName)
{
    QTextStream in(&a_themeListString);
    in.seek(0);
    QString newString("");
    QTextStream out(&newString);

    QString capturedThemeName("");

    QRegularExpression reThemeHeader("^\\[(.+)\\]"); // match for [preset name]
    QRegularExpression reThemeElements("(.+);(.+);(.+);(.+)"); // match for a;b;c;d

    while (!in.atEnd()) {
        QString line = in.readLine();

        QRegularExpressionMatch matchThemeHeader = reThemeHeader.match(line);
        QRegularExpressionMatch matchThemeElements = reThemeElements.match(line);

        if (matchThemeElements.hasMatch() ) {
            if (a_themeName != capturedThemeName)
                out << line << "\n";
        }

        if (matchThemeHeader.hasMatch()) {
            capturedThemeName = matchThemeHeader.captured(1);

            /* keep line if captured name not in selection */
            if (a_themeName != capturedThemeName)
                out << line << "\n";
        }
    }
    return newString;
}

// END OF void ThemeElementsModel::addColor(const QString & a_id,
//		const QString & a_text)
//==============================================================================

ThemeElementData ThemeElementsModel::getThemeElementData(const QString & a_id)
{
    for(int i = 0; i < m_themeElementsList.size(); ++i)
	{
		if(m_themeElementsList[i].id == a_id)
			return m_themeElementsList[i];
	}

    return ThemeElementData();
}

// END OF ThemeElementData ThemeElementsModel::getThemeElementData(
//		const QString & a_id)
//==============================================================================

bool ThemeElementsModel::setThemeElementData(const QString &a_id, ThemeElementData &a_themeElementData)
{
    for(int i = 0; i < m_themeElementsList.size(); ++i)
    {
        if(m_themeElementsList[i].id == a_id) {
            m_themeElementsList[i] = a_themeElementData;
            return true;
        }
    }
    return false;
}

void ThemeElementsModel::fromThemeElementsList(const ThemeElementsList &a_themeElementsList)
{
    m_themeElementsList = a_themeElementsList;
}

// END OF bool ThemeElementsModel::setThemeElementData(const QString &a_id,
// ThemeElementData &a_themeElementData)
//==============================================================================

ThemeElementsList ThemeElementsModel::toThemeElementsList()
{
    return m_themeElementsList;
}

// END OF ThemeElementsList ThemeElementsModel::getThemeElementList()
//==============================================================================

QString ThemeElementsModel::themeName()
{
    return m_themeName;
}

// END OF QString ThemeElementsModel::themeName
//==============================================================================

void ThemeElementsModel::clear()
{
    if (!m_themeElementsList.empty()) {
        beginResetModel();
            m_themeElementsList.clear();
        endResetModel();
    }
}
