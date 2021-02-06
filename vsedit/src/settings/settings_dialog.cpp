#include "settings_dialog.h"

#include "../../../common-src/settings/settings_manager.h"
#include "../../common-src/helpers.h"

#include "item_delegate_for_hotkey.h"
#include "theme_elements_model.h"
#include "theme_select_dialog.h"

#include <QFileDialog>
#include <QListWidgetItem>
#include <QFontDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QTextStream>
#include <QDesktopServices>

//==============================================================================

SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
    QDialog(a_pParent, Qt::WindowFlags()
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pActionsHotkeyEditModel(nullptr)
	, m_pItemDelegateForHotkey(nullptr)
	, m_pThemeElementsModel(nullptr)
    , m_themePresetsFileName("")
    , m_tempThemePresets("")
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":settings.png"));

	m_ui.addVSLibraryPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSLibraryPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSLibraryPathButton->setIcon(QIcon(":folder.png"));
	m_ui.addVSPluginsPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSPluginsPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSPluginsPathButton->setIcon(QIcon(":folder.png"));
	m_ui.addVSDocumentationPathButton->setIcon(QIcon(":folder_add.png"));
	m_ui.removeVSDocumentationPathButton->setIcon(QIcon(":folder_remove.png"));
	m_ui.selectVSDocumentationPathButton->setIcon(QIcon(":folder.png"));

	m_pActionsHotkeyEditModel = new ActionsHotkeyEditModel(m_pSettingsManager,
		this);
	m_ui.hotkeysTable->setModel(m_pActionsHotkeyEditModel);

	m_pItemDelegateForHotkey = new ItemDelegateForHotkey(this);
	m_ui.hotkeysTable->setItemDelegateForColumn(2, m_pItemDelegateForHotkey);

	m_ui.hotkeysTable->resizeColumnsToContents();

    /* theme setting */
    m_ui.themePresetCopyWidget->setVisible(false);
    loadThemePresets();

    connect(m_ui.settingsFileDirButton, &QPushButton::clicked,
            this, &SettingsDialog::slotOpenSettingsFileDir);

    // signal for theme preset
    connect(m_ui.copyThemePresetButton, &QPushButton::clicked,
            this, &SettingsDialog::slotShowThemePresetCopyWidget);
    connect(m_ui.cancelThemePresetCopyButton, &QPushButton::clicked,
            this, &SettingsDialog::slotCancelThemePresetCopy);
    connect(m_ui.saveThemePresetButton, &QPushButton::clicked,
            this, &SettingsDialog::slotSaveThemePreset);
    connect(m_ui. removeThemePresetButton, &QPushButton::clicked,
            this, &SettingsDialog::slotRemoveThemePreset);
    connect(m_ui.exportThemePresetButton, &QPushButton::clicked,
            this, &SettingsDialog::slotHandleThemeExport);
    connect(m_ui.importThemePresetButton, &QPushButton::clicked,
            this, &SettingsDialog::slotHandleThemeImport);
    connect(m_ui.themePresetSelectionComboBox, &QComboBox::currentTextChanged,
            this, &SettingsDialog::slotChangeThemePreset);


	connect(m_ui.okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(m_ui.applyButton, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(this, &QDialog::rejected, this, &SettingsDialog::slotCancel);

	connect(m_ui.addVSLibraryPathButton, SIGNAL(clicked()),
		this, SLOT(slotAddVSLibraryPath()));
	connect(m_ui.removeVSLibraryPathButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveVSLibraryPath()));
	connect(m_ui.selectVSLibraryPathButton, SIGNAL(clicked()),
		this, SLOT(slotSelectVSLibraryPath()));

	connect(m_ui.addVSPluginsPathButton, SIGNAL(clicked()),
		this, SLOT(slotAddVSPluginsPath()));
	connect(m_ui.removeVSPluginsPathButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveVSPluginsPath()));
	connect(m_ui.selectVSPluginsPathButton, SIGNAL(clicked()),
		this, SLOT(slotSelectVSPluginsPath()));

	connect(m_ui.addVSDocumentationPathButton, SIGNAL(clicked()),
		this, SLOT(slotAddVSDocumentationPath()));
	connect(m_ui.removeVSDocumentationPathButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveVSDocumentationPath()));
	connect(m_ui.selectVSDocumentationPathButton, SIGNAL(clicked()),
		this, SLOT(slotSelectVSDocumentationPath()));

    connect(m_ui.themeElementsListView, SIGNAL(clicked(const QModelIndex &)),
		this, SLOT(slotThemeElementSelected(const QModelIndex &)));
	connect(m_ui.fontButton, SIGNAL(clicked()),
		this, SLOT(slotFontButtonClicked()));
	connect(m_ui.colourButton, SIGNAL(clicked()),
		this, SLOT(slotColourButtonClicked()));
}

// END OF SettingsDialog::SettingsDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent)
//==============================================================================

SettingsDialog::~SettingsDialog()
{

}

