#include "job_edit_dialog.h"

#include "../settings/settings_manager.h"

#include <QMessageBox>
#include <QFileDialog>
#include <map>
#include <cassert>

//==============================================================================

JobEditDialog::JobEditDialog(SettingsManager * a_pSettingsManager,
	QWidget * a_pParent) :
	  QDialog(a_pParent, (Qt::WindowFlags)0
		| Qt::Dialog
		| Qt::CustomizeWindowHint
		| Qt::WindowTitleHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::WindowCloseButtonHint)
	, m_pSettingsManager(a_pSettingsManager)
{
	m_ui.setupUi(this);

	JobType jobTypes[] = {JobType::EncodeScriptCLI, JobType::RunProcess,
		JobType::RunShellCommand};
	for(const JobType & jobType : jobTypes)
		m_ui.jobTypeComboBox->addItem(vsedit::Job::typeName(jobType),
			(int)jobType);
	m_ui.jobTypeComboBox->setCurrentIndex(0);
	slotJobTypeChanged(m_ui.jobTypeComboBox->currentIndex());

	m_ui.encodingHeaderTypeComboBox->addItem(trUtf8("No header"),
		(int)EncodingHeaderType::NoHeader);
	m_ui.encodingHeaderTypeComboBox->addItem(trUtf8("Y4M"),
		(int)EncodingHeaderType::Y4M);

	setUpEncodingPresets();

	connect(m_ui.jobTypeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotJobTypeChanged(int)));
	connect(m_ui.encodingScriptBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingScriptBrowseButtonClicked()));
	connect(m_ui.encodingPresetComboBox, SIGNAL(activated(const QString &)),
		this, SLOT(slotEncodingPresetComboBoxActivated(const QString &)));
	connect(m_ui.encodingPresetSaveButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingPresetSaveButtonClicked()));
	connect(m_ui.encodingPresetDeleteButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingPresetDeleteButton()));
	connect(m_ui.encodingExecutableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingExecutableBrowseButtonClicked()));
	connect(m_ui.encodingArgumentsHelpButton, SIGNAL(clicked()),
		this, SLOT(slotEncodingArgumentsHelpButtonClicked()));
	connect(m_ui.processExecutableBrowseButton, SIGNAL(clicked()),
		this, SLOT(slotProcessExecutableBrowseButtonClicked()));
	connect(m_ui.jobSaveButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

// END OF JobEditDialog::JobEditDialog(SettingsManager * a_pSettingsManager,
//		QWidget * a_pParent)
//==============================================================================

JobEditDialog::~JobEditDialog()
{
}

// END OF JobEditDialog::~JobEditDialog()
//==============================================================================

JobType JobEditDialog::jobType() const
{
	return (JobType)m_ui.jobTypeComboBox->currentData().toInt();
}

// END OF JobType JobEditDialog::jobType() const
//==============================================================================

QString JobEditDialog::encodingScriptPath() const
{
	return m_ui.encodingScriptPathEdit->text();
}

// END OF QString JobEditDialog::encodingScriptPath() const
//==============================================================================

EncodingHeaderType JobEditDialog::encodingHeaderType() const
{
	return (EncodingHeaderType)m_ui.encodingHeaderTypeComboBox->
		currentData().toInt();
}

// END OF EncodingHeaderType JobEditDialog::encodingHeaderType() const
//==============================================================================

QString JobEditDialog::encodingExecutablePath() const
{
	return m_ui.encodingExecutablePathEdit->text();
}

// END OF QString JobEditDialog::encodingExecutablePath() const
//==============================================================================

QString JobEditDialog::encodingArguments() const
{
	return m_ui.encodingArgumentsTextEdit->toPlainText();
}

// END OF QString JobEditDialog::encodingArguments() const
//==============================================================================

QString JobEditDialog::processExecutablePath() const
{
	return m_ui.processExecutablePathEdit->text();
}

// END OF QString JobEditDialog::processExecutablePath() const
//==============================================================================

QString JobEditDialog::processArguments() const
{
	return m_ui.processArgumentsTextEdit->toPlainText();
}

// END OF QString JobEditDialog::processArguments() const
//==============================================================================

QString JobEditDialog::shellCommand() const
{
	return m_ui.shellCommandTextEdit->toPlainText();
}

// END OF QString JobEditDialog::shellCommand() const
//==============================================================================

int JobEditDialog::call(const vsedit::Job * a_pJob)
{
	setUpEncodingPresets();

	if(a_pJob)
	{
		int index = m_ui.jobTypeComboBox->findData((int)a_pJob->type());
		m_ui.jobTypeComboBox->setCurrentIndex(index);
		m_ui.encodingPresetComboBox->clearEditText();
		index = m_ui.encodingHeaderTypeComboBox->findData(
			(int)a_pJob->encodingHeaderType());
		m_ui.encodingHeaderTypeComboBox->setCurrentIndex(index);
		m_ui.encodingExecutablePathEdit->setText(a_pJob->executablePath());
		m_ui.encodingArgumentsTextEdit->setPlainText(a_pJob->arguments());
		m_ui.processExecutablePathEdit->setText(a_pJob->executablePath());
		m_ui.processArgumentsTextEdit->setPlainText(a_pJob->arguments());
		m_ui.shellCommandTextEdit->setPlainText(a_pJob->shellCommand());
	}
	else
	{
		m_ui.jobTypeComboBox->setCurrentIndex(0);
		m_ui.encodingPresetComboBox->setCurrentIndex(0);
		m_ui.processExecutablePathEdit->clear();
		m_ui.processArgumentsTextEdit->clear();
		m_ui.shellCommandTextEdit->clear();
	}

	return exec();
}

// END OF int JobEditDialog::call(const vsedit::Job * a_pJob)
//==============================================================================

void JobEditDialog::slotJobTypeChanged(int a_index)
{
	std::map<JobType, QWidget *> panels =
	{
		{JobType::EncodeScriptCLI, m_ui.encodingPanel},
		{JobType::RunProcess, m_ui.processPanel},
		{JobType::RunShellCommand, m_ui.shellCommandPanel},
	};

	JobType jobType = (JobType)m_ui.jobTypeComboBox->itemData(a_index).toInt();

	for(const std::pair<JobType, QWidget *> & pair : panels)
	{
		if(pair.first == jobType)
			pair.second->setVisible(true);
		else
			pair.second->setVisible(false);
	}
}

// END OF void JobEditDialog::slotJobTypeChanged(int a_index)
//==============================================================================

void JobEditDialog::slotEncodingScriptBrowseButtonClicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setWindowTitle(trUtf8("Open VapourSynth script"));
	fileDialog.setNameFilter(
		trUtf8("VapourSynth script (*.vpy);;All files (*)"));

	QString path = m_ui.encodingScriptPathEdit->text();
	if(path.isEmpty())
		path = m_pSettingsManager->getLastUsedPath();
	QFileInfo fileInfo(path);
	QString dirPath = fileInfo.absoluteDir().path();
	fileDialog.setDirectory(dirPath);
	fileDialog.selectFile(fileInfo.fileName());

	if(!fileDialog.exec())
		return;

	QStringList filesList = fileDialog.selectedFiles();
	m_ui.encodingScriptPathEdit->setText(filesList[0]);
	m_pSettingsManager->setLastUsedPath(filesList[0]);
}

