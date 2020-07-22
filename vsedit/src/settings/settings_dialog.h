#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <ui_settings_dialog.h>

#include "actions_hotkey_edit_model.h"
#include "../../common-src/qt_widgets_subclasses/generic_stringlist_model.h"

#include <QStringListModel>

class SettingsManager;
class ItemDelegateForHotkey;
class ThemeElementsModel;

QTextStream &operator<<(QTextStream &out, const QTextCharFormat&b);

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:

	SettingsDialog(SettingsManager * a_pSettingsManager,
		QWidget * a_pParent = nullptr);

	virtual ~SettingsDialog();

public slots:

	void slotCall();

protected:


signals:

	void signalSettingsChanged();

private:

    void loadDefaultThemePreset();

    void loadThemePresets();

    void addThemeToTextStream (QTextStream &out, const QString & a_themeName);

    void saveThemeSettings();

	Ui::SettingsDialog m_ui;

	SettingsManager * m_pSettingsManager;

	ActionsHotkeyEditModel * m_pActionsHotkeyEditModel;

	ItemDelegateForHotkey * m_pItemDelegateForHotkey;

	ThemeElementsModel * m_pThemeElementsModel;

    QString m_themePresetsFileName;

    GenericStringListModel * m_pThemePresetsListModel;

    QString m_tempThemePresets;

private slots:

	void slotOk();

	void slotApply();

    void slotCancel();

	void slotAddVSLibraryPath();

	void slotRemoveVSLibraryPath();

	void slotSelectVSLibraryPath();

	void slotAddVSPluginsPath();

	void slotRemoveVSPluginsPath();

	void slotSelectVSPluginsPath();

	void slotAddVSDocumentationPath();

	void slotRemoveVSDocumentationPath();

	void slotSelectVSDocumentationPath();

	void slotThemeElementSelected(const QModelIndex & a_index);

	void slotFontButtonClicked();

	void slotColourButtonClicked();

    void slotShowThemePresetCopyWidget();

    void slotCancelThemePresetCopy();

    void slotSaveThemePreset();

    void slotHandleThemeExport();

    void slotExportSelectedThemePresets(QStringList & a_selectedThemePresets);

    void slotHandleThemeImport();

    void slotImportSelectedThemePresets(QStringList & a_selectedThemePresets);

    void slotChangeThemePreset(const QString & a_themePreset);

    void slotRemoveThemePreset();
};

#endif // SETTINGSDIALOG_H