// END OF SettingsDialog::~SettingsDialog()
//==============================================================================

void SettingsDialog::slotCall()
{
	m_ui.autoLoadLastScriptCheckBox->setChecked(
		m_pSettingsManager->getAutoLoadLastScript());
	m_ui.promptToSaveChangesCheckBox->setChecked(
		m_pSettingsManager->getPromptToSaveChanges());
	m_ui.portableModeCheckBox->setChecked(
		m_pSettingsManager->getPortableMode());
    m_ui.maxRecentFilesSpinBox->setValue(
        int(m_pSettingsManager->getMaxRecentFilesNumber()));
	m_ui.charactersTypedToStartCompletionSpinBox->setValue(
		m_pSettingsManager->getCharactersTypedToStartCompletion());
	m_ui.useSpacesAsTabCheckBox->setChecked(
		m_pSettingsManager->getUseSpacesAsTab());
	m_ui.spacesInTabSpinBox->setValue(
		m_pSettingsManager->getSpacesInTab());
	m_ui.highlightSelectionMatchesCheckBox->setChecked(
		m_pSettingsManager->getHighlightSelectionMatches());
	m_ui.highlightSelectionMatchesMinLengthSpinBox->setValue(
		m_pSettingsManager->getHighlightSelectionMatchesMinLength());
	m_ui.alwaysKeepCurrentFrameCheckBox->setChecked(
		m_pSettingsManager->getAlwaysKeepCurrentFrame());

    BookmarkSavingFormat savedFormat = m_pSettingsManager->getBookmarkSavingFormat();

    switch (savedFormat) {
    case BookmarkSavingFormat::ChapterFormat:
        m_ui.bookmarkSavingRadioButton_chapter->setChecked(true);
        break;
    case BookmarkSavingFormat::BookmarkFormat:
        m_ui.bookmarkSavingRadioButton_bookmark->setChecked(true);
        break;
    }

    m_ui.bookmarkDelimiterLineEdit->setText(
        m_pSettingsManager->getBookmarkDelimiter());

	m_ui.vsLibraryPathsListWidget->clear();
	m_ui.vsLibraryPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthLibraryPaths());
	m_ui.vsLibraryPathEdit->clear();

	m_ui.vsPluginsPathsListWidget->clear();
	m_ui.vsPluginsPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthPluginsPaths());
	m_ui.vsPluginsPathEdit->clear();

	m_ui.vsDocumentationPathsListWidget->clear();
	m_ui.vsDocumentationPathsListWidget->addItems(
		m_pSettingsManager->getVapourSynthDocumentationPaths());
	m_ui.vsDocumentationPathEdit->clear();

	m_ui.settingsTabWidget->setCurrentIndex(0);

	m_pActionsHotkeyEditModel->reloadHotkeysSettings();

    QModelIndex firstElement = m_pActionsHotkeyEditModel->index(0, 0);
    m_ui.themeElementsListView->setCurrentIndex(firstElement);

    show();
}

// END OF void SettingsDialog::slotCall()
//==============================================================================

void SettingsDialog::loadDefaultThemePreset()
{
    delete m_pThemeElementsModel;
    m_pThemeElementsModel = new ThemeElementsModel(m_pSettingsManager,
        DEFAULT_THEME_NAME, this);

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT,
                tr("Common script text"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_KEYWORD,
                tr("Keyword"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_KEYWORD));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_OPERATOR,
                tr("Operator"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_OPERATOR));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_STRING,
                tr("String"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_STRING));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_NUMBER,
                tr("Number"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_NUMBER));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_COMMENT,
                tr("Comment"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_COMMENT));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_VS_CORE,
                tr("VapourSynth core"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_VS_CORE));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_VS_NAMESPACE,
                tr("VapourSynth namespace"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_VS_NAMESPACE));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_VS_FUNCTION,
                tr("VapourSynth function"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_VS_FUNCTION));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_VS_ARGUMENT,
                tr("VapourSynth argument"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_VS_ARGUMENT));

    m_pThemeElementsModel->addTextCharFormat(
                TEXT_FORMAT_ID_TIMELINE,
                tr("Timeline labels"),
                m_pSettingsManager->getDefaultTextFormat(TEXT_FORMAT_ID_TIMELINE));

    m_pThemeElementsModel->addNonTextCharFormat(
                COLOR_ID_TEXT_BACKGROUND,
                tr("Text background color"),
                m_pSettingsManager->getDefaultColor(COLOR_ID_TEXT_BACKGROUND));

    m_pThemeElementsModel->addNonTextCharFormat(
                COLOR_ID_ACTIVE_LINE,
                tr("Active line color"),
                m_pSettingsManager->getDefaultColor(COLOR_ID_ACTIVE_LINE));

    m_pThemeElementsModel->addNonTextCharFormat(
                COLOR_ID_SELECTION_MATCHES,
                tr("Selection matches color"),
                m_pSettingsManager->getDefaultColor(COLOR_ID_SELECTION_MATCHES));

    m_pThemeElementsModel->addNonTextCharFormat(
                COLOR_ID_TIMELINE_BOOKMARKS,
                tr("Timeline bookmarks color"),
                m_pSettingsManager->getDefaultColor(COLOR_ID_TIMELINE_BOOKMARKS));

    m_ui.themeElementsListView->setModel(m_pThemeElementsModel);
}
// END OF void SettingsDialog::addThemeElements()
//==============================================================================

