#ifndef MULTI_TAB_MAIN_WINDOW_H
#define MULTI_TAB_MAIN_WINDOW_H

#include "../../common-src/settings/settings_manager.h"
#include "../../common-src/vapoursynth/vs_script_library.h"
#include "../../src/script_status_bar_widget/script_status_bar_widget.h"
#include "../../vsedit/src/bookmark_manager/bookmark_manager_dialog.h"
#include "vapoursynth/vapoursynth_plugins_manager.h"

#include "script_editor/script_editor.h"
#include "preview/preview_area.h"
#include "settings/settings_dialog.h"

#include <QMainWindow>

namespace Ui {
class MultiTabMainWindow;
}

class ScriptProcessor;
class VSEditorLog;
class ScriptBenchmarkDialog;
class EncodeDialog;
class TemplatesDialog;
class FrameInfoDialog;
class JobServerWatcherSocket;

struct EditorPreview {
    ScriptEditor * editor;
    PreviewArea * previewArea;
    ScriptProcessor * processor;
    BookmarkModel * bookmarkModel;
    QString scriptName;
    int lastTimeLineFrameIndex;
    int lastZoomRatio;
};

class MultiTabMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MultiTabMainWindow(QWidget *a_pParent = nullptr);
    ~MultiTabMainWindow();

    VSEditorLog * m_logView;

private:
    Ui::MultiTabMainWindow *m_ui;

    ScriptEditor * m_pScriptEdit;

    SettingsManager * m_pSettingsManager;

    SettingsDialog * m_pSettingsDialog;

    VSScriptLibrary * m_pVSScriptLibrary;

    VapourSynthPluginsManager * m_pVapourSynthPluginsManager;

    ScriptStatusBarWidget * m_pStatusBarWidget;

    ScriptBenchmarkDialog * m_pBenchmarkDialog;
    EncodeDialog * m_pEncodeDialog;
    TemplatesDialog * m_pTemplatesDialog;
    FrameInfoDialog * m_pFrameInfoDialog;
    BookmarkManagerDialog * m_pBookmarkManagerDialog;

    JobServerWatcherSocket * m_pJobServerWatcherSocket;

//    VSPluginsList *m_vsPluginsList;

    void createSettingDialog();

    void createVSScriptLibrary();

    void createVapourSynthPluginsManager();

    void createBenchmarkDialog();
    void createEncodeDialog();
    void createJobServerWatcher();

    void createViewLog(); // create collapsable widget and view log

    void createMainToolBar();

    void createFrameInfoDialog();
    void createBookmarkManager();


    void setLogSignals();

    void setTabSignals(); // setup signals between editor and previewArea
    void setEditorSignals(); // setup signals for editor
    void setTimeLineSignals();


    void createMenuBarActionsAndMenus();
    void createContextMenuActionsAndMenus();
    void createStatusBar();
    void setPlaybackPanel();

    /* Toolbar */
    QToolBar * mainToolBar;


    /* menu actions */
    bool saveScriptToFile(const QString& a_filePath);
    bool loadScriptFromFile(const QString& a_filePath);
    bool safeToCloseFile();

    void setCurrentScriptFilePath(QString & a_storedFilePath, const QString & a_filePath);
    void loadStartUpScript();

    void fillRecentScriptsMenu();

    void setUpZoomPanel();

    double YCoCgValueAtPoint(size_t a_x, size_t a_y, int a_plane,
                             const VSAPI * a_cpVSAPI, const VSFrameRef * a_cpFrameRef);

    void createGeometrySaveTimer(); // obsolete?

    QVector<EditorPreview> m_pEditorPreviewVector;

    int m_previousTabIndex;

    /* MenuBar menus and actions */
    QAction * m_pActionNewScript;
    QAction * m_pActionNewTab;
    QAction * m_pActionOpenScript;
    QAction * m_pActionCloseTab;
    QAction * m_pActionSaveScript;
    QAction * m_pActionSaveScriptAs;
    QAction * m_pActionTemplates;
    QAction * m_pActionSettings;
    QAction * m_pActionPreview;
    QAction * m_pActionCheckScript;
    QAction * m_pActionBenchmark;
    QAction * m_pActionEncode;
    QAction * m_pActionEnqueueEncodeJob;
    QAction * m_pActionJobs;
    QAction * m_pActionExit;
    QAction * m_pActionAbout;

    std::vector<QAction *> m_settableActionsList;

    QMenu * m_pMenuRecentScripts;


    /* context menus and actions */
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
    QAction * m_pActionLoadChapters;
    QAction * m_pActionClearBookmarks;
    QAction * m_pActionBookmarkCurrentFrame;
    QAction * m_pActionUnbookmarkCurrentFrame;
    QAction * m_pActionGoToPreviousBookmark;
    QAction * m_pActionGoToNextBookmark;
    QAction * m_pActionPasteShownFrameNumberIntoScript;
    QAction * m_pActionSaveBookmarkToFile;

    std::map<QString, ZoomMode> m_actionIDToZoomModeMap;

    std::map<QString, Qt::TransformationMode> m_actionIDToZoomScaleModeMap;

    std::map<QString, TimeLineSlider::DisplayMode>
        m_actionIDToTimeLineModeMap;

    /* playback icon */
    QIcon m_iconPlay;
    QIcon m_iconPause;

    /* editor parameters */
    QString m_lastSavedText;

