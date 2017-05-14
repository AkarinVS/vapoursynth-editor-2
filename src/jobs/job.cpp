#include "job.h"

#include "../common/helpers.h"
#include "../settings/settings_manager.h"
#include "../vapoursynth/vs_script_library.h"
#include "../vapoursynth/vapoursynth_script_processor.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer_null.h"
#include "../frame_consumers/frame_header_writers/frame_header_writer_y4m.h"

#include <QFileInfo>
#include <QFile>
#include <cassert>
#include <algorithm>
#include <vapoursynth/VSHelper.h>

#ifdef Q_OS_WIN
	#include <windows.h>
#endif

//==============================================================================

vsedit::Job::Job(const JobProperties & a_properties,
	SettingsManager * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary,
	QObject * a_pParent) :
	  QObject(a_pParent)
	, m_properties(a_properties)
	, m_lastFrameProcessed(-1)
	, m_lastFrameRequested(-1)
	, m_encodingState(EncodingState::Idle)
	, m_bytesToWrite(0u)
	, m_bytesWritten(0u)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
	, m_pVapourSynthScriptProcessor(nullptr)
	, m_cpVSAPI(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_pFrameHeaderWriter(nullptr)
	, m_cachedFramesLimit(100)
	, m_framesInQueue(0)
	, m_framesInProcess(0)
	, m_maxThreads(0)
	, m_memorizedEncodingTime(0.0)
{
	fillVariables();
	if(a_pVSScriptLibrary)
		m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();

	connect(&m_process, SIGNAL(started()),
		this, SLOT(slotProcessStarted()));
	connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
		this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
	connect(&m_process, SIGNAL(error(QProcess::ProcessError)),
		this, SLOT(slotProcessError(QProcess::ProcessError)));
	connect(&m_process, SIGNAL(readChannelFinished()),
		this, SLOT(slotProcessReadChannelFinished()));
	connect(&m_process, SIGNAL(bytesWritten(qint64)),
		this, SLOT(slotProcessBytesWritten(qint64)));
	connect(&m_process, SIGNAL(readyReadStandardError()),
		this, SLOT(slotProcessReadyReadStandardError()));
}

// END OF
//==============================================================================

vsedit::Job::~Job()
{
}

// END OF
//==============================================================================

QString vsedit::Job::typeName(JobType a_type)
{
	static std::map<JobType, QString> typeNameMap =
	{
		{JobType::EncodeScriptCLI, trUtf8("CLI encoding")},
		{JobType::RunProcess, trUtf8("Process run")},
		{JobType::RunShellCommand, trUtf8("Shell command")},
	};

	return typeNameMap[a_type];
}

// END OF
//==============================================================================

QString vsedit::Job::stateName(JobState a_state)
{
	static std::map<JobState, QString> stateNameMap =
	{
		{JobState::Waiting, trUtf8("Waiting")},
		{JobState::Running, trUtf8("Running")},
		{JobState::Paused, trUtf8("Paused")},
		{JobState::Pausing, trUtf8("Pausing")},
		{JobState::Aborted, trUtf8("Aborted")},
		{JobState::Aborting, trUtf8("Aborting")},
		{JobState::FailedCleanUp, trUtf8("Failed. Cleaning up.")},
		{JobState::Failed, trUtf8("Failed")},
		{JobState::DependencyNotMet, trUtf8("Dependency not met")},
		{JobState::CompletedCleanUp, trUtf8("Completing")},
		{JobState::Completed, trUtf8("Completed")},
	};

	return stateNameMap[a_state];
}

// END OF
//==============================================================================

void vsedit::Job::start()
{
	if(m_properties.jobState == JobState::Waiting)
	{
		m_properties.timeStarted = QDateTime::currentDateTimeUtc();
		changeStateAndNotify(JobState::Running);
		if(m_properties.type == JobType::EncodeScriptCLI)
			startEncodeScriptCLI();
		else if(m_properties.type == JobType::RunProcess)
			startRunProcess();
		else if(m_properties.type == JobType::RunShellCommand)
			startRunShellCommand();
	}
	else if (m_properties.jobState == JobState::Paused)
	{
		changeStateAndNotify(JobState::Running);
		if(m_properties.type == JobType::EncodeScriptCLI)
			processFramesQueue();
		else if(m_properties.type == JobType::RunProcess)
		{
#ifdef Q_OS_WIN
			BOOL result = DebugActiveProcessStop((DWORD)m_process.processId());
			if(result)
				changeStateAndNotify(JobState::Running);
			else
				emit signalLogMessage(trUtf8("Failed to resume process. "
					"Error %1.") .arg(GetLastError()), LOG_STYLE_ERROR);
#endif
		}
	}
}

// END OF
//==============================================================================

void vsedit::Job::pause()
{
	if(!vsedit::contains(ACTIVE_JOB_STATES, m_properties.jobState))
		return;

	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		EncodingState invalidStates[] = {EncodingState::Idle,
			EncodingState::EncoderCrashed, EncodingState::Finishing,
			EncodingState::Aborting};
		if(vsedit::contains(invalidStates, m_encodingState))
			return;
		changeStateAndNotify(JobState::Pausing);
	}
	else if(m_properties.type == JobType::RunProcess)
	{
#ifdef Q_OS_WIN
		BOOL result = DebugActiveProcess((DWORD)m_process.processId());
		if(result)
			changeStateAndNotify(JobState::Paused);
		else
			emit signalLogMessage(trUtf8("Failed to pause process. Error %1.")
				.arg(GetLastError()), LOG_STYLE_ERROR);
#endif
	}
}

