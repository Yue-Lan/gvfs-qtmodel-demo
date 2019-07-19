#include "item.h"
#include "info.h"
#include "model.h"

#include "mounthelper.h"

#include <QMessageBox>

#include <QDebug>

Item::Item(std::shared_ptr<Info> info, Item *parent_item, Model *model, QObject *parent) : QObject(parent)
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
    m_info.reset();
    delete m_children;
}

void Item::findChildren()
{
    GError *err = nullptr;
    GFile *dir = (m_info->m_target_file != nullptr)?(m_info->m_target_file):(m_info->m_file);
    GFileEnumerator *enumerator = g_file_enumerate_children(dir,
                                                            "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            nullptr, &err);
    if (err != nullptr) {
        qDebug()<<"is dir:"<<this->m_info->isDir()<<"is volume:"<<this->m_info->isVolume();
        qDebug()<<"code:"<<err->code<<"message"<<err->message;
        int code = err->code;
        g_error_free(err);
        err = nullptr;
        g_object_unref(enumerator);
        if (this->m_info->isVolume() && code == G_IO_ERROR_NOT_DIRECTORY)
            findVolumeChildren();
        else if (code == G_IO_ERROR_NOT_MOUNTED) {
            //mountEnclosingVolume();
        } else {
            QMessageBox *msg_box = new QMessageBox;
            //how about path?
            char *uri = g_file_get_uri(m_info->m_file);
            msg_box->critical(msg_box, tr("error"), tr("can not load: ")+uri);
            delete uri;
            return;
        }
    } else {
        GFileInfo *ginfo;
        ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);

        while (ginfo) {
            //qDebug()<<g_file_info_get_name(ginfo);
            auto info = Info::fromGFileInfo(m_info->m_file, ginfo);
            info->querySync();
            Item *item = new Item(info, this, this->m_model, nullptr);
            m_children->append(item);
            g_object_unref (ginfo);
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
    item->m_info->m_target_file = target_location;
    g_free(target_uri);

    GFileEnumerator *enumerator = g_file_enumerate_children(target_location,
                                                            "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            nullptr, nullptr);

    GFileInfo *ginfo;
    ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);

    while (ginfo) {
        //qDebug()<<g_file_info_get_name(ginfo);
        auto info = Info::fromGFileInfo(target_location, ginfo);
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

void Item::mountEnclosingVolumeAndGoTo()
{
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
            //target_location = G_FILE (g_object_ref (this->m_info->m_file));
            target_location = m_info->m_file;
        } else {
            target_location = g_file_new_for_uri(target_uri);
            m_info->m_target_file = target_location;
        }

        g_free(target_uri);
        /*!
          \note
          Maybe I need use enumerator, but it never happend in tree view. Because we
          should mount enclosing volume first, then volume would show in computer. At
          that time, we can expand it directly.
        */
        this->m_model->setRoot(Info::fromGFile(target_location));
    });
    qDebug()<<"wait async";
    //mount enclosing volume, and set this location as model root.
}

void Item::mountEnclosingVolumeAndFetchMore()
{
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
            //target_location = G_FILE (g_object_ref (this->m_info->m_file));
            target_location = m_info->m_file;
        } else {
            target_location = g_file_new_for_uri(target_uri);
            m_info->m_target_file = target_location;
        }

        g_free(target_uri);
        /*!
          \note
          Maybe I need use enumerator, but it never happend in tree view. Because we
          should mount enclosing volume first, then volume would show in computer. At
          that time, we can expand it directly.
        */
        this->m_model->fetchMore(this->index());
    });
    qDebug()<<"wait async";
}

/*!
 * \brief Item::prepareForSetRoot
 * \return
 * \note
 * some item has children, but they can not reach directly.
 * We need do some special to let item can find its children correctly.
 * I may need to redesign the process of setRoot()
 */
bool Item::prepareForSetRoot()
{
    //i used sync enumerate method, so i can judge if it was prepared by children count.
    //i will used async method latter.
    if (m_children->count() > 0)
        return true;
    GError *err = nullptr;
    GFileEnumerator *enumerator = g_file_enumerate_children(m_info->m_file,
                                                            G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            nullptr, &err);
    if (err != nullptr) {
        qDebug()<<"is dir:"<<this->m_info->isDir()<<"is volume:"<<this->m_info->isVolume();
        qDebug()<<"code:"<<err->code<<"message"<<err->message;
        g_error_free(err);
        err = nullptr;
        g_object_unref(enumerator);
        qDebug()<<"not prepare";
        //i need do some err handling here,
        //at least we need to mount volume and get the real uri of item.
        //we should setRoot after we handled the problem, otherwise problems may occur.
        //but aslo remember dealing with the tree view volume expanding is necessary.
        //i'm not intent to change findChildren() method.
        //maybe enclosing volume handle need move here.
        //err handle always an async job.
        return false;
    }
    return true;
}

GAsyncReadyCallback Item::err_handle_mount_volume_callback(GObject *source_object,
                                                           GAsyncResult *res,
                                                           Item *item)
{
    GError *err = nullptr;
    g_file_mount_mountable_finish(G_FILE (source_object), res, &err);
    if (err) {
        qDebug()<<err->code<<err->message;
        int code = err->code;
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
                                                            "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                                            G_FILE_QUERY_INFO_NONE,
                                                            nullptr, nullptr);

    GFileInfo *ginfo;
    ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);

    while (ginfo) {
        //qDebug()<<g_file_info_get_name(ginfo);
        auto info = Info::fromGFileInfo(target_location, ginfo);
        info->querySync();
        Item *child_item = new Item(info, item, item->m_model, nullptr);
        item->m_children->append(child_item);
        ginfo = g_file_enumerator_next_file (enumerator, nullptr, nullptr);
    }
    g_object_unref(enumerator);
    g_object_unref(target_location);

    item->model()->beginResetModel();
    item->model()->m_rootItem = item;
    item->model()->m_root_item_children = item->m_children;
    item->model()->endResetModel();
}

/*!
 * \brief Item::handleErrorAndResetModelAsync
 * <br>
 * This is happening when model location changing,
 * but something error need to be handled at first.
 * </br>prepareForSetRoot
 * \see Item::prepareForSetRoot(), Model::setRoot()
 */
void Item::handleErrorAndResetModelAsync()
{
    //FIXME: volume and directory in network:/// need be handled correctly.
    //TODO: if enclosing volume has mount, do not show mount dialog again.
    if (m_info->isVolume()) {
        //is mounted
        if (m_info->m_target_file) {
            std::shared_ptr<Info> target_info = Info::fromGFile(m_info->m_target_file);
            m_model->setRoot(target_info);
        } else {
            g_file_mount_mountable(m_info->m_file,
                                   G_MOUNT_MOUNT_NONE,
                                   nullptr,
                                   nullptr,
                                   GAsyncReadyCallback (err_handle_mount_volume_callback),
                                   this);
        }
    } else {
        mountEnclosingVolumeAndGoTo();
    }
}


