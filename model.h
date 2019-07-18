#ifndef MODEL_H
#define MODEL_H

#include <QAbstractItemModel>
#include <QVector>

class Item;
class Info;

class Model : public QAbstractItemModel
{
    friend class Info;
    friend class Item;

    enum ColumnType {
        FileName,
        FileSize,
        FileType,
        ModifiedDate,
        Owner,
        Other
    };

public:
    explicit Model(QObject *parent = nullptr);
    ~Model() override;
    void setRoot(Info *rootInfo);

    Item *itemFromIndex(const QModelIndex &index) {return static_cast<Item*>(index.internalPointer());}
    Info *infoFromIndex(const QModelIndex &index);
    QModelIndex indexFromInfo(Info *info);
    QModelIndex indexFromItem(Item *item);
    Item *itemFromInfo(Info *info);


protected:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool hasChildren(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /*!
     * \brief canFetchMore
     * \param parent
     * \return
     * \retval true if tree view item has children and first expanded.
     * \retval false
     * <br>
     * QAbstractItemModel provide a lazy populate interface.
     * canFetchMore() is happening in tree view. If a parent index has children,
     * hasChildren() should return true. then tree view will set this parent index expandable.
     * if we expand the item, canFetchMore() will be called. we can dynamicly load our data after
     * set this method return true. After loading finished, we must set this method return fasle,
     * for telling the view data loading is done.
     * </br>
     * <br>
     * This desgin has a small defect. Once we expanded a parent index,
     * the data will not free until we delete them manually.
     * I know using custom view and model with a custom signal-control-mechanism can solve this problem,
     * but I don't want to let my model become sexceptional.
     * Just like QFileSystemModel does, I want my model perform well at all kinds of View,
     * whatever List, Icon, Tree or others.
     * </br>
     */
    bool canFetchMore(const QModelIndex &parent) const override;
    /*!
     * \brief fetchMore
     * \param parent
     * <br>
     * This method will shcedule when a parent index canFetchMore.
     * In this method, we usually load our data, and call beginInsertRows
     * to tell how many rows we has added into this index.
     * Then we need call endInsertRows(), and return. Actually, fetchMore just
     * tell model the new rowCount of parent index to control showing our newly data.
     * newly data will be shown after data() called, as same as other non-delayed data.
     * </br>
     */
    void fetchMore(const QModelIndex &parent) override;

private:
    QVector<Item*> *m_root_item_children = nullptr;
    Item *m_rootItem = nullptr;
};

#endif // MODEL_H
