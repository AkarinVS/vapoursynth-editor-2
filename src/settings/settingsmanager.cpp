#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>
#include <QPalette>
#include <QFontMetricsF>

#include "settingsmanager.h"

//==============================================================================

const char SETTINGS_FILE_NAME[] = "/vsedit.config";

const char COMMON_GROUP[] = "common";

const char MAIN_WINDOW_GEOMETRY_KEY[] = "main_window_geometry";
const char PREVIEW_DIALOG_GEOMETRY_KEY[] = "prewiew_dialog_geometry";
const char MAIN_WINDOW_MAXIMIZED_KEY[] = "main_window_maximized";
const bool DEFAULT_MAIN_WINDOW_MAXIMIZED = false;
const char PREVIEW_DIALOG_MAXIMIZED_KEY[] = "preview_dialog_maximized";
const bool DEFAULT_PREVIEW_DIALOG_MAXIMIZED = false;
const char AUTO_LOAD_LAST_SCRIPT_KEY[] = "auto_load_last_script";
const bool DEFAULT_AUTO_LOAD_LAST_SCRIPT = true;
const char ZOOM_PANEL_VISIBLE_KEY[] = "zoom_panel_visible";
const bool DEFAULT_ZOOM_PANEL_VISIBLE = true;
const char ZOOM_MODE_KEY[] = "zoom_mode";
const ZoomMode DEFAULT_ZOOM_MODE = ZoomMode::NoZoom;
const char ZOOM_RATIO_KEY[] = "zoom_ratio";
const double DEFAULT_ZOOM_RATIO = 2.0;
const char SCALE_MODE_KEY[] = "scale_mode";
const Qt::TransformationMode DEFAULT_SCALE_MODE = Qt::FastTransformation;
const char CROP_MODE_KEY[] = "crop_mode";
const CropMode DEFAULT_CROP_MODE = CropMode::Relative;
const char CROP_ZOOM_RATIO_KEY[] = "crop_zoom_ratio";
const int DEFAULT_CROP_ZOOM_RATIO = 1;
const char PROMPT_TO_SAVE_CHANGES_KEY[] = "prompt_to_save_changes";
const bool DEFAULT_PROMPT_TO_SAVE_CHANGES = true;
const char RECENT_FILES_LIST_KEY[] = "recent_files_list";
const char MAX_RECENT_FILES_NUMBER_KEY[] = "max_recent_files_number";
const unsigned int DEFAULT_MAX_RECENT_FILES_NUMBER = 10;
const char VAPOURSYNTH_LIBRARY_PATHS_KEY[] = "vapoursynth_library_paths";
const char VAPOURSYNTH_PLUGINS_PATHS_KEY[] = "vapoursynth_plugins_paths";
const char VAPOURSYNTH_DOCUMENTATION_PATHS_KEY[] =
	"vapoursynth_documentation_paths";
const QStringList DEFAULT_DOCUMENTATION_PATHS("./documentation");
const char CHARACTERS_TYPED_TO_START_COMPLETION_KEY[] =
	"characters_typed_to_start_completion";
const int DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION = 2;
const char TIMELINE_MODE_KEY[] = "timeline_mode";
const TimeLineSlider::DisplayMode DEFAULT_TIMELINE_MODE =
	TimeLineSlider::DisplayMode::Time;
const char TIME_STEP_KEY[] = "time_step_mode";
const double DEFAULT_TIME_STEP = 5.0;
const char CHROMA_RESAMPLING_FILTER_KEY[] = "chroma_resampling_filter";
const ResamplingFilter DEFAULT_CHROMA_RESAMPLING_FILTER =
	ResamplingFilter::Spline16;
const char YUV_MATRIX_COEFFICIENTS_KEY[] = "yuv_matrix_coefficients";
const YuvMatrixCoefficients DEFAULT_YUV_MATRIX_COEFFICIENTS =
	YuvMatrixCoefficients::m709;
const char CHROMA_PLACEMENT_KEY[] = "chroma_placement";
const ChromaPlacement DEFAULT_CHROMA_PLACEMENT = ChromaPlacement::MPEG2;
const char BICUBIC_FILTER_PARAMETER_B_KEY[] = "bicubic_filter_parameter_b";
const double DEFAULT_BICUBIC_FILTER_PARAMETER_B = 1.0 / 3.0;
const char BICUBIC_FILTER_PARAMETER_C_KEY[] = "bicubic_filter_parameter_c";
const double DEFAULT_BICUBIC_FILTER_PARAMETER_C = 1.0 / 3.0;
const char LANCZOS_FILTER_TAPS_KEY[] = "lanczos_filter_taps";
const int DEFAULT_LANCZOS_FILTER_TAPS = 3;
const char COLOR_PICKER_VISIBLE_KEY[] = "color_picker_visible";
const bool DEFAULT_COLOR_PICKER_VISIBLE = false;
const char PLAY_FPS_LIMIT_MODE_KEY[] = "play_fps_limit_mode";
const PlayFPSLimitMode DEFAULT_PLAY_FPS_LIMIT_MODE =
	PlayFPSLimitMode::FromVideo;
