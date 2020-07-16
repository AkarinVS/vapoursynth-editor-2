#include "multi_tab_main_window.h"
#include "ui_multi_tab_main_window.h"
#include "preview/script_processor.h"
#include "preview/frame_info_dialog.h"
#include "../../common-src/qt_widgets_subclasses/collapse_expand_widget.h"
#include "../../common-src/log/vs_editor_log.h"
#include "../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "../../common-src/ipc_defines.h"
#include "vapoursynth/vs_script_processor_dialog.h"

#include "settings/settings_dialog.h"
#include "frame_consumers/benchmark_dialog.h"
#include "frame_consumers/encode_dialog.h"
#include "script_templates/templates_dialog.h"
#include "job_server_watcher_socket.h"

#include <QStandardPaths>
#include <QMessageBox>
#include <QDesktopServices>
#include <QResource>
#include <QFileInfo>
#include <QFileDialog>
#include <QTimer>
#include <QToolBar>

MultiTabMainWindow::MultiTabMainWindow(QWidget *a_pParent) :
    QMainWindow(a_pParent),
    m_ui(new Ui::MultiTabMainWindow)
  , m_pSettingsManager(nullptr)
  , m_pVSScriptLibrary(nullptr)
  , m_pVapourSynthPluginsManager(nullptr)
  , m_pStatusBarWidget(nullptr)
  , m_previousTabIndex(0)
{
    m_ui->setupUi(this);

    createSettingDialog();
    createBookmarkManager();


    slotCreateTab();
    setEditorSignals();
    setTabSignals();

    createViewLog();
    setLogSignals();

//    m_ui->verticalLayout_3->setContentsMargins(0,0,0,0);
//    m_ui->scriptEditorDockWidget->setStyleSheet("QDockWidget:title {background-color: red;}");

    setTimeLineSignals();

    createMenuBarActionsAndMenus();
    createMainToolBar();

    createContextMenuActionsAndMenus();
    createStatusBar();

    setPlaybackPanel();

    m_iconPlay = QIcon(":play.png");
    m_iconPause = QIcon(":pause.png");


    setUpZoomPanel();

    createBenchmarkDialog();
    createEncodeDialog();
    createFrameInfoDialog();
    createJobServerWatcher();

    connect(m_ui->displayModeToggleButton, &QPushButton::clicked,
            this, &MultiTabMainWindow::slotDisplayModeChanged);

    connect(m_ui->frameNumberIndicatorSpinBox, QOverload<int>::of(&GenericSpinBox::valueChanged),
            this, &MultiTabMainWindow::slotShowFrameFromTimeLine);

}

MultiTabMainWindow::~MultiTabMainWindow()
{
    delete m_ui;
    qInstallMessageHandler(nullptr);
}

void MultiTabMainWindow::slotCreateTab()
{
    EditorPreview ep;
    ep.editor = new ScriptEditor();
    ep.previewArea = new PreviewArea();
    ep.processor = new ScriptProcessor(m_pSettingsManager, m_pVSScriptLibrary);

    ep.bookmarkModel = new BookmarkModel();

    connect(ep.processor, &ScriptProcessor::signalSetTimeLineAndIndicator,
            this, &MultiTabMainWindow::slotSetTimeLineAndIndicator);

    // retrieve frame from processor and send it to preview area
    connect(ep.processor, &ScriptProcessor::signalSetCurrentFrame,
            this, &MultiTabMainWindow::setPreviewPixmap);

    // resize pixmap when zoom mode = fitToFrame and previewArea size change
    connect(ep.previewArea, &PreviewArea::signalSizeChanged,
            this, &MultiTabMainWindow::slotPreviewAreaSizeChanged);

    // signal to update status bar queue state icons
    connect(ep.processor, &ScriptProcessor::signalFrameQueueStateChanged,
            this, &MultiTabMainWindow::slotUpdateStatusBarQueueState);

    // signal for playback framechange
    connect(ep.processor, &ScriptProcessor::signalFrameChanged, // frame change
            this, &MultiTabMainWindow::slotTimeLineFrameChanged);

    connect(ep.processor, &ScriptProcessor::signalWriteLogMessage,
        this, QOverload<int, const QString &>::of(&MultiTabMainWindow::slotWriteLogMessage));

    // signal for file drop load
    connect(ep.editor, &ScriptEditor::signalScriptFileDropped,
            this, &MultiTabMainWindow::slotScriptFileDropped);

    // eyedropper
    connect(ep.previewArea, &PreviewArea::signalMouseOverPoint,
        this, &MultiTabMainWindow::slotPreviewAreaMouseOverPoint);


    // autocomplete feature in editor
    VSPluginsList vsPluginsList = m_pVapourSynthPluginsManager->pluginsList();
    ep.editor->setPluginsList(vsPluginsList);
    ep.editor->setSettingsManager(m_pSettingsManager);


    // create new tab for editor and previewArea
    int tabCount = m_ui->scriptTabWidget->count();
    QString tabName = "tab_" + QVariant(tabCount + 1).toString();
    m_ui->scriptTabWidget->addTab(ep.editor, tabName);
    m_ui->previewTabWidget->addTab(ep.previewArea, tabName);

    ep.scriptName = "";
    m_pEditorPreviewVector.append(ep);

    // add script to bookmark manager
    m_pBookmarkManagerDialog->slotAddScriptBookmark(tabName);

    m_ui->scriptTabWidget->setCurrentIndex(tabCount);
}

void MultiTabMainWindow::setTabSignals()
{
    connect(m_ui->scriptTabWidget, &GenericTabWidget::signalAboutToChanged,
            this, &MultiTabMainWindow::slotSaveTabBeforeChanged);
    connect(m_ui->scriptTabWidget, &GenericTabWidget::currentChanged,
            this, &MultiTabMainWindow::slotChangeScriptTab);

    connect(m_ui->previewTabWidget, &GenericTabWidget::currentChanged,
            this, &MultiTabMainWindow::slotChangePreviewTab);

}

void MultiTabMainWindow::createSettingDialog()
{
    m_pSettingsManager = new SettingsManager(this);
    m_pSettingsDialog = new SettingsDialog(m_pSettingsManager, nullptr);
    m_pVSScriptLibrary = new VSScriptLibrary(m_pSettingsManager, this);
    m_pVapourSynthPluginsManager = new VapourSynthPluginsManager(m_pSettingsManager, this);

    connect(m_pVSScriptLibrary, &VSScriptLibrary::signalWriteLogMessage,
        this, QOverload<int, const QString &>::of(&MultiTabMainWindow::slotWriteLogMessage));

    connect(m_pSettingsDialog, &SettingsDialog::signalSettingsChanged,
            this, &MultiTabMainWindow::slotSettingsChanged);
}

void MultiTabMainWindow::createBenchmarkDialog()
{
    m_pBenchmarkDialog = new ScriptBenchmarkDialog(m_pSettingsManager, m_pVSScriptLibrary);
    connect(m_pBenchmarkDialog,
            SIGNAL(signalWriteLogMessage(int, const QString &)),
            this, SLOT(slotWriteLogMessage(int, const QString &)));
}

void MultiTabMainWindow::createEncodeDialog()
{
    m_pEncodeDialog = new EncodeDialog(m_pSettingsManager, m_pVSScriptLibrary);
    connect(m_pEncodeDialog,
            SIGNAL(signalWriteLogMessage(const QString &, const QString &)),
            this, SLOT(slotWriteLogMessage(const QString &, const QString &)));
}

void MultiTabMainWindow::createJobServerWatcher()
{
    m_pJobServerWatcherSocket = new JobServerWatcherSocket(this);
    connect(m_pJobServerWatcherSocket,
        SIGNAL(signalWriteLogMessage(const QString &, const QString &)),
        this, SLOT(slotWriteLogMessage(const QString &, const QString &)));
}

void MultiTabMainWindow::createViewLog()
{
//    m_ui->collapseExpandWidget = new CollapseExpandWidget();
    m_ui->collapseExpandWidget->setTitle("");
    m_ui->collapseExpandWidget->setAnimationDuration(300);

    m_logView = new VSEditorLog();

    auto * anyLayout = new QVBoxLayout();
    anyLayout->addWidget(m_logView);
    m_ui->collapseExpandWidget->setContentLayout(*anyLayout);

    m_logView->setName("main_log");
    m_logView->setSettingsManager(m_pSettingsManager);
    m_logView->loadSettings();

}

void MultiTabMainWindow::createMainToolBar()
{
    mainToolBar = addToolBar(tr("Toolbar"));
    mainToolBar->setMovable(false);
    mainToolBar->toggleViewAction()->setEnabled(false); // not hidable

    mainToolBar->addAction(m_pActionNewTab);
    mainToolBar->addAction(m_pActionOpenScript);
    mainToolBar->addAction(m_pActionCloseTab);
    mainToolBar->addAction(m_pActionSaveScript);
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_pActionPreview);
    mainToolBar->addAction(m_pActionBenchmark);
}

void MultiTabMainWindow::createFrameInfoDialog()
{
    m_pFrameInfoDialog = new FrameInfoDialog(m_pSettingsManager ,this);
    connect(m_ui->frameInfoButton, &QToolButton::clicked,
            this, &MultiTabMainWindow::slotShowFrameInfoDialog);

}