void SettingsDialog::loadThemePresets()
{
    m_tempThemePresets.clear();
    m_pThemePresetsListModel = new GenericStringListModel();

    /* copy theme_presets.txt content to tempThemePresets and read it off stream
     * load from temp string and add preset names to theme list model
     * the list model will then update combobox automatically
    */
    QFile file("theme_presets.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        file.open(QIODevice::WriteOnly); // create file if it doesn't exist
        file.open(QIODevice::ReadOnly | QIODevice::Text);
    }

    QTextStream out(&file);
    QTextStream in(&m_tempThemePresets);

    /* copy theme_presets content to local string */
    m_tempThemePresets = file.readAll();
    file.close();

    bool foundOne(false);

    QRegularExpression rePresetName("^\\[(.+)\\]"); // match for [preset name]

    in.seek(0);
    while (!in.atEnd()) {
        QString line = in.readLine();

        QRegularExpressionMatch reMatchPresetName = rePresetName.match(line);
        QString presetName("");

        if (reMatchPresetName.hasMatch()) {
            foundOne = true;
            presetName = reMatchPresetName.captured(1);
            m_pThemePresetsListModel->append(presetName);
        }
    }   

    if (!foundOne) {
        /* if no theme found, empty file and set default theme to text, then recopy text to stream */
        file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate );

        loadDefaultThemePreset();
        addThemeToTextStream(out, DEFAULT_THEME_NAME);

        file.close();
        file.open(QIODevice::ReadOnly | QIODevice::Text );
        m_tempThemePresets.clear();
        in.seek(0);
        in << file.readAll();

        m_pThemePresetsListModel->append(DEFAULT_THEME_NAME);
    }

    m_ui.themePresetSelectionComboBox->setModel(m_pThemePresetsListModel);

    // catch if saved theme selection doesn't exist
    QString savedThemePreset = m_pSettingsManager->getThemeName();
    if (!m_pThemePresetsListModel->stringList().contains(savedThemePreset)) {
        m_pSettingsManager->setThemeName(DEFAULT_THEME_NAME);
    }

    /* set selection from saved setting */
    savedThemePreset = m_pSettingsManager->getThemeName();
    if (savedThemePreset != m_ui.themePresetSelectionComboBox->currentText()) {
        m_ui.themePresetSelectionComboBox->setCurrentText(savedThemePreset);
    }

    slotChangeThemePreset(m_pSettingsManager->getThemeName());
}

void SettingsDialog::addThemeToTextStream(QTextStream &out, const QString &a_themeName)
{
    ThemeElementsList themeElementsList = m_pThemeElementsModel->toThemeElementsList();
    out << "[" << a_themeName << "]\n";

    for (auto &themeElement : themeElementsList) {
        out << themeElement.id << "; ";
        out << int(themeElement.type) << "; ";
        out << themeElement.text << "; ";

        if (themeElement.type == ThemeElementType::TextCharFormat) {
            out << themeElement.textCharFormat << "\n";
        } else {
            out << themeElement.color.name() << "\n";
        }
    }
}

// END OF void SettingsDialog::loadThemeGroupPresets()
//==============================================================================

void SettingsDialog::saveThemeSettings()
{
    m_pSettingsManager->setThemeName(
                m_ui.themePresetSelectionComboBox->currentText());

    QFile file("theme_presets.txt");
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text )) {
        QMessageBox::information(this, tr("Unable to write to file"),
                                 file.errorString());
        return;
    }

    QTextStream out(&file);
    out << m_tempThemePresets;
}

// END OF void SettingsDialog::saveThemeSettings()
//==============================================================================


void SettingsDialog::slotOk()
{
	slotApply();
	accept();
}

// END OF void SettingsDialog::slotOk()
//==============================================================================

