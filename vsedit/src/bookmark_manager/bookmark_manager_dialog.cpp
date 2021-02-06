#include "bookmark_manager_dialog.h"
#include "ui_bookmark_manager_dialog.h"
#include "math.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

BookmarkManagerDialog::BookmarkManagerDialog(SettingsManager * a_pSettingsManager,
                                             QWidget * a_pParent) :
    QDialog(a_pParent),
    ui(new Ui::BookmarkManagerDialog),
    m_pSettingsManager(a_pSettingsManager)
{
    ui->setupUi(this);
    setWindowGeometry();

    // signal/slot for scriptSelectComboBox
    connect(ui->scriptSelectComboBox, &QComboBox::currentTextChanged,
            this, &BookmarkManagerDialog::signalScriptBookmarkChanged);

    connect(ui->addButton, &QPushButton::clicked,
            this, &BookmarkManagerDialog::signalAddButtonPressed);
    connect(ui->removeButton, &QPushButton::clicked,
            this, &BookmarkManagerDialog::slotSendRemoveBookmarkSignal);
    connect(ui->loadButton, &QPushButton::clicked,
            this, &BookmarkManagerDialog::slotLoadFile);
    connect(ui->saveButton, &QPushButton::clicked,
            this, &BookmarkManagerDialog::signalSaveBookmarksToFile);
    connect(ui->clearButton, &QPushButton::clicked,
            this, &BookmarkManagerDialog::signalClearBookmark);
    connect(ui->closeButton, &QPushButton::clicked,
            this, &BookmarkManagerDialog::slotCloseDialog);
    connect(ui->bookmarkTableView, &QTableView::doubleClicked,
            this, &BookmarkManagerDialog::slotGotoBookmarkFromIndex);
}

BookmarkManagerDialog::~BookmarkManagerDialog()
{
    if(m_pGeometrySaveTimer->isActive())
    {
        m_pGeometrySaveTimer->stop();
        slotSaveGeometry();
    }
}

void BookmarkManagerDialog::moveEvent(QMoveEvent *a_pEvent)
{
    QDialog::moveEvent(a_pEvent);
    saveGeometryDelayed();
}

void BookmarkManagerDialog::hideEvent(QHideEvent *a_pEvent)
{    
    emit signalDialogHidden();
    saveGeometryDelayed();
    QDialog::hideEvent(a_pEvent);    
}

void BookmarkManagerDialog::setWindowGeometry()
{
    m_pGeometrySaveTimer = new QTimer(this);
    m_pGeometrySaveTimer->setInterval(DEFAULT_WINDOW_GEOMETRY_SAVE_DELAY);
    connect(m_pGeometrySaveTimer, &QTimer::timeout,
        this, &BookmarkManagerDialog::slotSaveGeometry);

    m_windowGeometry = m_pSettingsManager->getBookmarkManagerDialogGeometry();
    if(!m_windowGeometry.isEmpty())
        restoreGeometry(m_windowGeometry);
}

void BookmarkManagerDialog::saveGeometryDelayed()
{
    QApplication::processEvents();
    if(!isMaximized())
    {
        m_windowGeometry = saveGeometry();
        m_pGeometrySaveTimer->start();
    }
}

void BookmarkManagerDialog::slotAddScriptBookmark(QString &a_scriptName)
{
    // check for duplicate
    int foundIndex = ui->scriptSelectComboBox->findText(a_scriptName);
    if (foundIndex > -1) return;

    ui->scriptSelectComboBox->addItem(a_scriptName);
//    slotUpdateScriptBookmarkSelection(a_scriptName);
}

void BookmarkManagerDialog::slotRemoveScriptBookmark(QString &a_scriptName)
{
    int count = ui->scriptSelectComboBox->count();
    if (count < 0) return;

    int foundIndex = ui->scriptSelectComboBox->findText(a_scriptName);
    ui->scriptSelectComboBox->removeItem(foundIndex);
}

void BookmarkManagerDialog::slotSetTableViewModel(BookmarkModel *a_model)
{
    ui->bookmarkTableView->setModel(a_model);
}

void BookmarkManagerDialog::slotUpdateScriptBookmarkSelection(QString &a_scriptName)
{
    if (ui->scriptSelectComboBox->currentText() != a_scriptName)
        ui->scriptSelectComboBox->setCurrentText(a_scriptName);
}

void BookmarkManagerDialog::slotUpdateScriptName(const QString &a_oldName, const QString &a_newName)
{
    if (a_oldName == a_newName) return;
    int foundIndex = ui->scriptSelectComboBox->findText(a_oldName);
    if (foundIndex == -1) return;
    ui->scriptSelectComboBox->setItemText(foundIndex, a_newName);
}


void BookmarkManagerDialog::slotSendRemoveBookmarkSignal()
{
    /* first remove the cel from view table, then send signal to remove bookmark in vector */
    const QModelIndexList indexList= ui->bookmarkTableView->selectionModel()->selectedIndexes();

    if (indexList.isEmpty()){
           return;        
    }

    /* deselect cel */
    ui->bookmarkTableView->selectionModel()->select(
                ui->bookmarkTableView->selectionModel()->selection(),
                QItemSelectionModel::Deselect);

    emit signalRemoveBookmark(indexList.first());
}

