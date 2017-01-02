#include "benchmark_dialog.h"

#include "../common/helpers.h"
#include "../vapoursynth/vapoursynth_script_processor.h"

#include <vapoursynth/VapourSynth.h>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

//==============================================================================

ScriptBenchmarkDialog::ScriptBenchmarkDialog(
	SettingsManager * a_pSettingsManager, VSScriptLibrary * a_pVSScriptLibrary,
	QWidget * a_pParent):
	VSScriptProcessorDialog(a_pSettingsManager, a_pVSScriptLibrary, a_pParent,
	(Qt::WindowFlags)0
		| Qt::Window
		| Qt::CustomizeWindowHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowCloseButtonHint
		)
	, m_processing(false)
	, m_framesTotal(0)
	, m_framesProcessed(0)
	, m_framesFailed(0)

#ifdef Q_OS_WIN
	, m_pWinTaskbarButton(nullptr)
	, m_pWinTaskbarProgress(nullptr)
#endif
{
	m_ui.setupUi(this);
	setWindowIcon(QIcon(":benchmark.png"));

	createStatusBar();

	m_ui.feedbackTextEdit->setName("benchmark_log");
	m_ui.feedbackTextEdit->setSettingsManager(m_pSettingsManager);
	m_ui.feedbackTextEdit->loadSettings();

	connect(m_ui.wholeVideoButton, SIGNAL(clicked()),
		this, SLOT(slotWholeVideoButtonPressed()));
	connect(m_ui.startStopBenchmarkButton, SIGNAL(clicked()),
		this, SLOT(slotStartStopBenchmarkButtonPressed()));
}

// END OF ScriptBenchmarkDialog::ScriptBenchmarkDialog(
//		SettingsManager * a_pSettingsManager,
//		VSScriptLibrary * a_pVSScriptLibrary, QWidget * a_pParent
//==============================================================================

ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
{
}

// END OF ScriptBenchmarkDialog::~ScriptBenchmarkDialog()
//==============================================================================

bool ScriptBenchmarkDialog::initialize(const QString & a_script,
	const QString & a_scriptName)
{
	bool initialized =
		VSScriptProcessorDialog::initialize(a_script, a_scriptName);
	if(!initialized)
		emit signalWriteLogMessage(mtCritical,
			m_pVapourSynthScriptProcessor->error());
	return initialized;
}

// END OF bool ScriptBenchmarkDialog::initialize(const QString & a_script,
//		const QString & a_scriptName)
//==============================================================================

void ScriptBenchmarkDialog::call()
{
	if(m_processing)
	{
		show();
		return;
	}

	if((!m_pVapourSynthScriptProcessor->isInitialized()) || m_wantToFinalize)
		return;

	assert(m_cpVideoInfo);

	m_ui.feedbackTextEdit->clear();
	setWindowTitle(trUtf8("Benchmark: %1").arg(scriptName()));
	QString text = trUtf8("Ready to benchmark script %1").arg(scriptName());
	m_ui.feedbackTextEdit->addEntry(text);
	m_ui.metricsEdit->clear();
	int lastFrame = m_cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setMaximum(lastFrame);
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setMaximum(lastFrame);
	m_ui.toFrameSpinBox->setValue(lastFrame);
	m_ui.processingProgressBar->setMaximum(m_cpVideoInfo->numFrames);
	m_ui.processingProgressBar->setValue(0);
	show();

#ifdef Q_OS_WIN
	if(!m_pWinTaskbarButton)
	{
		m_pWinTaskbarButton = new QWinTaskbarButton(this);
		m_pWinTaskbarButton->setWindow(windowHandle());
		m_pWinTaskbarProgress = m_pWinTaskbarButton->progress();
	}

	m_pWinTaskbarProgress->setVisible(false);
#endif
}

// END OF void ScriptBenchmarkDialog::call()
//==============================================================================

void ScriptBenchmarkDialog::stopAndCleanUp()
{
	stopProcessing();
	m_ui.metricsEdit->clear();
	m_ui.processingProgressBar->setValue(0);
}

// END OF void ScriptBenchmarkDialog::stopAndCleanUp()
//==============================================================================

void ScriptBenchmarkDialog::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	QString style = vsMessageTypeToStyleName(a_messageType);
	m_ui.feedbackTextEdit->addEntry(a_message, style);
}

// END OF void ScriptBenchmarkDialog::slotWriteLogMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void ScriptBenchmarkDialog::slotWholeVideoButtonPressed()
{
	assert(m_cpVideoInfo);
	int lastFrame = m_cpVideoInfo->numFrames - 1;
	m_ui.fromFrameSpinBox->setValue(0);
	m_ui.toFrameSpinBox->setValue(lastFrame);
}

// END OF void ScriptBenchmarkDialog::slotWholeVideoButtonPressed()
//==============================================================================

