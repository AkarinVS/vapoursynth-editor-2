#include "script_processor.h"
#include "../../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "math.h"

#include <vapoursynth/VapourSynth.h>

#include <QTimer>

ScriptProcessor::ScriptProcessor(SettingsManager * a_pSettingsManager,
                                 VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent) :
    VSScriptProcessorDialog(a_pSettingsManager, a_pVSScriptLibrary, a_pParent)

//  , m_pAdvancedSettingsDialog(nullptr)
  , m_frameExpected(0)
  , m_frameShown(-1)
  , m_lastFrameRequestedForPlay(-1)
//  , m_bigFrameStep(10)
  , m_cpFrameRef(nullptr)
//  , m_changingCropValues(false)
//  , m_pPreviewContextMenu(nullptr)
//  , m_pActionFrameToClipboard(nullptr)
//  , m_pActionSaveSnapshot(nullptr)
//  , m_pActionToggleZoomPanel(nullptr)
//  , m_pMenuZoomModes(nullptr)
//  , m_pActionGroupZoomModes(nullptr)
//  , m_pActionSetZoomModeNoZoom(nullptr)
//  , m_pActionSetZoomModeFixedRatio(nullptr)
//  , m_pActionSetZoomModeFitToFrame(nullptr)
//  , m_pMenuZoomScaleModes(nullptr)
//  , m_pActionGroupZoomScaleModes(nullptr)
//  , m_pActionSetZoomScaleModeNearest(nullptr)
//  , m_pActionSetZoomScaleModeBilinear(nullptr)
//  , m_pActionToggleCropPanel(nullptr)
//  , m_pActionToggleTimeLinePanel(nullptr)
//  , m_pMenuTimeLineModes(nullptr)
//  , m_pActionGroupTimeLineModes(nullptr)
//  , m_pActionSetTimeLineModeTime(nullptr)
//  , m_pActionSetTimeLineModeFrames(nullptr)
//  , m_pActionTimeStepForward(nullptr)
//  , m_pActionTimeStepBack(nullptr)
//  , m_pActionPasteCropSnippetIntoScript(nullptr)
//  , m_pActionAdvancedSettingsDialog(nullptr)
//  , m_pActionToggleColorPicker(nullptr)
//  , m_pActionPlay(nullptr)
//  , m_pActionLoadChapters(nullptr)
//  , m_pActionClearBookmarks(nullptr)
//  , m_pActionBookmarkCurrentFrame(nullptr)
//  , m_pActionUnbookmarkCurrentFrame(nullptr)
//  , m_pActionGoToPreviousBookmark(nullptr)
//  , m_pActionGoToNextBookmark(nullptr)
//  , m_pActionPasteShownFrameNumberIntoScript(nullptr)
  , m_playing(false)
  , m_processingPlayQueue(false)
  , m_secondsBetweenFrames(0)
  , m_pPlayTimer(nullptr)
//  , m_alwaysKeepCurrentFrame(DEFAULT_ALWAYS_KEEP_CURRENT_FRAME)
//  , m_pGeometrySaveTimer(nullptr)
{
    m_pPlayTimer = new QTimer(this);
    m_pPlayTimer->setTimerType(Qt::PreciseTimer);
    m_pPlayTimer->setSingleShot(true);   

    connect(m_pPlayTimer, SIGNAL(timeout()),
        this, SLOT(slotProcessPlayQueue()));
}

void ScriptProcessor::setScriptName(const QString &a_scriptName)
{
    VSScriptProcessorDialog::setScriptName(a_scriptName);
}

const VSVideoInfo * ScriptProcessor::vsVideoInfo()
{
    const VSVideoInfo * cpVideoInfo = m_cpVideoInfo;
    return cpVideoInfo;
}