void BookmarkManagerDialog::slotLoadBookmarkFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        return;
    }

    QRegularExpression reBookmark("(\\d+)");

    while(!file.atEnd())
    {
        const QByteArray line = file.readLine();
        QRegularExpressionMatchIterator i = reBookmark.globalMatch(line);

        while (i.hasNext()) {
            QRegularExpressionMatch bookmarkMatch = i.next();
            int frameIndex = bookmarkMatch.captured(0).toInt();

            int timeInMilli = int(double(frameIndex) / m_fps * double(1000));

            m_bookmarkModel->addBookmark(frameIndex, timeInMilli);
        }
    }
    emit signalBookmarkFileLoaded(fileName);
    // auto save
//	saveTimelineBookmarks();
}

void BookmarkManagerDialog::slotLoadChapterFile(QString a_fileName)
{
    QFile file(a_fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        return;
    }

    QRegularExpression reChapter(R"((\d{2}):(\d{2}):(\d{2})[\.:](\d{3})?)");
    QRegularExpression reTitle("CHAPTER\\d{2,3}NAME=(.*)");

    if(!reTitle.isValid()) {
        qDebug() << reTitle.errorString();
        return;
    }

    QString dummyDate = QString("2020-01-01T"); // add this to make Datetime to milliseconds work

    QStringList titleList;
    QList<int> timeStampList;

    while(!file.atEnd())
    {
        const QByteArray line = file.readLine();
        QRegularExpressionMatch matchChapter = reChapter.match(line);
        QRegularExpressionMatch matchTitle = reTitle.match(line);

        QString timestamp("");
        int timeInMilli = 0;
        QString title;

        if (matchChapter.hasMatch()) {
            timestamp = matchChapter.captured(0);
            timeInMilli = QDateTime::fromString(dummyDate+timestamp, Qt::ISODateWithMs)
                    .time().msecsSinceStartOfDay(); // convert to milliseconds

            timeStampList.append(timeInMilli);
        }
        if (matchTitle.hasMatch()) {
            title = matchTitle.captured(1);
            titleList.append(title);
        }
    }

    // loop through both list
    auto t = titleList.begin();
    auto ts = timeStampList.begin();
    while (t != titleList.end() && ts != timeStampList.end())
    {
      auto  x = *t++;
      auto& y = *ts++;

      m_bookmarkModel->addChapter(x, y, m_fps);
    }

    // use this instead when upgraded to C++17
//    for (auto& [t, ts] : zip(titleList, timeStampList)) {
//        m_bookmarkModel->addChapter(t, ts, m_fps);
//    }

}

void BookmarkManagerDialog::slotLoadFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load chapter/bookmark file"), m_lastUsedFilePath,
        tr("Chapter/bookmark file (*.txt *.xml)"));

    if (fileName.isEmpty())
        return;
    else {

        // check the file for format
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }

        // using regex to check if file is chapter file or bookmark file
        QRegularExpression regExChapterCheck("CHAPTER\\d{2}=(\\d{2}:\\d{2}:\\d{2}[\\.]\\d{3}?)");
        QRegularExpression regExBookmarkCheck("^\\d+(\\D+)\\d+(?:\\1\\d+)*$");
        QString detectedFormat = "";

        QRegularExpressionMatch bookmarkCheckMatch;
        QRegularExpressionMatch chapterCheckMatch;

        while(!file.atEnd()) {
            const QByteArray line = file.readLine();
            bookmarkCheckMatch = regExBookmarkCheck.match(line);
            chapterCheckMatch = regExChapterCheck.match(line);

            if (bookmarkCheckMatch.hasMatch()) {
                detectedFormat = "bookmarkFormat";
                break;
            } else
            if (chapterCheckMatch.hasMatch()) {
                detectedFormat = "chapterFormat";
                break;
            } else {
                QMessageBox::information(this, "Error", "Not a chapter file");
                return;
            }
        }

        /* check passed, assigning to their loading functions */
        file.seek(0); // reset file
        if (detectedFormat == "bookmarkFormat") {
            emit signalLoadBookmarkFile(file);
        } else if (detectedFormat == "chapterFormat") {
            emit signalLoadChapterFile(file);
        }
    }
}

void BookmarkManagerDialog::slotGotoBookmarkFromIndex(const QModelIndex &a_index)
{
    if (!a_index.isValid())
        return;

    emit signalGotoBookmark(a_index);

    // deselect cel
    ui->bookmarkTableView->selectionModel()->select(
                ui->bookmarkTableView->selectionModel()->selection(),
                QItemSelectionModel::Deselect);
}

void BookmarkManagerDialog::slotCloseDialog()
{
    this->hide();
}

void BookmarkManagerDialog::slotSaveGeometry()
{
    m_pGeometrySaveTimer->stop();
    m_pSettingsManager->setBookmarkManagerDialogGeometry(m_windowGeometry);
}
