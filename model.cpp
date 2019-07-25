#include "model.h"

#include "info.h"
#include "item.h"

#include <QIcon>
#include <QDebug>

Model::Model(QObject *parent) : QAbstractItemModel (parent)
{

}

Model::~Model()
{
    delete m_rootItem;

    //QAbstractItemModel::~QAbstractItemModel();
}

void Model::setRoot(std::shared_ptr<Info> rootInfo)
{
    beginResetModel();
    /*!
      \note
      if we delete root item here, we can not use infoFromIndex() for doubleclicked
      location changing in icon view.
      for now i use std::share_ptr manage info.
     */
    delete m_rootItem;
    auto info = rootInfo;
    info->querySync();
    m_rootItem = new Item(info, nullptr, this);
    if (m_rootItem->prepareForSetRoot()) {
        beginResetModel();
        m_rootItem->findChildren();
        m_root_item_children = m_rootItem->m_children;
        endResetModel();
    } else {
        beginResetModel();
        m_root_item_children = m_rootItem->m_children;
        endResetModel();
        //handleErrorAndResetModelAsync() is an async method,
        //and it will recall setRoot when err handle done.
        m_rootItem->handleErrorAndResetModelAsync();
    }
    //does m_root_item_children need use weak ref?


}

std::shared_ptr<Info> Model::infoFromIndex(const QModelIndex &index)
{
    Item *item = static_cast<Item*>(index.internalPointer());
    //item->m_info->querySync();
    return item->m_info;
}

/*!
 * \brief Model::indexFromItem
 * \param item
 * \return first column index of model item.
 * \deprecated I'm not sure if I will use it,
 * it may work well in list view. But, in other view,
 * there might be more than one columns, so there are
 * several index at one row.
 */
QModelIndex Model::indexFromItem(Item *item)
{
    if (item == m_rootItem) {
        for (int i = 0; i < m_root_item_children->count(); i++) {
            if (m_root_item_children->at(i) == item)
                return createIndex(i, 0, item);
        }
        return QModelIndex();
    } else if (item->m_parent) {
        if (item->m_parent->m_children) {
            for (int i = 0; i < item->m_parent->m_children->count(); i++) {
                if (item->m_parent->m_children->at(i) == item)
                    return createIndex(i, 0, item);
            }
        }
    }
    return QModelIndex();
}

/*!
  \note
  <br>
  Children item must return index correctly. For now, it just return first row index.
  So, other columns of children won't be shown.
  </br>
*/
QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
    /*children of root item*/
    //qDebug()<<row<<column<<parent;
    if (!parent.isValid()) {
        if (row < 0 || row > m_root_item_children->count()-1)
            return QModelIndex();
        return createIndex(row, column, m_root_item_children->at(row));
        //do I need createIndex by column?
    }

    /*children of item which has parent*/
    Item *parent_item = static_cast<Item*>(parent.internalPointer());
    //qDebug()<<parent_item->m_info->displayName();
    if (row < 0 && row >parent_item->m_children->count())
        return QModelIndex();
    if (column < 0 && column > ModifiedDate)
        return QModelIndex();
    if (parent_item->m_children) {
        //should indexFromItem be de deprecated?
        //return parent_item->m_children->at(row)->index();
        return createIndex(row, column, parent_item->m_children->at(row));
    } else {
        return QModelIndex();
    }
}

QModelIndex Model::parent(const QModelIndex &child) const
{
    Item *item = static_cast<Item*>(child.internalPointer());
    if (item->m_parent)
        return item->m_parent->index();
    return QModelIndex();
}

int Model::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColumnType::ModifiedDate+1;
}

int Model::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_root_item_children->count();
    Item *item = static_cast<Item*>(parent.internalPointer());
    if (item->hasChildren())
        return item->m_children->count();
    return 0;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    Item *item = static_cast<Item*>(index.internalPointer());
    item->m_info->querySync();
    //qDebug()<<item->m_info->displayName()<<index.row()<<index.column()<<role;
    if (item->m_info == nullptr)
        return QVariant();

    switch (index.column()) {
    case FileName:
        switch (role) {
        case Qt::DisplayRole:
            return item->m_info->displayName();
        case Qt::DecorationRole:
            return QVariant(QIcon::fromTheme(item->m_info->iconName()));
        default:
            return QVariant();
        }
    case FileSize:
        if (role == Qt::DisplayRole && !(item->hasChildren()))
            return item->m_info->fileSize();
        return QVariant();
    case FileType:
        //qDebug()<<item->m_info->displayName()<<item->m_info->fileType();
        if (role == Qt::DisplayRole)
            return item->m_info->fileType();
        return QVariant();
    case ModifiedDate:
        if (role == Qt::DisplayRole)
            return item->m_info->modifiedDate();
        return QVariant();
    default:
        return QVariant();
    }
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();
    if (role == Qt::DisplayRole) {
        switch (section) {
        case FileName:
            return tr("File Name");
        case FileSize:
            return tr("File Size");
        case FileType:
            return tr("File Type");
        case ModifiedDate:
            return tr("Modified Date");
        default:
            return QVariant();
        }
    }
    //if (role == Qt::DecorationRole)
    //    return QVariant(QIcon::fromTheme("folder"));
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool Model::hasChildren(const QModelIndex &parent) const
{
    /*root children index doesn't have a parent*/
    if (!parent.isValid())
        return true;
    Item *item = static_cast<Item*>(parent.internalPointer());
    if (item->hasChildren())
        return true;
    return false;
}

bool Model::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return false;
    Item *item = static_cast<Item*>(parent.internalPointer());
    //qDebug()<<"can fetch more"<<item->m_info->displayName()<<(!item->m_had_expanded && item->hasChildren());
    if (!item->m_had_expanded && item->hasChildren())
        return true;
    return false;
}

void Model::fetchMore(const QModelIndex &parent)
{
    qDebug()<<"fetchMore"<<parent.column();
    if (!parent.isValid())
        return;
    Item *parent_item = static_cast<Item*>(parent.internalPointer());
    //for unmounted volume, we need insert rows async, when data load finished,
    //I call fetchMore() again in item.
    if (parent_item->m_children->count() > 0) {
        beginInsertRows(parent, 0, parent_item->m_children->size());
        //once we have fetched more, we can not fetch more again.
        endInsertRows();
        return;
    }
    parent_item->findChildren();
}

Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}