void MultiTabMainWindow::createBookmarkManager()
{
    m_pBookmarkManagerDialog = new BookmarkManagerDialog(m_pSettingsManager, this);

    // connect
    connect(m_ui->bookmarkManagerButton, &QPushButton::clicked,
            this, &MultiTabMainWindow::slotShowBookmarkManager);


    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalScriptBookmarkChanged,
            this, &MultiTabMainWindow::slotSetScriptBookmark);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalAddButtonPressed,
            this, &MultiTabMainWindow::slotAddBookmark);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalRemoveBookmark,
            this, &MultiTabMainWindow::slotRemoveBookmark);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalGotoBookmark,
            this, &MultiTabMainWindow::slotGoToBookmark);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalClearBookmark,
            this, &MultiTabMainWindow::slotClearBookmark);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalLoadBookmarkFile,
            this, &MultiTabMainWindow::slotLoadBookmarkFile);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalLoadChapterFile,
            this, &MultiTabMainWindow::slotLoadChapterFile);

    connect(m_pBookmarkManagerDialog, &BookmarkManagerDialog::signalSaveBookmarksToFile,
            this, &MultiTabMainWindow::slotSaveBookmarksToFile);

}

void MultiTabMainWindow::setLogSignals()
{


}

void MultiTabMainWindow::setEditorSignals()
{

//	connect(m_ui->scriptTabWidget->widget(), SIGNAL(textChanged()),
//		this, SLOT(slotEditorTextChanged()));
//	connect(m_ui.scriptEdit, SIGNAL(modificationChanged(bool)),
//		this, SLOT(slotChangeWindowTitle()));
}

void MultiTabMainWindow::setTimeLineSignals()
{
    connect(m_ui->timeLineView, &TimeLineView::signalFrameChanged, // frame change
            this, &MultiTabMainWindow::slotTimeLineFrameChanged);
    connect(m_ui->timeLineView, &TimeLineView::signalJumpToFrame,
            this, &MultiTabMainWindow::slotJumpPlayFromTimeLine);
    connect(m_ui->timeLineView, &TimeLineView::signalHoverTime,
            this, &MultiTabMainWindow::slotUpdateHoverTimeIndicator);
}

void MultiTabMainWindow::createMenuBarActionsAndMenus()
{
    struct ActionToCreate
    {
        QAction ** ppAction;
        const char * id;
        QObject * pObjectToConnect;
        const char * slotToConnect;
    };

    ActionToCreate actionsToCreate[] =
    {
        {&m_pActionNewScript, ACTION_ID_NEW_SCRIPT,
            this, SLOT(slotNewScript())},
        {&m_pActionNewTab, ACTION_ID_NEW_TAB,
            this, SLOT(slotCreateTab())},
        {&m_pActionOpenScript, ACTION_ID_OPEN_SCRIPT,
            this, SLOT(slotOpenScript())},
        {&m_pActionCloseTab, ACTION_ID_CLOSE_TAB,
            this, SLOT(slotRemoveTab())},
        {&m_pActionSaveScript, ACTION_ID_SAVE_SCRIPT,
            this, SLOT(slotSaveScript())},
        {&m_pActionSaveScriptAs, ACTION_ID_SAVE_SCRIPT_AS,
            this, SLOT(slotSaveScriptAs())},
        {&m_pActionExit, ACTION_ID_EXIT,
            this, SLOT(close())},
        {&m_pActionTemplates, ACTION_ID_TEMPLATES,
            this, SLOT(slotTemplates())},
        {&m_pActionSettings, ACTION_ID_SETTINGS,
            m_pSettingsDialog, SLOT(slotCall())},
        {&m_pActionPreview, ACTION_ID_PREVIEW,
            this, SLOT(slotPreviewScript())},
        {&m_pActionCheckScript, ACTION_ID_CHECK_SCRIPT,
            this, SLOT(slotCheckScript())},
        {&m_pActionBenchmark, ACTION_ID_BENCHMARK,
            this, SLOT(slotBenchmark())},
        {&m_pActionEncode, ACTION_ID_CLI_ENCODE,
            this, SLOT(slotEncode())},
        {&m_pActionEnqueueEncodeJob, ACTION_ID_ENQUEUE_ENCODE_JOB,
            this, SLOT(slotEnqueueEncodeJob())},
        {&m_pActionJobs, ACTION_ID_JOBS,
            this, SLOT(slotJobs())},
        {&m_pActionAbout, ACTION_ID_ABOUT,
            this, SLOT(slotAbout())},
    };

    for(ActionToCreate & item : actionsToCreate)
    {
        QAction * pAction = m_pSettingsManager->createStandardAction(
            item.id, this);
        *item.ppAction = pAction;
        m_settableActionsList.push_back(pAction);
        connect(pAction, SIGNAL(triggered()), item.pObjectToConnect, item.slotToConnect);
    }

//------------------------------------------------------------------------------

    QMenu * pFileMenu = m_ui->menuBar->addMenu(tr("File"));
    pFileMenu->addAction(m_pActionNewTab);
    pFileMenu->addAction(m_pActionOpenScript);
    pFileMenu->addAction(m_pActionCloseTab);
    pFileMenu->addAction(m_pActionSaveScript);
    pFileMenu->addAction(m_pActionSaveScriptAs);
    pFileMenu->addSeparator();

    m_pMenuRecentScripts = new QMenu(tr("Recent scripts"), this);
    pFileMenu->addMenu(m_pMenuRecentScripts);
    fillRecentScriptsMenu();

    pFileMenu->addSeparator();
    pFileMenu->addAction(m_pActionExit);

//------------------------------------------------------------------------------

    QMenu * pEditMenu = m_ui->menuBar->addMenu(tr("Edit"));

//    std::vector<QAction *> editorActions = m_ui->scriptEdit->actionsForMenu();
//    for(QAction * pAction : editorActions)
//        pEditMenu->addAction(pAction);

    pEditMenu->addSeparator();
    pEditMenu->addAction(m_pActionTemplates);
    pEditMenu->addAction(m_pActionSettings);

//------------------------------------------------------------------------------

    QMenu * pScriptMenu = m_ui->menuBar->addMenu(tr("Script"));
    pScriptMenu->addAction(m_pActionPreview);
    pScriptMenu->addAction(m_pActionCheckScript);
    pScriptMenu->addAction(m_pActionBenchmark);
    pScriptMenu->addAction(m_pActionEncode);
    pScriptMenu->addAction(m_pActionEnqueueEncodeJob);
    pScriptMenu->addAction(m_pActionJobs);

//------------------------------------------------------------------------------

    QMenu * pHelpMenu = m_ui->menuBar->addMenu(tr("Help"));
    pHelpMenu->addAction(m_pActionAbout);

}

