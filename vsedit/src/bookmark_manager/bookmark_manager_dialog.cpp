#include "bookmark_manager_dialog.h"
#include "ui_bookmark_manager_dialog.h"
#include "math.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

BookMarkManagerDialog::BookMarkManagerDialog(SettingsManager *a_pSettingsManager,
    const VSVideoInfo *a_pVideoInfo,
    QString a_scriptName, QString a_lastUsedFilePath, QWidget * a_pParent) :
    QDialog(a_pParent),
    ui(new Ui::BookMarkManagerDialog),
    m_pSettingsManager(a_pSettingsManager)
{
    ui->setupUi(this);

    // set fileInfo and fps
    m_scriptName = a_scriptName;
    m_fps = double(a_pVideoInfo->fpsNum) / double(a_pVideoInfo->fpsDen);
    m_lastUsedFilePath = a_lastUsedFilePath;

    m_bookmarkModel = new BookmarkModel(this);
    ui->bookmarkTableView->setModel(m_bookmarkModel);

    connect(ui->addButton, &QPushButton::clicked, this, &BookMarkManagerDialog::signalAddButtonPressed);
    connect(ui->removeButton, &QPushButton::clicked, this, &BookMarkManagerDialog::slotRemoveBookmark);
    connect(ui->loadButton, &QPushButton::clicked, this, &BookMarkManagerDialog::slotLoadBookmarks);
    connect(ui->loadChapterButton, &QPushButton::clicked, this, &BookMarkManagerDialog::slotLoadChapters);
    connect(ui->saveButton, &QPushButton::clicked, this, &BookMarkManagerDialog::slotSaveBookmarksToFile);
    connect(ui->clearButton, &QPushButton::clicked, this, &BookMarkManagerDialog::slotRemoveAll);
    connect(ui->gotoButton, &QPushButton::clicked, this, &BookMarkManagerDialog::slotGotoBookmark);
    connect(ui->bookmarkTableView, &QTableView::doubleClicked, this, &BookMarkManagerDialog::slotGotoBookmarkFromIndex);
//    connect(ui->closeButton, &QPushButton::clicked), this, &);

}

BookMarkManagerDialog::~BookMarkManagerDialog()
{
    delete ui;
}


void BookMarkManagerDialog::slotAddBookmark(int a_frameIndex)
{
    if (a_frameIndex < 0) return;

    int timeInMilli = double(a_frameIndex) / m_fps * double(1000);
    m_bookmarkModel->addBookmark(a_frameIndex, timeInMilli);
}


void BookMarkManagerDialog::slotRemoveBookmark()
{
    const QModelIndexList indexList= ui->bookmarkTableView->selectionModel()->selectedIndexes();

    if (indexList.isEmpty()){
           return;
        }

    m_bookmarkModel->removeBookmark(indexList.first());

    ui->bookmarkTableView->selectionModel()->select(
                ui->bookmarkTableView->selectionModel()->selection(),
                QItemSelectionModel::Deselect);
}

void BookMarkManagerDialog::slotLoadBookmarks()
{
    const QString filePath = QFileDialog::getOpenFileName(this,
        tr("Load bookmarks"), m_lastUsedFilePath,
        tr("Bookmark file (*.txt;);;All files (*)"));
    QFile chaptersFile(filePath);
    if(!chaptersFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QRegularExpression reBookmark("(\\d+)");

    while(!chaptersFile.atEnd())
    {
        const QByteArray line = chaptersFile.readLine();
        QRegularExpressionMatchIterator i = reBookmark.globalMatch(line);

        while (i.hasNext()) {
            QRegularExpressionMatch bookmarkMatch = i.next();
            int frameIndex = bookmarkMatch.captured(0).toInt();

            int timeInMilli = (double(frameIndex) / m_fps) * 1000;

            m_bookmarkModel->addBookmark(frameIndex, timeInMilli);
        }
    }

    // auto save
//	saveTimelineBookmarks();


}

void BookMarkManagerDialog::slotLoadChapters()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load chapters"), m_lastUsedFilePath,
        tr("Chapters file (*.txt;*.xml);;All files (*)"));

    if (fileName.isEmpty())
        return;
    else {
        QFile chaptersFile(fileName);
        if(!chaptersFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QRegularExpression reChapter(R"((\d{2}):(\d{2}):(\d{2})[\.:](\d{3})?)");
        QRegularExpression reTitle("CHAPTER\\d{2,3}NAME=(.*)");


        if(!reTitle.isValid()) {
            qDebug() << reTitle.errorString();
            return;
        }

        QString dummyDate = QString("2020-01-01T"); // add this to make Datetime to milliseconds work

        while(!chaptersFile.atEnd())
        {
            const QByteArray line = chaptersFile.readLine();
            QRegularExpressionMatch matchChapter = reChapter.match(line);
            QRegularExpressionMatch matchTitle = reTitle.match(line);

            QString timestamp("");
            int timeInMilli = 0;
            QString title;

            if (matchChapter.hasMatch()) {
                timestamp = matchChapter.captured(0);
            }
            if (matchTitle.hasMatch()) {
                title = matchTitle.captured(1);
            }

            timeInMilli = QDateTime::fromString(dummyDate+timestamp, Qt::ISODateWithMs)
                    .time().msecsSinceStartOfDay();

            m_bookmarkModel->addChapter(title, timeInMilli, m_fps);
        }
    }
}

