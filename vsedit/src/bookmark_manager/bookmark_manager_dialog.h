#ifndef BOOKMARK_MANAGER_DIALOG_H
#define BOOKMARK_MANAGER_DIALOG_H

#include "bookmark_model.h"
#include "vapoursynth/VapourSynth.h"
#include "../../common-src/settings/settings_manager.h"
#include "../../vsedit/src/preview/script_processor.h"

#include <QDialog>
#include <QFileInfo>

class ScriptEditor;
class ScriptProcessor;
class PreviewArea;

namespace Ui {
class BookmarkManagerDialog;
}

class BookmarkManagerDialog : public QDialog
{
    Q_OBJECT

    enum SaveMode {
        CHAPTER_MODE,
        BOOKMARK_MODE,
    };

public:
    explicit BookmarkManagerDialog(SettingsManager * a_pSettingsManager, QWidget *a_pParent = nullptr);

    ~BookmarkManagerDialog() override;

signals:

    void signalScriptBookmarkChanged(const QString & a_text);
    void signalAddButtonPressed();
    void signalRemoveBookmark(QModelIndex);
    void signalGotoBookmark(const QModelIndex a_index);
    void signalBookmarkSavedToFile(QString a_fileName);
    void signalBookmarkFileLoaded(QString a_fileName);
    void signalClearBookmark();
    void signalLoadBookmarkFile(QFile & a_file);
    void signalLoadChapterFile(QFile & a_file);
    void signalSaveBookmarksToFile();

    void signalDialogHidden();

private:
    Ui::BookmarkManagerDialog *ui;

    BookmarkModel * m_bookmarkModel;

    SettingsManager * m_pSettingsManager;

    double timeToSecond(double fps);

    QString m_scriptName;
    double m_fps;
    QString m_lastUsedFilePath;

protected:

    void moveEvent(QMoveEvent * a_pEvent) override;
    void hideEvent(QHideEvent * a_pEvent) override;

    void setWindowGeometry();
    void saveGeometryDelayed();

    QTimer * m_pGeometrySaveTimer;
    QByteArray m_windowGeometry;

public slots:

    void slotAddScriptBookmark(QString &a_scriptName); // add script to dropdown selection
    void slotRemoveScriptBookmark(QString &a_scriptName);
    void slotSetTableViewModel(BookmarkModel * a_model);
    void slotUpdateScriptBookmarkSelection(QString &a_scriptName);
    void slotUpdateScriptName(const QString &a_oldName, const QString &a_newName);

private slots:

    void slotSendRemoveBookmarkSignal();
    void slotLoadBookmarkFile(QString a_fileName);
    void slotLoadChapterFile(QString a_fileName);
    void slotLoadFile();
    void slotGotoBookmarkFromIndex(const QModelIndex &a_index);
    void slotCloseDialog();

protected slots:

    void slotSaveGeometry();

};

#endif // BOOKMARK_MANAGER_DIALOG_H