void MultiTabMainWindow::createContextMenuActionsAndMenus()
{
    struct ActionToCreate
    {
        QAction ** ppAction;
        const char * id;
        bool checkable;
        const char * slotToConnect;
    };

    ActionToCreate actionsToCreate[] =
    {
        {&m_pActionFrameToClipboard, ACTION_ID_FRAME_TO_CLIPBOARD,
         false, SLOT(slotFrameToClipboard())},
        {&m_pActionSaveSnapshot, ACTION_ID_SAVE_SNAPSHOT,
         false, SLOT(slotSaveSnapshot())},
        {&m_pActionToggleZoomPanel, ACTION_ID_TOGGLE_ZOOM_PANEL,
         true, SLOT(slotToggleZoomPanelVisible(bool))},
        {&m_pActionSetZoomModeNoZoom, ACTION_ID_SET_ZOOM_MODE_NO_ZOOM,
         true, SLOT(slotZoomModeChanged())},
        {&m_pActionSetZoomModeFixedRatio, ACTION_ID_SET_ZOOM_MODE_FIXED_RATIO,
         true, SLOT(slotZoomModeChanged())},
        {&m_pActionSetZoomModeFitToFrame, ACTION_ID_SET_ZOOM_MODE_FIT_TO_FRAME,
         true, SLOT(slotZoomModeChanged())},
        {&m_pActionSetZoomScaleModeNearest,
         ACTION_ID_SET_ZOOM_SCALE_MODE_NEAREST,
         true, SLOT(slotScaleModeChanged())},
        {&m_pActionSetZoomScaleModeBilinear,
         ACTION_ID_SET_ZOOM_SCALE_MODE_BILINEAR,
         true, SLOT(slotScaleModeChanged())},
        {&m_pActionToggleCropPanel, ACTION_ID_TOGGLE_CROP_PANEL,
         true, SLOT(slotToggleCropPanelVisible(bool))},
        {&m_pActionToggleTimeLinePanel, ACTION_ID_TOGGLE_TIMELINE_PANEL,
         true, SLOT(slotToggleTimeLinePanelVisible(bool))},
        {&m_pActionSetTimeLineModeTime, ACTION_ID_SET_TIMELINE_MODE_TIME,
         true, SLOT(slotTimeLineModeChanged())},
        {&m_pActionSetTimeLineModeFrames, ACTION_ID_SET_TIMELINE_MODE_FRAMES,
         true, SLOT(slotTimeLineModeChanged())},
        {&m_pActionTimeStepForward, ACTION_ID_TIME_STEP_FORWARD,
         false, SLOT(slotTimeStepForward())},
        {&m_pActionTimeStepBack, ACTION_ID_TIME_STEP_BACK,
         false, SLOT(slotTimeStepBack())},
        {&m_pActionPasteCropSnippetIntoScript,
         ACTION_ID_PASTE_CROP_SNIPPET_INTO_SCRIPT,
         false, SLOT(slotPasteCropSnippetIntoScript())},
        {&m_pActionAdvancedSettingsDialog, ACTION_ID_ADVANCED_PREVIEW_SETTINGS,
         false, SLOT(slotCallAdvancedSettingsDialog())},
        {&m_pActionToggleColorPicker, ACTION_ID_TOGGLE_COLOR_PICKER,
         true, SLOT(slotToggleColorPicker(bool))},
        {&m_pActionPlay, ACTION_ID_PLAY,
         true, SLOT(slotPlay(bool))},
        {&m_pActionLoadChapters, ACTION_ID_TIMELINE_LOAD_CHAPTERS,
         false, SLOT(slotLoadChapters())},
        {&m_pActionClearBookmarks, ACTION_ID_TIMELINE_CLEAR_BOOKMARKS,
         false, SLOT(slotClearBookmarks())},
        {&m_pActionBookmarkCurrentFrame,
         ACTION_ID_TIMELINE_BOOKMARK_CURRENT_FRAME,
         false, SLOT(slotBookmarkCurrentFrame())},
        {&m_pActionUnbookmarkCurrentFrame,
         ACTION_ID_TIMELINE_UNBOOKMARK_CURRENT_FRAME,
         false, SLOT(slotUnbookmarkCurrentFrame())},
        {&m_pActionGoToPreviousBookmark,
         ACTION_ID_TIMELINE_GO_TO_PREVIOUS_BOOKMARK,
         false, SLOT(slotGoToPreviousBookmark())},
        {&m_pActionGoToNextBookmark, ACTION_ID_TIMELINE_GO_TO_NEXT_BOOKMARK,
         false, SLOT(slotGoToNextBookmark())},
        {&m_pActionPasteShownFrameNumberIntoScript,
         ACTION_ID_PASTE_SHOWN_FRAME_NUMBER_INTO_SCRIPT,
         false, SLOT(slotPasteShownFrameNumberIntoScript())},
        //        {&m_pActionSaveBookmarkToFile,
        //            ACTION_ID_SAVE_BOOKMARK_TO_FILE,
        //            false, SLOT(slotSaveBookmarkToFile())},
    };

    for(ActionToCreate & item : actionsToCreate)
    {
        QAction * pAction =
                m_pSettingsManager->createStandardAction(item.id, this);
        *item.ppAction = pAction;
        pAction->setCheckable(item.checkable);
        m_settableActionsList.push_back(pAction);
    }

    //------------------------------------------------------------------------------

    m_pPreviewContextMenu = new QMenu(this);
    m_pPreviewContextMenu->addAction(m_pActionFrameToClipboard);
    m_pPreviewContextMenu->addAction(m_pActionSaveSnapshot);
    m_pActionToggleZoomPanel->setChecked(
                m_pSettingsManager->getZoomPanelVisible());
    m_pPreviewContextMenu->addAction(m_pActionToggleZoomPanel);

    //------------------------------------------------------------------------------

    m_pMenuZoomModes = new QMenu(m_pPreviewContextMenu);
    m_pMenuZoomModes->setTitle(tr("Zoom mode"));
    m_pPreviewContextMenu->addMenu(m_pMenuZoomModes);

    m_pActionGroupZoomModes = new QActionGroup(this);

    ZoomMode zoomMode = m_pSettingsManager->getZoomMode();

    struct ZoomModeAction
    {
        QAction * pAction;
        ZoomMode zoomMode;
    };

    ZoomModeAction zoomModeActions[] =
    {
        {m_pActionSetZoomModeNoZoom, ZoomMode::NoZoom},
        {m_pActionSetZoomModeFixedRatio, ZoomMode::FixedRatio},
        {m_pActionSetZoomModeFitToFrame, ZoomMode::FitToFrame},
    };

    for(ZoomModeAction & action : zoomModeActions)
    {
        QString id = action.pAction->data().toString();
        action.pAction->setActionGroup(m_pActionGroupZoomModes);
        m_pMenuZoomModes->addAction(action.pAction);
        m_actionIDToZoomModeMap[id] = action.zoomMode;
        addAction(action.pAction);
        if(zoomMode == action.zoomMode)
            action.pAction->setChecked(true);
    }

    //------------------------------------------------------------------------------

    m_pMenuZoomScaleModes = new QMenu(m_pPreviewContextMenu);
    m_pMenuZoomScaleModes->setTitle(tr("Zoom scale mode"));
    m_pPreviewContextMenu->addMenu(m_pMenuZoomScaleModes);

    m_pActionGroupZoomScaleModes = new QActionGroup(this);

    Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();

    struct ScaleModeAction
    {
        QAction * pAction;
        Qt::TransformationMode scaleMode;
    };

    ScaleModeAction scaleModeActions[] =
    {
        {m_pActionSetZoomScaleModeNearest, Qt::FastTransformation},
        {m_pActionSetZoomScaleModeBilinear, Qt::SmoothTransformation},
    };

    for(ScaleModeAction & action : scaleModeActions)
    {
        QString id = action.pAction->data().toString();
        action.pAction->setActionGroup(m_pActionGroupZoomScaleModes);
        m_pMenuZoomScaleModes->addAction(action.pAction);
        m_actionIDToZoomScaleModeMap[id] = action.scaleMode;
        addAction(action.pAction);
        if(scaleMode == action.scaleMode)
            action.pAction->setChecked(true);
    }

    bool noZoom = (zoomMode == ZoomMode::NoZoom);
    m_pMenuZoomScaleModes->setEnabled(!noZoom);

    //------------------------------------------------------------------------------

    m_pPreviewContextMenu->addAction(m_pActionToggleCropPanel);
    m_pPreviewContextMenu->addAction(m_pActionToggleTimeLinePanel);
    m_pActionToggleTimeLinePanel->setChecked(
                m_pSettingsManager->getTimeLinePanelVisible());

    //------------------------------------------------------------------------------

    m_pMenuTimeLineModes= new QMenu(m_pPreviewContextMenu);
    m_pMenuTimeLineModes->setTitle(tr("Timeline display mode"));
    m_pPreviewContextMenu->addMenu(m_pMenuTimeLineModes);

    m_pActionGroupTimeLineModes = new QActionGroup(this);

    TimeLineSlider::DisplayMode timeLineMode =
            m_pSettingsManager->getTimeLineMode();

    struct TimeLineModeAction
    {
        QAction * pAction;
        TimeLineSlider::DisplayMode timeLineMode;
    };

    TimeLineModeAction timeLineModeAction[] =
    {
        {m_pActionSetTimeLineModeTime, TimeLineSlider::DisplayMode::Time},
        {m_pActionSetTimeLineModeFrames, TimeLineSlider::DisplayMode::Frames},
    };

    for(TimeLineModeAction & action : timeLineModeAction)
    {
        QString id = action.pAction->data().toString();
        action.pAction->setActionGroup(m_pActionGroupTimeLineModes);
        m_pMenuTimeLineModes->addAction(action.pAction);
        m_actionIDToTimeLineModeMap[id] = action.timeLineMode;
        addAction(action.pAction);
        if(timeLineMode == action.timeLineMode)
            action.pAction->setChecked(true);
    }

    //------------------------------------------------------------------------------

    addAction(m_pActionTimeStepForward);
    addAction(m_pActionTimeStepBack);

    m_pActionToggleColorPicker->setChecked(
                m_pSettingsManager->getColorPickerVisible());
    m_pPreviewContextMenu->addAction(m_pActionToggleColorPicker);

    m_pActionPlay->setChecked(false);
    addAction(m_pActionPlay);

    addAction(m_pActionLoadChapters);
    addAction(m_pActionClearBookmarks);
    addAction(m_pActionBookmarkCurrentFrame);
    addAction(m_pActionUnbookmarkCurrentFrame);
    addAction(m_pActionGoToPreviousBookmark);
    addAction(m_pActionGoToNextBookmark);

    addAction(m_pActionPasteShownFrameNumberIntoScript);
    m_pPreviewContextMenu->addAction(m_pActionPasteShownFrameNumberIntoScript);

    //        m_pPreviewContextMenu->addAction(m_pActionSaveBookmarkToFile);

    //------------------------------------------------------------------------------

    for(ActionToCreate & item : actionsToCreate)
    {
        const char * signal =
                item.checkable ? SIGNAL(toggled(bool)) : SIGNAL(triggered());
        connect(*item.ppAction, signal, this, item.slotToConnect);
    }
}

void MultiTabMainWindow::createStatusBar()
{
    m_pStatusBarWidget = new ScriptStatusBarWidget();
    m_ui->statusBar->addPermanentWidget(m_pStatusBarWidget, 1);
    m_pStatusBarWidget->hide(); // hide by default
}

void MultiTabMainWindow::setPlaybackPanel()
{
    m_ui->playButton->setDefaultAction(m_pActionPlay);
//    m_ui.timeLineCheckButton->setDefaultAction(m_pActionToggleTimeLinePanel);
//    m_ui.timeStepForwardButton->setDefaultAction(m_pActionTimeStepForward);
//    m_ui.timeStepBackButton->setDefaultAction(m_pActionTimeStepBack);

    QVector<QString> mode = {"From Video", "No Limit", "Custom", "23.976", "25", "29.97", "30",
                             "50", "59.94", "60"};

    for (int i = 0; i < mode.size(); ++i) {
        m_ui->playFpsLimitModeComboBox->addItem(mode.at(i), mode.at(i));
    }

//	PlayFPSLimitMode playFpsLimitMode =
//		m_pSettingsManager->getPlayFPSLimitMode();
//	int comboIndex = m_ui.playFpsLimitModeComboBox->findData(
//		(int)playFpsLimitMode);
//	if(comboIndex != -1)
//		m_ui.playFpsLimitModeComboBox->setCurrentIndex(comboIndex);

//	m_ui.playFpsLimitSpinBox->setLocale(QLocale("C"));
//	double customFPS = m_pSettingsManager->getPlayFPSLimit();
//    m_ui.playFpsLimitSpinBox->setValue(customFPS);

    m_ui->playFpsLimitLineEdit->setLocale(QLocale("C"));
    m_ui->playFpsLimitLineEdit->setValidator(
        new QDoubleValidator(12, 120, 3, this)); // numbers only

    slotSetPlayFPSLimit();

//    m_ui.loadChaptersButton->setDefaultAction(m_pActionLoadChapters);
//    m_ui.clearBookmarksButton->setDefaultAction(m_pActionClearBookmarks);
//    m_ui.bookmarkCurrentFrameButton->setDefaultAction(
//        m_pActionBookmarkCurrentFrame);
//    m_ui.unbookmarkCurrentFrameButton->setDefaultAction(
//        m_pActionUnbookmarkCurrentFrame);
//    m_ui.goToPreviousBookmarkButton->setDefaultAction(
//        m_pActionGoToPreviousBookmark);
//    m_ui.goToNextBookmarkButton->setDefaultAction(
//        m_pActionGoToNextBookmark);

//    double timeStep = m_pSettingsManager->getTimeStep();
//    m_ui.timeStepEdit->setTime(vsedit::secondsToQTime(timeStep));


//    connect(m_ui.timeStepEdit, SIGNAL(timeChanged(const QTime &)),
//        this, SLOT(slotTimeStepChanged(const QTime &)));


    connect(m_ui->playFpsLimitModeComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(slotSetPlayFPSLimit()));

    connect(m_ui->playFpsLimitLineEdit, &QLineEdit::editingFinished,
        this, &MultiTabMainWindow::slotSetPlayFPSLimit);


}

