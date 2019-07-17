#include "item.h"
#include "info.h"
#include "model.h"

#include "mounthelper.h"

#include <QDebug>

Item::Item(Info *info, Item *parent_item, Model *model, QObject *parent) : QObject(parent)
{
    m_info = info;
    m_parent = parent_item;
    m_model = model;
    m_children = new QVector<Item*>();

    if (info->isDir() || info->isVolume())
        m_has_children = true;
}

Item::~Item()
{
    for (Item *item : *m_children)
        delete item;
    delete m_info;
    delete m_children;
}

void Item::findChildren()
{
    GError *err = nullptr;
    GFileEnumerator *enumerator = g_file_enumerate_children(m_info->m_file,
                                                            "standard::*," G_FILE_ATTRIBUTE_ID_FILE,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            nullptr, &err);
    if (err != nullptr) {
        qDebug()<<"is dir:"<<this->m_info->isDir()<<"is volume:"<<this->m_info->isVolume();
        qDebug()<<"code:"<<err->code<<"message"<<err->message;
        g_error_free(err);
        err = nullptr;
        g_object_unref(enumerator);
        if (this->m_info->isVolume())
            findVolumeChildren();
        else {
            qDebug()<<"mount enclosing volume";
            MountHelper *mount_helper = new MountHelper(this->m_info->m_file, this);
            mount_helper->showMountDialog();
            connect(mount_helper, &MountHelper::mountFinished, [=](int state){
                qDebug()<<"finish state:"<<state;
                //same as before
                GFileInfo *target_uri_info = g_file_query_info(this->m_info->m_file, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI,
                                                               G_FILE_QUERY_INFO_NONE, nullptr, nullptr);

                char *target_uri = g_file_info_get_attribute_as_string(target_uri_info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
                g_object_unref(target_uri_info);

                GFile *target_location = nullptr;
                //failed to mount. I need add Mount Interaction later.
                if (!target_uri) {
                    target_location = G_FILE (g_object_ref (this->m_info->m_file));
                } else {
                    target_location = g_file_new_for_uri(target_uri);
                }

                g_free(target_uri);

                this->m_model->setRoot(Info::fromGFile(target_location));
            });
            qDebug()<<"wait async";
            //mount enclosing volume, and find children.
        }
    } else {
        GFileInfo *ginfo;
        ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);

        while (ginfo) {
            //qDebug()<<g_file_info_get_name(ginfo);
            Info *info = Info::fromGFileInfo(m_info->m_file, ginfo);
            info->querySync();
            Item *item = new Item(info, this, this->m_model, nullptr);
            m_children->append(item);
            ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);
        }
        g_object_unref(enumerator);
    }
    m_had_expanded = true;
}

/*!
 * \brief Item::index
 * \return index
 * \retval QModelIndex of this item,
 * <br>
 * index binds with the model which we use to initialize this instance.
 * </br>
 */
QModelIndex Item::index()
{
    return m_model->indexFromItem(this);
}

GAsyncReadyCallback Item::mounted_callback(GObject *source_object,
                                           GAsyncResult *res,
                                           Item *item)
{
    GError *err = nullptr;
    g_file_mount_mountable_finish(G_FILE (source_object), res, &err);
    if (err) {
        qDebug()<<err->message;
        g_error_free(err);
    }
    //TODO: add exception handling
    GFileInfo *target_uri_info = g_file_query_info(item->m_info->m_file, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI,
                                                   G_FILE_QUERY_INFO_NONE, nullptr, nullptr);

    char *target_uri = g_file_info_get_attribute_as_string(target_uri_info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
    g_object_unref(target_uri_info);

    //failed to mount. I need add Mount Interaction later.
    if (!target_uri)
        return nullptr;

    GFile *target_location = g_file_new_for_uri(target_uri);
    g_free(target_uri);

    GFileEnumerator *enumerator = g_file_enumerate_children(target_location,
                                                            "standard::*," G_FILE_ATTRIBUTE_ID_FILE,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            nullptr, nullptr);

    GFileInfo *ginfo;
    ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);

    while (ginfo) {
        //qDebug()<<g_file_info_get_name(ginfo);
        Info *info = Info::fromGFileInfo(target_location, ginfo);
        info->querySync();
        Item *child_item = new Item(info, item, item->m_model, nullptr);
        item->m_children->append(child_item);
        ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);
    }
    g_object_unref(enumerator);
    g_object_unref(target_location);
    item->m_model->fetchMore(item->index());
    item->m_had_expanded = true;
}

void Item::findVolumeChildren()
{
    g_file_mount_mountable(m_info->m_file, G_MOUNT_MOUNT_NONE,
                           nullptr, nullptr, GAsyncReadyCallback(mounted_callback), this);
}