void SettingsDialog::slotApply()
{
	m_pSettingsManager->setAutoLoadLastScript(
		m_ui.autoLoadLastScriptCheckBox->isChecked());
	m_pSettingsManager->setPromptToSaveChanges(
		m_ui.promptToSaveChangesCheckBox->isChecked());
	m_pSettingsManager->setPortableMode(
		m_ui.portableModeCheckBox->isChecked());
	m_pSettingsManager->setMaxRecentFilesNumber(
        uint32_t(m_ui.maxRecentFilesSpinBox->value()));
	m_pSettingsManager->setCharactersTypedToStartCompletion(
		m_ui.charactersTypedToStartCompletionSpinBox->value());
	m_pSettingsManager->setUseSpacesAsTab(
		m_ui.useSpacesAsTabCheckBox->isChecked());
	m_pSettingsManager->setSpacesInTab(
		m_ui.spacesInTabSpinBox->value());
	m_pSettingsManager->setHighlightSelectionMatches(
		m_ui.highlightSelectionMatchesCheckBox->isChecked());
	m_pSettingsManager->setHighlightSelectionMatchesMinLength(
		m_ui.highlightSelectionMatchesMinLengthSpinBox->value());
	m_pSettingsManager->setAlwaysKeepCurrentFrame(
		m_ui.alwaysKeepCurrentFrameCheckBox->isChecked());

    // bookmark format setting
    if (m_ui.bookmarkSavingRadioButton_chapter->isChecked()) {
        m_pSettingsManager->setBookmarkSavingFormat(BookmarkSavingFormat::ChapterFormat);
    } else {
        m_pSettingsManager->setBookmarkSavingFormat(BookmarkSavingFormat::BookmarkFormat);
    }

    m_pSettingsManager->setBookmarkDelimiter(
        m_ui.bookmarkDelimiterLineEdit->text());

	QStringList vapourSynthLibraryPaths;
	int vsLibraryPathsNumber = m_ui.vsLibraryPathsListWidget->count();
	for(int i = 0; i < vsLibraryPathsNumber; ++i)
	{
		QString path = m_ui.vsLibraryPathsListWidget->item(i)->text();
		vapourSynthLibraryPaths.append(path);
	}
	vapourSynthLibraryPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthLibraryPaths(vapourSynthLibraryPaths);

	QStringList vapourSynthPluginsPaths;
	int vsPluginsPathsNumber = m_ui.vsPluginsPathsListWidget->count();
	for(int i = 0; i < vsPluginsPathsNumber; ++i)
	{
		QString path = m_ui.vsPluginsPathsListWidget->item(i)->text();
		vapourSynthPluginsPaths.append(path);
	}
	vapourSynthPluginsPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthPluginsPaths(vapourSynthPluginsPaths);

	QStringList vapourSynthDocumentationPaths;
	int vsDocumentationPathsNumber =
		m_ui.vsDocumentationPathsListWidget->count();
	for(int i = 0; i < vsDocumentationPathsNumber; ++i)
	{
		QString path = m_ui.vsDocumentationPathsListWidget->item(i)->text();
		vapourSynthDocumentationPaths.append(path);
	}
	vapourSynthDocumentationPaths.removeDuplicates();
	m_pSettingsManager->setVapourSynthDocumentationPaths(
		vapourSynthDocumentationPaths);

	m_pActionsHotkeyEditModel->slotSaveActionsHotkeys();

    saveThemeSettings();

    emit signalSettingsChanged();
}

// END OF void SettingsDialog::slotApply()
//==============================================================================

void SettingsDialog::slotCancel()
{
    loadThemePresets();
}

void SettingsDialog::slotOpenSettingsFileDir()
{
    QString settingsFileDir = m_pSettingsManager->getSettingsFileDir();
    QDesktopServices::openUrl(settingsFileDir);
}

// END OF void SettingsDialog::slotCancel()
//==============================================================================

void SettingsDialog::slotAddVSLibraryPath()
{
	QString newPath = m_ui.vsLibraryPathEdit->text();
	if(newPath.isEmpty())
		return;
	int pathsNumber = m_ui.vsLibraryPathsListWidget->count();
	for(int i = 0; i < pathsNumber; ++i)
	{
		QString path = m_ui.vsLibraryPathsListWidget->item(i)->text();
		if(path == newPath)
			return;
	}
	QListWidgetItem * pListItem = new QListWidgetItem(newPath,
		m_ui.vsLibraryPathsListWidget);
	pListItem->setToolTip(newPath);
}

// END OF void SettingsDialog::slotAddVSLibraryPath()
//==============================================================================

void SettingsDialog::slotRemoveVSLibraryPath()
{
	QListWidgetItem * pCurrentItem =
		m_ui.vsLibraryPathsListWidget->currentItem();
	if(pCurrentItem)
		delete pCurrentItem;
}

// END OF void SettingsDialog::slotRemoveVSPluginsPath()
//==============================================================================