void ScriptProcessor::setCurrentFrame(const VSFrameRef *a_cpOutputFrameRef, const VSFrameRef *a_cpPreviewFrameRef)
{
    Q_ASSERT(m_cpVSAPI);
    m_cpVSAPI->freeFrame(m_cpFrameRef);
    m_cpFrameRef = a_cpOutputFrameRef;
    m_framePixmap = pixmapFromCompatBGR32(a_cpPreviewFrameRef);
    m_cpVSAPI->freeFrame(a_cpPreviewFrameRef);

    emit signalSetCurrentFrame(); // receiver can grab framePixmap
//	m_ui.previewArea->checkMouseOverPreview(QCursor::pos());

}

QPixmap ScriptProcessor::framePixmap()
{
    return m_framePixmap;
}

const VSFrameRef *ScriptProcessor::frameRef()
{
    return m_cpFrameRef;
}

int ScriptProcessor::currentFrame()
{
    return m_frameShown;
}

bool ScriptProcessor::previewScript(const QString &a_script, const QString &a_scriptName)
{
    QString previousScript = script();
    QString previousScriptName = scriptName();

    stopAndCleanUp();

    bool initialized = initialize(a_script, a_scriptName);
    if(!initialized)
        return false;

//    setTitle();

    int lastFrameNumber = m_cpVideoInfo->numFrames - 1;

    // emit signal to setup timeline and range of spinbox
    emit signalSetTimeLineAndIndicator(lastFrameNumber, m_cpVideoInfo->fpsNum, m_cpVideoInfo->fpsDen);

    bool scriptChanged = ((previousScript != a_script) &&
        (previousScriptName != a_scriptName));

//    if(scriptChanged && (!m_alwaysKeepCurrentFrame))
//	{
//		m_frameExpected = 0;
//		m_ui.previewArea->setPixmap(QPixmap());
//	}

    if(m_frameExpected > lastFrameNumber)
        m_frameExpected = lastFrameNumber;

//    resetCropSpinBoxes();

//	slotSetPlayFPSLimit();

    setScriptName(a_scriptName);

//    setupBookmarkManager();


//	loadTimelineBookmarks();

//    if(m_pSettingsManager->getPreviewDialogMaximized())
//		showMaximized();
//	else
//		showNormal();

    slotShowFrame(m_frameExpected);
    return true;
}

void ScriptProcessor::cleanUpOnClose()
{
    stopAndCleanUp();
    m_pVapourSynthScriptProcessor->finalize();
}

void ScriptProcessor::showFrameFromTimeLine(int a_frameNumber)
{
    slotShowFrame(a_frameNumber);
}

void ScriptProcessor::showFrameFromFrameIndicator(int a_frameNumber)
{

}

void ScriptProcessor::stopAndCleanUp()
{
    slotPlay(false);

//	if(m_ui.cropCheckButton->isChecked())
//		m_ui.cropCheckButton->click();

//	bool rememberLastPreviewFrame =
//		m_pSettingsManager->getRememberLastPreviewFrame();
//	if(rememberLastPreviewFrame && (!scriptName().isEmpty()) &&
//		(m_frameShown > -1))
//		m_pSettingsManager->setLastPreviewFrame(m_frameShown);

    m_frameShown = -1;
    m_framePixmap = QPixmap();
    // Replace shown image with a blank one of the same dimension:
    // -helps to keep the scrolling position when refreshing the script;
    // -leaves the image blank on sudden error;
    // -creates a blinking effect indicating the script is being refreshed.
//	const QPixmap * pPreviewPixmap = m_ui.previewArea->pixmap();
//	int pixmapWidth = pPreviewPixmap->width();
//	int pixmapHeight = pPreviewPixmap->height();
//	QPixmap blackPixmap(pixmapWidth, pixmapHeight);
//	blackPixmap.fill(Qt::black);
//	m_ui.previewArea->setPixmap(blackPixmap);

    if(m_cpFrameRef)
    {
        Q_ASSERT(m_cpVSAPI);
        m_cpVSAPI->freeFrame(m_cpFrameRef);
        m_cpFrameRef = nullptr;
    }

    VSScriptProcessorDialog::stopAndCleanUp();

}

