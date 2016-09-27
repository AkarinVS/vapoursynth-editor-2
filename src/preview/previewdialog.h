#ifndef PREVIEWDIALOG_H_INCLUDED
#define PREVIEWDIALOG_H_INCLUDED

#include <QPixmap>
#include <QIcon>
#include <map>
#include <vector>
#include <set>
#include <chrono>

#include "../vapoursynth/vs_script_processor_dialog.h"
#include "../settings/settingsmanager.h"
#include "../common/chrono.h"

#include <ui_previewdialog.h>

class QEvent;
class QCloseEvent;
class QMoveEvent;
class QResizeEvent;
class QKeyEvent;
class QEvent;
class QStatusBar;
class QMenu;
class QActionGroup;
class QAction;
class QTimer;
class SettingsDialog;
class PreviewAdvancedSettingsDialog;
class ZimgPreviewConverter;

class PreviewDialog : public VSScriptProcessorDialog
{
	Q_OBJECT

	public:

		PreviewDialog(SettingsManager * a_pSettingsManager,
			SettingsDialog * a_pSettingsDialog,
			QWidget * a_pParent = nullptr);
		virtual ~PreviewDialog();

		void previewScript(const QString& a_script,
			const QString& a_scriptName);

	signals:

		void signalInsertLineIntoScript(const QString& a_line);

	protected slots:

		virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
			const VSFrameRef * a_cpFrameRef) override;

		void slotShowFrame(int a_frameNumber);

		void slotSaveSnapshot();

		void slotToggleZoomPanelVisible(bool a_zoomPanelVisible);

		void slotZoomModeChanged();

		void slotZoomRatioChanged(double a_zoomRatio);

		void slotScaleModeChanged();

		void slotToggleCropPanelVisible(bool a_cropPanelVisible);

		void slotCropModeChanged();

		void slotCropLeftValueChanged(int a_value);

		void slotCropTopValueChanged(int a_value);

		void slotCropWidthValueChanged(int a_value);

		void slotCropHeightValueChanged(int a_value);

		void slotCropRightValueChanged(int a_value);

		void slotCropBottomValueChanged(int a_value);

		void slotCropZoomRatioValueChanged(int a_cropZoomRatio);

		void slotPasteCropSnippetIntoScript();

		void slotToggleTimeLinePanelVisible(bool a_timeLinePanelVisible);

		void slotTimeLineModeChanged();

		void slotTimeStepChanged(const QTime & a_time);

		void slotTimeStepForward();

		void slotTimeStepBack();

		void slotSettingsChanged();

		void slotPreviewAreaSizeChanged();

		void slotPreviewAreaCtrlWheel(QPoint a_angleDelta);

		void slotPreviewAreaMouseMiddleButtonReleased();

		void slotPreviewAreaMouseRightButtonReleased();

		void slotPreviewAreaMouseOverPoint(float a_normX, float a_normY);

		void slotFrameToClipboard();

		void slotAdvancedSettingsChanged();

		void slotToggleColorPicker(bool a_colorPickerVisible);

		void slotSetPlayFPSLimit();

		void slotPlay(bool a_play);

		void slotProcessPlayQueue();

	protected:

		virtual void stopAndCleanUp() override;

		void moveEvent(QMoveEvent * a_pEvent) override;

		void resizeEvent(QResizeEvent * a_pEvent) override;

		void changeEvent(QEvent * a_pEvent) override;

		void keyPressEvent(QKeyEvent * a_pEvent) override;

		void createActionsAndMenus();

		void setUpZoomPanel();

		void setUpTimeLinePanel();

		void setUpCropPanel();

		bool requestShowFrame(int a_frameNumber);

		void setPreviewPixmap();

		void recalculateCropMods();

		void resetCropSpinBoxes();

		void setCurrentFrame(const VSFrameRef * a_cpFrameRef);

		double valueAtPoint(size_t a_x, size_t a_y, int a_plane);

		Ui::PreviewDialog m_ui;

		SettingsManager * m_pSettingsManager;

		SettingsDialog * m_pSettingsDialog;

		PreviewAdvancedSettingsDialog * m_pAdvancedSettingsDialog;

		int m_frameExpected;
		int m_frameShown;
		int m_lastFrameRequestedForPlay;

		int m_bigFrameStep;

		const VSFrameRef * m_cpFrameRef;
		QPixmap m_framePixmap;

		bool m_changingCropValues;

		QMenu * m_pPreviewContextMenu;
		QAction * m_pActionFrameToClipboard;
		QAction * m_pActionSaveSnapshot;
		QAction * m_pActionToggleZoomPanel;
		QMenu * m_pMenuZoomModes;
		QActionGroup * m_pActionGroupZoomModes;
		QAction * m_pActionSetZoomModeNoZoom;
		QAction * m_pActionSetZoomModeFixedRatio;
		QAction * m_pActionSetZoomModeFitToFrame;
		QMenu * m_pMenuZoomScaleModes;
		QActionGroup * m_pActionGroupZoomScaleModes;
		QAction * m_pActionSetZoomScaleModeNearest;
		QAction * m_pActionSetZoomScaleModeBilinear;
		QAction * m_pActionToggleCropPanel;
		QAction * m_pActionToggleTimeLinePanel;
		QMenu * m_pMenuTimeLineModes;
		QActionGroup * m_pActionGroupTimeLineModes;
		QAction * m_pActionSetTimeLineModeTime;
		QAction * m_pActionSetTimeLineModeFrames;
		QAction * m_pActionTimeStepForward;
		QAction * m_pActionTimeStepBack;
		QAction * m_pActionPasteCropSnippetIntoScript;
		QAction * m_pActionAdvancedSettingsDialog;
		QAction * m_pActionToggleColorPicker;
		QAction * m_pActionPlay;

		std::map<QString, ZoomMode> m_actionIDToZoomModeMap;

		std::map<QString, Qt::TransformationMode> m_actionIDToZoomScaleModeMap;

		std::map<QString, TimeLineSlider::DisplayMode>
			m_actionIDToTimeLineModeMap;

		std::vector<QAction *> m_settableActionsList;

		bool m_playing;
		bool m_processingPlayQueue;
		double m_secondsBetweenFrames;
		hr_time_point m_lastFrameShowTime;
		QTimer * m_pPlayTimer;
		QIcon m_iconPlay;
		QIcon m_iconPause;

		ZimgPreviewConverter * m_pPreviewConverter;
};

#endif // PREVIEWDIALOG_H_INCLUDED