const char PLAY_FPS_LIMIT_KEY[] = "play_fps_limit";
const double DEFAULT_PLAY_FPS_LIMIT = 23.976;
const char USE_SPACES_AS_TAB_KEY[] = "use_spaces_as_tab";
bool DEFAULT_USE_SPACES_AS_TAB = false;
const char SPACES_IN_TAB_KEY[] = "spaces_in_tab";
const int DEFAULT_SPACES_IN_TAB = 4;
const char REMEMBER_LAST_PREVIEW_FRAME_KEY[] = "remember_last_preview_frame";
const bool DEFAULT_REMEMBER_LAST_PREVIEW_FRAME = false;
const char LAST_PREVIEW_FRAME_KEY[] = "last_preview_frame";
const int DEFAULT_LAST_PREVIEW_FRAME = 0;
const char NEW_SCRIPT_TEMPLATE_KEY[] = "new_script_template";

//==============================================================================

const char HOTKEYS_GROUP[] = "hotkeys";

const char ACTION_ID_NEW_SCRIPT[] = "new_script";
const char ACTION_ID_OPEN_SCRIPT[] = "open_script";
const char ACTION_ID_SAVE_SCRIPT[] = "save_script";
const char ACTION_ID_SAVE_SCRIPT_AS[] = "save_script_as";
const char ACTION_ID_TEMPLATES[] = "templates";
const char ACTION_ID_SETTINGS[] = "settings";
const char ACTION_ID_PREVIEW[] = "preview";
const char ACTION_ID_CHECK_SCRIPT[] = "check_script";
const char ACTION_ID_BENCHMARK[] = "benchmark";
const char ACTION_ID_CLI_ENCODE[] = "cli_encode";
const char ACTION_ID_EXIT[] = "exit";
const char ACTION_ID_ABOUT[] = "about";
const char ACTION_ID_AUTOCOMPLETE[] = "autocomplete";
const char ACTION_ID_SAVE_SNAPSHOT[] = "save_snapshot";
const char ACTION_ID_TOGGLE_ZOOM_PANEL[] = "toggle_zoom_panel";
const char ACTION_ID_SET_ZOOM_MODE_NO_ZOOM[] = "set_zoom_mode_no_zoom";
const char ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO[] = "set_zoom_mode_fixed_ratio";
const char ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME[] =
	"set_zoom_mode_fit_to_frame";
const char ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST[] =
	"set_zoom_scale_mode_nearest";
const char ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR[] =
	"set_zoom_scale_mode_bilinear";
const char ACTION_ID_TOGGLE_CROP_PANEL[] = "toggle_crop_panel";
const char ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT[] =
	"paste_crop_snippet_into_script";
const char ACTION_ID_FRAME_TO_CLIPBOARD[] = "frame_to_clipboard";
const char ACTION_ID_TOGGLE_TIMELINE_PANEL[] = "toggle_timeline_panel";
const char ACTION_ID_SET_TIMELINE_MODE_TIME[] = "set_timeline_mode_time";
const char ACTION_ID_SET_TIMELINE_MODE_FRAMES[] = "set_timeline_mode_frames";
const char ACTION_ID_TIME_STEP_FORWARD[] = "time_step_forward";
const char ACTION_ID_TIME_STEP_BACK[] = "time_step_back";
const char ACTION_ID_ADVANCED_PREVIEW_SETTINGS[] = "advanced_preview_settings";
const char ACTION_ID_TOGGLE_COLOR_PICKER[] = "toggle_color_picker";
const char ACTION_ID_PLAY[] = "play";
const char ACTION_ID_DUPLICATE_SELECTION[] = "duplicate_selection";
const char ACTION_ID_COMMENT_SELECTION[] = "comment_selection";
const char ACTION_ID_UNCOMMENT_SELECTION[] = "uncomment_selection";
const char ACTION_ID_REPLACE_TAB_WITH_SPACES[] = "replace_tab_with_spaces";

//==============================================================================

const char THEME_GROUP[] = "theme";

const char TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT[] = "common_script_text";
const char TEXT_FORMAT_ID_KEYWORD[] = "keyword";
const char TEXT_FORMAT_ID_OPERATOR[] = "operator";
const char TEXT_FORMAT_ID_STRING[] = "string";
const char TEXT_FORMAT_ID_NUMBER[] = "number";
const char TEXT_FORMAT_ID_COMMENT[] = "comment";
const char TEXT_FORMAT_ID_VS_CORE[] = "vs_core";
const char TEXT_FORMAT_ID_VS_NAMESPACE[] = "vs_namespace";
const char TEXT_FORMAT_ID_VS_FUNCTION[] = "vs_function";
const char TEXT_FORMAT_ID_VS_ARGUMENT[] = "vs_argument";
const char TEXT_FORMAT_ID_TIMELINE[] = "timeline_text";

const char COLOR_ID_TEXT_BACKGROUND[] = "text_background_color";
const char COLOR_ID_ACTIVE_LINE[] = "active_line_color";

const double DEFAULT_TIMELINE_LABELS_HEIGHT = 5.0;

