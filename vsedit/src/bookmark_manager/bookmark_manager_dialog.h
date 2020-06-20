#ifndef BOOKMARK_MANAGER_DIALOG_H
#define BOOKMARK_MANAGER_DIALOG_H

#include "bookmark_model.h"
#include "vapoursynth/VapourSynth.h"
#include "../../common-src/settings/settings_manager.h"

#include <QDialog>
#include <QFileInfo>

namespace Ui {
class BookMarkManagerDialog;
}

class BookMarkManagerDialog : public QDialog
{
    Q_OBJECT

    enum SaveMode {
        CHAPTER_MODE,
        BOOKMARK_MODE,
    };

public:
    explicit BookMarkManagerDialog(SettingsManager *a_pSettingsManager,  const VSVideoInfo *a_pVideoInfo,
                 QString a_scriptName, QString a_lastUsedFilePath, QWidget *a_pParent = nullptr);

    ~BookMarkManagerDialog();

signals:

    void signalAddButtonPressed();
    void signalGotoBookmark(int a_frameIndex);
    void signalBookmarkSavedToFile(QString fileName);



private:
    Ui::BookMarkManagerDialog *ui;

    BookmarkModel * m_bookmarkModel;

    SettingsManager * m_pSettingsManager;

    double timeToSecond(double fps);

    QString m_scriptName;

    double m_fps;

    QString m_lastUsedFilePath;

public slots:

    void slotAddBookmark(int a_frameIndex);


private slots:


    void slotRemoveBookmark();
    void slotLoadBookmarks();
    void slotLoadChapters();
    void slotLoadVariousFormat();
    void slotRemoveAll();
    void slotSaveBookmarksToFile();
    void slotGotoBookmark();
    void slotGotoBookmarkFromIndex(const QModelIndex &a_index);

};

#endif // BOOKMARK_MANAGER_DIALOG_H