bool ScriptProcessor::requestShowFrame(int a_frameNumber)
{
    if(!m_pVapourSynthScriptProcessor->isInitialized())
        return false;

    if((m_frameShown != -1) && (m_frameShown != m_frameExpected))
        return false;

    m_pVapourSynthScriptProcessor->requestFrameAsync(a_frameNumber, 0, true);
    return true;
}

QPixmap ScriptProcessor::pixmapFromCompatBGR32(const VSFrameRef *a_cpFrameRef)
{
    if((!m_cpVSAPI) || (!a_cpFrameRef))
        return QPixmap();

    const VSFormat * cpFormat = m_cpVSAPI->getFrameFormat(a_cpFrameRef);
    Q_ASSERT(cpFormat);

    if(cpFormat->id != pfCompatBGR32)
    {
        QString errorString = tr("Error forming pixmap from frame. "
            "Expected format CompatBGR32. Instead got \'%1\'.")
            .arg(cpFormat->name);
        emit signalWriteLogMessage(mtCritical, errorString);
        return QPixmap();
    }

    int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
    int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
    const void * pData = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
    int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);
    QImage frameImage((const uchar *)pData, width, height,
        stride, QImage::Format_RGB32);
    QImage flippedImage = frameImage.mirrored();
    QPixmap framePixmap = QPixmap::fromImage(flippedImage);
    return framePixmap;

}

void ScriptProcessor::slotReceiveFrame(int a_frameNumber, int a_outputIndex, const VSFrameRef *a_cpOutputFrameRef, const VSFrameRef *a_cpPreviewFrameRef)
{
    if(!a_cpOutputFrameRef)
        return;

    Q_ASSERT(m_cpVSAPI);
    const VSFrameRef * cpOutputFrameRef =
        m_cpVSAPI->cloneFrameRef(a_cpOutputFrameRef);
    const VSFrameRef * cpPreviewFrameRef =
        m_cpVSAPI->cloneFrameRef(a_cpPreviewFrameRef);

    if(m_playing)
    {
        Frame newFrame(a_frameNumber, a_outputIndex,
            cpOutputFrameRef, cpPreviewFrameRef);
        m_framesCache.push_back(newFrame);
        slotProcessPlayQueue();
    }
    else
    {
        setCurrentFrame(cpOutputFrameRef, cpPreviewFrameRef);
        m_frameShown = a_frameNumber;
        if(m_frameShown == m_frameExpected) {
//            m_ui.frameStatusLabel->setPixmap(m_readyPixmap);
        }
    }

}

//void PreviewTab::slotFrameRequestDiscarded(int a_frameNumber, int a_outputIndex, const QString &a_reason)
//{
//    (void)a_outputIndex;
//    (void)a_reason;

//    if(m_playing)
//    {
//        slotPlay(false);
//    }
//    else
//    {
//        if(a_frameNumber != m_frameExpected)
//            return;

//        if(m_frameShown == -1)
//        {
//            if(m_frameExpected == 0)
//            {
//                // Nowhere to roll back
////				m_ui.frameNumberSlider->setFrame(0);
//                m_ui.timeLineView->setFrame(0);
//                m_ui.frameNumberSpinBox->setValue(0);
//                m_ui.frameStatusLabel->setPixmap(m_errorPixmap);
//            }
//            else
//                slotShowFrame(0);
//            return;
//        }

//        m_frameExpected = m_frameShown;
//        m_ui.frameNumberSlider->setFrame(m_frameShown);
//        m_ui.timeLineView->setFrame(m_frameShown);
//        m_ui.frameNumberSpinBox->setValue(m_frameShown);
//        m_ui.frameStatusLabel->setPixmap(m_readyPixmap);
//    }

//}


void ScriptProcessor::slotFrameRequestDiscarded(int a_frameNumber, int a_outputIndex, const QString &a_reason)
{

}