void SettingsDialog::slotSelectVSLibraryPath()
{
	QString path = QFileDialog::getExistingDirectory(this,
        tr("Select VapourSynth library search path"),
		m_ui.vsLibraryPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsLibraryPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSLibraryPath()
//==============================================================================

void SettingsDialog::slotAddVSPluginsPath()
{
	QString newPath = m_ui.vsPluginsPathEdit->text();
	if(newPath.isEmpty())
		return;
	int pathsNumber = m_ui.vsPluginsPathsListWidget->count();
	for(int i = 0; i < pathsNumber; ++i)
	{
		QString path = m_ui.vsPluginsPathsListWidget->item(i)->text();
		if(path == newPath)
			return;
	}
	QListWidgetItem * pListItem = new QListWidgetItem(newPath,
		m_ui.vsPluginsPathsListWidget);
	pListItem->setToolTip(newPath);
}

// END OF void SettingsDialog::slotAddVSPluginsPath()
//==============================================================================

void SettingsDialog::slotRemoveVSPluginsPath()
{
	QListWidgetItem * pCurrentItem =
		m_ui.vsPluginsPathsListWidget->currentItem();
	if(pCurrentItem)
		delete pCurrentItem;
}

// END OF void SettingsDialog::slotRemoveVSPluginsPath()
//==============================================================================

void SettingsDialog::slotSelectVSPluginsPath()
{
	QString path = QFileDialog::getExistingDirectory(this,
        tr("Select VapourSynth plugins path"),
		m_ui.vsPluginsPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsPluginsPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSPluginsPath()
//==============================================================================

void SettingsDialog::slotAddVSDocumentationPath()
{
	QString newPath = m_ui.vsDocumentationPathEdit->text();
	if(newPath.isEmpty())
		return;
	int pathsNumber = m_ui.vsDocumentationPathsListWidget->count();
	for(int i = 0; i < pathsNumber; ++i)
	{
		QString path = m_ui.vsDocumentationPathsListWidget->item(i)->text();
		if(path == newPath)
			return;
	}
	QListWidgetItem * pListItem = new QListWidgetItem(newPath,
		m_ui.vsDocumentationPathsListWidget);
	pListItem->setToolTip(newPath);
}

// END OF void SettingsDialog::slotAddVSDocumentationPath()
//==============================================================================

void SettingsDialog::slotRemoveVSDocumentationPath()
{
	QListWidgetItem * pCurrentItem =
		m_ui.vsDocumentationPathsListWidget->currentItem();
	if(pCurrentItem)
		delete pCurrentItem;
}

// END OF void SettingsDialog::slotRemoveVSDocumentationPath()
//==============================================================================

void SettingsDialog::slotSelectVSDocumentationPath()
{
	QString path = QFileDialog::getExistingDirectory(this,
        tr("Select documentation path"),
		m_ui.vsDocumentationPathEdit->text());
	if(!path.isEmpty())
		m_ui.vsDocumentationPathEdit->setText(path);
}

// END OF void SettingsDialog::slotSelectVSDocumentationPath()
//==============================================================================

void SettingsDialog::slotThemeElementSelected(const QModelIndex & a_index)
{
	if(!a_index.isValid())
		return;

    QString themeElementId = m_pThemeElementsModel->data(
		a_index, Qt::UserRole).toString();
    ThemeElementData themeElementData =
		m_pThemeElementsModel->getThemeElementData(themeElementId);

	if(themeElementData.id.isEmpty())
		return;

	if(themeElementData.type == ThemeElementType::TextCharFormat)
	{
		m_ui.fontButton->setEnabled(true);
		m_ui.colourButton->setEnabled(true);

		m_ui.fontLabel->setText(
			themeElementData.textCharFormat.font().family());
		m_ui.fontLabel->setFont(themeElementData.textCharFormat.font());
		QPalette newPalette = m_ui.fontLabel->palette();
		newPalette.setColor(QPalette::WindowText,
			themeElementData.textCharFormat.foreground().color());
		m_ui.fontLabel->setPalette(newPalette);
		m_ui.fontLabel->update();

		newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window,
			themeElementData.textCharFormat.foreground().color());
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}
    else if(themeElementData.type == ThemeElementType::NonTextCharFormat)
	{
		m_ui.fontButton->setEnabled(false);
		m_ui.colourButton->setEnabled(true);

		m_ui.fontLabel->setText(QString());
		m_ui.fontLabel->setFont(QFont());
		m_ui.fontLabel->setPalette(QPalette());
		m_ui.fontLabel->update();

		QPalette newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window, themeElementData.color);
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}
}

// END OF void SettingsDialog::slotThemeElementSelected(
//		const QModelIndex & a_index)
//==============================================================================

void SettingsDialog::slotFontButtonClicked()
{
    QModelIndex index = m_ui.themeElementsListView->currentIndex();
	QString id = m_pThemeElementsModel->data(index, Qt::UserRole).toString();
    QString selectedThemeName = m_pThemeElementsModel->themeName();
	ThemeElementData themeElementData =
		m_pThemeElementsModel->getThemeElementData(id);

	QFontDialog fontDialog;
	fontDialog.setCurrentFont(themeElementData.textCharFormat.font());
	int returnCode = fontDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QFont newFont = fontDialog.selectedFont();
	themeElementData.textCharFormat.setFont(newFont);
	m_ui.fontLabel->setText(newFont.family());
	m_ui.fontLabel->setFont(newFont);

    // store to stream
    QString tempStream("");
    QTextStream out(&tempStream);

    QTextStream in(&m_tempThemePresets);
    in.seek(0);

    QRegularExpression reThemeHeader("^\\[(.+)\\]"); // match for [preset name]

    /* match for element with textCharFormat string  [a; 0; c; textChar] */
    QRegularExpression reTextCharElements("^([\\w\\d\\s]+)\\s*;\\s*([0]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*([\\w\\d\\s,-|#]+)$");

//    /* match for element with non textCharFormat string  [a; 1; c; #ffffff] */
    QRegularExpression reNonTextElements("^([\\w\\d\\s]+)\\s*;\\s*([1]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*(#[a-z0-9]{6})\\s*$");

    QString capturedThemeName("");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch matchThemeHeader = reThemeHeader.match(line);
        QRegularExpressionMatch matchTextCharElements = reTextCharElements.match(line);
        QRegularExpressionMatch matchNonTextCharElements = reNonTextElements.match(line);

        if (matchTextCharElements.hasMatch()) {
            // match found, rewrite this line
            if (id == matchTextCharElements.captured(1).trimmed() &&
                    capturedThemeName == selectedThemeName) {
                out << matchTextCharElements.captured(1).trimmed() << "; ";
                out << matchTextCharElements.captured(2).trimmed() << "; ";
                out << matchTextCharElements.captured(3).trimmed() << "; ";
                out << newFont.toString() << " | ";
                out << themeElementData.textCharFormat.foreground().color().name() << " | ";
                out << newFont.weight() << "\n";
            } else {
                out << line << "\n";
            }
        }

        if (matchNonTextCharElements.hasMatch()) {
            out << line << "\n";
        }

        if (matchThemeHeader.hasMatch()) {
            capturedThemeName = matchThemeHeader.captured(1).trimmed();
            out << line << "\n";
        }
    }
    /* copy temp stream to active stream */
    m_tempThemePresets.clear();
    in.seek(0);
    in << out.readAll();
}

// END OF void SettingsDialog::slotFontButtonClicked()
//==============================================================================

void SettingsDialog::slotColourButtonClicked()
{
    QModelIndex index = m_ui.themeElementsListView->currentIndex();
	QString id = m_pThemeElementsModel->data(index, Qt::UserRole).toString();
    QString selectedThemeName = m_pThemeElementsModel->themeName();

	ThemeElementData themeElementData =
		m_pThemeElementsModel->getThemeElementData(id);

	QColorDialog colorDialog;

	if(themeElementData.type == ThemeElementType::TextCharFormat)
	{
		colorDialog.setCurrentColor(
		themeElementData.textCharFormat.foreground().color());
	}
    else if(themeElementData.type == ThemeElementType::NonTextCharFormat)
		colorDialog.setCurrentColor(themeElementData.color);

	int returnCode = colorDialog.exec();
	if(returnCode == QDialog::Rejected)
		return;

	QColor newColor = colorDialog.selectedColor();

	if(themeElementData.type == ThemeElementType::TextCharFormat)
	{
		QBrush brush = themeElementData.textCharFormat.foreground();
		brush.setColor(newColor);
		themeElementData.textCharFormat.setForeground(brush);

		QPalette newPalette = m_ui.fontLabel->palette();
		newPalette.setColor(QPalette::WindowText,
			themeElementData.textCharFormat.foreground().color());
		m_ui.fontLabel->setPalette(newPalette);
		m_ui.fontLabel->update();

		newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window,
			themeElementData.textCharFormat.foreground().color());
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();


	}
    else if(themeElementData.type == ThemeElementType::NonTextCharFormat)
	{
		themeElementData.color = newColor;

		QPalette newPalette = m_ui.colourFrame->palette();
		newPalette.setColor(QPalette::Window, themeElementData.color);
		m_ui.colourFrame->setPalette(newPalette);
		m_ui.colourFrame->update();
	}

    /* update color dialog to reflect selected color */
    m_pThemeElementsModel->setThemeElementData(id,themeElementData);

    // store to stream
    QString tempStream("");
    QTextStream out(&tempStream);

    QTextStream in(&m_tempThemePresets);
    in.seek(0);

    QRegularExpression reThemeHeader("^\\[(.+)\\]"); // match for [preset name]

    /* match for element with textCharFormat string  [a; 0; c; textChar] */
    QRegularExpression reTextCharElements("^([\\w\\d\\s]+)\\s*;\\s*([0]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*([\\w\\d\\s,-|#]+)$");

//    /* match for element with non textCharFormat string  [a; 1; c; #ffffff] */
    QRegularExpression reNonTextElements("^([\\w\\d\\s]+)\\s*;\\s*([1]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*(#[a-z0-9]{6})\\s*$");

    QString capturedThemeName("");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch matchThemeHeader = reThemeHeader.match(line);
        QRegularExpressionMatch matchTextCharElements = reTextCharElements.match(line);
        QRegularExpressionMatch matchNonTextCharElements = reNonTextElements.match(line);

        if (matchTextCharElements.hasMatch()) {
            // match found, rewrite this line
            if (id == matchTextCharElements.captured(1).trimmed() &&
                    capturedThemeName == selectedThemeName) {
                out << matchTextCharElements.captured(1).trimmed() << "; ";
                out << matchTextCharElements.captured(2).trimmed() << "; ";
                out << matchTextCharElements.captured(3).trimmed() << "; ";

                QStringList fontStrings = matchTextCharElements.captured(4).split("|");
                out << fontStrings[0].trimmed() << " | ";
                out << newColor.name() << " | ";
                out << fontStrings[2].trimmed() << "\n";
            } else {
                out << line << "\n";
            }
        }

        if (matchNonTextCharElements.hasMatch()) {
            // match found, rewrite this line
            if (id == matchNonTextCharElements.captured(1).trimmed() &&
                    capturedThemeName == selectedThemeName) {
                out << matchNonTextCharElements.captured(1).trimmed() << "; ";
                out << matchNonTextCharElements.captured(2).trimmed() << "; ";
                out << matchNonTextCharElements.captured(3).trimmed() << "; ";
                out << newColor.name() << "\n";
            } else {
                out << line << "\n";
            }
        }

        if (matchThemeHeader.hasMatch()) {
            capturedThemeName = matchThemeHeader.captured(1).trimmed();
            out << line << "\n";
        }
    }
    /* copy temp stream to active stream */
    m_tempThemePresets.clear();
    in.seek(0);
    in << out.readAll();
}