void ScriptBenchmarkDialog::slotStartStopBenchmarkButtonPressed()
{
	if(m_processing)
	{
		stopProcessing();
		return;
	}

	m_framesProcessed = 0;
	m_framesFailed = 0;
	int firstFrame = m_ui.fromFrameSpinBox->value();
	int lastFrame = m_ui.toFrameSpinBox->value();

	if(firstFrame > lastFrame)
	{
		m_ui.feedbackTextEdit->addEntry(trUtf8(
			"First frame number is larger than the last frame number."),
			LOG_STYLE_WARNING);
			return;
	}

	m_framesTotal = lastFrame - firstFrame + 1;
	m_ui.processingProgressBar->setMaximum(m_framesTotal);
	m_ui.startStopBenchmarkButton->setText(trUtf8("Stop"));
	setWindowTitle(trUtf8("0% Benchmark: %1").arg(scriptName()));

#ifdef Q_OS_WIN
	assert(m_pWinTaskbarProgress);
	m_pWinTaskbarProgress->setMaximum(m_framesTotal);
	m_pWinTaskbarProgress->setValue(0);
	m_pWinTaskbarProgress->resume();
	m_pWinTaskbarProgress->setVisible(true);
#endif

	m_processing = true;
	m_benchmarkStartTime = hr_clock::now();

	for(int i = firstFrame; i <= lastFrame; ++i)
		m_pVapourSynthScriptProcessor->requestFrameAsync(i);
}

// END OF void ScriptBenchmarkDialog::slotStartStopBenchmarkButtonPressed()
//==============================================================================

void ScriptBenchmarkDialog::slotReceiveFrame(int a_frameNumber,
	int a_outputIndex, const VSFrameRef * a_cpOutputFrameRef,
	const VSFrameRef * a_cpPreviewFrameRef)
{
	(void)a_frameNumber;
	(void)a_outputIndex;
	(void)a_cpOutputFrameRef;
	(void)a_cpPreviewFrameRef;

	if(!m_processing)
		return;

	m_framesProcessed++;
	updateMetrics();
}

// END OF void ScriptBenchmarkDialog::slotReceiveFrame(int a_frameNumber,
//		int a_outputIndex, const VSFrameRef * a_cpOutputFrameRef,
//		const VSFrameRef * a_cpPreviewFrameRef)
//==============================================================================

void ScriptBenchmarkDialog::slotFrameRequestDiscarded(int a_frameNumber,
	int a_outputIndex, const QString & a_reason)
{
	(void)a_frameNumber;
	(void)a_outputIndex;
	(void)a_reason;

	if(!m_processing)
		return;

	m_framesProcessed++;
	m_framesFailed++;
	updateMetrics();
}

// END OF void ScriptBenchmarkDialog::slotFrameRequestDiscarded(
//		int a_frameNumber, int a_outputIndex, const QString & a_reason)
//==============================================================================

void ScriptBenchmarkDialog::stopProcessing()
{
	if(!m_processing)
		return;

	m_processing = false;
	m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
	m_ui.startStopBenchmarkButton->setText(trUtf8("Start"));

#ifdef Q_OS_WIN
	assert(m_pWinTaskbarProgress);
	if(m_framesProcessed == m_framesTotal)
		m_pWinTaskbarProgress->setVisible(false);
	else
		m_pWinTaskbarProgress->stop();
#endif
}

// END OF void ScriptBenchmarkDialog::stopProcessing()
//==============================================================================

void ScriptBenchmarkDialog::updateMetrics()
{
	hr_time_point now = hr_clock::now();
	m_ui.processingProgressBar->setValue(m_framesProcessed);
	double passed = duration_to_double(now - m_benchmarkStartTime);
	QString passedString = vsedit::timeToString(passed);
	double fps = (double)m_framesProcessed / passed;
	QString text = trUtf8("Time elapsed: %1 - %2 FPS")
		.arg(passedString).arg(QString::number(fps, 'f', 20));

	if(m_framesFailed > 0)
		text += trUtf8("; %1 frames failed").arg(m_framesFailed);

	if(m_framesProcessed < m_framesTotal)
	{
		double estimated = (m_framesTotal - m_framesProcessed) / fps;
		QString estimatedString = vsedit::timeToString(estimated);
		text += trUtf8("; estimated time to finish: %1").arg(estimatedString);
	}

	m_ui.metricsEdit->setText(text);

	int percentage = (int)((double)m_framesProcessed * 100.0 /
		(double)m_framesTotal);
	setWindowTitle(trUtf8("%1% Benchmark: %2")
		.arg(percentage).arg(scriptName()));

#ifdef Q_OS_WIN
	assert(m_pWinTaskbarProgress);
	m_pWinTaskbarProgress->setValue(m_framesProcessed);
#endif

	if(m_framesProcessed == m_framesTotal)
		stopProcessing();
}

// END OF void ScriptBenchmarkDialog::updateMetrics()
//==============================================================================