//    std::vector<QAction *> m_settableActionsList;

    QTimer * m_pGeometrySaveTimer;
    QByteArray m_windowGeometry;

    bool m_setScriptBookmarkFromBookmarkManager;


public slots:

    void slotWriteLogMessage(int a_messageType, const QString & a_message);
    void slotWriteLogMessage(const QString & a_message,
        const QString & a_style = LOG_STYLE_DEFAULT);

private slots:

    void slotCreateTab();
    void slotRemoveTab();
    void slotChangePreviewTab(int a_index);
    void slotSaveTabBeforeChanged(int a_currentTabIndex, int a_selectedTabIndex);
    void slotChangeScriptTab(int a_index);
    void slotPreviewScript();
    void slotSetTimeLineAndIndicator(int a_numFrames, int64_t a_fpsNum, int64_t a_fpsDen);

    void slotDisplayModeChanged();
    void slotShowFrameFromTimeLine(int a_frameNumber);
    void slotJumpPlayFromTimeLine(int a_frameNumber);
    void slotUpdateFrameTimeIndicators(int a_frameNumber, const QTime &a_time); // update frame spinbox and time edit
    void slotUpdateHoverTimeIndicator(const QTime &a_time);
    void slotTimeLineFrameChanged(int a_frame);

    void slotUpdateStatusBarQueueState(size_t a_framesInQueue, size_t a_frameInProcess, size_t a_maxThreads);
    void slotShowFrameInfoDialog();
    void slotShowBookmarkManager();

    void slotZoomModeChanged(int a_mode);
    void slotZoomRatioChanged(double a_zoomRatio);
    void setPreviewPixmap();
    void slotPreviewAreaSizeChanged();
    void slotPreviewAreaMouseOverPoint(float a_normX, float a_normY);

    void slotPlay(bool a_play);
    void slotSetPlayFPSLimit();

    /* bookmark */
    void slotUpdateScriptBookmarkList();
    void slotSetScriptBookmark(int);
    void slotAddBookmark();
    void slotRemoveBookmark(QModelIndex a_index);
    void slotGoToBookmark(const QModelIndex a_modelIndex);
    void slotClearBookmark();
    void slotLoadBookmarkFile(QFile & a_file);
    void slotLoadChapterFile(QFile & a_file);
    void slotSaveBookmarksToFile();

    /* slot for menu actions */
    void slotNewScript();
    bool slotSaveScript();
    bool slotSaveScriptAs(QString & a_offeredFilePath);
    bool slotOpenScript();

    void slotTemplates();

    void slotCheckScript();
    void slotBenchmark();
    void slotEncode();
    void slotEnqueueEncodeJob();
    void slotJobs();

    void slotAbout();

    void slotChangeWindowTitle(const QString & a_storedFilePath);
    void slotEditorTextChanged();
    void slotOpenRecentScriptActionTriggered();

    void slotSettingsChanged();
    void slotScriptFileDropped(const QString & a_filePath, bool * a_pHandled);

    void slotSaveGeometry();

signals:

};

#endif // MULTI_TAB_MAIN_WINDOW_H
