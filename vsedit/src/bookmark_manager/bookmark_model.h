#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#include "../../../common-src/settings/settings_definitions.h"

#include <QAbstractTableModel>

const int COLS = 3;
const int ROWS = 1;

using namespace std;

class BookmarkModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit BookmarkModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void addBookmark(int a_frameIndex, int a_timeInMilli);

    void removeBookmark(const QModelIndex &parent);

    void addChapter(QString a_title, int a_timeInMilli, double a_fps);

    void removeChapter(const QModelIndex &parent);

    void clearAll();

    int selectedFrameIndex(const QModelIndex &index);

    QVector<BookmarkData> bookmarks();

private:

    QVector<BookmarkData> m_bookmarkData;

    double m_fps;
};

#endif // BOOKMARKMODEL_H