//==============================================================================

const char ENCODING_PRESETS_GROUP[] = "encoding_presets";

const char ENCODING_PRESET_ENCODING_TYPE_KEY[] = "encoding_type";
const char ENCODING_PRESET_HEADER_TYPE_KEY[] = "header_type";
const char ENCODING_PRESET_EXECUTABLE_PATH_KEY[] = "executable_path";
const char ENCODING_PRESET_ARGUMENTS_KEY[] = "arguments";

const EncodingType DEFAULT_ENCODING_TYPE = EncodingType::CLI;
const EncodingHeaderType DEFAULT_ENCODING_HEADER_TYPE =
	EncodingHeaderType::NoHeader;

//==============================================================================

EncodingPreset::EncodingPreset():
	  type(DEFAULT_ENCODING_TYPE)
	, headerType(DEFAULT_ENCODING_HEADER_TYPE)
{
}

EncodingPreset::EncodingPreset(const QString & a_name):
	  name(a_name)
	, type(DEFAULT_ENCODING_TYPE)
	, headerType(DEFAULT_ENCODING_HEADER_TYPE)
{
}

bool EncodingPreset::operator==(const EncodingPreset & a_other) const
{
	return (name == a_other.name);
}

bool EncodingPreset::operator<(const EncodingPreset & a_other) const
{
	return (name < a_other.name);
}

bool EncodingPreset::isEmpty() const
{
	return name.isEmpty();
}

//==============================================================================

SettingsManager::SettingsManager(QObject* a_pParent) : QObject(a_pParent)
{
	QString applicationDir = QCoreApplication::applicationDirPath();

	bool portableMode = getPortableMode();
	if(portableMode)
		m_settingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	else
	{
		m_settingsFilePath = QStandardPaths::writableLocation(
			QStandardPaths::GenericConfigLocation) + SETTINGS_FILE_NAME;
	}

	initializeDefaultHotkeysMap();
}

SettingsManager::~SettingsManager()
{

}

//==============================================================================

bool SettingsManager::getPortableMode() const
{
	QString applicationDir = QCoreApplication::applicationDirPath();
	QString settingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	QFileInfo settingsFileInfo(settingsFilePath);

	bool portableMode = (settingsFileInfo.exists() &&
		settingsFileInfo.isWritable());
	return portableMode;
}

bool SettingsManager::setPortableMode(bool a_portableMod)
{
	bool currentModePortable = getPortableMode();

	if(a_portableMod == currentModePortable)
		return true;

	QString applicationDir = QCoreApplication::applicationDirPath();
	QString genericConfigDir = QStandardPaths::writableLocation(
		QStandardPaths::GenericConfigLocation);

	QString newSettingsFilePath;
	if(a_portableMod)
		newSettingsFilePath = applicationDir + SETTINGS_FILE_NAME;
	else
		newSettingsFilePath = genericConfigDir + SETTINGS_FILE_NAME;

	// When copying portable settings to common folder - another settings
	// file may already exist there. Need to delete it first.
	if(QFile::exists(newSettingsFilePath))
	{
		bool settingsFileDeleted = QFile::remove(newSettingsFilePath);
		if(!settingsFileDeleted)
			return false;
	}

	bool settingsFileCopied =
		QFile::copy(m_settingsFilePath, newSettingsFilePath);
	QString oldSettingsFilePath = m_settingsFilePath;
	m_settingsFilePath = newSettingsFilePath;

	if(a_portableMod)
		return settingsFileCopied;
	else if(settingsFileCopied)
	{
		bool portableSettingsFileDeleted = QFile::remove(oldSettingsFilePath);
		return portableSettingsFileDeleted;
	}

	return false;
}

//==============================================================================

QVariant SettingsManager::valueInGroup(const QString & a_group,
	const QString & a_key, const QVariant & a_defaultValue) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(a_group);
	return settings.value(a_key, a_defaultValue);
}

bool SettingsManager::setValueInGroup(const QString & a_group,
	const QString & a_key, const QVariant & a_value)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(a_group);
	settings.setValue(a_key, a_value);
	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

//==============================================================================

QVariant SettingsManager::value(const QString & a_key,
	const QVariant & a_defaultValue) const
{
	return valueInGroup(COMMON_GROUP, a_key, a_defaultValue);
}

bool SettingsManager::setValue(const QString & a_key,
	const QVariant & a_value)
{
	return setValueInGroup(COMMON_GROUP, a_key, a_value);
}

//==============================================================================

