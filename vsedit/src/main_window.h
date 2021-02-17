#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

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
class MainWindow;
}

class ScriptProcessor;
class VSEditorLog;
class ScriptBenchmarkDialog;
class EncodeDialog;
class TemplatesDialog;
class PreviewAdvancedSettingsDialog;
class FrameInfoDialog;
class PreviewFiltersDialog;
class FindDialog;
class SelectionToolsDialog;
class JobServerWatcherSocket;

struct EditorPreview {
    ScriptEditor * editor;
    PreviewArea * previewArea;
    ScriptProcessor * processor;
    BookmarkModel * bookmarkModel;
    QString scriptName; /* unique id use for grouping */
    QString scriptFilePath;
    int lastTimeLineFrameIndex;
    int lastZoomRatio;
    QPair<int,int> previewScrollBarPos;
    QString tabName;
    int tabNumber;
    int group;
    QMap<QString, int> previewFilters;

    EditorPreview():
        editor(nullptr), previewArea(nullptr), processor(nullptr), bookmarkModel(nullptr),
        scriptName(""), scriptFilePath(""), lastTimeLineFrameIndex(), lastZoomRatio(1),
        previewScrollBarPos(), tabName(""), tabNumber(-1), group(-1), previewFilters({{"channels", -2}})
    {}

    ~EditorPreview()
    {}
};

struct ClipProp {
    int frameNums;
    int width;
    int height;

    ClipProp(int fn, int w, int h)
        : frameNums(fn), width(w), height(h)
    {}

    auto tie() const {
        return std::tie(frameNums, width, height);
    }
};

struct ClipItem {
    QString scriptName;
    ClipProp properties;

    ClipItem(QString sn, ClipProp cp)
        : scriptName(sn), properties(cp)
    {}
};

typedef QVector<ClipItem> CompareGroup;

/* operator implementation to compare clip item.properties for grouping */
inline bool operator==(const ClipProp& lhs, const ClipProp& rhs);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *a_pParent = nullptr);
    ~MainWindow() override;

    VSEditorLog * m_logView;

protected:

    void closeEvent(QCloseEvent * a_pEvent) override;

//    void moveEvent(QMoveEvent * a_pEvent) override;

//    void resizeEvent(QResizeEvent * a_pEvent) override;

//    void changeEvent(QEvent * a_pEvent) override;


