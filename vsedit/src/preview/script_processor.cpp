#include "script_processor.h"
#include "../../../common-src/vapoursynth/vapoursynth_script_processor.h"
#include "math.h"

#include <vapoursynth/VapourSynth.h>

#include <QTimer>

ScriptProcessor::ScriptProcessor(SettingsManager * a_pSettingsManager,
                                 VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent) :
    VSScriptProcessorDialog(a_pSettingsManager, a_pVSScriptLibrary, a_pParent)
  , m_frameExpected(0)
  , m_frameShown(-1)
  , m_lastFrameRequestedForPlay(-1)
  , m_cpFrameRef(nullptr)
  , m_playing(false)
  , m_processingPlayQueue(false)
  , m_secondsBetweenFrames(0)
  , m_pPlayTimer(nullptr)
//  , m_alwaysKeepCurrentFrame(DEFAULT_ALWAYS_KEEP_CURRENT_FRAME)
{
    m_pPlayTimer = new QTimer(this);
    m_pPlayTimer->setTimerType(Qt::PreciseTimer);
    m_pPlayTimer->setSingleShot(true);   

    connect(m_pPlayTimer, SIGNAL(timeout()),
            this, SLOT(slotProcessPlayQueue()));
}

ScriptProcessor::~ScriptProcessor()
{    
    stopAndCleanUp();
}

void ScriptProcessor::setScriptName(const QString &a_scriptName)
{
    VSScriptProcessorDialog::setScriptName(a_scriptName);
}

const VSVideoInfo * ScriptProcessor::vsVideoInfo()
{
    return m_cpVideoInfo;
}

void ScriptProcessor::setCurrentFrame(const VSFrameRef *a_cpOutputFrameRef, const VSFrameRef *a_cpPreviewFrameRef)
{
    Q_ASSERT(m_cpVSAPI);
    m_cpVSAPI->freeFrame(m_cpFrameRef); // free frame from last reference frame
    m_cpFrameRef = a_cpOutputFrameRef;
    QPixmap framePixmap = pixmapFromCompatBGR32(a_cpPreviewFrameRef);

    QString framePropsString = m_pVapourSynthScriptProcessor->framePropsString(m_cpFrameRef);

    emit signalUpdateFramePropsString(framePropsString); // send frame properties string
    emit signalSetCurrentFrame(framePixmap); // send framePixmap to preview area

    m_cpVSAPI->freeFrame(a_cpPreviewFrameRef);
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

    int lastFrameNumber = m_cpVideoInfo->numFrames - 1;

    // emit signal to setup timeline and range of spinbox
    emit signalSetTimeLineAndIndicator(lastFrameNumber, m_cpVideoInfo->fpsNum, m_cpVideoInfo->fpsDen);

//    bool scriptChanged = ((previousScript != a_script) &&
//        (previousScriptName != a_scriptName));

//    if(scriptChanged && (!m_alwaysKeepCurrentFrame))
//	{
//		m_frameExpected = 0;
//		m_ui.previewArea->setPixmap(QPixmap());
//	}

    if(m_frameExpected > lastFrameNumber)
        m_frameExpected = lastFrameNumber;

    setScriptName(a_scriptName);

//    if(m_pSettingsManager->getPreviewDialogMaximized())
//		showMaximized();
//	else
//		showNormal();

    slotShowFrame(m_frameExpected);
    return true;
}

void ScriptProcessor::showFrameFromTimeLine(int a_frameNumber)
{
    slotShowFrame(a_frameNumber);
}

void ScriptProcessor::showFrameFromFrameIndicator(int a_frameNumber)
{

}

bool ScriptProcessor::isPlaying()
{
    return m_playing;
}

void ScriptProcessor::stopAndCleanUp()
{
    slotPlay(false);

    m_frameShown = -1;

    if(m_cpFrameRef)
    {
        Q_ASSERT(m_cpVSAPI);
        m_cpVSAPI->freeFrame(m_cpFrameRef);
        m_cpFrameRef = nullptr;
    }

    VSScriptProcessorDialog::stopAndCleanUp();
}

bool ScriptProcessor::requestFrame(int a_frameNumber)
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
    QImage frameImage(static_cast<const uchar *>(pData), width, height,
        stride, QImage::Format_RGB32);
    QImage flippedImage = frameImage.mirrored();
    QPixmap framePixmap = QPixmap::fromImage(flippedImage);
    return framePixmap;
}

void ScriptProcessor::slotReceiveFrame(int a_frameNumber, int a_outputIndex,
    const VSFrameRef * a_cpOutputFrameRef, const VSFrameRef * a_cpPreviewFrameRef)
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

void ScriptProcessor::slotFrameRequestDiscarded(int a_frameNumber, int a_outputIndex, const QString &a_reason)
{
    (void)a_outputIndex;
    (void)a_reason;

    if(m_playing)
    {
        slotPlay(false);
    }
    else
    {
        if(a_frameNumber != m_frameExpected)
            return;

        if(m_frameShown == -1)
        {
            if(m_frameExpected == 0)
            {
                // Nowhere to roll back
                emit signalRollBackFrame(0);
//                m_ui.frameStatusLabel->setPixmap(m_errorPixmap);
            }
            else
                slotShowFrame(0);
            return;
        }

        m_frameExpected = m_frameShown;
        emit signalRollBackFrame(m_frameShown);
//        m_ui.frameStatusLabel->setPixmap(m_readyPixmap);
    }
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
        //  find frame in frame cache by frame number
        auto it = std::find(m_framesCache.begin(), m_framesCache.end(),
                            referenceFrame);

        if(it == m_framesCache.end())
            break;

        hr_time_point now = hr_clock::now();
        double passed = duration_to_double(now - m_lastFrameShowTime);
        double secondsToNextFrame = m_secondsBetweenFrames - passed;

        if(secondsToNextFrame > 0)
        {
            int millisecondsToNextFrame = int(std::ceil(secondsToNextFrame * 1000));
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

    /* request next frame to que */
    /* capping request queue for frames, bigger number will eat up more memory */
    while(((m_framesInQueue + m_framesInProcess) < 2) &&
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

    if(m_playing)
    {
        m_lastFrameRequestedForPlay = m_frameShown;
        slotProcessPlayQueue();
    }
    else
    {
        m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
        clearFramesCache();
    }

    return m_playing;
}

void ScriptProcessor::slotSetPlaySpeed(double a_secondsPerFrames)
{
//    if (m_secondsBetweenFrames == a_secondsPerFrames) return;
    m_secondsBetweenFrames = a_secondsPerFrames;
}

void ScriptProcessor::slotGotoFrame(int a_frameNumber)
{
    if ((a_frameNumber > m_cpVideoInfo->numFrames) || (a_frameNumber < 0))
        m_frameShown = 0;

    m_frameShown = a_frameNumber;
}

void ScriptProcessor::slotResetSettings()
{
    m_pVapourSynthScriptProcessor->slotResetSettings();
}

void ScriptProcessor::slotShowFrame(int a_frameNumber)
{
    if(m_playing)
        return;

    if(m_frameShown == a_frameNumber)
        return;

    if ((a_frameNumber > m_cpVideoInfo->numFrames) || (a_frameNumber < 0))
        return;

    static bool requestingFrame = false;
    if(requestingFrame)
        return;
    requestingFrame = true;

    bool requested = requestFrame(a_frameNumber); // request to output frame
    if(requested)
    {
        m_frameExpected = a_frameNumber;
//		m_ui.frameStatusLabel->setPixmap(m_busyPixmap);
    }

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
