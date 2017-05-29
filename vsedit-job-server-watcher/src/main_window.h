#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ui_main_window.h>

#include "../../common-src/settings/settings_definitions.h"

#include <QWebSocket>

class SettingsManager;
class JobsModel;
class JobEditDialog;
class JobStateDelegate;
class JobDependenciesDelegate;
class QMenu;
class VSScriptLibrary;

#ifdef Q_OS_WIN
	class QWinTaskbarButton;
	class QWinTaskbarProgress;
#endif

class SettingsManager;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow();

	virtual ~MainWindow();

public slots:

	void slotWriteLogMessage(int a_messageType, const QString & a_message);
	void slotWriteLogMessage(const QString & a_message,
		const QString & a_style = LOG_STYLE_DEFAULT);

protected:

	void moveEvent(QMoveEvent * a_pEvent) override;
	void resizeEvent(QResizeEvent * a_pEvent) override;
	void changeEvent(QEvent * a_pEvent) override;

private slots:

	void slotJobNewButtonClicked();
	void slotJobEditButtonClicked();
	void slotJobMoveUpButtonClicked();
	void slotJobMoveDownButtonClicked();
	void slotJobDeleteButtonClicked();
	void slotJobResetStateButtonClicked();
	void slotStartButtonClicked();
	void slotPauseButtonClicked();
	void slotResumeButtonClicked();
	void slotAbortButtonClicked();

	void slotJobDoubleClicked(const QModelIndex & a_index);

	void slotSelectionChanged();

	void slotSaveHeaderState();

	void slotJobsHeaderContextMenu(const QPoint & a_point);
	void slotShowJobsHeaderSection(bool a_show);

	void slotJobsStateChanged(int a_job, int a_jobsTotal, JobState a_state,
		int a_progress, int a_progressMax);

	void slotServerConnected();
	void slotServerDisconnected();
	void slotBinaryMessageReceived(const QByteArray & a_message);
	void slotTextMessageReceived(const QString & a_message);
	void slotServerError(QAbstractSocket::SocketError a_error);

private:

	void createActionsAndMenus();

	void saveGeometrySettings();

	void editJob(const QModelIndex & a_index);

	bool updateJob(int a_index);

	static const char WINDOW_TITLE[];

	Ui::MainWindow m_ui;

	SettingsManager * m_pSettingsManager;

//	JobsModel * m_pJobsModel;
//	VSScriptLibrary * m_pVSScriptLibrary;
//	JobStateDelegate * m_pJobStateDelegate;
//	JobDependenciesDelegate * m_pJobDependenciesDelegate;
//
//	JobEditDialog * m_pJobEditDialog;
//
//	QMenu * m_pJobsHeaderMenu;

	QWebSocket * m_pServerSocket;

#ifdef Q_OS_WIN
	QWinTaskbarButton * m_pWinTaskbarButton;
	QWinTaskbarProgress * m_pWinTaskbarProgress;
#endif
};

#endif // MAINWINDOW_H