private:
    Ui::MainWindow *m_ui;

    SettingsManager * m_pSettingsManager;

    VSScriptLibrary * m_pVSScriptLibrary;

    VapourSynthPluginsManager * m_pVapourSynthPluginsManager;

    ScriptStatusBarWidget * m_pStatusBarWidget;

    SettingsDialog * m_pSettingsDialog;
    ScriptBenchmarkDialog * m_pBenchmarkDialog;
    EncodeDialog * m_pEncodeDialog;
    TemplatesDialog * m_pTemplatesDialog;
    FrameInfoDialog * m_pFrameInfoDialog;
    FindDialog * m_pFindDialog;
    PreviewAdvancedSettingsDialog * m_pPreviewAdvancedSettingsDialog;
    BookmarkManagerDialog * m_pBookmarkManagerDialog;
    PreviewFiltersDialog * m_pPreviewFiltersDialog;
    SelectionToolsDialog * m_pSelectionToolsDialog;

    JobServerWatcherSocket * m_pJobServerWatcherSocket;

    QVector<QObject **> m_orphanQObjects;

    VSPluginsList m_vsPluginsList;
    VSPyScriptsList m_vsPyScriptsList;

    void createSettingDialog();

    void createVSScriptLibrary();

    void createVapourSynthPluginsManager();

    void createAdvancedSettingsDialog();
    void createTemplatesDialog();
    void createBenchmarkDialog();
    void createEncodeDialog();
    void createJobServerWatcher();

    void createLogView(); // create collapsable widget and view log

    void createTab(const QString & a_tabName = "",
                       const QString & a_scriptFilePath = "",
                       const QString & a_scriptText = "");

    void createMainToolBar();

    void createFindDialog();   
    void createFrameInfoDialog();
    void createBookmarkManager();
    void createPreviewFilters();
    void createSelectionToolsDialog();

    void setTabs();
    void setTabSignals(); // setup signals between editor and previewArea
    void setTimeLineSignals();

    void createMenuActionsAndContextMenuActions();
    void createContextMenuActionsAndMenus();
    void createTabBarContextMenuActions();
    void createStatusBar();
    void setPlaybackPanel();

    /* Toolbar */
    QToolBar * mainToolBar;

    /* menu functions */
    bool saveScriptToFile(const QString& a_filePath);
    bool loadScriptFromFile(const QString& a_filePath);
    bool safeToCloseFile();
    int isScriptOpened(const QString & a_filePath);

    void setCurrentScriptFilePath(const QString & a_filePath);
    void loadStartUpScript();

    void fillRecentScriptsMenu();

    void setUpZoomPanel();
    double currentPreviewZoomRatio();

    QString createPreviewFilterScript(const QString &a_script, const QMap<QString, int>&);

    QFlags<QTextDocument::FindFlag> extractFindFlags(const QMap<QString, bool> & a_flagsMap);

    double YCoCgValueAtPoint(size_t a_x, size_t a_y, int a_plane,
                             const VSAPI * a_cpVSAPI, const VSFrameRef * a_cpFrameRef);

    void createGeometrySaveTimer(); // obsolete?

    void createGarbageCollection();
    void destroyOrphanQObjects();

    QVector<EditorPreview> m_pEditorPreviewVector;

    QMenu * m_pTabBarContectMenu;

    /* MenuBar menus and actions */
    QAction * m_pActionNewTab;
    QAction * m_pActionOpenScript;
    QAction * m_pActionCloseTab;
    QAction * m_pActionCloseAllTabs;
    QAction * m_pActionSaveScript;
    QAction * m_pActionSaveScriptAs;

    QAction * m_pActionUndo;
    QAction * m_pActionRedo;
    QAction * m_pActionCut;
    QAction * m_pActionCopy;
    QAction * m_pActionPaste;
    QAction * m_pActionSelectAll;
    QAction * m_pActionCopyToNewTab;
    QAction * m_pActionFind;

    QAction * m_pActionDuplicateSelection;
    QAction * m_pActionCommentSelection;
    QAction * m_pActionUncommentSelection;
    QAction * m_pActionReplaceTabWithSpaces;
    QAction * m_pActionAutocomplete;
    QAction * m_pActionMoveTextBlockUp;
    QAction * m_pActionMoveTextBlockDown;
    QAction * m_pActionToggleComment;

    QAction * m_pActionTemplates;
    QAction * m_pActionSettings;

    QAction * m_pActionChangeToTab1;
    QAction * m_pActionChangeToTab2;
    QAction * m_pActionChangeToTab3;
    QAction * m_pActionChangeToTab4;
    QAction * m_pActionChangeToTab5;
    QAction * m_pActionChangeToTab6;
    QAction * m_pActionChangeToTab7;
    QAction * m_pActionChangeToTab8;
    QAction * m_pActionChangeToTab9;

    QAction * m_pActionPreview;
    QAction * m_pActionCheckScript;
    QAction * m_pActionReleaseMemory;
    QAction * m_pActionBenchmark;
    QAction * m_pActionEncode;
    QAction * m_pActionEnqueueEncodeJob;
    QAction * m_pActionJobs;
    QAction * m_pActionExit;
    QAction * m_pActionShowBookmarkManager;
    QAction * m_pActionShowFrameInfoDialog;
    QAction * m_pActionShowPreivewFiltersDialog;
    QAction * m_pActionShowSelectionToolsDialog;

    QMenu * m_pAboutVapoursynth;
    QAction * m_pActionVapourSynthVersion;
    QAction * m_pActionPluginFolder;
    QAction * m_pActionScriptFolder;

    QAction * m_pActionAbout;

    std::vector<QAction *> m_settableActionsList;

    QMenu * m_pMenuRecentScripts;

    /* context menus and actions */
    QMenu * m_pPreviewContextMenu;
    QAction * m_pActionFrameToClipboard;
    QAction * m_pActionSaveSnapshot;
    QMenu * m_pMenuZoomModes;
    QActionGroup * m_pActionGroupZoomModes;
    QAction * m_pActionSetZoomModeNoZoom;
    QAction * m_pActionSetZoomModeFixedRatio;
    QAction * m_pActionSetZoomModeFitToFrame;
    QAction * m_pActionTimeStepForward;
    QAction * m_pActionTimeStepBack;
    QAction * m_pActionPasteCropSnippetIntoScript;
    QAction * m_pActionAdvancedSettingsDialog;
    QAction * m_pActionPlay;
    QAction * m_pActionBookmarkCurrentFrame;
    QAction * m_pActionPasteShownFrameNumberIntoScript;

    std::map<QString, ZoomMode> m_actionIDToZoomModeMap;

//    std::map<QString, Qt::TransformationMode> m_actionIDToZoomScaleModeMap;

    /* playback icon */
    QIcon m_iconPlay;
    QIcon m_iconPause;
    bool m_playing;

    /* Window */
    QString m_currentTabScriptFilePath;
    QList<int> m_tabNumberList;
    bool m_closingApp;
    bool m_closingTab;
    int m_rightClickedTab;

    QTimer * m_pGeometrySaveTimer;
    QByteArray m_windowGeometry;

    bool m_setScriptBookmarkFromBookmarkManager;

    /* compare group */
    QVector<CompareGroup> m_compareGroupList;

    QStringList m_tabScriptsList;

public slots:

    void slotWriteLogMessage(int a_messageType, const QString & a_message);
    void slotWriteLogMessage(const QString & a_message,
        const QString & a_style = LOG_STYLE_DEFAULT);