// END OF void SettingsDialog::slotColourButtonClicked()
//==============================================================================

void SettingsDialog::slotShowThemePresetCopyWidget()
{
    if (m_ui.themePresetCopyWidget->isVisible()) return;
    m_ui.themePresetCopyWidget->show();
    m_ui.themePresetControllerWidget->hide();

    QString selectedTheme = m_ui.themePresetSelectionComboBox->currentText();
    m_ui.themePresetLineEdit->setText(selectedTheme + "*");
    m_ui.themePresetLineEdit->selectAll();
    m_ui.themePresetLineEdit->setFocus();
}

void SettingsDialog::slotCancelThemePresetCopy()
{
    // empty stuff then set import widget visible
    if (m_ui.themePresetCopyWidget->isVisible())
        m_ui.themePresetCopyWidget->hide();

    m_ui.themePresetLineEdit->clear();

    if (m_ui.themePresetControllerWidget->isVisible()) return;
    m_ui.themePresetControllerWidget->show();
}

void SettingsDialog::slotSaveThemePreset()
{
    if (!m_ui.themePresetCopyWidget->isVisible()) return;

    QString themeToCopy = m_ui.themePresetSelectionComboBox->currentText();
    QString newThemeName = m_ui.themePresetLineEdit->text().trimmed();

    // check for duplicate name
    if (m_pThemePresetsListModel->stringList().contains(newThemeName)) {
        QMessageBox::information(this, tr("Duplicated Theme Name"),
                                 tr("Theme name already existed"));
        m_ui.themePresetLineEdit->selectAll();
        m_ui.themePresetLineEdit->setFocus();
        return;
    }
    m_ui.themePresetLineEdit->clear();
    m_ui.themePresetCopyWidget->hide();
    m_ui.themePresetControllerWidget->show();

    QTextStream out(&m_tempThemePresets);
    addThemeToTextStream(out, newThemeName);
    m_pThemePresetsListModel->append(newThemeName);
}