void SettingsManager::initializeDefaultHotkeysMap()
{
	m_defaultHotkeysMap =
	{
		{ACTION_ID_NEW_SCRIPT, QKeySequence(Qt::CTRL + Qt::Key_N)},
		{ACTION_ID_OPEN_SCRIPT, QKeySequence(Qt::CTRL + Qt::Key_O)},
		{ACTION_ID_SAVE_SCRIPT, QKeySequence(Qt::CTRL + Qt::Key_S)},
		{ACTION_ID_PREVIEW, QKeySequence(Qt::Key_F5)},
		{ACTION_ID_CHECK_SCRIPT, QKeySequence(Qt::Key_F6)},
		{ACTION_ID_BENCHMARK, QKeySequence(Qt::Key_F7)},
		{ACTION_ID_CLI_ENCODE, QKeySequence(Qt::Key_F8)},
		{ACTION_ID_EXIT, QKeySequence(Qt::ALT + Qt::Key_F4)},
		{ACTION_ID_AUTOCOMPLETE, QKeySequence(Qt::CTRL + Qt::Key_Space)},
		{ACTION_ID_SAVE_SNAPSHOT, QKeySequence(Qt::Key_S)},
		{ACTION_ID_TOGGLE_ZOOM_PANEL, QKeySequence(Qt::Key_Z)},
		{ACTION_ID_TOGGLE_CROP_PANEL, QKeySequence(Qt::Key_C)},
		{ACTION_ID_SET_ZOOM_MODE_NO_ZOOM, QKeySequence(Qt::Key_1)},
		{ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO, QKeySequence(Qt::Key_2)},
		{ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME, QKeySequence(Qt::Key_3)},
		{ACTION_ID_TIME_STEP_FORWARD, QKeySequence(Qt::CTRL + Qt::Key_Right)},
		{ACTION_ID_TIME_STEP_BACK, QKeySequence(Qt::CTRL + Qt::Key_Left)},
		{ACTION_ID_FRAME_TO_CLIPBOARD, QKeySequence(Qt::Key_X)},
		{ACTION_ID_DUPLICATE_SELECTION, QKeySequence(Qt::CTRL + Qt::Key_D)},
		{ACTION_ID_COMMENT_SELECTION,
			QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C)},
		{ACTION_ID_UNCOMMENT_SELECTION,
			QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_X)},
	};
}

QKeySequence SettingsManager::getDefaultHotkey(const QString & a_actionID) const
{
	std::map<QString, QKeySequence>::const_iterator it =
		m_defaultHotkeysMap.find(a_actionID);
	if(it != m_defaultHotkeysMap.end())
		return it->second;

	return QKeySequence();
}

QKeySequence SettingsManager::getHotkey(const QString & a_actionID) const
{
	QKeySequence hotkey =
		valueInGroup(HOTKEYS_GROUP, a_actionID).value<QKeySequence>();
	if(hotkey.isEmpty())
		hotkey = getDefaultHotkey(a_actionID);
	return hotkey;
}

bool SettingsManager::setHotkey(const QString & a_actionID,
	const QKeySequence & a_hotkey)
{
	return setValueInGroup(HOTKEYS_GROUP, a_actionID, a_hotkey);
}

//==============================================================================