bool MultiTabMainWindow::saveScriptToFile(const QString &a_filePath)
{
    if(a_filePath.isEmpty())
        return false;

    QFile scriptFile(a_filePath);
    bool openSuccess = scriptFile.open(QIODevice::WriteOnly | QIODevice::Text);
    if(!openSuccess)
    {
        QMessageBox::critical(this,
            QString::fromUtf8("File open error"),
            QString::fromUtf8("Failed to open the file ") + a_filePath +
            QString::fromUtf8("for writing!"));
        return false;
    }

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString &storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    QByteArray utf8Script = editor->text().toUtf8();
    qint64 writtenBytes = scriptFile.write(utf8Script);

    if(writtenBytes != utf8Script.size())
    {
        QMessageBox::critical(this,
            QString::fromUtf8("File write error"),
            QString::fromUtf8("Error while writing to the file ") + a_filePath);
        return false;
    }

    setCurrentScriptFilePath(storedFilePath, a_filePath);
    m_lastSavedText = editor->text();
    editor->setModified(false);

    return true;
}

bool MultiTabMainWindow::loadScriptFromFile(const QString &a_filePath)
{
    if(a_filePath.isEmpty())
        return false;

    QFile scriptFile(a_filePath);
    bool loadSuccess = scriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if(!loadSuccess)
    {
        QMessageBox::critical(this,
            QString::fromUtf8("File open error"),
            QString::fromUtf8("Failed to open the file %1.").arg(a_filePath));
        return false;
    }

//    setCurrentScriptFilePath(a_filePath);
    QByteArray utf8Script = scriptFile.readAll();
    QString scriptText = QString::fromUtf8(utf8Script);
    m_lastSavedText = scriptText;

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString &storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    setCurrentScriptFilePath(storedFilePath, a_filePath);

    editor->setPlainText(scriptText);

    m_pBenchmarkDialog->resetSavedRange();

    return true;

}

