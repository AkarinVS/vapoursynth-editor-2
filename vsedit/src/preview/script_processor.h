#ifndef PREVIEWTAB_H
#define PREVIEWTAB_H

#include "../../vsedit/src/vapoursynth/vs_script_processor_dialog.h"
#include "../../../common-src/chrono.h"

#include <QObject>
#include <QWidget>


class ScriptProcessor : public VSScriptProcessorDialog
{
    Q_OBJECT
public:
    explicit ScriptProcessor(SettingsManager * a_pSettingsManager,
                        VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent = nullptr);

    ~ScriptProcessor() override;

    virtual void setScriptName(const QString & a_scriptName) override;

    const VSVideoInfo * vsVideoInfo();

    QPixmap framePixmap();
    const VSFrameRef *frameRef();

    int currentFrame();

    bool previewScript(const QString& a_script, const QString& a_scriptName);

    void cleanUpOnClose();

    void showFrameFromTimeLine(int a_frameNumber);

    void showFrameFromFrameIndicator(int a_frameNumber);

    bool isPlaying();

protected:

    virtual void stopAndCleanUp() override;

    bool requestShowFrame(int a_frameNumber);

    void setCurrentFrame(const VSFrameRef * a_cpOutputFrameRef,
        const VSFrameRef * a_cpPreviewFrameRef);

    QPixmap pixmapFromCompatBGR32(const VSFrameRef * a_cpFrameRef);

    const VSFrameRef * m_cpFrameRef;
    QPixmap m_framePixmap;

    bool m_playing;
    bool m_processingPlayQueue;
    hr_time_point m_lastFrameShowTime;
    double m_secondsBetweenFrames;
    QTimer * m_pPlayTimer;

    int m_frameExpected;
    int m_frameShown;
    int m_lastFrameRequestedForPlay;


protected slots:

    virtual void slotReceiveFrame(int a_frameNumber, int a_outputIndex,
        const VSFrameRef * a_cpOutputFrameRef,
        const VSFrameRef * a_cpPreviewFrameRef) override;

    virtual void slotFrameRequestDiscarded(int a_frameNumber,
        int a_outputIndex, const QString & a_reason) override;

    void slotProcessPlayQueue();

    void slotShowFrame(int a_frameNumber);

public slots:

    void slotJumpPlay(int a_frameNumber); // set frame when jump in timeline during play
    bool slotPlay(bool a_play);
    void slotSetPlaySpeed(double a_secondsPerFrames);
    void slotGotoFrame(int a_frameNumber);

signals:

    void signalSetTimeLineAndIndicator(int a_numFrame, int64_t a_fpsNum, int64_t a_fpsDen);

    void signalSetCurrentFrame();

    void signalFrameChanged(int a_frame);

    void signalRollBackFrame(int a_frame);

    void signalUpdateFrameTimeIndicators(int a_frameIndex, const QTime &a_time);

    void signalUpdateFramePropsString(const QString & a_framePropsString);

};

#endif // PREVIEWTAB_H