void BookMarkManagerDialog::slotLoadVariousFormat()
{
    const QString filePath = QFileDialog::getOpenFileName(this,
        tr("Load chapters"), m_lastUsedFilePath,
        tr("Chapters file (*.txt;*.xml);;All files (*)"));
    QFile InputFile(filePath);
    if(!InputFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

}

void BookMarkManagerDialog::slotRemoveAll()
{
    m_bookmarkModel->clearAll();
}

void BookMarkManagerDialog::slotSaveBookmarksToFile()
{
    QVector<BookmarkData> bookmarks = m_bookmarkModel->bookmarks();

    QString delimiter = m_pSettingsManager->getBookmarkDelimiter();

    if(bookmarks.count() > 0)
    {
        QFileInfo fileInfo(m_scriptName);
        //get file path and fileName without extension
        QString filePath = fileInfo.absolutePath();
        QString scriptFileName = fileInfo.baseName();

        QString fileName = QFileDialog::getSaveFileName(this,
                            tr("Save bookmark"), filePath + QDir::separator() + scriptFileName,
                            tr("Text file (*.txt)"));

        if (fileName.isEmpty())
            return;
        else
        {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly))
            {
                QMessageBox::information(this, tr("Unable to open file"),
                             file.errorString());
                return;
            }

            QStringList bookmarksStringList;
            QString bookmarksString = "";

            BookmarkSavingFormat savingFormat = m_pSettingsManager->getBookmarkSavingFormat();

            switch (savingFormat) {
            case BookmarkSavingFormat::ChapterFormat:
                // looks for timestamp, and title later
                for (auto it = bookmarks.begin(); it != bookmarks.end(); ++it) {
                    int index = std::distance(bookmarks.begin(), it) + 1; // +1 to start index at 1
                    QTime time = QTime::fromMSecsSinceStartOfDay(it->timeInMilli);
                    int fieldWidth = 2;

                    if (index > 99) // 2 digit for < 100, 3 digit for > 100
                        fieldWidth = 3;

                    // format: CHAPTER01=00:00:00.000
                    QString chapterTime = QString("CHAPTER%1=%2")
                            .arg(index, fieldWidth, 10, QLatin1Char('0'))
                            .arg(time.toString("hh:mm:ss.zzz"));

                    bookmarksStringList.append(chapterTime);

                    // format: CHAPTER01NAME=abcde
                    QString chapterName = QString("CHAPTER%1NAME=")
                            .arg(index, fieldWidth, 10, QLatin1Char('0'));

                    bookmarksStringList.append(chapterName);
                }
                bookmarksString = bookmarksStringList.join("\n");

                break;
            case BookmarkSavingFormat::BookmarkFormat:
                // only looks for frame number
                for (auto i : bookmarks) {
                    bookmarksStringList.append(QVariant(i.frame).toString());
                }

                bookmarksString = bookmarksStringList.join(delimiter);
                break;
            }

            QTextStream out(&file);
            out << bookmarksString;

            emit signalBookmarkSavedToFile(fileName);
        }
    }
}

void BookMarkManagerDialog::slotGotoBookmark()
{
    const QModelIndexList indexList= ui->bookmarkTableView->selectionModel()->selectedIndexes();

    if (indexList.isEmpty()){
           return;
    }

    slotGotoBookmarkFromIndex(indexList.first());
}

void BookMarkManagerDialog::slotGotoBookmarkFromIndex(const QModelIndex &a_index)
{
    if (!a_index.isValid())
        return;

    int frameIndex = m_bookmarkModel->selectedFrameIndex(a_index);
    emit signalGotoBookmark(frameIndex);

    // deselect
    ui->bookmarkTableView->selectionModel()->select(
                ui->bookmarkTableView->selectionModel()->selection(),
                QItemSelectionModel::Deselect);
}