bool MultiTabMainWindow::safeToCloseFile()
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    bool needPrompt = (m_pSettingsManager->getPromptToSaveChanges() &&
        editor->isModified());

    if(!needPrompt)
        return true;

    QMessageBox::StandardButton choice = QMessageBox::NoButton;
    if(storedFilePath.isEmpty())
    {
        choice = QMessageBox::question(this, tr("Save script?"),
            tr("Would you like to save your script before closing?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if(choice == QMessageBox::Yes)
        {
            bool saved = slotSaveScriptAs(storedFilePath);
            if(!saved)
                return false;
        }
    }
    else
    {
        choice = QMessageBox::question(this, tr("Save script?"),
            tr("Would you like to save script \"%1\" before closing?")
            .arg(storedFilePath),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if(choice == QMessageBox::Yes)
        {
            bool saved = slotSaveScript();
            if(!saved)
                return false;
        }
    }

    if(choice == QMessageBox::Cancel)
        return false;

    return true;

}

void MultiTabMainWindow::setCurrentScriptFilePath(QString & a_storedFilePath,
                                                  const QString & a_filePath)
{
    if(a_storedFilePath == a_filePath)
        return;

    a_storedFilePath = a_filePath;
    m_pSettingsManager->setLastUsedPath(a_filePath);
    slotChangeWindowTitle(a_storedFilePath);
    fillRecentScriptsMenu();
}

void MultiTabMainWindow::loadStartUpScript()
{
    slotNewScript();

    QStringList argumentsList = QCoreApplication::arguments();
    if(argumentsList.size() > 1)
        loadScriptFromFile(argumentsList.at(1));
    else if(m_pSettingsManager->getAutoLoadLastScript())
    {
        QString lastUsedPath = m_pSettingsManager->getLastUsedPath();
        if(!lastUsedPath.isEmpty())
            loadScriptFromFile(lastUsedPath);
    }
}


void MultiTabMainWindow::fillRecentScriptsMenu()
{
    m_pMenuRecentScripts->clear();
    QStringList recentSciptsList = m_pSettingsManager->getRecentFilesList();
    for(const QString & filePath : recentSciptsList)
    {
        QAction * pAction = new QAction(m_pMenuRecentScripts);
        pAction->setIconText(filePath);
        pAction->setData(filePath);
        m_pMenuRecentScripts->addAction(pAction);

        connect(pAction, SIGNAL(triggered()),
            this, SLOT(slotOpenRecentScriptActionTriggered()));
    }

}

void MultiTabMainWindow::setUpZoomPanel()
{
    m_ui->zoomRatioSpinBox->setLocale(QLocale("C"));

    m_ui->zoomModeComboBox->addItem(QIcon(":zoom_no_zoom.png"),
        tr("No zoom"), int(ZoomMode::NoZoom));
    m_ui->zoomModeComboBox->addItem(QIcon(":zoom_fixed_ratio.png"),
        tr("Fixed ratio"), int(ZoomMode::FixedRatio));
    m_ui->zoomModeComboBox->addItem(QIcon(":zoom_fit_to_frame.png"),
        tr("Fit to frame"), int(ZoomMode::FitToFrame));

    ZoomMode zoomMode = m_pSettingsManager->getZoomMode();
    int comboIndex = m_ui->zoomModeComboBox->findData(int(zoomMode));
    if(comboIndex != -1)
        m_ui->zoomModeComboBox->setCurrentIndex(comboIndex);

    bool fixedRatio(zoomMode == ZoomMode::FixedRatio);
    m_ui->zoomRatioSpinBox->setEnabled(fixedRatio);

    double zoomRatio = m_pSettingsManager->getZoomRatio();
    m_ui->zoomRatioSpinBox->setValue(zoomRatio);

//    m_ui->resizerComboBox->addItem(tr("Nearest"),
//        int(Qt::FastTransformation));
//    m_ui->resizerComboBox->addItem(tr("Bilinear"),
//        int(Qt::SmoothTransformation));
//    bool noZoom = (zoomMode == ZoomMode::NoZoom);
//    m_ui->resizerComboBox->setEnabled(!noZoom);

//    Qt::TransformationMode scaleMode = m_pSettingsManager->getScaleMode();
//    comboIndex = m_ui->resizerComboBox->findData(int(scaleMode));
//    if(comboIndex != -1)
//        m_ui->resizerComboBox->setCurrentIndex(comboIndex);

    connect(m_ui->zoomModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MultiTabMainWindow::slotZoomModeChanged);

    connect(m_ui->zoomRatioSpinBox, QOverload<double>::of(&ZoomRatioSpinBox::valueChanged),
        this, &MultiTabMainWindow::slotZoomRatioChanged);
//    connect(m_ui->resizerComboBox, SIGNAL(currentIndexChanged(int)),
//        this, SLOT(slotScaleModeChanged()));

}

double MultiTabMainWindow::YCoCgValueAtPoint(size_t a_x, size_t a_y, int a_plane, const VSAPI *a_cpVSAPI, const VSFrameRef *a_cpFrameRef)
{
    Q_ASSERT(a_cpVSAPI);

    if(!a_cpFrameRef)
        return 0.0;

    const VSFormat * cpFormat = a_cpVSAPI->getFrameFormat(a_cpFrameRef);

    Q_ASSERT((a_plane >= 0) && (a_plane < cpFormat->numPlanes));

    const uint8_t * cpPlane =
        a_cpVSAPI->getReadPtr(a_cpFrameRef, a_plane);

    size_t x = a_x;
    size_t y = a_y;

    if(a_plane != 0)
    {
        x = (a_x >> cpFormat->subSamplingW);
        y = (a_y >> cpFormat->subSamplingH);
    }
    int stride = a_cpVSAPI->getStride(a_cpFrameRef, a_plane);
    const uint8_t * cpLine = cpPlane + y * stride;

    double value = 0.0;

    if(cpFormat->sampleType == stInteger)
    {
        if(cpFormat->bytesPerSample == 1)
            value = (double)cpLine[x];
        else if(cpFormat->bytesPerSample == 2)
            value = (double)((uint16_t *)cpLine)[x];
        else if(cpFormat->bytesPerSample == 4)
            value = (double)((uint32_t *)cpLine)[x];
    }
    else if(cpFormat->sampleType == stFloat)
    {
        if(cpFormat->bytesPerSample == 2)
        {
            vsedit::FP16 half;
            half.u = ((uint16_t *)cpLine)[x];
            vsedit::FP32 single = vsedit::halfToSingle(half);
            value = (double)single.f;
        }
        else if(cpFormat->bytesPerSample == 4)
            value = (double)((float *)cpLine)[x];
    }

    return value;

}

void MultiTabMainWindow::createGeometrySaveTimer()
{
    m_pGeometrySaveTimer = new QTimer(this);
    m_pGeometrySaveTimer->setInterval(DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY);
    connect(m_pGeometrySaveTimer, &QTimer::timeout,
        this, &MultiTabMainWindow::slotSaveGeometry);
}

void MultiTabMainWindow::slotRemoveTab()
{
    int currentTabIndex = m_ui->previewTabWidget->currentIndex();

    // cleanup cache from processor
    ScriptProcessor * processor = m_pEditorPreviewVector[currentTabIndex].processor;
    QString scriptName = m_pEditorPreviewVector[currentTabIndex].scriptName;
    processor->cleanUpOnClose();

    m_ui->scriptTabWidget->removeTab(currentTabIndex); // remove tab
    m_ui->previewTabWidget->removeTab(currentTabIndex);
    m_pEditorPreviewVector.remove(currentTabIndex); // remove widgets from vector
    m_pBookmarkManagerDialog->slotRemoveScriptBookmark(scriptName); // remove script from bookmark listing

    // create new tab when there is none
    int tabCount = m_ui->scriptTabWidget->count();
    if (tabCount < 1)
        slotCreateTab();
}

void MultiTabMainWindow::slotChangePreviewTab(int a_index)
{
    int currentIndex = m_ui->scriptTabWidget->currentIndex(); // unnessacery?
    if (currentIndex != a_index)
        m_ui->scriptTabWidget->setCurrentIndex(a_index);
}

void MultiTabMainWindow::slotSaveTabBeforeChanged(int a_currentTabIndex, int a_selectedTabIndex)
{
    QString script = m_pEditorPreviewVector[a_currentTabIndex].processor->script();
    if (script.isEmpty())
        return;

    // save timeline frame number and zoom ratio from last tab
    int currentFrame = m_pEditorPreviewVector[a_currentTabIndex].processor->currentFrame();
    int currentZoomRatio = m_ui->timeLineView->zoomFactor();

    int savedFrame = m_pEditorPreviewVector[a_currentTabIndex].lastTimeLineFrameIndex;
    int savedRatio = m_pEditorPreviewVector[a_currentTabIndex].lastZoomRatio;

    if (savedFrame != currentFrame)
        m_pEditorPreviewVector[a_currentTabIndex].lastTimeLineFrameIndex = currentFrame;

    if (savedRatio != currentZoomRatio)
        m_pEditorPreviewVector[a_currentTabIndex].lastZoomRatio = currentZoomRatio;
}

void MultiTabMainWindow::slotChangeScriptTab(int a_index)
{    
    int currentIndex = m_ui->previewTabWidget->currentIndex();
    if (currentIndex != a_index)
        m_ui->previewTabWidget->setCurrentIndex(a_index); // change preview tab

    // don't run if vector has nothing or if tab didn't exist
    if (m_pEditorPreviewVector.count() < 1) return;
    if (a_index < 0) return;

    // check if current tab have script running
    QString script = m_pEditorPreviewVector[a_index].processor->script();
    if (!script.isEmpty()) {
        m_ui->timeLineView->setEnabled(true);

        // load last saved frame and zoom factor for timelineview
        int lastFrame = m_pEditorPreviewVector[a_index].lastTimeLineFrameIndex;
        int lastZoomRatio = m_pEditorPreviewVector[a_index].lastZoomRatio;

        /* reset time line and set last saved frame and zoomfactor */
        ScriptProcessor * processor = m_pEditorPreviewVector[a_index].processor;
        const VSVideoInfo * vsVideoInfo = processor->vsVideoInfo(); // retrieve numFrames and fps

        slotSetTimeLineAndIndicator(vsVideoInfo->numFrames, vsVideoInfo->fpsNum, vsVideoInfo->fpsDen);
        m_ui->timeLineView->setZoomFactor(lastZoomRatio);
        m_ui->timeLineView->setFrame(lastFrame);

        m_ui->timeLineView->centerSliderOnCurrentFrame();

        m_pStatusBarWidget->show();
        m_pStatusBarWidget->setVideoInfo(vsVideoInfo);

        /* change script bookmark in bookmark manager */
        QString scriptName = m_pEditorPreviewVector[a_index].scriptName;
        m_pBookmarkManagerDialog->slotUpdateScriptBookmarkSelection(scriptName);
    } else {
        // set satusbar to null
        m_pStatusBarWidget->hide();
        m_ui->timeLineView->setEnabled(false);
    }
}

void MultiTabMainWindow::slotPreviewScript()
{
    int tabCount = m_ui->scriptTabWidget->count();
    if (tabCount < 1)
        return;

    int currentIndex = m_ui->scriptTabWidget->currentIndex();

    QString script = m_pEditorPreviewVector[currentIndex].editor->text();
    QString scriptName = m_pEditorPreviewVector[currentIndex].scriptName;

    ScriptProcessor * processor = m_pEditorPreviewVector[currentIndex].processor;


    if (processor->previewScript(script, scriptName)) {
        if (!m_ui->timeLineView->isEnabled())
            m_ui->timeLineView->setEnabled(true);

        slotDisplayModeChanged(); // set display mode

        const VSVideoInfo * vsVideoInfo = processor->vsVideoInfo();
        m_pStatusBarWidget->setVideoInfo(vsVideoInfo); // set status bar
        m_pStatusBarWidget->show();
    }
}

void MultiTabMainWindow::slotSetTimeLineAndIndicator(int a_numFrames, int64_t a_fpsNum, int64_t a_fpsDen)
{
    m_ui->frameNumberIndicatorSpinBox->setMaximum(a_numFrames - 1);
    m_ui->timeLineView->slotSetTimeLine(a_numFrames, a_fpsNum, a_fpsDen);
}

void MultiTabMainWindow::slotDisplayModeChanged()
{
    TimeLine::DisplayMode timeLineDisplayMode = TimeLine::DisplayMode::Frames;

    if (m_ui->displayModeToggleButton->isChecked()) {
        m_ui->displayModeToggleButton->setText(tr("Time"));
        timeLineDisplayMode = TimeLine::DisplayMode::Time;
    } else {
        m_ui->displayModeToggleButton->setText(tr("Frame"));
    }

    m_ui->timeLineView->setDisplayMode(timeLineDisplayMode);
}

void MultiTabMainWindow::slotShowFrameFromTimeLine(int a_frameNumber)
{
    if (m_pEditorPreviewVector.count() < 1) return;
    // retrieve procceor of current script, and pass in frame to showframe
    int currentIndex = m_ui->scriptTabWidget->currentIndex();

    ScriptProcessor * processor = m_pEditorPreviewVector[currentIndex].processor;

    if (processor->script().isEmpty()) return;


    /* update frame time indicator from tab change */
    const VSVideoInfo * vsVideoInfo = processor->vsVideoInfo();
    double fps = double(vsVideoInfo->fpsNum) / double(vsVideoInfo->fpsDen);
    int milliSeconds = int((double(a_frameNumber) / fps) * 1000);
    QTime time = QTime::fromMSecsSinceStartOfDay(milliSeconds);

    // frame change from timeline
    if (m_ui->timeLineView->frame() != a_frameNumber)
        slotUpdateFrameTimeIndicators(a_frameNumber, time);
    else
        m_ui->frameTimeTimeEdit->setTime(time);

    processor->showFrameFromTimeLine(a_frameNumber);
    /* processor and timeline signal/slot was created on tab creation */
}


void MultiTabMainWindow::slotJumpPlayFromTimeLine(int a_frameNumber)
{
    int currentIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptProcessor * processor = m_pEditorPreviewVector[currentIndex].processor;

    processor->slotJumpPlay(a_frameNumber);
}

void MultiTabMainWindow::slotUpdateFrameTimeIndicators(int a_frameNumber, const QTime &a_time)
{
    m_ui->timeLineView->setFrame(a_frameNumber);
    m_ui->frameTimeTimeEdit->setTime(a_time);
}

void MultiTabMainWindow::slotUpdateHoverTimeIndicator(const QTime &a_time)
{
    m_ui->hoverTimeTimeEdit->setTime(a_time);
}

void MultiTabMainWindow::slotTimeLineFrameChanged(int a_frameNumber)
{
    /* catch frame change signal from timeline
     * and pass to frame number indicator */
    m_ui->frameNumberIndicatorSpinBox->setValue(a_frameNumber);
}

void MultiTabMainWindow::slotUpdateStatusBarQueueState(size_t a_framesInQueue, size_t a_frameInProcess, size_t a_maxThreads)
{
    m_pStatusBarWidget->setQueueState(a_framesInQueue, a_frameInProcess, a_maxThreads);
}

void MultiTabMainWindow::slotShowFrameInfoDialog()
{
    if (!m_pFrameInfoDialog->isVisible())
        m_pFrameInfoDialog->show();
}

void MultiTabMainWindow::slotShowBookmarkManager()
{
    if (!m_pBookmarkManagerDialog->isVisible())
        m_pBookmarkManagerDialog->show();
}

void MultiTabMainWindow::slotZoomModeChanged(int a_mode)
{
    int currentIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptProcessor * processor = m_pEditorPreviewVector[currentIndex].processor;
    if (processor->script().isEmpty()) return; // disable if no script running

    static bool changingZoomMode = false;
    if(changingZoomMode)
        return;
    changingZoomMode = true;

    ZoomMode zoomMode = ZoomMode(a_mode);

    /* context menu update */
    QObject * pSender = sender();
    if(pSender == m_ui->zoomModeComboBox)
    {
        for(QAction * pAction : m_pActionGroupZoomModes->actions())
        {
            ZoomMode actionZoomMode =
                m_actionIDToZoomModeMap[pAction->data().toString()];
            if(actionZoomMode == zoomMode)
            {
                pAction->setChecked(true);
                break;
            }
        }
    }
    else
    {
        // If signal wasn't sent by combo box - presume it was sent by action.
        QAction * pSenderAction = qobject_cast<QAction *>(pSender);
        zoomMode = m_actionIDToZoomModeMap[pSenderAction->data().toString()];
        int zoomModeIndex = m_ui->zoomModeComboBox->findData(int(zoomMode));
        m_ui->zoomModeComboBox->setCurrentIndex(zoomModeIndex);
    }


    bool fixedRatio(zoomMode == ZoomMode::FixedRatio);
    m_ui->zoomRatioSpinBox->setEnabled(fixedRatio);
    bool noZoom = (zoomMode == ZoomMode::NoZoom);
//	m_ui.scaleModeComboBox->setEnabled(!noZoom);
//	m_pMenuZoomScaleModes->setEnabled(!noZoom);
    m_pSettingsManager->setZoomMode(zoomMode);

    setPreviewPixmap();
    changingZoomMode = false;
}

void MultiTabMainWindow::slotZoomRatioChanged(double a_zoomRatio)
{
    if (m_pEditorPreviewVector.count() < 1) return;
    setPreviewPixmap();
    m_pSettingsManager->setZoomRatio(a_zoomRatio);
}

void MultiTabMainWindow::setPreviewPixmap()
{
    if (m_ui->previewTabWidget->count() < 1) return;
    if (m_pEditorPreviewVector.count() < 1) return;

    int currentTabIndex = m_ui->previewTabWidget->currentIndex();
    QString script = m_pEditorPreviewVector[currentTabIndex].processor->script();

    if (script.isEmpty()) return; // only run if there is active script


//    if(m_ui.cropPanel->isVisible())
//	{
//		int cropLeft = m_ui.cropLeftSpinBox->value();
//		int cropTop = m_ui.cropTopSpinBox->value();
//		int cropWidth = m_ui.cropWidthSpinBox->value();
//		int cropHeight = m_ui.cropHeightSpinBox->value();
//		QPixmap croppedPixmap = m_framePixmap.copy(cropLeft, cropTop,
//			cropWidth, cropHeight);
//		int ratio = m_ui.cropZoomRatioSpinBox->value();

//		if(ratio == 1)
//		{
//			m_ui.previewArea->setPixmap(croppedPixmap);
//			return;
//		}

//		QPixmap zoomedPixmap = croppedPixmap.scaled(
//			croppedPixmap.width() * ratio, croppedPixmap.height() * ratio,
//			Qt::KeepAspectRatio, Qt::FastTransformation);
//		m_ui.previewArea->setPixmap(zoomedPixmap);
//		return;
//	}

    // get pixmap from processor as base
    QPixmap framePixmap = m_pEditorPreviewVector[currentTabIndex].processor->framePixmap();
    PreviewArea * previewArea = m_pEditorPreviewVector[currentTabIndex].previewArea;

    ZoomMode zoomMode = ZoomMode(m_ui->zoomModeComboBox->currentData().toInt());
    if(zoomMode == ZoomMode::NoZoom)
    {
        previewArea->setPixmap(framePixmap);
        return;
    }

    QPixmap previewPixmap;
    int frameWidth = 0;
    int frameHeight = 0;
//	Qt::TransformationMode scaleMode = (Qt::TransformationMode)
//		m_ui.scaleModeComboBox->currentData().toInt();

    if(zoomMode == ZoomMode::FixedRatio)
    {
        double ratio = m_ui->zoomRatioSpinBox->value();
        frameWidth = int (double(framePixmap.width()) * ratio);
        frameHeight = int (double(framePixmap.height()) * ratio);
    }
    else /* zoomMode::fitToFrame */
    {
        QRect previewRect = previewArea->geometry();
        int cropSize = previewArea->frameWidth() * 2;
        frameWidth = previewRect.width() - cropSize;
        frameHeight = previewRect.height() - cropSize;
    }

    previewPixmap = framePixmap.scaled(frameWidth, frameHeight,
        Qt::KeepAspectRatio);
    previewArea->setPixmap(previewPixmap);
}

void MultiTabMainWindow::slotPreviewAreaSizeChanged()
{
    ZoomMode zoomMode = ZoomMode (m_ui->zoomModeComboBox->currentData().toInt());
    if(zoomMode == ZoomMode::FitToFrame)
        setPreviewPixmap();
}

void MultiTabMainWindow::slotPreviewAreaMouseOverPoint(float a_normX, float a_normY)
{
    if(!m_pFrameInfoDialog->isVisible())
        return;

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptProcessor *processor = m_pEditorPreviewVector[currentTabIndex].processor;
    const VSAPI * cpVSAPI = m_pVSScriptLibrary->getVSAPI();
    const VSFrameRef * cpFrameRef = processor->frameRef();
    const VSVideoInfo * cpVideoInfo =  processor->vsVideoInfo();

    if(!cpFrameRef)
        return;

    QPixmap framePixmap = processor->framePixmap();

    double value1 = 0.0;
    double value2 = 0.0;
    double value3 = 0.0;

    size_t frameX = 0;
    size_t frameY = 0;

    frameX = size_t(float(framePixmap.width()) * a_normX);
    frameY = size_t(float(framePixmap.height()) * a_normY);

    int width = cpVSAPI->getFrameWidth(cpFrameRef, 0);
    int height = cpVSAPI->getFrameHeight(cpFrameRef, 0);
    const VSFormat * cpFormat = cpVSAPI->getFrameFormat(cpFrameRef);

    if((frameX >= size_t(width)) || (frameY >= size_t(height)))
        return;

    if(cpFormat->id == pfCompatBGR32)
    {
        const uint8_t * cpData = cpVSAPI->getReadPtr(cpFrameRef, 0);
        int stride = cpVSAPI->getStride(cpFrameRef, 0);
        const uint32_t * cpLine = (const uint32_t *)(cpData + frameY * stride);
        uint32_t packedValue = cpLine[frameX];
        value3 = double(packedValue & 0xFF);
        value2 = double((packedValue >> 8) & 0xFF);
        value1 = double((packedValue >> 16) & 0xFF);
    }
    else if(cpFormat->id == pfCompatYUY2)
    {
        size_t x = frameX >> 1;
        size_t rem = frameX & 0x1;
        const uint8_t * cpData = cpVSAPI->getReadPtr(cpFrameRef, 0);
        int stride = cpVSAPI->getStride(cpFrameRef, 0);
        const uint32_t * cpLine = (const uint32_t *)(cpData + frameY * stride);
        uint32_t packedValue = cpLine[x];

        if(rem == 0)
            value1 = double(packedValue & 0xFF);
        else
            value1 = double((packedValue >> 16) & 0xFF);
        value2 = double((packedValue >> 8) & 0xFF);
        value3 = double((packedValue >> 24) & 0xFF);
    }
    else
    {
        value1 = YCoCgValueAtPoint(frameX, frameY, 0, cpVSAPI, cpFrameRef);
        if(cpFormat->numPlanes > 1)
            value2 = YCoCgValueAtPoint(frameX, frameY, 1, cpVSAPI, cpFrameRef);
        if(cpFormat->numPlanes > 2)
            value3 = YCoCgValueAtPoint(frameX, frameY, 2, cpVSAPI, cpFrameRef);
    }

    QString l1("1");
    QString l2("2");
    QString l3("3");

    int colorFamily = cpVideoInfo->format->colorFamily;
    int formatID = cpVideoInfo->format->id;

    if((colorFamily == cmYUV) || (formatID == pfCompatYUY2))
    {
        l1 = "Y";
        l2 = "U";
        l3 = "V";
    }
    else if((colorFamily == cmRGB) || (formatID == pfCompatBGR32))
    {
        l1 = "R";
        l2 = "G";
        l3 = "B";
    }
    else if(colorFamily == cmYCoCg)
    {
        l1 = "Y";
        l2 = "Co";
        l3 = "Cg";
    }

    QString colorString = QString("%1:%2|%3:%4|%5:%6")
        .arg(l1).arg(value1).arg(l2).arg(value2).arg(l3).arg(value3);

    if(colorFamily == cmGray)
        colorString = QString("G:%1").arg(value1);

    m_pFrameInfoDialog->setColorPickerString(colorString);

    QString mousePosString = QString("x:%1  y:%2").arg(frameX).arg(frameY);
    m_pFrameInfoDialog->setMousePositionString(mousePosString);

}

void MultiTabMainWindow::slotPlay(bool a_play)
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptProcessor *processor = m_pEditorPreviewVector[currentTabIndex].processor;

    QString script = processor->script();
    if (script.isEmpty()) return;

    bool playing = processor->slotPlay(a_play);

    if (playing) {
        m_pActionPlay->setIcon(m_iconPause);
    } else {
        m_pActionPlay->setIcon(m_iconPlay);
    }

    m_ui->timeLineView->setPlay(playing); // passing the flag into timeline

}

void MultiTabMainWindow::slotSetPlayFPSLimit()
{
    if (!m_ui->playFpsLimitLineEdit->isReadOnly()) {
        m_ui->playFpsLimitLineEdit->setReadOnly(true);
    }

    double limit = vsedit::round(QVariant(m_ui->playFpsLimitLineEdit->text()).toDouble());

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptProcessor *processor = m_pEditorPreviewVector[currentTabIndex].processor;
    const VSVideoInfo * cpVideoInfo = processor->vsVideoInfo();


//    PlayFPSLimitMode mode =
//            PlayFPSLimitMode(m_ui.playFpsLimitModeComboBox->currentData().toInt());

    QVariant mode = m_ui->playFpsLimitModeComboBox->currentData();
    QString outputText;
    double secondsBetweenFrames;

    if(mode == "No Limit") {
        secondsBetweenFrames = 0.0;
        outputText = mode.toString();
    }
    else if(mode == "Custom") {
        m_ui->playFpsLimitLineEdit->setReadOnly(false);
        secondsBetweenFrames = 1.0 / limit;
        outputText = QVariant(limit).toString();
    }
    else if(mode == "From Video")
    {
        if(!cpVideoInfo) {
            secondsBetweenFrames = 0.0;
            outputText = "No video";        }

        else if(cpVideoInfo->fpsNum == 0ll) {
            secondsBetweenFrames = 0.0;
            outputText = "Unknown FPS";
        }
        else
        {
            secondsBetweenFrames = double(cpVideoInfo->fpsDen) / double(cpVideoInfo->fpsNum);
            double fps = double(cpVideoInfo->fpsNum) / double(cpVideoInfo->fpsDen);
            outputText = QVariant(vsedit::round(fps)).toString();
        }
    }
    else {
        secondsBetweenFrames = 1.0 / mode.toDouble();
        outputText = mode.toString();
    }

    processor->slotSetPlaySpeed(secondsBetweenFrames);
    m_ui->playFpsLimitLineEdit->setText(outputText);


//	m_pSettingsManager->setPlayFPSLimitMode(selectedValue);
//	m_pSettingsManager->setPlayFPSLimit(limit);

}

void MultiTabMainWindow::slotUpdateScriptBookmarkList()
{

}

void MultiTabMainWindow::slotSetScriptBookmark(int a_index)
{
    if (a_index < 0) return;

    /* set bookmark model for tableview and update script bookmark selection */
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[a_index].bookmarkModel;
    m_pBookmarkManagerDialog->slotSetTableViewModel(bookmarkModel);

//    QString scriptName = m_pEditorPreviewVector[a_index].ScriptName;
//    m_pBookmarkManagerDialog->slotUpdateScriptBookmarkSelection(scriptName);

    m_ui->scriptTabWidget->setCurrentIndex(a_index);
//    slotChangeScriptTab(a_index);
}

void MultiTabMainWindow::slotAddBookmark()
{
    // get current frame and add to bookmark model
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptProcessor *processor = m_pEditorPreviewVector[currentTabIndex].processor;
    int currentFrame = processor->currentFrame();
    const VSVideoInfo * vsVideoInfo = processor->vsVideoInfo();
    double fps = double(vsVideoInfo->fpsNum) / double(vsVideoInfo->fpsDen);

    if (currentFrame < 0) return;
    int timeInMilli = double(currentFrame) / fps * double(1000);

    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;
    bookmarkModel->addBookmark(currentFrame, timeInMilli);

//    if (m_playing)
//        return;
//    emit signalBookmarkCurrentFrame(m_frameShown);

}

void MultiTabMainWindow::slotRemoveBookmark(QModelIndex a_index)
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;

    bookmarkModel->removeBookmark(a_index);
}