void SettingsDialog::slotHandleThemeExport()
{
    /* create a selection dialog to choose themes to export */
    ThemeSelectDialog * themeSelectDialog = new ThemeSelectDialog(
                m_pThemePresetsListModel->stringList(),
                ThemeSelectionFlag::Export, this);
    themeSelectDialog->show();

    connect(themeSelectDialog, &ThemeSelectDialog::signalExportSelectedList,
            this, &SettingsDialog::slotExportSelectedThemePresets);
}

void SettingsDialog::slotExportSelectedThemePresets(QStringList &a_selectedThemePresets)
{
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Export theme preset"), defaultDir + QDir::separator() + tr("theme"),
                        tr("Theme file (*.txt)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Truncate ))
    {
        QMessageBox::information(this, tr("Unable to open destination file"),
                                 file.errorString());
        return;
    }

    QTextStream in(&m_tempThemePresets);
    in.seek(0);
    QTextStream out(&file);

    QRegularExpression reThemeHeader("^\\[(.+)\\]"); // match for [preset name]
    QRegularExpression reThemeElements("^(.+);(.+);(.+);(.+)"); // match for a;b;c;d

    QString capturedThemeName("");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch matchThemeHeader = reThemeHeader.match(line);
        QRegularExpressionMatch matchThemeElements = reThemeElements.match(line);

        if (matchThemeElements.hasMatch()) {
            if (a_selectedThemePresets.contains(capturedThemeName))
                out << line << "\n";
        }

        if (matchThemeHeader.hasMatch()) {
            capturedThemeName = matchThemeHeader.captured(1);
            if (a_selectedThemePresets.contains(capturedThemeName)) { // copy to export
                out << line << "\n";
            }
        }
    }
}

