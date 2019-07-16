#ifndef ITEM_H
#define ITEM_H

#include <QObject>
#include <QVector>
#include <gio/gio.h>

class Info;
class Model;

class Item : public QObject
{
    friend class Info;
    friend class Model;
    Q_OBJECT
public:
    /*!
     * \brief Item
     * \param info
     * \param parent_item
     * \param model
     * <br>
     * model is not nullable
     * </br>
     * \param parent
     */
    explicit Item(Info *info, Item *parent_item = nullptr, Model *model = nullptr, QObject *parent = nullptr);
    ~Item();

    bool operator==(const Item &item) {
        return this->m_info == item.m_info;
    }

    void findChildren();
    bool hasChildren(){return m_has_children;}
    QModelIndex index();

Q_SIGNALS:
    void fileAdded(Item *addedFileItem);
    void fileRemoved(Item *removedFileItem);
    void fileChanged(Item *changedItem);

protected:
    /*!
     * \brief findVolumeChildren
     * <br>
     * This will call by findChildren() if parent is a volume.
     * Normally, an item which has children is almost present a
     * folder, but not all.
     * </br>
     * <br>
     * In gvfs, a volume which has been mounted can find a target uri.
     * to find a volume children, first we need get the GFile handle of
     * this target uri.
     * </br>
     * <br>
     * If a volume is not mounted yet, we need implemet GMountOperation for
     * mounting a volume.
     * </br>
     */
    void findVolumeChildren();

private:
    static GAsyncReadyCallback mounted_callback(GObject *source_object,
                                         GAsyncResult *res,
                                         Item *item);
    /*!
     * \brief m_has_children is a convinience value for check if item has children.
     * <br>
     * i use this value because in my desgin, even though the item has children,
     * it might not enumerator them at once. i will let the enumerate when it is real needed.
     * </br>
     */
    bool m_has_children = false;
    /*!
     * \brief m_had_expanded is a helper for lazy enumerating.
     * <br>
     * \value true, need enumerate children.
     * </br>
     * <br>
     * \value false, do not enumerate, even there are children.
     * </br>
     */
    bool m_had_expanded = false;
    Info *m_info = nullptr;
    Item *m_parent = nullptr;

    Model *m_model;
    QVector<Item*> *m_children = nullptr;
};

#endif // ITEM_H
