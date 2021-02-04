#ifndef VAPOURSYNTHPLUGINSMANAGER_H
#define VAPOURSYNTHPLUGINSMANAGER_H

#include "vs_plugin_data.h"

#include <vapoursynth/VapourSynth.h>

#include <QObject>
#include <QStringList>

class SettingsManager;

class VapourSynthPluginsManager : public QObject
{
	Q_OBJECT

public:

	VapourSynthPluginsManager(SettingsManager * a_pSettingsManager,
        QObject * a_pParent = nullptr);

    virtual ~VapourSynthPluginsManager() override;

    void getCorePlugins();

    void getPyScripts();

    QString vSRepoPath();

    QString pluginsPath();

    QString pyScriptsPath();

    QString definitionsPath();

	void pollPaths(const QStringList & a_pluginsPaths);

	QStringList functions() const;

	VSPluginsList pluginsList() const;

    VSPyScriptsList pyScriptsList() const;

	static VSData::Function parseFunctionSignature(const QString & a_name,
		const QString & a_arguments);

	friend void VS_CC fakeConfigPlugin(const char * a_identifier,
		const char * a_defaultNamespace, const char * a_name,
		int a_apiVersion, int a_readonly, VSPlugin * a_pPlugin);

	friend void VS_CC fakeRegisterFunction(const char * a_name,
		const char * a_args, VSPublicFunction a_argsFunc,
		void * a_pFunctionData, VSPlugin * a_pPlugin);

public slots:

	void slotClear();

	void slotSort();

	void slotRefill();

signals:

	void signalWriteLogMessage(int a_messageType,
		const QString & a_message);

private:

    void loadVSRepoPath();

    QString getPathsByVSRepo(const QString &a_key);

	VSPluginsList m_pluginsList;

    VSPyScriptsList m_pyScriptsList;

	QString m_currentPluginPath;

	bool m_pluginAlreadyLoaded;

	SettingsManager * m_pSettingsManager;

    QString m_vsRepoPath;
};

#endif // VAPOURSYNTHPLUGINSMANAGER_H