void SettingsDialog::slotHandleThemeImport()
{
    m_themePresetsFileName = QFileDialog::getOpenFileName(this,
        tr("Load theme file"), "",
        tr("Theme file (*.txt)"));

    if (m_themePresetsFileName.isEmpty())
        return;

    QFile file(m_themePresetsFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        return;
    }

    QRegularExpression rePresetName("^\\[(.+)\\]"); // match for [preset name]
    QStringList themePresetNameList;
    QString capturedThemeName("");

    /* scan stream for preset names, store them in a qstringlist for filtering */
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        QRegularExpressionMatch matchPresetName = rePresetName.match(line);

        if (matchPresetName.hasMatch()) {
            capturedThemeName = matchPresetName.captured(1).trimmed();
            themePresetNameList.append(capturedThemeName);
        }
    }

    if (themePresetNameList.count() < 1) {
        QMessageBox::information(this, tr("No theme found"),
                                 tr("No theme preset found"));
        return;
    }
    ThemeSelectDialog * themeSelectDialog = new ThemeSelectDialog(
                themePresetNameList, ThemeSelectionFlag::Import, this);
    themeSelectDialog->show();

    connect(themeSelectDialog, &ThemeSelectDialog::signalImportSelectedList,
            this, &SettingsDialog::slotImportSelectedThemePresets);
}

void SettingsDialog::slotImportSelectedThemePresets(QStringList & a_selectedThemePresets)
{
    if (m_themePresetsFileName == "") return;

    QFile readFile(m_themePresetsFileName);
    readFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&readFile);
    QTextStream out(&m_tempThemePresets);
    out.seek(0);

    QRegularExpression reThemeHeader("^\\[(.+)\\]"); // match for [preset name]

    /* match for element with textCharFormat string  [a; 0; c; textChar] */
    QRegularExpression reTextCharElements("^([\\w\\d\\s]+)\\s*;\\s*([0]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*([\\w\\d\\s,-|#]+)$");

    /* match for element with non textCharFormat string  [a; 1; c; #ffffff] */
    QRegularExpression reNonTextElements("^([\\w\\d\\s]+)\\s*;\\s*([1]{1})\\s*;\\s*([\\w\\d\\s]+)\\s*;\\s*(#[a-z0-9]{6})\\s*$");

    QString capturedThemeName("");

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch matchThemeHeader = reThemeHeader.match(line);
        QRegularExpressionMatch matchTextCharElements = reTextCharElements.match(line);
        QRegularExpressionMatch matchNonTextCharElements = reNonTextElements.match(line);

        if (matchTextCharElements.hasMatch()|| matchNonTextCharElements.hasMatch()) {
            if (a_selectedThemePresets.contains(capturedThemeName))
                out << line << "\n";
        }

        if (matchThemeHeader.hasMatch()) {
            capturedThemeName = matchThemeHeader.captured(1).trimmed();

            if (a_selectedThemePresets.contains(capturedThemeName)) {
                // check import name to local, rename it if name exist
                if (m_pThemePresetsListModel->stringList().contains(capturedThemeName)) {
                    QString newName = capturedThemeName + "_"
                            + QVariant(rand() % 10000 + 1).toString();

                    m_pThemePresetsListModel->append(newName);
                    out << "[" << newName << "]" << "\n";
                } else {
                    m_pThemePresetsListModel->append(capturedThemeName);
                    out << line << "\n";
                }
            }
        }
    }
}

void SettingsDialog::slotChangeThemePreset(const QString &a_themePreset)
{
    delete m_pThemeElementsModel;
    m_pThemeElementsModel = new ThemeElementsModel(m_pSettingsManager, a_themePreset);

    ThemeElementsList elementlist =
            ThemeElementsModel::getThemeFromListStringByName(m_tempThemePresets, a_themePreset);

    m_pThemeElementsModel->fromThemeElementsList(elementlist);
    m_ui.themeElementsListView->setModel(m_pThemeElementsModel);
}

void SettingsDialog::slotRemoveThemePreset()
{
    if (m_pThemePresetsListModel->stringList().count() == 1) return;
    QString selectedThemePreset = m_ui.themePresetSelectionComboBox->currentText();

    /* don't remove default */
    if (selectedThemePreset == DEFAULT_THEME_NAME) return;
    m_pThemePresetsListModel->removeOne(selectedThemePreset);   

    m_tempThemePresets =
            ThemeElementsModel::removeThemeFromListString(m_tempThemePresets, selectedThemePreset);
}

/* overloaded operator for QTextCharFormat data */
/* outputting as [monospace,10,-1,7,50,0,0,0,1,0 | #ffffff | 75] */
QTextStream &operator<<(QTextStream &out, const QTextCharFormat &b)
{
    out << b.font().toString() << " | ";
    out << b.foreground().color().name() << " | ";
    out << b.fontWeight();
    return out;
}