void MultiTabMainWindow::slotGoToBookmark(const QModelIndex a_modelIndex)
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;

    /* ignore signal when double clicked on title colum */
    if (a_modelIndex.column() == TITLE_COL) return;

    int frameIndex = bookmarkModel->selectedFrameIndex(a_modelIndex);
    m_ui->timeLineView->setFrame(frameIndex);
}

void MultiTabMainWindow::slotClearBookmark()
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;
    bookmarkModel->clearAll();
}

void MultiTabMainWindow::slotLoadBookmarkFile(QFile & a_file)
{
    slotClearBookmark();

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;

    QRegularExpression reBookmark("(\\d+)");

    while(!a_file.atEnd())
    {
        const QByteArray line = a_file.readLine();
        QRegularExpressionMatchIterator i = reBookmark.globalMatch(line);

        while (i.hasNext()) {
            QRegularExpressionMatch bookmarkMatch = i.next();
            int frameIndex = bookmarkMatch.captured(0).toInt();

            ScriptProcessor * processor = m_pEditorPreviewVector[currentTabIndex].processor;
            const VSVideoInfo * vsVideoInfo = processor->vsVideoInfo();
            double fps = double(vsVideoInfo->fpsNum) / double(vsVideoInfo->fpsDen);
            int timeInMilli = (double(frameIndex) / fps) * 1000;

            bookmarkModel->addBookmark(frameIndex, timeInMilli);
        }
    }
}

