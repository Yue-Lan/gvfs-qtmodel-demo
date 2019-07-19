#include "info.h"

#include <QDebug>
#include <QDateTime>

Info::Info()
{

}

Info::~Info()
{
    qDebug()<<"~Info";
    g_object_unref(m_parent);
    m_parent = nullptr;
    g_object_unref(m_file);
    m_file = nullptr;
    g_object_unref(m_file_info);
    m_file_info = nullptr;
}

std::shared_ptr<Info> Info::fromUri(QString uri)
{
    std::shared_ptr<Info> p = std::make_shared<Info>();
    Info *info = p.get();
    info->m_file = g_file_new_for_uri(uri.toUtf8());
    info->m_parent = g_file_get_parent(info->m_file);
    info->m_is_remote = !g_file_is_native(info->m_file);
    info->m_file_info = g_file_query_info(info->m_file,
                                          "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    return p;
}

std::shared_ptr<Info> Info::fromPath(QString path)
{
    std::shared_ptr<Info> p = std::make_shared<Info>();
    Info *info = p.get();
    info->m_file = g_file_new_for_path(path.toUtf8());
    info->m_parent = g_file_get_parent(info->m_file);
    info->m_is_remote = !g_file_is_native(info->m_file);
    info->m_file_info = g_file_query_info(info->m_file,
                                          "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    return p;
}

std::shared_ptr<Info> Info::fromGFile(GFile *file)
{
    std::shared_ptr<Info> p = std::make_shared<Info>();
    Info *info = p.get();
    info->m_file = g_file_dup(file);
    info->m_parent = g_file_get_parent(info->m_file);
    info->m_is_remote = !g_file_is_native(info->m_file);
    info->m_file_info = g_file_query_info(info->m_file,
                                          "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    return p;
}

std::shared_ptr<Info> Info::fromGFileInfo(GFile *parent, GFileInfo *fileInfo)
{
    std::shared_ptr<Info> p = std::make_shared<Info>();
    Info *info = p.get();
    info->m_parent = parent;
    info->m_file = g_file_get_child(parent, g_file_info_get_name(fileInfo));
    //info->m_file_info = g_file_info_dup(fileInfo);
    info->m_file_info = g_file_query_info(info->m_file,
                                          "standard::*," G_FILE_ATTRIBUTE_TIME_MODIFIED G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    info->querySync();
    return p;
}

void Info::querySync()
{
    if (m_is_loaded)
        return;
    if (m_file_info) {
        GFileType type = g_file_info_get_file_type (m_file_info);
        switch (type) {
        case G_FILE_TYPE_DIRECTORY:
            //qDebug()<<"dir";
            m_is_dir = true;
            break;
        case G_FILE_TYPE_MOUNTABLE:
            //qDebug()<<"mountable";
            m_is_volume = true;
            break;
        default:
            break;
        }
    }
    m_display_name = QString (g_file_info_get_display_name(m_file_info));
    GIcon *g_icon = g_file_info_get_icon (m_file_info);
    const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
    if (icon_names)
        m_icon_name = QString (*icon_names);
    //qDebug()<<m_display_name<<m_icon_name;
    m_file_id = g_file_info_get_attribute_string(m_file_info, G_FILE_ATTRIBUTE_ID_FILE);

    m_content_type = g_file_info_get_content_type (m_file_info);
    m_size = g_file_info_get_attribute_uint64(m_file_info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
    m_modified_time = g_file_info_get_attribute_uint64(m_file_info, G_FILE_ATTRIBUTE_TIME_MODIFIED);

    /*!
      \note
      g_content_type_get_description is an allocated str, and I have to deal it with memcheck.
      maybe use smart ptr?
    */
    char *content_type = g_content_type_get_description (g_file_info_get_content_type (m_file_info));
    m_file_type = content_type;
    g_free (content_type);
    content_type = nullptr;

    m_file_size = g_format_size_full(m_size, G_FORMAT_SIZE_LONG_FORMAT);

    QDateTime date = QDateTime::fromMSecsSinceEpoch(m_modified_time*1000);
    m_modified_date = date.toString(Qt::SystemLocaleShortDate);

    m_is_loaded = true;
}
