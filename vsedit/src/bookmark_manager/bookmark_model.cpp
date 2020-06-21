#include "bookmark_model.h"
#include "math.h"

BookmarkModel::BookmarkModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant BookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString("Title");
        case 1:
            return QString("Frame");
        case 2:
            return QString("Time");
        }
    }
    return QVariant();
}

bool BookmarkModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}


int BookmarkModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_bookmarkData.count();
}

int BookmarkModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 3;
}

QVariant BookmarkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();

    if (role == Qt::DisplayRole && col == TITLE_COL)
            return m_bookmarkData[row].title;

    if (role == Qt::DisplayRole && col == FRAME_COL)
            return m_bookmarkData[row].frame;

    if (role == Qt::DisplayRole && col == TIME_COL) {

           int timeInMilli =  m_bookmarkData[row].timeInMilli;
           QTime time = QTime::fromMSecsSinceStartOfDay(timeInMilli);
           return time.toString("hh:mm:ss.zzz"); // convert milliseconds to time format

    }

    // do not clear previous cell when editing
    if (role == Qt::EditRole && col == TITLE_COL) {
        return m_bookmarkData[row].title;
    }

    return QVariant();
}

bool BookmarkModel::setData(const QModelIndex &index, const QVariant &value, int role)
{    
    if (role == Qt::EditRole) {
        if (!checkIndex(index))
            return false;

        int row = index.row();
        int col = index.column();

        switch (col) {
        case TITLE_COL:
            m_bookmarkData[row].title = value.toString();
            break;
        case FRAME_COL:
            m_bookmarkData[row].frame = value.toInt();
            break;
        case TIME_COL:
            m_bookmarkData[row].timeInMilli = value.toInt();
        }

        return true;
    }
    return false;
}

Qt::ItemFlags BookmarkModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;     

    int col = index.column();
    if (col == TITLE_COL) // enable title editing
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool BookmarkModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
}

bool BookmarkModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
}

QVector<BookmarkData> BookmarkModel::bookmarks()
{
    return m_bookmarkData;
}


void BookmarkModel::addBookmark(int a_frameIndex, int a_timeInMilli)
{
    // check for duplicate, only add new when no duplicate
    for (auto & data: m_bookmarkData) {
        if (data.frame == a_frameIndex){
            return;
        }
    }

    const int newRow = m_bookmarkData.size();
    beginInsertRows(QModelIndex(), newRow, newRow);

    BookmarkData data = {QString(""), a_frameIndex, a_timeInMilli };
    m_bookmarkData.append(data);

    // sort by frameindex, only sort when adding new
    auto sortFunc = [](BookmarkData a, BookmarkData b) -> bool {
        return a.frame < b.frame;
    };

    std::sort(m_bookmarkData.begin(), m_bookmarkData.end(), sortFunc);

    endInsertRows();
}

void BookmarkModel::removeBookmark(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    int row = index.row();

    beginRemoveRows(QModelIndex(), row, row);
        m_bookmarkData.remove(row);
    endRemoveRows();
}

void BookmarkModel::addChapter(QString a_title, int a_timeInMilli, double a_fps)
{
    // find duplicate
    for (auto & data: m_bookmarkData) {
        if (data.timeInMilli == a_timeInMilli){
            return;
        }
    }

    const int newRow = m_bookmarkData.size();
    const int frameIndex  = double(a_timeInMilli) / double(1000) * a_fps;

    beginInsertRows(QModelIndex(), newRow, newRow);
        BookmarkData data = {a_title, frameIndex, a_timeInMilli };
    m_bookmarkData.append(data);

    endInsertRows();
}

void BookmarkModel::removeChapter(const QModelIndex &index)
{
    removeBookmark(index);
}

void BookmarkModel::clearAll()
{
    if (!m_bookmarkData.isEmpty()) {
        beginResetModel();
            m_bookmarkData.clear();
        endResetModel();
    }
}

int BookmarkModel::selectedFrameIndex(const QModelIndex &index)
{
    int row = index.row();
    return data(createIndex(row, 1)).toInt();
}