// END OF void JobEditDialog::slotEncodingScriptBrowseButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingPresetComboBoxActivated(const QString & a_text)
{
	if(a_text.isEmpty())
	{
		m_ui.encodingExecutablePathEdit->clear();
		m_ui.encodingArgumentsTextEdit->clear();
		return;
	}

	EncodingPreset preset(a_text);

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
		return;

	preset = *it;

	m_ui.encodingExecutablePathEdit->setText(preset.executablePath);
	m_ui.encodingArgumentsTextEdit->setPlainText(preset.arguments);

	int headerTypeIndex =
		m_ui.encodingHeaderTypeComboBox->findData((int)preset.headerType);
	if(headerTypeIndex < 0)
		headerTypeIndex = 0;
	m_ui.encodingHeaderTypeComboBox->setCurrentIndex(headerTypeIndex);
}

// END OF void JobEditDialog::slotEncodingPresetComboBoxActivated(
//		const QString & a_text)
//==============================================================================

void JobEditDialog::slotEncodingPresetSaveButtonClicked()
{
	EncodingPreset preset(m_ui.encodingPresetComboBox->currentText());
	if(preset.name.isEmpty())
	{
		QMessageBox::warning(this, trUtf8("Preset save error."),
			trUtf8("Preset name must not be empty."));
		return;
	}

	if(preset.type == EncodingType::CLI)
	{
		preset.executablePath = m_ui.encodingExecutablePathEdit->text();
		if(preset.executablePath.isEmpty())
		{
			QMessageBox::warning(this, trUtf8("Preset save error."),
				trUtf8("Executable path must not be empty."));
			return;
		}

		preset.arguments = m_ui.encodingArgumentsTextEdit->toPlainText();
	}

	preset.headerType = (EncodingHeaderType)
		m_ui.encodingHeaderTypeComboBox->currentData().toInt();

	bool success = m_pSettingsManager->saveEncodingPreset(preset);
	if(!success)
	{
		QMessageBox::critical(this, trUtf8("Preset save error."),
			trUtf8("Error saving encoding preset."));
		return;
	}

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
	{
		assert(m_ui.encodingPresetComboBox->findText(preset.name) == -1);
		m_encodingPresets.push_back(preset);
		m_ui.encodingPresetComboBox->addItem(preset.name);
		m_ui.encodingPresetComboBox->model()->sort(0);
	}
	else
	{
		assert(m_ui.encodingPresetComboBox->findText(preset.name) != -1);
		*it = preset;
	}
}