private slots:

    bool slotRemoveTab(int a_index = -1);
    bool slotRemoveAllTabs();
    void slotChangePreviewTab(int a_index);
    void slotSaveTabBeforeChanged(int a_leftTabIndex, int a_rightTabIndex);
    void slotChangeScriptTab(int a_index);
    void slotPreviewScript();
    void slotTabBarContextMenu(int a_tabIndex);
    void slotSetTimeLineAndIndicator(int a_numFrames, int64_t a_fpsNum, int64_t a_fpsDen);

    /* time line */
    void slotTimeLineDisplayModeChanged();
    void slotShowFrameFromTimeLine(int a_frameNumber);
    void slotJumpPlayFromTimeLine(int a_frameNumber);
    void slotUpdateFrameTimeIndicators(int a_frameNumber, const QTime &a_time); // update frame spinbox and time edit
    void slotUpdateHoverTimeIndicator(const QTime &a_time);
    void slotTimeLineFrameChanged(int a_frameNumber);

    void slotProcessorFrameChanged(int a_frameNumber);

    void slotUpdateStatusBarQueueState(size_t a_framesInQueue, size_t a_frameInProcess, size_t a_maxThreads);
    void slotShowFrameInfoDialog(bool a_visible);
    void slotShowBookmarkManager(bool a_visible);
    void slotShowPreviewFiltersDialog(bool a_visible);
    void slotShowSelectionToolsDialog(bool a_visible);

    void slotZoomModeChanged();
    void slotZoomRatioChanged(double a_zoomRatio);
    void slotSetPreviewPixmap(const QPixmap &a_framePixmap);
    void slotPreviewAreaSizeChanged();
    void slotPreviewAreaMouseOverPoint(float a_normX, float a_normY);
    void slotPreviewAreaMouseRightButtonReleased();
    void slotCallAdvancedSettingsDialog();
    void slotPasteShownFrameNumberIntoScript();
    void slotSavePreviewScrollBarPos(const QPair<int, int>&);

    /* preview filters */
    void slotUpdateTabPreviewFilters(QMap<QString, int>);

    void slotPlay(bool a_play);
    void slotSetPlayFPSLimit();

    void slotUpdateFramePropsString(const QString & a_framePropsString);

//    void slotPreviewFiltersChanged();

    /* editor */
    void slotEditorUndo();
    void slotEditorRedo();
    void slotEditorCut();
    void slotEditorCopy();
    void slotEditorPaste();
    void slotEditorSelectAll();
    void slotEditorCopyToNewTab();

    void slotOpenFind();
    void slotDuplicateSelection();
    void slotReplaceTabWithSpaces();
    void slotComplete();
    void slotMoveTextBlockUp();
    void slotMoveTextBlockDown();
    void slotToggleComment();
    void slotEditorFindText(const QString & a_text, const QMap<QString, bool> & a_flagsMap);
    void slotEditorReplaceText(const QString & a_findText, const QString & a_replaceText,
                               const QMap<QString, bool> & a_flagsMap);
    void slotReplaceAllText(const QString & a_findText, const QString & a_replaceText,
                            const QMap<QString, bool> & a_flagsMap);

    /* bookmark */
    void slotUpdateScriptBookmarkList();
    void slotSetScriptBookmark(const QString & a_text);
    void slotAddBookmark();
    void slotRemoveBookmark(QModelIndex a_index);
    void slotGoToBookmark(const QModelIndex a_modelIndex);
    void slotClearBookmark();
    void slotLoadBookmarkFile(QFile & a_file);
    void slotLoadChapterFile(QFile & a_file);
    void slotSaveBookmarksToFile();

    /* selection tool dialog */
    void slotSendPixmapToSelectionCanvas();
    void slotPasteSelectionPointsToScript(const QString&);

    /* slot for menu actions */
    void slotNewScript();
    bool slotSaveScript();
    bool slotSaveScriptAs();
    bool slotOpenScript();

    void slotTemplates();

    void slotChangeTabByAction();

    void slotCheckScript();
    void slotReleaseMemory();
    void slotBenchmark();
    void slotEncode();
    void slotEnqueueEncodeJob();
    void slotJobs();

    void slotFrameToClipboard();
    void slotSaveSnapshot();

    void slotAbout();
    void slotVapourSynthVersion();
    void slotOpenPluginsFolder();
    void slotOpenScriptsFolder();

    void slotChangeWindowTitle(const QString & a_title);
    void slotEditorTextChanged();
    void slotOpenRecentScriptActionTriggered();

    void slotSettingsChanged();
    void slotScriptFileDropped(const QString & a_filePath, bool * a_pHandled);

    void slotSaveGeometry();

signals:

    void signalTabNameChanged(const QString &a_oldName, const QString &a_newName);

    void signalUpdatePreviewFiltersDisplay(const QMap<QString,int> &);

};

#endif // MAIN_WINDOW_H