QTextCharFormat SettingsManager::getDefaultTextFormat(
	const QString & a_textFormatID) const
{
	// Standard "Icecream" theme

	QTextCharFormat defaultFormat;

	if(a_textFormatID == TEXT_FORMAT_ID_COMMON_SCRIPT_TEXT)
	{
		QFont commonScriptFont = defaultFormat.font();
		commonScriptFont.setFamily("monospace");
		commonScriptFont.setStyleHint(QFont::Monospace);
		commonScriptFont.setFixedPitch(true);
		commonScriptFont.setKerning(false);
		commonScriptFont.setPointSize(10);
		defaultFormat.setFont(commonScriptFont);

		return defaultFormat;
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_KEYWORD)
	{
		defaultFormat.setForeground(QColor("#0EAA95"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_OPERATOR)
	{
		defaultFormat.setForeground(QColor("#b9672a"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_STRING)
	{
		defaultFormat.setForeground(QColor("#a500bc"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_NUMBER)
	{
		defaultFormat.setForeground(QColor("#3f8300"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_COMMENT)
	{
		defaultFormat.setForeground(QColor("#808080"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_CORE)
	{
		defaultFormat.setForeground(QColor("#0673E0"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_NAMESPACE)
	{
		defaultFormat.setForeground(QColor("#0673E0"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_FUNCTION)
	{
		defaultFormat.setForeground(QColor("#0673E0"));
		defaultFormat.setFontWeight(QFont::Bold);
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_VS_ARGUMENT)
	{
		defaultFormat.setForeground(QColor("#a500bc"));
	}
	else if(a_textFormatID == TEXT_FORMAT_ID_TIMELINE)
	{
		QFont timelineLabelsFont = defaultFormat.font();
		timelineLabelsFont.setFamily(QString("Digital Mini"));

		QFontMetricsF metrics(timelineLabelsFont);
		qreal factor = (qreal)DEFAULT_TIMELINE_LABELS_HEIGHT /
			metrics.tightBoundingRect("9").height();
		qreal currentFontSize = timelineLabelsFont.pointSizeF();
		timelineLabelsFont.setPointSizeF(currentFontSize * factor);

		defaultFormat.setFont(timelineLabelsFont);
	}

	return defaultFormat;
}

QTextCharFormat SettingsManager::getTextFormat(const QString & a_textFormatID)
	const
{
	QVariant textFormatValue = valueInGroup(THEME_GROUP, a_textFormatID,
		getDefaultTextFormat(a_textFormatID));
	return qvariant_cast<QTextFormat>(textFormatValue).toCharFormat();
}

bool SettingsManager::setTextFormat(const QString & a_textFormatID,
	const QTextCharFormat & a_format)
{
	return setValueInGroup(THEME_GROUP, a_textFormatID, a_format);
}

//==============================================================================

QColor SettingsManager::getDefaultColor(const QString & a_colorID) const
{
	QColor defaultColor;

	QPalette defaultPalette;

	if(a_colorID == COLOR_ID_TEXT_BACKGROUND)
	{
		defaultColor = defaultPalette.color(QPalette::Active, QPalette::Base);
	}
	else if(a_colorID == COLOR_ID_ACTIVE_LINE)
	{
		// TODO: find a better way to achieve default active line color?
		defaultColor = getColor(COLOR_ID_TEXT_BACKGROUND);
		qreal lightness = defaultColor.lightnessF();
		if(lightness >= 0.5)
			defaultColor = defaultColor.darker(110);
		else
			defaultColor = defaultColor.lighter(150);
	}

	return defaultColor;
}

QColor SettingsManager::getColor(const QString & a_colorID) const
{
	QVariant colorValue = valueInGroup(THEME_GROUP, a_colorID,
		getDefaultColor(a_colorID));
	return qvariant_cast<QColor>(colorValue);
}

bool SettingsManager::setColor(const QString & a_colorID,
	const QColor & a_color)
{
	return setValueInGroup(THEME_GROUP, a_colorID, a_color);
}

//==============================================================================

QString SettingsManager::getLastUsedPath() const
{
	QStringList recentFilesList = getRecentFilesList();
	if(!recentFilesList.isEmpty())
		return recentFilesList.first();

	return QString();
}

bool SettingsManager::setLastUsedPath(const QString& a_lastUsedPath)
{
	return addToRecentFilesList(a_lastUsedPath);
}

//==============================================================================

QByteArray SettingsManager::getMainWindowGeometry() const
{
	return value(MAIN_WINDOW_GEOMETRY_KEY).toByteArray();
}

bool SettingsManager::setMainWindowGeometry(
	const QByteArray & a_mainWindowGeometry)
{
	return setValue(MAIN_WINDOW_GEOMETRY_KEY, a_mainWindowGeometry);
}

//==============================================================================

bool SettingsManager::getMainWindowMaximized() const
{
	return value(MAIN_WINDOW_MAXIMIZED_KEY,
		DEFAULT_MAIN_WINDOW_MAXIMIZED).toBool();
}

bool SettingsManager::setMainWindowMaximized(bool a_mainWindowMaximized)
{
	return setValue(MAIN_WINDOW_MAXIMIZED_KEY, a_mainWindowMaximized);
}

//==============================================================================

QByteArray SettingsManager::getPreviewDialogGeometry() const
{
	return value(PREVIEW_DIALOG_GEOMETRY_KEY).toByteArray();
}

bool SettingsManager::setPreviewDialogGeometry(
	const QByteArray & a_previewDialogGeometry)
{
	return setValue(PREVIEW_DIALOG_GEOMETRY_KEY, a_previewDialogGeometry);
}

//==============================================================================

bool SettingsManager::getPreviewDialogMaximized() const
{
	return value(PREVIEW_DIALOG_MAXIMIZED_KEY,
		DEFAULT_PREVIEW_DIALOG_MAXIMIZED).toBool();
}

bool SettingsManager::setPreviewDialogMaximized(bool a_previewDialogMaximized)
{
	return setValue(PREVIEW_DIALOG_MAXIMIZED_KEY, a_previewDialogMaximized);
}

//==============================================================================

bool SettingsManager::getAutoLoadLastScript() const
{
	return value(AUTO_LOAD_LAST_SCRIPT_KEY, DEFAULT_AUTO_LOAD_LAST_SCRIPT)
		.toBool();
}

bool SettingsManager::setAutoLoadLastScript(bool a_autoLoadLastScript)
{
	return setValue(AUTO_LOAD_LAST_SCRIPT_KEY, a_autoLoadLastScript);
}

//==============================================================================

bool SettingsManager::getZoomPanelVisible() const
{
	return value(ZOOM_PANEL_VISIBLE_KEY, DEFAULT_ZOOM_PANEL_VISIBLE).toBool();
}

bool SettingsManager::setZoomPanelVisible(bool a_zoomPanelVisible)
{
	return setValue(ZOOM_PANEL_VISIBLE_KEY, a_zoomPanelVisible);
}

//==============================================================================

ZoomMode SettingsManager::getZoomMode() const
{
	return (ZoomMode)value(ZOOM_MODE_KEY, (int)DEFAULT_ZOOM_MODE).toInt();
}

bool SettingsManager::setZoomMode(ZoomMode a_zoomMode)
{
	return setValue(ZOOM_MODE_KEY, (int)a_zoomMode);
}

//==============================================================================

double SettingsManager::getZoomRatio() const
{
	return value(ZOOM_RATIO_KEY, DEFAULT_ZOOM_RATIO).toDouble();
}

bool SettingsManager::setZoomRatio(double a_zoomRatio)
{
	return setValue(ZOOM_RATIO_KEY, a_zoomRatio);
}

//==============================================================================

Qt::TransformationMode SettingsManager::getScaleMode() const
{
	return (Qt::TransformationMode)value(SCALE_MODE_KEY,
		(int)DEFAULT_SCALE_MODE).toInt();
}

bool SettingsManager::setScaleMode(Qt::TransformationMode a_scaleMode)
{
	return setValue(SCALE_MODE_KEY, (int)a_scaleMode);
}

//==============================================================================

CropMode SettingsManager::getCropMode() const
{
	return (CropMode)value(CROP_MODE_KEY, (int)DEFAULT_CROP_MODE).toInt();
}

bool SettingsManager::setCropMode(CropMode a_cropMode)
{
	return setValue(CROP_MODE_KEY, (int)a_cropMode);
}

//==============================================================================

int SettingsManager::getCropZoomRatio() const
{
	return value(CROP_ZOOM_RATIO_KEY, DEFAULT_CROP_ZOOM_RATIO).toInt();
}

bool SettingsManager::setCropZoomRatio(int a_cropZoomRatio)
{
	return setValue(CROP_ZOOM_RATIO_KEY, a_cropZoomRatio);
}

//==============================================================================

bool SettingsManager::getPromptToSaveChanges() const
{
	return value(PROMPT_TO_SAVE_CHANGES_KEY,
		DEFAULT_PROMPT_TO_SAVE_CHANGES).toBool();
}

bool SettingsManager::setPromptToSaveChanges(bool a_prompt)
{
	return setValue(PROMPT_TO_SAVE_CHANGES_KEY, a_prompt);
}

//==============================================================================

QStringList SettingsManager::getRecentFilesList() const
{
	return value(RECENT_FILES_LIST_KEY).toStringList();
}

bool SettingsManager::addToRecentFilesList(const QString & a_filePath)
{
	QFileInfo fileInfo(a_filePath);
	QString canonicalPath = fileInfo.canonicalFilePath();
	QStringList recentFilesList = getRecentFilesList();
	recentFilesList.removeAll(canonicalPath);
	recentFilesList.prepend(canonicalPath);
	unsigned int maxRecentFilesNumber = getMaxRecentFilesNumber();
	while((unsigned int)recentFilesList.size() > maxRecentFilesNumber)
		recentFilesList.removeLast();
	return setValue(RECENT_FILES_LIST_KEY, recentFilesList);
}

//==============================================================================

unsigned int SettingsManager::getMaxRecentFilesNumber() const
{
	return value(MAX_RECENT_FILES_NUMBER_KEY,
		DEFAULT_MAX_RECENT_FILES_NUMBER).toUInt();
}

bool SettingsManager::setMaxRecentFilesNumber(
	unsigned int a_maxRecentFilesNumber)
{
	return setValue(MAX_RECENT_FILES_NUMBER_KEY, a_maxRecentFilesNumber);
}

//==============================================================================

QStringList SettingsManager::getVapourSynthLibraryPaths() const
{
	QStringList paths = value(VAPOURSYNTH_LIBRARY_PATHS_KEY).toStringList();
	paths.removeDuplicates();
	return paths;
}

bool SettingsManager::setVapourSynthLibraryPaths(
	const QStringList & a_pathsList)
{
	return setValue(VAPOURSYNTH_LIBRARY_PATHS_KEY, a_pathsList);
}

//==============================================================================

QStringList SettingsManager::getVapourSynthPluginsPaths() const
{
	QStringList paths = value(VAPOURSYNTH_PLUGINS_PATHS_KEY).toStringList();
	paths.removeDuplicates();
	return paths;
}

bool SettingsManager::setVapourSynthPluginsPaths(
	const QStringList & a_pathsList)
{
	return setValue(VAPOURSYNTH_PLUGINS_PATHS_KEY, a_pathsList);
}

//==============================================================================

QStringList SettingsManager::getVapourSynthDocumentationPaths() const
{
	QStringList paths = value(VAPOURSYNTH_DOCUMENTATION_PATHS_KEY,
		DEFAULT_DOCUMENTATION_PATHS).toStringList();
	paths.removeDuplicates();
	return paths;
}

bool SettingsManager::setVapourSynthDocumentationPaths(
	const QStringList & a_pathsList)
{
	return setValue(VAPOURSYNTH_DOCUMENTATION_PATHS_KEY, a_pathsList);
}

//==============================================================================

int SettingsManager::getCharactersTypedToStartCompletion() const
{
	return value(CHARACTERS_TYPED_TO_START_COMPLETION_KEY,
		DEFAULT_CHARACTERS_TYPED_TO_START_COMPLETION).toInt();
}

bool SettingsManager::setCharactersTypedToStartCompletion(
	int a_charactersNumber)
{
	return setValue(CHARACTERS_TYPED_TO_START_COMPLETION_KEY,
		a_charactersNumber);
}

//==============================================================================

TimeLineSlider::DisplayMode SettingsManager::getTimeLineMode() const
{
	return (TimeLineSlider::DisplayMode)value(TIMELINE_MODE_KEY,
		(int)DEFAULT_TIMELINE_MODE).toInt();
}

bool SettingsManager::setTimeLineMode(
	TimeLineSlider::DisplayMode a_timeLineMode)
{
	return setValue(TIMELINE_MODE_KEY, (int)a_timeLineMode);
}

//==============================================================================

double SettingsManager::getTimeStep() const
{
	return value(TIME_STEP_KEY, DEFAULT_TIME_STEP).toDouble();
}

bool SettingsManager::setTimeStep(double a_timeStep)
{
	return setValue(TIME_STEP_KEY, a_timeStep);
}

//==============================================================================

ResamplingFilter SettingsManager::getChromaResamplingFilter() const
{
	return (ResamplingFilter)value(CHROMA_RESAMPLING_FILTER_KEY,
		(int)DEFAULT_CHROMA_RESAMPLING_FILTER).toInt();
}

bool SettingsManager::setChromaResamplingFilter(ResamplingFilter a_filter)
{
	return setValue(CHROMA_RESAMPLING_FILTER_KEY, (int)a_filter);
}

//==============================================================================

YuvMatrixCoefficients SettingsManager::getYuvMatrixCoefficients() const
{
	return (YuvMatrixCoefficients)value(YUV_MATRIX_COEFFICIENTS_KEY,
		(int)DEFAULT_YUV_MATRIX_COEFFICIENTS).toInt();
}

bool SettingsManager::setYuvMatrixCoefficients(
	YuvMatrixCoefficients a_matrix)
{
	return setValue(YUV_MATRIX_COEFFICIENTS_KEY, (int)a_matrix);
}

//==============================================================================

ChromaPlacement SettingsManager::getChromaPlacement() const
{
	return (ChromaPlacement)value(CHROMA_PLACEMENT_KEY,
		(int)DEFAULT_CHROMA_PLACEMENT).toInt();
}

bool SettingsManager::setChromaPlacement(ChromaPlacement a_placement)
{
	return setValue(CHROMA_PLACEMENT_KEY, (int)a_placement);
}

//==============================================================================

double SettingsManager::getBicubicFilterParameterB() const
{
	return value(BICUBIC_FILTER_PARAMETER_B_KEY,
		DEFAULT_BICUBIC_FILTER_PARAMETER_B).toDouble();
}

bool SettingsManager::setBicubicFilterParameterB(double a_parameterB)
{
	return setValue(BICUBIC_FILTER_PARAMETER_B_KEY, a_parameterB);
}

//==============================================================================

double SettingsManager::getBicubicFilterParameterC() const
{
	return value(BICUBIC_FILTER_PARAMETER_C_KEY,
		DEFAULT_BICUBIC_FILTER_PARAMETER_C).toDouble();
}

bool SettingsManager::setBicubicFilterParameterC(double a_parameterC)
{
	return setValue(BICUBIC_FILTER_PARAMETER_C_KEY, a_parameterC);
}

//==============================================================================

int SettingsManager::getLanczosFilterTaps() const
{
	return value(LANCZOS_FILTER_TAPS_KEY, DEFAULT_LANCZOS_FILTER_TAPS).toInt();
}

bool SettingsManager::setLanczosFilterTaps(int a_taps)
{
	return setValue(LANCZOS_FILTER_TAPS_KEY, a_taps);
}

//==============================================================================

bool SettingsManager::getColorPickerVisible() const
{
	return value(COLOR_PICKER_VISIBLE_KEY,
		DEFAULT_COLOR_PICKER_VISIBLE).toBool();
}

bool SettingsManager::setColorPickerVisible(bool a_colorPickerVisible)
{
	return setValue(COLOR_PICKER_VISIBLE_KEY, a_colorPickerVisible);
}

//==============================================================================

PlayFPSLimitMode SettingsManager::getPlayFPSLimitMode() const
{
	return (PlayFPSLimitMode)value(PLAY_FPS_LIMIT_MODE_KEY,
		(int)DEFAULT_PLAY_FPS_LIMIT_MODE).toInt();
}

bool SettingsManager::setPlayFPSLimitMode(PlayFPSLimitMode a_mode)
{
	return setValue(PLAY_FPS_LIMIT_MODE_KEY, (int)a_mode);
}

//==============================================================================

double SettingsManager::getPlayFPSLimit() const
{
	return value(PLAY_FPS_LIMIT_KEY, DEFAULT_PLAY_FPS_LIMIT).toDouble();
}

bool SettingsManager::setPlayFPSLimit(double a_limit)
{
	return setValue(PLAY_FPS_LIMIT_KEY, a_limit);
}

//==============================================================================

std::vector<EncodingPreset> SettingsManager::getAllEncodingPresets() const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);

	std::vector<EncodingPreset> presets;

	QStringList presetNames = settings.childGroups();
	for(const QString & presetName : presetNames)
	{
		settings.beginGroup(presetName);

		EncodingPreset preset;
		preset.name = presetName;
		preset.type = (EncodingType)settings.value(
			ENCODING_PRESET_ENCODING_TYPE_KEY, (int)DEFAULT_ENCODING_TYPE)
			.toInt();
		preset.headerType = (EncodingHeaderType)settings.value(
			ENCODING_PRESET_HEADER_TYPE_KEY, (int)DEFAULT_ENCODING_HEADER_TYPE)
			.toInt();
		preset.executablePath = settings.value(
			ENCODING_PRESET_EXECUTABLE_PATH_KEY).toString();
		preset.arguments = settings.value(
			ENCODING_PRESET_ARGUMENTS_KEY).toString();
		presets.push_back(preset);

		settings.endGroup();
	}

	return presets;
}

EncodingPreset SettingsManager::getEncodingPreset(const QString & a_name) const
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);

	EncodingPreset preset;

	QStringList presetNames = settings.childGroups();
	if(!presetNames.contains(a_name))
		return preset;

	preset.name = a_name;
	settings.beginGroup(a_name);

	preset.type = (EncodingType)settings.value(
		ENCODING_PRESET_ENCODING_TYPE_KEY, (int)DEFAULT_ENCODING_TYPE)
		.toInt();
	preset.headerType = (EncodingHeaderType)settings.value(
		ENCODING_PRESET_HEADER_TYPE_KEY, (int)DEFAULT_ENCODING_HEADER_TYPE)
		.toInt();
	preset.executablePath = settings.value(
		ENCODING_PRESET_EXECUTABLE_PATH_KEY).toString();
	preset.arguments = settings.value(
		ENCODING_PRESET_ARGUMENTS_KEY).toString();

	return preset;
}

bool SettingsManager::saveEncodingPreset(const EncodingPreset & a_preset)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);
	settings.beginGroup(a_preset.name);

	settings.setValue(ENCODING_PRESET_ENCODING_TYPE_KEY, (int)a_preset.type);
	settings.setValue(ENCODING_PRESET_HEADER_TYPE_KEY,
		(int)a_preset.headerType);
	settings.setValue(ENCODING_PRESET_EXECUTABLE_PATH_KEY,
		a_preset.executablePath);
	settings.setValue(ENCODING_PRESET_ARGUMENTS_KEY, a_preset.arguments);

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

bool SettingsManager::deleteEncodingPreset(const EncodingPreset & a_preset)
{
	QSettings settings(m_settingsFilePath, QSettings::IniFormat);
	settings.beginGroup(ENCODING_PRESETS_GROUP);

	QStringList subGroups = settings.childGroups();
	if(!subGroups.contains(a_preset.name))
		return false;
	settings.remove(a_preset.name);

	settings.sync();
	bool success = (QSettings::NoError == settings.status());
	return success;
}

//==============================================================================

bool SettingsManager::getUseSpacesAsTab() const
{
	return value(USE_SPACES_AS_TAB_KEY, DEFAULT_USE_SPACES_AS_TAB).toBool();
}

bool SettingsManager::setUseSpacesAsTab(bool a_value)
{
	return setValue(USE_SPACES_AS_TAB_KEY, a_value);
}

//==============================================================================

int SettingsManager::getSpacesInTab() const
{
	return value(SPACES_IN_TAB_KEY, DEFAULT_SPACES_IN_TAB).toInt();
}

bool SettingsManager::setSpacesInTab(int a_spacesNumber)
{
	return setValue(SPACES_IN_TAB_KEY, a_spacesNumber);
}

//==============================================================================

QString SettingsManager::getTabText() const
{
	QString text = "\t";
	bool useSpacesAsTab = getUseSpacesAsTab();
	if(useSpacesAsTab)
	{
		int spacesInTab = getSpacesInTab();
		text.fill(' ', spacesInTab);
	}
	return text;
}

//==============================================================================

bool SettingsManager::getRememberLastPreviewFrame() const
{
	return value(REMEMBER_LAST_PREVIEW_FRAME_KEY,
		DEFAULT_REMEMBER_LAST_PREVIEW_FRAME).toBool();
}

bool SettingsManager::setRememberLastPreviewFrame(bool a_remember)
{
	return setValue(REMEMBER_LAST_PREVIEW_FRAME_KEY, a_remember);
}

//==============================================================================

int SettingsManager::getLastPreviewFrame() const
{
	return value(LAST_PREVIEW_FRAME_KEY, DEFAULT_LAST_PREVIEW_FRAME).toInt();
}

bool SettingsManager::setLastPreviewFrame(int a_frameNumber)
{
	return setValue(LAST_PREVIEW_FRAME_KEY, a_frameNumber);
}

//==============================================================================

QString SettingsManager::getDefaultNewScriptTemplate()
{
	return QString(
		"import vapoursynth as vs\n"
		"core = vs.get_core()\n"
	);
}

QString SettingsManager::getNewScriptTemplate()
{
	return value(NEW_SCRIPT_TEMPLATE_KEY,
		getDefaultNewScriptTemplate()).toString();
}

bool SettingsManager::setNewScriptTemplate(const QString & a_text)
{
	return setValue(NEW_SCRIPT_TEMPLATE_KEY, a_text);
}

//==============================================================================