void ScriptProcessor::slotProcessPlayQueue()
{
    if(!m_playing)
        return;

    if (m_frameShown == m_cpVideoInfo->numFrames - 1) {
        slotPlay(false); // stop playing at the end
        return;
    }

    if(m_processingPlayQueue)
        return;

    m_processingPlayQueue = true;

    int nextFrame = (m_frameShown + 1) % m_cpVideoInfo->numFrames;
    Frame referenceFrame(nextFrame, 0, nullptr);

    while(!m_framesCache.empty())
    {
        std::list<Frame>::const_iterator it =
            std::find(m_framesCache.begin(), m_framesCache.end(),
            referenceFrame);

        if(it == m_framesCache.end())
            break;

        hr_time_point now = hr_clock::now();
        double passed = duration_to_double(now - m_lastFrameShowTime);
        double secondsToNextFrame = m_secondsBetweenFrames - passed;

        if(secondsToNextFrame > 0)
        {
            int millisecondsToNextFrame = std::ceil(secondsToNextFrame * 1000);
            m_pPlayTimer->start(millisecondsToNextFrame);
            break;
        }

        setCurrentFrame(it->cpOutputFrameRef, it->cpPreviewFrameRef); // set pix and send to previewarea

        m_lastFrameShowTime = hr_clock::now();

        m_frameShown = nextFrame;
        m_frameExpected = m_frameShown;

        emit signalFrameChanged(m_frameExpected); // signal to change timeline slider position

        m_framesCache.erase(it);
        nextFrame = (m_frameShown + 1) % m_cpVideoInfo->numFrames;
        referenceFrame.number = nextFrame;
    }

    nextFrame = (m_lastFrameRequestedForPlay + 1) %
        m_cpVideoInfo->numFrames;

    while(((m_framesInQueue + m_framesInProcess) < m_maxThreads) &&
        (m_framesCache.size() <= m_cachedFramesLimit))
    {
        m_pVapourSynthScriptProcessor->requestFrameAsync(nextFrame, 0, true);
        m_lastFrameRequestedForPlay = nextFrame;
        nextFrame = (nextFrame + 1) % m_cpVideoInfo->numFrames;
    }

    m_processingPlayQueue = false;

}

bool ScriptProcessor::slotPlay(bool a_play)
{
    if(m_playing == a_play)
        return true;

    m_playing = a_play;
//    m_pActionPlay->setChecked(m_playing);

    if(m_playing)
    {
        m_lastFrameRequestedForPlay = m_frameShown;
        slotProcessPlayQueue();
    }
    else
    {
        clearFramesCache();
        m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
    }

    return m_playing;
}

void ScriptProcessor::slotSetPlaySpeed(double a_secondsPerFrames)
{
//    if (m_secondsBetweenFrames == a_secondsPerFrames) return;
    m_secondsBetweenFrames = a_secondsPerFrames;
}

void ScriptProcessor::slotShowFrame(int a_frameNumber)
{
    if(m_playing)
        return;

    if((m_frameShown == a_frameNumber) && (!m_framePixmap.isNull()))
        return;

    if ((a_frameNumber > m_cpVideoInfo->numFrames) || (a_frameNumber < 0))
        return;

    static bool requestingFrame = false;
    if(requestingFrame)
        return;
    requestingFrame = true;


    bool requested = requestShowFrame(a_frameNumber); // request to output frame
    if(requested)
    {
        m_frameExpected = a_frameNumber;
//		m_ui.frameStatusLabel->setPixmap(m_busyPixmap);
    }
    // if requested frame failed, send last success requested frame

    /* sends signal to update frame/time indicators */
//    double fps = double(m_cpVideoInfo->fpsNum) / double(m_cpVideoInfo->fpsDen);
//    int milliSeconds = int((double(m_frameExpected) / fps) * 1000);
//    QTime time = QTime::fromMSecsSinceStartOfDay(milliSeconds);

//    emit signalUpdateFrameTimeIndicators(m_frameExpected, time);

    requestingFrame = false;
}

void ScriptProcessor::slotJumpPlay(int a_frameNumber)
{
    if (!m_playing)
        return;

    m_frameShown = a_frameNumber;
    slotPlay(false);
    slotPlay(true);
}