void MultiTabMainWindow::slotLoadChapterFile(QFile & a_file)
{
    slotClearBookmark();

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;

    QRegularExpression reChapter(R"((\d{2}):(\d{2}):(\d{2})[\.:](\d{3})?)");
    QRegularExpression reTitle("CHAPTER\\d{2,3}NAME=(.*)");

    if(!reTitle.isValid()) {
        return;
    }

    QString dummyDate = QString("2020-01-01T"); // add this to make Datetime to milliseconds work

    QStringList titleList;
    QList<int> timeStampList;

    while(!a_file.atEnd())
    {
        const QByteArray line = a_file.readLine();
        QRegularExpressionMatch matchChapter = reChapter.match(line);
        QRegularExpressionMatch matchTitle = reTitle.match(line);

        QString timestamp("");
        int timeInMilli = 0;
        QString title;

        if (matchChapter.hasMatch()) {
            timestamp = matchChapter.captured(0);
            timeInMilli = QDateTime::fromString(dummyDate+timestamp, Qt::ISODateWithMs)
                    .time().msecsSinceStartOfDay(); // convert to milliseconds

            timeStampList.append(timeInMilli);
        }
        if (matchTitle.hasMatch()) {
            title = matchTitle.captured(1);
            titleList.append(title);
        }
    }

    ScriptProcessor * processor = m_pEditorPreviewVector[currentTabIndex].processor;
    const VSVideoInfo * vsVideoInfo = processor->vsVideoInfo();
    double fps = double(vsVideoInfo->fpsNum) / double(vsVideoInfo->fpsDen);

    // loop through both list
    auto t = titleList.begin();
    auto ts = timeStampList.begin();
    while (t != titleList.end() && ts != timeStampList.end())
    {
      auto  x = *t++;
      auto& y = *ts++;

      bookmarkModel->addChapter(x, y, fps);
    }

    // use this instead when upgraded to C++17
//    for (auto& [t, ts] : zip(titleList, timeStampList)) {
//        m_bookmarkModel->addChapter(t, ts, m_fps);
//    }

}

void MultiTabMainWindow::slotSaveBookmarksToFile()
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    BookmarkModel * bookmarkModel = m_pEditorPreviewVector[currentTabIndex].bookmarkModel;
    QString scriptName = m_pEditorPreviewVector[currentTabIndex].scriptName;

    QVector<BookmarkData> bookmarks = bookmarkModel->bookmarks();
    QString delimiter = m_pSettingsManager->getBookmarkDelimiter();

    if(bookmarks.count() > 0)
    {
        QFileInfo fileInfo(scriptName);
        //get file path and fileName without extension
        QString filePath = fileInfo.absolutePath();
        QString scriptFileName = fileInfo.baseName();

        QString fileName = QFileDialog::getSaveFileName(this,
                            tr("Save bookmark"), filePath + QDir::separator() + scriptFileName,
                            tr("Text file (*.txt)"));

        if (fileName.isEmpty())
            return;
        else
        {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly))
            {
                QMessageBox::information(this, tr("Unable to open file"),
                             file.errorString());
                return;
            }

            QStringList bookmarksStringList;
            QString bookmarksString = "";

            BookmarkSavingFormat savingFormat = m_pSettingsManager->getBookmarkSavingFormat();

            switch (savingFormat) {
            case BookmarkSavingFormat::ChapterFormat:
                // looks for timestamp and title
                for (auto it = bookmarks.begin(); it != bookmarks.end(); ++it) {
                    int index = std::distance(bookmarks.begin(), it);
                    int chapterCounterIndex = index + 1; // +1 to start counter at 1
                    QTime time = QTime::fromMSecsSinceStartOfDay(it->timeInMilli);
                    int fieldWidth = 2;

                    if (chapterCounterIndex > 99) // 2 digit for < 100, 3 digit for > 100
                        fieldWidth = 3;

                    // format: CHAPTER01=00:00:00.000
                    QString chapterTime = QString("CHAPTER%1=%2")
                            .arg(chapterCounterIndex, fieldWidth, 10, QLatin1Char('0'))
                            .arg(time.toString("hh:mm:ss.zzz"));

                    bookmarksStringList.append(chapterTime);

                    // format: CHAPTER01NAME=abcde
                    QString chapterName = QString("CHAPTER%1NAME=%2")
                            .arg(chapterCounterIndex, fieldWidth, 10, QLatin1Char('0'))
                            .arg(bookmarks[index].title);

                    bookmarksStringList.append(chapterName);
                }
                bookmarksString = bookmarksStringList.join("\n");
                break;

            case BookmarkSavingFormat::BookmarkFormat:
                // only looks for frame number
                for (auto i : bookmarks) {
                    bookmarksStringList.append(QVariant(i.frame).toString());
                }

                bookmarksString = bookmarksStringList.join(delimiter);
                break;
            }

            QTextStream out(&file);
            out << bookmarksString;