// END OF
//==============================================================================

void vsedit::Job::abort()
{
	changeStateAndNotify(JobState::Aborting);
	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		m_encodingState = EncodingState::Aborting;
		cleanUpEncoding();
		return;
	}
	else if(m_properties.type == JobType::RunProcess)
	{
		if(m_process.state() == QProcess::Running)
		{
			m_process.kill();
			m_process.waitForFinished(-1);
		}
	}

	changeStateAndNotify(JobState::Aborted);
}

// END OF
//==============================================================================

QUuid vsedit::Job::id() const
{
	return m_properties.id;
}

// END OF
//==============================================================================

bool vsedit::Job::setId(const QUuid & a_id)
{
	m_properties.id = a_id;
	return true;
}

// END OF
//==============================================================================

JobType vsedit::Job::type() const
{
	return m_properties.type;
}

// END OF
//==============================================================================

bool vsedit::Job::setType(JobType a_type)
{
	m_properties.type = a_type;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::scriptName() const
{
	return m_properties.scriptName;
}

// END OF
//==============================================================================

bool vsedit::Job::setScriptName(const QString & a_scriptName)
{
	m_properties.scriptName = a_scriptName;
	return true;
}

// END OF
//==============================================================================

EncodingHeaderType vsedit::Job::encodingHeaderType() const
{
	return m_properties.encodingHeaderType;
}

// END OF
//==============================================================================

bool vsedit::Job::setEncodingHeaderType(EncodingHeaderType a_headerType)
{
	m_properties.encodingHeaderType = a_headerType;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::executablePath() const
{
	return m_properties.executablePath;
}

// END OF
//==============================================================================

bool vsedit::Job::setExecutablePath(const QString & a_path)
{
	m_properties.executablePath = a_path;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::arguments() const
{
	return m_properties.arguments;
}

// END OF
//==============================================================================

bool vsedit::Job::setArguments(const QString & a_arguments)
{
	m_properties.arguments = a_arguments;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::shellCommand() const
{
	return m_properties.shellCommand;
}

// END OF
//==============================================================================

bool vsedit::Job::setShellCommand(const QString & a_command)
{
	m_properties.shellCommand = a_command;
	return true;
}

// END OF
//==============================================================================

JobState vsedit::Job::state() const
{
	return m_properties.jobState;
}

// END OF
//==============================================================================

bool vsedit::Job::setState(JobState a_state)
{
	changeStateAndNotify(a_state);
	return true;
}

// END OF
//==============================================================================

std::vector<QUuid> vsedit::Job::dependsOnJobIds() const
{
	return m_properties.dependsOnJobIds;
}

// END OF
//==============================================================================

bool vsedit::Job::setDependsOnJobIds(const std::vector<QUuid> & a_ids)
{
	m_properties.dependsOnJobIds = a_ids;
	return true;
}

// END OF
//==============================================================================

QString vsedit::Job::subject() const
{
	QString subjectString;

	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		subjectString = QString("%sn%:\n\"%ep%\" %arg%");
		subjectString = subjectString.replace("%sn%",
			resolvePathFromApplication(m_properties.scriptName));
		subjectString = subjectString.replace("%ep%",
			resolvePathFromApplication(m_properties.executablePath));
		subjectString = subjectString.replace("%arg%",
			decodeArguments(m_properties.arguments));
	}
	else if(m_properties.type == JobType::RunProcess)
	{
		subjectString = QString("\"%ep%\" %arg%");
		subjectString = subjectString.replace("%ep%",
			resolvePathFromApplication(m_properties.executablePath));
		subjectString = subjectString.replace("%arg%",
			m_properties.arguments.simplified());
	}
	else if(m_properties.type == JobType::RunShellCommand)
		subjectString = m_properties.shellCommand.simplified();

	return subjectString;
}

// END OF
//==============================================================================

std::vector<vsedit::VariableToken> vsedit::Job::variables() const
{
	std::vector<vsedit::VariableToken> cutVariables;
	for(const vsedit::VariableToken & variable : m_variables)
	{
		vsedit::VariableToken cutVariable =
			{variable.token, variable.description, std::function<QString()>()};
		cutVariables.push_back(cutVariable);
	}
	return cutVariables;
}

// END OF
//==============================================================================

int vsedit::Job::framesProcessed() const
{
	if(m_properties.type == JobType::EncodeScriptCLI)
		return m_properties.framesProcessed;
	return 0;
}

// END OF
//==============================================================================

int vsedit::Job::framesTotal() const
{
	if(m_properties.type == JobType::EncodeScriptCLI)
		return (m_properties.lastFrameReal - m_properties.firstFrameReal + 1);
	return 0;
}

// END OF
//==============================================================================

double vsedit::Job::fps() const
{
	return m_properties.fps;
}

// END OF
//==============================================================================

double vsedit::Job::secondsToFinish() const
{
	int framesLeft = framesTotal() - framesProcessed();
	double seconds = (double)framesLeft / m_properties.fps;
	return seconds;
}

// END OF
//==============================================================================

size_t vsedit::Job::framesInQueue() const
{
	return m_framesInQueue;
}

// END OF
//==============================================================================

size_t vsedit::Job::framesInProcess() const
{
	return m_framesInProcess;
}

// END OF
//==============================================================================

size_t vsedit::Job::maxThreads() const
{
	return m_maxThreads;
}

// END OF
//==============================================================================

int64_t vsedit::Job::coreFramebufferBytes() const
{
	return m_coreFramebufferBytes;
}

// END OF
//==============================================================================

JobProperties vsedit::Job::properties() const
{
	return m_properties;
}

// END OF
//==============================================================================

bool vsedit::Job::setProperties(const JobProperties & a_properties)
{
	// TODO: sanity checks
	m_properties = a_properties;
	return true;
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessStarted()
{
	if(m_encodingState == EncodingState::CheckingEncoderSanity)
		return;

	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		emit signalLogMessage(trUtf8("Encoder started. Beginning encoding."));

		if(!m_process.isWritable())
		{
			m_encodingState = EncodingState::Aborting;
			m_properties.jobState = JobState::Aborting;
			emit signalLogMessage(trUtf8("Can not write to encoder. Aborting."),
				LOG_STYLE_ERROR);
			cleanUpEncoding();
			return;
		}

		assert(m_pFrameHeaderWriter);
		if(m_pFrameHeaderWriter->needVideoHeader())
		{
			QByteArray videoHeader =
				m_pFrameHeaderWriter->videoHeader(framesTotal());

			if(m_properties.encodingHeaderType == EncodingHeaderType::Y4M)
				emit signalLogMessage(trUtf8("Y4M header: ") +
					QString::fromLatin1(videoHeader), LOG_STYLE_DEBUG);

			m_bytesToWrite = videoHeader.size();
			if(m_bytesToWrite > 0)
			{
				m_bytesWritten = 0;
				m_encodingState = EncodingState::WritingHeader;
				qint64 bytesWritten = m_process.write(videoHeader);
				if(bytesWritten < 0)
				{
					m_encodingState = EncodingState::Aborting;
					changeStateAndNotify(JobState::FailedCleanUp);
					emit signalLogMessage(
						trUtf8("Error on writing header to encoder. Aborting."),
						LOG_STYLE_ERROR);
					cleanUpEncoding();
					return;
				}

				return;
			}
		}

		m_memorizedEncodingTime = 0.0;
		m_encodeRangeStartTime = hr_clock::now();

		m_encodingState = EncodingState::WaitingForFrames;
		processFramesQueue();
	}
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessFinished(int a_exitCode,
	QProcess::ExitStatus a_exitStatus)
{
	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		EncodingState workingStates[] = {EncodingState::WaitingForFrames,
			EncodingState::WritingFrame, EncodingState::WritingHeader};

		if(m_encodingState == EncodingState::CheckingEncoderSanity)
			return;
		else if(m_encodingState == EncodingState::Idle)
			return;
		else if(m_encodingState == EncodingState::Finishing)
			changeStateAndNotify(JobState::CompletedCleanUp);
		else if(vsedit::contains(workingStates, m_encodingState))
		{
			QString exitStatusString = (a_exitStatus == QProcess::CrashExit) ?
				trUtf8("crash") : trUtf8("normal exit");
			emit signalLogMessage(trUtf8("Encoder has finished "
				"unexpectedly.\nReason: %1; exit code: %2")
				.arg(exitStatusString).arg(a_exitCode), LOG_STYLE_ERROR);
			changeStateAndNotify(JobState::FailedCleanUp);
		}

		cleanUpEncoding();
		finishEncodingCLI();
	}
	else if(m_properties.type == JobType::RunProcess)
	{
		QString message = trUtf8("Process has finished.");
		QString logStyle = LOG_STYLE_POSITIVE;
		JobState nextState = JobState::Completed;

		if(a_exitStatus == QProcess::CrashExit)
		{
			message = trUtf8("Process has crashed.");
			logStyle = LOG_STYLE_ERROR;
			nextState = JobState::Failed;
		}
		else if(a_exitCode != 0)
			logStyle = LOG_STYLE_WARNING;

		emit signalLogMessage(trUtf8("%1 Exit code: %2")
			.arg(message).arg(a_exitCode), logStyle);
		changeStateAndNotify(nextState);
	}
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessError(QProcess::ProcessError a_error)
{
	if(m_properties.type == JobType::EncodeScriptCLI)
	{
		if(m_encodingState == EncodingState::CheckingEncoderSanity)
			return;

		if(m_encodingState == EncodingState::Idle)
		{
			emit signalLogMessage(trUtf8("Encoder has reported "
				"an error while it shouldn't be running at all. Ignoring."),
				LOG_STYLE_WARNING);
			return;
		}

		switch(a_error)
		{
		case QProcess::FailedToStart:
			emit signalLogMessage(trUtf8("Encoder has failed to start. "
				"Aborting."), LOG_STYLE_ERROR);
			m_encodingState = EncodingState::Aborting;
			changeStateAndNotify(JobState::FailedCleanUp);
			cleanUpEncoding();
			break;

		case QProcess::Crashed:
			emit signalLogMessage(trUtf8("Encoder has crashed. "
				"Aborting."), LOG_STYLE_ERROR);
			m_encodingState = EncodingState::EncoderCrashed;
			changeStateAndNotify(JobState::FailedCleanUp);
			cleanUpEncoding();
			break;

		case QProcess::Timedout:
			break;

		case QProcess::WriteError:
			if(m_encodingState == EncodingState::WritingFrame)
			{
				emit signalLogMessage(trUtf8("Writing to encoder "
					"failed. Aborting."), LOG_STYLE_ERROR);
				m_encodingState = EncodingState::Aborting;
				changeStateAndNotify(JobState::FailedCleanUp);
				cleanUpEncoding();
			}
			else
			{
				emit signalLogMessage(trUtf8("Encoder has returned a "
					"writing error, but we were not writing. Ignoring."),
					LOG_STYLE_WARNING);
			}
			break;

		case QProcess::ReadError:
			emit signalLogMessage(trUtf8("Error on reading the "
				"encoder feedback."), LOG_STYLE_WARNING);
			break;

		case QProcess::UnknownError:
			emit signalLogMessage(trUtf8("Unknown error in encoder."),
				LOG_STYLE_WARNING);
			break;

		default:
			assert(false);
		}
	}
	else if(m_properties.type == JobType::RunProcess)
	{
		switch(a_error)
		{
		case QProcess::FailedToStart:
			emit signalLogMessage(trUtf8("Process has failed to start."),
				LOG_STYLE_ERROR);
			changeStateAndNotify(JobState::Failed);
			break;

		case QProcess::Crashed:
			emit signalLogMessage(trUtf8("Process has crashed."),
				LOG_STYLE_ERROR);
			changeStateAndNotify(JobState::Failed);
			break;

		default:
			break;
		}
	}
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessReadChannelFinished()
{
	if(m_properties.type != JobType::EncodeScriptCLI)
		return;

	if(m_encodingState == EncodingState::CheckingEncoderSanity)
		return;

	if(m_encodingState == EncodingState::Idle)
	{
		emit signalLogMessage(trUtf8("Encoder has suddenly stopped "
			"accepting data while it shouldn't be running at all. Ignoring."),
			LOG_STYLE_WARNING);
		return;
	}

	if((m_encodingState != EncodingState::Finishing) &&
		(m_encodingState != EncodingState::Aborting))
	{
		emit signalLogMessage(trUtf8("Encoder has suddenly stopped "
			"accepting data. Aborting."), LOG_STYLE_ERROR);
		m_encodingState = EncodingState::Aborting;
		changeStateAndNotify(JobState::FailedCleanUp);
		cleanUpEncoding();
	}
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessBytesWritten(qint64 a_bytes)
{
	if(m_properties.type != JobType::EncodeScriptCLI)
		return;

	if(m_encodingState == EncodingState::CheckingEncoderSanity)
		return;

	if(m_encodingState == EncodingState::Idle)
	{
		emit signalLogMessage(trUtf8("Encoder has reported written "
			"data while it shouldn't be running at all. Ignoring."),
			LOG_STYLE_WARNING);
		return;
	}

	if((m_encodingState == EncodingState::Aborting) ||
		(m_encodingState == EncodingState::Finishing))
		return;

	if((m_encodingState != EncodingState::WritingFrame) &&
		(m_encodingState != EncodingState::WritingHeader))
	{
		emit signalLogMessage(trUtf8("Encoder reports successful "
			"write, but we were not writing anything.\nData written: "
			"%1 bytes.").arg(a_bytes), LOG_STYLE_WARNING);
		return;
	}

	if(a_bytes <= 0)
	{
		emit signalLogMessage(trUtf8("Error on writing data to "
			"encoder.\nExpected to write: %1 bytes. Data written: %2 bytes.\n"
			"Aborting.").arg(m_bytesToWrite).arg(m_bytesWritten),
			LOG_STYLE_ERROR);
		m_encodingState = EncodingState::Aborting;
		changeStateAndNotify(JobState::FailedCleanUp);
		cleanUpEncoding();
		return;
	}

	m_bytesWritten += a_bytes;

	if((m_bytesWritten + m_process.bytesToWrite()) < m_bytesToWrite)
	{
		emit signalLogMessage(trUtf8("Encoder has lost written "
			"data. Aborting."), LOG_STYLE_ERROR);
		m_encodingState = EncodingState::Aborting;
		changeStateAndNotify(JobState::FailedCleanUp);
		cleanUpEncoding();
		return;
	}

	if(m_bytesWritten < m_bytesToWrite)
		return;

	assert(m_cpVSAPI);
	if(m_encodingState == EncodingState::WritingHeader)
	{
		m_memorizedEncodingTime = 0.0;
		m_encodeRangeStartTime = hr_clock::now();
	}
	else if(m_encodingState == EncodingState::WritingFrame)
	{
		Frame referenceFrame(m_lastFrameProcessed + 1, 0, nullptr);
		std::list<Frame>::iterator it =
			std::find(m_framesCache.begin(), m_framesCache.end(),
			referenceFrame);
		assert(it != m_framesCache.end());

		m_cpVSAPI->freeFrame(it->cpOutputFrameRef);
		m_framesCache.erase(it);
		m_lastFrameProcessed++;
		m_properties.framesProcessed++;
		updateFPS();

		emit signalProgressChanged();
	}

	m_encodingState = EncodingState::WaitingForFrames;

	if((m_properties.jobState == JobState::Pausing) && (m_framesInProcess == 0))
	{
		changeStateAndNotify(JobState::Paused);
		return;
	}

	processFramesQueue();
}

// END OF
//==============================================================================

void vsedit::Job::slotProcessReadyReadStandardError()
{
	QByteArray standardError = m_process.readAllStandardError();
	QString standardErrorText = QString::fromUtf8(standardError);
	standardErrorText = standardErrorText.trimmed();
	if(!standardErrorText.isEmpty())
		emit signalLogMessage(standardErrorText);
}

// END OF
//==============================================================================

void vsedit::Job::slotWriteLogMessage(int a_messageType,
	const QString & a_message)
{
	QString style = vsMessageTypeToStyleName(a_messageType);
	emit signalLogMessage(a_message, style);
}

// END OF
//==============================================================================

void vsedit::Job::slotFrameQueueStateChanged(size_t a_inQueue,
	size_t a_inProcess, size_t a_maxThreads)
{
	m_framesInQueue = a_inQueue;
	m_framesInProcess = a_inProcess;
	m_maxThreads = a_maxThreads;
	emit signalCoreInfoChanged();
}

// END OF
//==============================================================================

void vsedit::Job::slotCoreFramebufferUsedBytes(int64_t a_bytes)
{
	m_coreFramebufferBytes = a_bytes;
	emit signalCoreInfoChanged();
}

// END OF
//==============================================================================

void vsedit::Job::slotScriptProcessorFinalized()
{
	assert(m_properties.type == JobType::EncodeScriptCLI);
	finishEncodingCLI();
}

// END OF
//==============================================================================

void vsedit::Job::slotReceiveFrame(int a_frameNumber, int a_outputIndex,
	const VSFrameRef * a_cpOutputFrameRef,
	const VSFrameRef * a_cpPreviewFrameRef)
{
	(void)a_cpPreviewFrameRef;

	EncodingState validStates[] = {EncodingState::WaitingForFrames,
		EncodingState::WritingHeader, EncodingState::WritingFrame};
	if(!vsedit::contains(validStates, m_encodingState))
		return;

	if((a_frameNumber < m_properties.firstFrameReal) ||
		(a_frameNumber > m_properties.lastFrameReal))
		return;

	assert(m_cpVSAPI);
	const VSFrameRef * cpFrameRef =
		m_cpVSAPI->cloneFrameRef(a_cpOutputFrameRef);
	Frame newFrame(a_frameNumber, a_outputIndex, cpFrameRef);
	m_framesCache.push_back(newFrame);

	if(m_encodingState == EncodingState::WaitingForFrames)
		processFramesQueue();
}

// END OF
//==============================================================================

void vsedit::Job::slotFrameRequestDiscarded(int a_frameNumber,
	int a_outputIndex, const QString & a_reason)
{
	(void)a_frameNumber;
	(void)a_outputIndex;
	(void)a_reason;

	EncodingState validStates[] = {EncodingState::WaitingForFrames,
		 EncodingState::WritingHeader, EncodingState::WritingFrame};
	if(!vsedit::contains(validStates, m_encodingState))
		return;

	m_encodingState = EncodingState::Aborting;
	changeStateAndNotify(JobState::FailedCleanUp);
	cleanUpEncoding();
}

// END OF
//==============================================================================

void vsedit::Job::fillVariables()
{
	static const QString TOKEN_WIDTH = "{w}";
	static const QString TOKEN_HEIGHT = "{h}";
	static const QString TOKEN_FPS_NUMERATOR = "{fpsn}";
	static const QString TOKEN_FPS_DENOMINATOR = "{fpsd}";
	static const QString TOKEN_FPS = "{fps}";
	static const QString TOKEN_BITDEPTH = "{bits}";
	static const QString TOKEN_SCRIPT_DIRECTORY = "{sd}";
	static const QString TOKEN_SCRIPT_NAME = "{sn}";
	static const QString TOKEN_FRAMES_NUMBER = "{f}";
	static const QString TOKEN_SUBSAMPLING = "{ss}";

	m_variables =
	{
		{TOKEN_WIDTH, trUtf8("video width"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_WIDTH;
				return QString::number(m_cpVideoInfo->width);
			}
		},

		{TOKEN_HEIGHT, trUtf8("video height"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_HEIGHT;
				return QString::number(m_cpVideoInfo->height);
			}
		},

		{TOKEN_FPS_NUMERATOR, trUtf8("video framerate numerator"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_FPS_NUMERATOR;
				return QString::number(m_cpVideoInfo->fpsNum);
			}
		},

		{TOKEN_FPS_DENOMINATOR, trUtf8("video framerate denominator"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_FPS_DENOMINATOR;
				return QString::number(m_cpVideoInfo->fpsDen);
			}
		},

		{TOKEN_FPS, trUtf8("video framerate as fraction"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_FPS;
				double fps = (double)m_cpVideoInfo->fpsNum /
					(double)m_cpVideoInfo->fpsDen;
				return QString::number(fps, 'f', 10);
			}
		},

		{TOKEN_BITDEPTH, trUtf8("video colour bitdepth"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_BITDEPTH;
				return QString::number(m_cpVideoInfo->format->bitsPerSample);
			}
		},

		{TOKEN_SCRIPT_DIRECTORY, trUtf8("script directory"),
			[&]() -> QString
			{
				QFileInfo scriptFile(m_properties.scriptName);
				return scriptFile.canonicalPath();
			}
		},

		{TOKEN_SCRIPT_NAME, trUtf8("script name without extension"),
			[&]() -> QString
			{
				QFileInfo scriptFile(m_properties.scriptName);
				return scriptFile.completeBaseName();
			}
		},

		{TOKEN_FRAMES_NUMBER, trUtf8("total frames number"),
			[&]() -> QString
			{
				return QString::number(framesTotal());
			}
		},

		{TOKEN_SUBSAMPLING, trUtf8("subsampling string (like 420)"),
			[&]() -> QString
			{
				if(!m_cpVideoInfo)
					return TOKEN_SUBSAMPLING;
				const VSFormat * cpFormat = m_cpVideoInfo->format;
				if(!cpFormat)
					return QString();
				return vsedit::subsamplingString(cpFormat->subSamplingW,
					cpFormat->subSamplingH);
			}
		},
	};

	std::sort(m_variables.begin(), m_variables.end(),
		[&](const vsedit::VariableToken & a_first,
			const vsedit::VariableToken & a_second) -> bool
		{
			return (a_first.token.length() > a_second.token.length());
		});
}

// END OF
//==============================================================================

void vsedit::Job::changeStateAndNotify(JobState a_state)
{
	if(m_properties.jobState == a_state)
		return;

	JobState oldState = m_properties.jobState;
	m_properties.jobState = a_state;

	if(oldState == JobState::Waiting)
		m_properties.timeStarted = QDateTime::currentDateTimeUtc();

	const JobState finishStates[] = {JobState::Aborted, JobState::Failed,
		JobState::DependencyNotMet, JobState::Completed};
	if(vsedit::contains(finishStates, a_state))
	{
		m_properties.timeEnded = QDateTime::currentDateTimeUtc();
		memorizeEncodingTime();
	}

	if(a_state == JobState::Paused)
		memorizeEncodingTime();

	if((oldState == JobState::Paused) && (a_state == JobState::Running))
		m_encodeRangeStartTime = hr_clock::now();

	if(a_state == JobState::Waiting)
	{
		m_properties.timeStarted = QDateTime();
		m_properties.timeEnded = QDateTime();
		m_memorizedEncodingTime = 0.0;
		m_properties.fps = 0.0;
		m_properties.framesProcessed = 0;
	}

	emit signalStateChanged(m_properties.jobState, oldState);
}

// END OF
//==============================================================================

void vsedit::Job::startEncodeScriptCLI()
{
	changeStateAndNotify(JobState::Running);

	QString absoluteScriptPath =
		resolvePathFromApplication(m_properties.scriptName);
	QFile scriptFile(absoluteScriptPath);
	bool opened = scriptFile.open(QIODevice::ReadOnly);
	if(!opened)
	{
		emit signalLogMessage(trUtf8("Could not open script file \"%1\".")
			.arg(m_properties.scriptName), LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::Failed);
		return;
	}

	QString script = QString::fromUtf8(scriptFile.readAll());
	scriptFile.close();

	if((!m_pVSScriptLibrary) || (!m_pSettingsManager))
	{
		emit signalLogMessage(trUtf8("Job is not porerly initialized."),
			LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::Failed);
		return;
	}

	m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();
	assert(m_cpVSAPI);

	if(!m_pVapourSynthScriptProcessor)
	{
		m_pVapourSynthScriptProcessor = new VapourSynthScriptProcessor(
			m_pSettingsManager, m_pVSScriptLibrary, this);
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalWriteLogMessage(int, const QString &)),
			this, SLOT(slotWriteLogMessage(int, const QString &)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalFrameQueueStateChanged(size_t, size_t, size_t)),
			this, SLOT(slotFrameQueueStateChanged(size_t, size_t, size_t)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalCoreFramebufferUsedBytes(int64_t)),
			this, SLOT(slotCoreFramebufferUsedBytes(int64_t)));
		connect(m_pVapourSynthScriptProcessor, SIGNAL(signalFinalized()),
			this, SLOT(slotScriptProcessorFinalized()));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalDistributeFrame(int, int, const VSFrameRef *,
				const VSFrameRef *)),
			this, SLOT(slotReceiveFrame(int, int, const VSFrameRef *,
				const VSFrameRef *)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalFrameRequestDiscarded(int, int, const QString &)),
			this, SLOT(slotFrameRequestDiscarded(int, int, const QString &)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalFrameQueueStateChanged(size_t, size_t, size_t)),
			this, SLOT(slotFrameQueueStateChanged(size_t, size_t, size_t)));
		connect(m_pVapourSynthScriptProcessor,
			SIGNAL(signalCoreFramebufferUsedBytes(int64_t)),
			this, SLOT(slotCoreFramebufferUsedBytes(int64_t)));
	}

	bool scriptProcessorInitialized = m_pVapourSynthScriptProcessor->initialize(
		script, m_properties.scriptName);
	if(!scriptProcessorInitialized)
	{
		emit signalLogMessage(trUtf8("Failed to initialize script.\n%1")
			.arg(m_pVapourSynthScriptProcessor->error()), LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::Failed);
		return;
	}

	m_cpVideoInfo = m_pVapourSynthScriptProcessor->videoInfo();
	assert(m_cpVideoInfo);

	m_properties.framesProcessed = 0;
	m_properties.firstFrameReal = m_properties.firstFrame;
	vsedit::clamp(m_properties.firstFrameReal, 0, m_cpVideoInfo->numFrames - 1);
	m_properties.lastFrameReal = m_properties.lastFrame;
	if((m_properties.lastFrameReal < 0) ||
		(m_properties.lastFrameReal >= m_cpVideoInfo->numFrames))
		m_properties.lastFrameReal = m_cpVideoInfo->numFrames - 1;
	m_lastFrameRequested = m_properties.firstFrameReal - 1;
	m_lastFrameProcessed = m_lastFrameRequested;
	m_encodingState = EncodingState::Idle;
	m_bytesToWrite = 0u;
	m_bytesWritten = 0u;

	emit signalProgressChanged();

	if(m_pFrameHeaderWriter)
		delete m_pFrameHeaderWriter;

	if(m_properties.encodingHeaderType == EncodingHeaderType::Y4M)
		m_pFrameHeaderWriter =
			new FrameHeaderWriterY4M(m_cpVSAPI, m_cpVideoInfo, this);
	else
		m_pFrameHeaderWriter =
			new FrameHeaderWriterNull(m_cpVSAPI, m_cpVideoInfo, this);

	bool compatibleHeader = m_pFrameHeaderWriter->isCompatible();
	if(!compatibleHeader)
	{
		emit signalLogMessage(trUtf8("Video is not compatible "
			"with the chosen header."), LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::FailedCleanUp);
		cleanUpEncoding();
		return;
	}

	QString executable = vsedit::resolvePathFromApplication(
		m_properties.executablePath);
	QString decodedArguments =
		decodeArguments(m_properties.arguments);
	QString commandLine = QString("\"%1\" %2").arg(executable)
		.arg(decodedArguments);

	emit signalLogMessage(trUtf8("Command line:"));
	emit signalLogMessage(commandLine);

	emit signalLogMessage(trUtf8("Checking the encoder sanity."));
	m_encodingState = EncodingState::CheckingEncoderSanity;

	m_process.start(commandLine);
	if(!m_process.waitForStarted(3000))
	{
		emit signalLogMessage(trUtf8("Encoder wouldn't start."),
			LOG_STYLE_ERROR);
		changeStateAndNotify(JobState::FailedCleanUp);
		cleanUpEncoding();
		return;
	}

	m_process.closeWriteChannel();
	if(!m_process.waitForFinished(3000))
	{
		emit signalLogMessage(trUtf8("Program is not behaving "
			"like a CLI encoder. Terminating."), LOG_STYLE_ERROR);
		m_process.kill();
		m_process.waitForFinished(-1);
		changeStateAndNotify(JobState::FailedCleanUp);
		cleanUpEncoding();
		return;
	}

	emit signalLogMessage(trUtf8("Encoder seems sane. Starting."));
	m_encodingState = EncodingState::StartingEncoder;
	m_process.start(commandLine);
}

// END OF
//==============================================================================

void vsedit::Job::startRunProcess()
{
	changeStateAndNotify(JobState::Running);

	QString executable = vsedit::resolvePathFromApplication(
		m_properties.executablePath);
	QString commandLine = QString("\"%1\" %2").arg(executable)
		.arg(m_properties.arguments);

	emit signalLogMessage(trUtf8("Command line:"));
	emit signalLogMessage(commandLine);

	m_process.start(commandLine);
}

// END OF
//==============================================================================

void vsedit::Job::startRunShellCommand()
{
	changeStateAndNotify(JobState::Running);

	QString command = "%1";
#ifdef Q_OS_WIN
	command = "cmd.exe /c %1";
#else
	command = "/bin/sh -c %1";
#endif

	QProcess::startDetached(command.arg(m_properties.shellCommand));
	changeStateAndNotify(JobState::Completed);
}

// END OF
//==============================================================================

QString vsedit::Job::decodeArguments(const QString & a_arguments) const
{
	QString decodedString = a_arguments.simplified();

	for(const vsedit::VariableToken & variable : m_variables)
	{
		decodedString = decodedString.replace(variable.token,
			variable.evaluate());
	}

	return decodedString;
}

// END OF
//==============================================================================

void vsedit::Job::cleanUpEncoding()
{
	if(m_process.state() == QProcess::Running)
	{
		if(m_encodingState != EncodingState::Aborting)
			m_encodingState = EncodingState::Finishing;
		m_process.closeWriteChannel();
	}

	m_pVapourSynthScriptProcessor->flushFrameTicketsQueue();
	clearFramesCache();
	m_framebuffer.clear();

	m_cpVideoInfo = nullptr;
	m_pVapourSynthScriptProcessor->finalize();
}

// END OF
//==============================================================================

void vsedit::Job::clearFramesCache()
{
	if(m_framesCache.empty())
		return;

	assert(m_cpVSAPI);
	for(Frame & frame : m_framesCache)
	{
		m_cpVSAPI->freeFrame(frame.cpOutputFrameRef);
		m_cpVSAPI->freeFrame(frame.cpPreviewFrameRef);
	}
	m_framesCache.clear();
}

// END OF
//==============================================================================

void vsedit::Job::processFramesQueue()
{
	if(m_encodingState != EncodingState::WaitingForFrames)
		return;

	if(m_properties.framesProcessed == framesTotal())
	{
		assert(m_framesCache.empty());
		memorizeEncodingTime();
		updateFPS();
		changeStateAndNotify(JobState::CompletedCleanUp);
		m_encodingState = EncodingState::Finishing;
		cleanUpEncoding();
		return;
	}

	while((m_lastFrameRequested < m_properties.lastFrameReal) &&
		(m_framesInProcess < m_maxThreads) &&
		(m_framesCache.size() < m_cachedFramesLimit) &&
		(m_properties.jobState == JobState::Running))
	{
		m_pVapourSynthScriptProcessor->requestFrameAsync(
			m_lastFrameRequested + 1);
		m_lastFrameRequested++;
	}

	Frame frame(m_lastFrameProcessed + 1, 0, nullptr);
	std::list<Frame>::iterator it = std::find(m_framesCache.begin(),
		m_framesCache.end(), frame);
	if(it == m_framesCache.end())
		return;

	frame.cpOutputFrameRef = it->cpOutputFrameRef;

	// VapourSynth frames are padded so every line has aligned address.
	// But encoder expects frames tightly packed. We pack frame lines
	// into an intermediate buffer, because writing whole frame at once
	// is faster than feeding it to encoder line by line.

	size_t currentDataSize = 0;

	assert(m_cpVideoInfo);
	const VSFormat * cpFormat = m_cpVideoInfo->format;
	assert(cpFormat);

	if(m_pFrameHeaderWriter->needFramePrefix())
	{
		QByteArray framePrefix =
			m_pFrameHeaderWriter->framePrefix(frame.cpOutputFrameRef);
		int prefixSize = framePrefix.size();
		if(prefixSize > 0)
		{
			if((size_t)prefixSize > m_framebuffer.size())
				m_framebuffer.resize(prefixSize);
			memcpy(m_framebuffer.data(), framePrefix.data(), prefixSize);
			currentDataSize += prefixSize;
		}
	}

	for(int i = 0; i < cpFormat->numPlanes; ++i)
	{
		const uint8_t * cpPlane =
			m_cpVSAPI->getReadPtr(frame.cpOutputFrameRef, i);
		int stride = m_cpVSAPI->getStride(frame.cpOutputFrameRef, i);
		int width = m_cpVSAPI->getFrameWidth(frame.cpOutputFrameRef, i);
		int height = m_cpVSAPI->getFrameHeight(frame.cpOutputFrameRef, i);
		int bytes = cpFormat->bytesPerSample;

		size_t planeSize = width * bytes * height;
		size_t neededFramebufferSize = currentDataSize + planeSize;
		if(neededFramebufferSize > m_framebuffer.size())
			m_framebuffer.resize(neededFramebufferSize);
		int framebufferStride = width * bytes;

		vs_bitblt(m_framebuffer.data() + currentDataSize, framebufferStride,
			cpPlane, stride, framebufferStride, height);

		currentDataSize += planeSize;
	}

	if(m_pFrameHeaderWriter->needFramePostfix())
	{
		QByteArray framePostfix =
			m_pFrameHeaderWriter->framePostfix(frame.cpOutputFrameRef);
		int postfixSize = framePostfix.size();
		if(postfixSize > 0)
		{
			size_t neededFramebufferSize = currentDataSize + postfixSize;
			if(neededFramebufferSize > m_framebuffer.size())
				m_framebuffer.resize(neededFramebufferSize);
			memcpy(m_framebuffer.data() + currentDataSize,
				framePostfix.data(), postfixSize);
			currentDataSize += postfixSize;
		}
	}

	m_encodingState = EncodingState::WritingFrame;
	m_bytesToWrite = currentDataSize;
	m_bytesWritten = 0;
	qint64 bytesWritten =
		m_process.write(m_framebuffer.data(), (qint64)m_bytesToWrite);
	if(bytesWritten < 0)
	{
		m_encodingState = EncodingState::Aborting;
		changeStateAndNotify(JobState::FailedCleanUp);
		emit signalLogMessage(trUtf8("Error on writing data to encoder. "
			"Aborting."), LOG_STYLE_ERROR);
		cleanUpEncoding();
		return;
	}

	// Wait until encoder reads the frame.
	// Then this function will be called again.
}

// END OF
//==============================================================================

void vsedit::Job::finishEncodingCLI()
{
	if((m_process.state() == QProcess::Running) ||
		m_pVapourSynthScriptProcessor->isInitialized())
		return;

	if(m_encodingState == EncodingState::Finishing)
	{
		emit signalLogMessage(trUtf8("Finished encoding."), LOG_STYLE_POSITIVE);
		changeStateAndNotify(JobState::CompletedCleanUp);
	}
	else if(m_encodingState == EncodingState::Aborting)
	{
		emit signalLogMessage(trUtf8("Aborted encoding."), LOG_STYLE_WARNING);
	}

	m_encodingState = EncodingState::Idle;

	const std::map<JobState, JobState> stateToSwitch =
	{
		{JobState::Aborting, JobState::Aborted},
		{JobState::FailedCleanUp, JobState::Failed},
		{JobState::CompletedCleanUp, JobState::Completed},
	};

	std::map<JobState, JobState>::const_iterator it =
		stateToSwitch.find(m_properties.jobState);
	if(it != stateToSwitch.cend())
		changeStateAndNotify(it->second);
}

// END OF
//==============================================================================

void vsedit::Job::memorizeEncodingTime()
{
	if(m_properties.type != JobType::EncodeScriptCLI)
		return;

	m_memorizedEncodingTime += currentEncodingRangeTime();
	m_encodeRangeStartTime = hr_clock::now();
}

// END OF
//==============================================================================

void vsedit::Job::updateFPS()
{
	if(m_properties.type != JobType::EncodeScriptCLI)
		return;

	double totalTime = m_memorizedEncodingTime;
	const JobState validStates[] = {JobState::Running, JobState::Pausing};
	if(vsedit::contains(validStates, m_properties.jobState))
		totalTime += currentEncodingRangeTime();
	m_properties.fps = (double)m_properties.framesProcessed / totalTime;
}

// END OF
//==============================================================================

double vsedit::Job::currentEncodingRangeTime() const
{
	if(m_properties.type != JobType::EncodeScriptCLI)
		return 0.0;

	hr_time_point now = hr_clock::now();
	double rangeTime = duration_to_double(now - m_encodeRangeStartTime);
	return rangeTime;
}

// END OF
//==============================================================================
