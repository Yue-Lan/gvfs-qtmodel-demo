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

void Model::setRoot(Info *rootInfo)
{
    beginResetModel();
    delete m_rootItem;
    m_rootItem = new Item(rootInfo, nullptr, this);
    m_rootItem->findChildren();
    //does m_root_item_children need use weak ref?
    m_root_item_children = m_rootItem->m_children;
    endResetModel();
}

Info *Model::infoFromIndex(const QModelIndex &index)
{
    Item *item = static_cast<Item*>(index.internalPointer());
    //item->m_info->querySync();
    return item->m_info;
}

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

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
    //qDebug()<<row<<column<<parent;
    if (column != 0)
        return QModelIndex();
    if (!parent.isValid()) {
        if (row < 0 || row > m_root_item_children->count()-1)
            return QModelIndex();
        return createIndex(row, column, m_root_item_children->at(row));
    }

    Item *parent_item = static_cast<Item*>(parent.internalPointer());
    //qDebug()<<parent_item->m_info->displayName();
    if (row < 0 && row >parent_item->m_children->count())
        return QModelIndex();
    if (parent_item->m_children)
        return parent_item->m_children->at(row)->index();
    else {
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
    return 1;
}

int Model::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_root_item_children->count();
    Item *item = static_cast<Item*>(parent.internalPointer());
    if (item->hasChildren())
        return item->m_children->count();
    /*
    //qDebug()<<item->m_info->displayName();
    if (item == m_rootItem)
        return m_root_item_children->count();
    else if (item->m_parent != nullptr) {
        qDebug()<<"has parent";
        return item->m_parent->m_children->count();
    }
    */
    return 0;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    Item *item = static_cast<Item*>(index.internalPointer());
    item->m_info->querySync();
    //qDebug()<<item->m_info->displayName();
    if (item->m_info == nullptr)
        return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        return item->m_info->displayName();
    case Qt::DecorationRole:
        return QVariant(QIcon::fromTheme(item->m_info->iconName()));
    default:
        break;
    }
    return QVariant();
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
        return tr("File Name");
    //if (role == Qt::DecorationRole)
    //    return QVariant(QIcon::fromTheme("folder"));
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool Model::hasChildren(const QModelIndex &parent) const
{
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
    //qDebug()<<"fetchMore"<<parent;
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