// END OF void JobEditDialog::slotEncodingPresetSaveButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingPresetDeleteButton()
{
	EncodingPreset preset(m_ui.encodingPresetComboBox->currentText());
	if(preset.name.isEmpty())
		return;

	QMessageBox::StandardButton result = QMessageBox::question(this,
		trUtf8("Delete preset"), trUtf8("Do you really want to delete "
		"preset \'%1\'?").arg(preset.name),
		QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::No);
	if(result == QMessageBox::No)
		return;

	std::vector<EncodingPreset>::iterator it = std::find(
		m_encodingPresets.begin(), m_encodingPresets.end(), preset);
	if(it == m_encodingPresets.end())
	{
		assert(m_ui.encodingPresetComboBox->findText(preset.name) == -1);
		QMessageBox::critical(this, trUtf8("Preset delete error."),
			trUtf8("Error deleting preset. Preset was never saved."));
		return;
	}

	int index = m_ui.encodingPresetComboBox->findText(preset.name);
	assert(index != -1);
	m_ui.encodingPresetComboBox->removeItem(index);
	m_encodingPresets.erase(it);
	m_ui.encodingPresetComboBox->setCurrentIndex(0);
	slotEncodingPresetComboBoxActivated(
		m_ui.encodingPresetComboBox->currentText());

	bool success = m_pSettingsManager->deleteEncodingPreset(preset.name);
	if(!success)
	{
		QMessageBox::critical(this, trUtf8("Preset delete error."),
			trUtf8("Error deleting preset \'%1\'.").arg(preset.name));
		return;
	}
}

// END OF void JobEditDialog::slotEncodingPresetDeleteButton()
//==============================================================================

void JobEditDialog::slotEncodingExecutableBrowseButtonClicked()
{
	QString executablePath = chooseExecutable(
		trUtf8("Choose encoder executable"),
		m_ui.encodingExecutablePathEdit->text());

	if(!executablePath.isEmpty())
		m_ui.encodingExecutablePathEdit->setText(executablePath);
}

// END OF void JobEditDialog::slotEncodingExecutableBrowseButtonClicked()
//==============================================================================

void JobEditDialog::slotEncodingArgumentsHelpButtonClicked()
{
	//TODO: Make variables accessible from the Job class and implement.
}

// END OF void JobEditDialog::slotEncodingArgumentsHelpButtonClicked()
//==============================================================================

void JobEditDialog::slotProcessExecutableBrowseButtonClicked()
{
	QString executablePath = chooseExecutable(
		trUtf8("Choose process executable"),
		m_ui.processExecutablePathEdit->text());

	if(!executablePath.isEmpty())
		m_ui.processExecutablePathEdit->setText(executablePath);
}

// END OF void JobEditDialog::slotProcessExecutableBrowseButtonClicked()
//==============================================================================

void JobEditDialog::setUpEncodingPresets()
{
	m_ui.encodingPresetComboBox->clear();

	m_encodingPresets = m_pSettingsManager->getAllEncodingPresets();
	for(const EncodingPreset & preset : m_encodingPresets)
		m_ui.encodingPresetComboBox->addItem(preset.name);

	m_ui.encodingPresetComboBox->setCurrentIndex(0);
	slotEncodingPresetComboBoxActivated(
		m_ui.encodingPresetComboBox->currentText());
}

// END OF void JobEditDialog::setUpEncodingPresets()
//==============================================================================

QString JobEditDialog::chooseExecutable(const QString & a_dialogTitle,
	const QString & a_initialPath)
{
	QString applicationPath = QCoreApplication::applicationDirPath();

	QFileDialog fileDialog;
	fileDialog.setWindowTitle(a_dialogTitle);
	if(a_initialPath.isEmpty())
		fileDialog.setDirectory(applicationPath);
	else
		fileDialog.selectFile(a_initialPath);

#ifdef Q_OS_WIN
	fileDialog.setNameFilter("*.exe");
#endif

	if(!fileDialog.exec())
		return QString();

	QStringList filesList = fileDialog.selectedFiles();
	return filesList[0];
}

// END OF QString JobEditDialog::chooseExecutable(const QString & a_dialogTitle,
//		const QString & a_initialPath)
//==============================================================================