//            emit signalBookmarkSavedToFile(fileName);
        }
    }

}

void MultiTabMainWindow::slotNewScript()
{
    if(!safeToCloseFile())
        return;

    QString newScriptTemplate = m_pSettingsManager->getNewScriptTemplate();


    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString &storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    storedFilePath.clear();
    m_lastSavedText = newScriptTemplate;


    editor->setPlainText(newScriptTemplate);
    editor->moveCursor(QTextCursor::End);
    editor->setModified(true);

    m_pBenchmarkDialog->resetSavedRange();
}

// END OF void MainWindow::slotNewScript()
//==============================================================================

bool MultiTabMainWindow::slotSaveScript()
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    if(!editor->isModified())
        return false;

    if(storedFilePath.isEmpty())
    {
        slotSaveScriptAs(storedFilePath);
        return false;
    }

    return saveScriptToFile(storedFilePath);
}

// END OF bool MainWindow::slotSaveScript()
//==============================================================================

bool MultiTabMainWindow::slotSaveScriptAs(QString & a_offeredFilePath)
{
    if(a_offeredFilePath.isEmpty())
    {
        QFileInfo fileInfo(m_pSettingsManager->getLastUsedPath());
        a_offeredFilePath = fileInfo.absoluteDir().path() +
            tr("/Untitled.vpy");
    }

    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Save VapourSynth script"), a_offeredFilePath,
        tr("VapourSynth script (*.vpy);;All files (*)"));

    if(!filePath.isEmpty())
    {
        bool success = saveScriptToFile(filePath);

        // Copy bookmarks
//        if((!success) || offeredFilePath.isEmpty())
//            return success;
//        QFile::copy(offeredFilePath + TIMELINE_BOOKMARKS_FILE_SUFFIX,
//            filePath + TIMELINE_BOOKMARKS_FILE_SUFFIX);
    }

    return false;
}

// END OF bool MultiTabMainWindow::slotSaveScriptAs()
//==============================================================================

bool MultiTabMainWindow::slotOpenScript()
{
    if(!safeToCloseFile())
        return false;

    QFileInfo fileInfo(m_pSettingsManager->getLastUsedPath());
    QString offeredPath = fileInfo.absoluteDir().path();

    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open VapourSynth script"), offeredPath,
        tr("VapourSynth script (*.vpy);;All files (*)"));

    return loadScriptFromFile(filePath);
}

// END OF bool MainWindow::slotOpenScript()
//==============================================================================

void MultiTabMainWindow::slotTemplates()
{
    m_pTemplatesDialog->call();
}

// END OF void MainWindow::slotTemplates()
//==============================================================================

void MultiTabMainWindow::slotCheckScript()
{
    VapourSynthScriptProcessor tempProcessor(m_pSettingsManager, m_pVSScriptLibrary, this);
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString &storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;


    connect(&tempProcessor, SIGNAL(signalWriteLogMessage(int, const QString &)),
        this, SLOT(slotWriteLogMessage(int, const QString &)));

    bool correct = tempProcessor.initialize(editor->text(), storedFilePath);
    if(correct)
    {
        QString message = tr("Script was successfully evaluated. "
            "Output video info:\n");
        message += vsedit::videoInfoString(tempProcessor.videoInfo());
        m_logView->addEntry(message, LOG_STYLE_POSITIVE);
    }
}

// END OF void MainWindow::slotCheckScript()
//==============================================================================

void MultiTabMainWindow::slotBenchmark()
{
    if(m_pBenchmarkDialog->busy())
    {
        QString message = tr("Benchmark dialog appears busy processing "
            "frames. Please stop any active actions in the dialog and wait "
            "for script processor to finish processing.");
        m_logView->addEntry(message, LOG_STYLE_WARNING);
        return;
    }

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    m_pBenchmarkDialog->initialize(editor->text(), storedFilePath);
    m_pBenchmarkDialog->call();
}

// END OF void MainWindow::slotBenchmark()
//==============================================================================

void MultiTabMainWindow::slotEncode()
{
    if(m_pEncodeDialog->busy())
    {
        m_pEncodeDialog->showActive();
        return;
    }

    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    ScriptEditor * editor = m_pEditorPreviewVector[currentTabIndex].editor;
    QString storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    bool initialized = m_pEncodeDialog->initialize(
        editor->text(), storedFilePath);
    if(initialized)
        m_pEncodeDialog->showActive();
}

// END OF void MainWindow::slotEncode()
//==============================================================================

void MultiTabMainWindow::slotEnqueueEncodeJob()
{
    int currentTabIndex = m_ui->scriptTabWidget->currentIndex();
    QString storedFilePath = m_pEditorPreviewVector[currentTabIndex].scriptName;

    if(storedFilePath.isEmpty())
        return;

    JobProperties properties;
    properties.type = JobType::EncodeScriptCLI;
    properties.scriptName = storedFilePath;

    QByteArray message = vsedit::jsonMessage(WMSG_CLI_ENCODE_JOB,
        properties.toJson());
    m_pJobServerWatcherSocket->sendMessage(message);
}

// END OF void MainWindow::slotEnqueueEncodeJob()
//==============================================================================

void MultiTabMainWindow::slotJobs()
{
    m_pJobServerWatcherSocket->sendMessage(WMSG_SHOW_WINDOW);
}

// END OF void MainWindow::slotJobs()
//==============================================================================

void MultiTabMainWindow::slotAbout()
{
    QResource aboutResource(":readme");
    QByteArray aboutData(reinterpret_cast<const char*>(aboutResource.data()),
        aboutResource.size());
    QString aboutString = QString::fromUtf8(aboutData);
    QMessageBox::about(this, "VapourSynth Editor", aboutString);
}

void MultiTabMainWindow::slotChangeWindowTitle(const QString & a_storedFilePath)
{
    QString windowTitleText = QString::fromUtf8("VapourSynth Editor - ");

    if(a_storedFilePath.isEmpty())
        windowTitleText += QString::fromUtf8("(Untitled)");
    else
        windowTitleText += a_storedFilePath;

//    if(m_ui.scriptEdit->isModified())
//        windowTitleText += "*";

    setWindowTitle(windowTitleText);
}

// END OF void MainWindow::slotChangeWindowTitle()
//==============================================================================

void MultiTabMainWindow::slotEditorTextChanged()
{
//    bool textMatchesSaved = (m_lastSavedText == m_ui.scriptEdit->text());
//    m_ui.scriptEdit->setModified(!textMatchesSaved);
//    slotChangeWindowTitle();
}

// END OF void MainWindow::slotEditorTextChanged()
//==============================================================================

void MultiTabMainWindow::slotOpenRecentScriptActionTriggered()
{
    QAction * pAction = qobject_cast<QAction *>(sender());
    if(pAction == nullptr)
        return;
    if(!safeToCloseFile())
        return;
    loadScriptFromFile(pAction->data().toString());
}

void MultiTabMainWindow::slotSettingsChanged()
{
    QKeySequence hotkey;
    for(QAction * pAction : m_settableActionsList)
    {
        hotkey = m_pSettingsManager->getHotkey(pAction->data().toString());
        pAction->setShortcut(hotkey);
    }

    m_pVapourSynthPluginsManager->slotRefill();
    VSPluginsList vsPluginsList = m_pVapourSynthPluginsManager->pluginsList();

    // update each editor with new setting
    for (EditorPreview &item : m_pEditorPreviewVector) {
        item.editor->setPluginsList(vsPluginsList);
        item.editor->slotLoadSettings();
    }

//    m_pTemplatesDialog->setPluginsList(vsPluginsList);
//    m_pTemplatesDialog->slotLoadSettings();

}

void MultiTabMainWindow::slotScriptFileDropped(const QString &a_filePath, bool *a_pHandled)
{
    *a_pHandled = true;

    if(!safeToCloseFile())
        return;

    loadScriptFromFile(a_filePath);
}

void MultiTabMainWindow::slotSaveGeometry()
{
    m_pGeometrySaveTimer->stop();
    m_pSettingsManager->setMainWindowGeometry(m_windowGeometry);

}

// END OF bool MainWindow::safeToCloseFile()
//==============================================================================

void MultiTabMainWindow::slotWriteLogMessage(int a_messageType,
    const QString & a_message)
{
    QString style = vsMessageTypeToStyleName(a_messageType);
    slotWriteLogMessage(a_message, style);
}

// END OF void MainWindow::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void MultiTabMainWindow::slotWriteLogMessage(const QString & a_message,
    const QString & a_style)
{
    m_logView->addEntry(a_message, a_style);

    QString fatalTypes[] = {LOG_STYLE_VS_FATAL, LOG_STYLE_QT_FATAL};
    if(!vsedit::contains(fatalTypes, a_style))
        return;

    QDateTime now = QDateTime::currentDateTime();
    QString timeString = now.toString("hh:mm:ss.zzz");
    QString dateString = now.toString("yyyy-MM-dd");
    QString caption = QObject::tr("VapourSynth Editor fatal error!");
    QString fullMessage = dateString + QString(" ") + timeString +
        QString("\n") + caption + QString("\n") + a_message;

    QString tempPath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if(tempPath.isEmpty())
    {
        QMessageBox::critical(nullptr, caption, fullMessage);
        return;
    }

    QString filePath = tempPath + QString("/") +
        QString("VapourSynth-Editor-crashlog-") + dateString + QString("-") +
        timeString.replace(':', '-') + QString(".html");

    bool saved = m_logView->saveHtml(filePath);
    if(!saved)
    {
        QMessageBox::critical(nullptr, caption, fullMessage);
        return;
    }

    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    QDesktopServices::openUrl(fileUrl);
}
