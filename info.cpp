#include "info.h"

#include <QDebug>

Info::Info()
{

}

Info::~Info()
{
    g_object_unref(m_file);
    m_file = nullptr;
    g_object_unref(m_file_info);
    m_file_info = nullptr;
}

Info *Info::fromUri(QString uri)
{
    Info *info = new Info;
    info->m_file = g_file_new_for_uri(uri.toUtf8());
    info->m_parent = g_file_get_parent(info->m_file);
    info->m_is_remote = !g_file_is_native(info->m_file);
    info->m_file_info = g_file_query_info(info->m_file, "standard::*," G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    return info;
}

Info *Info::fromPath(QString path)
{
    Info *info = new Info;
    info->m_file = g_file_new_for_path(path.toUtf8());
    info->m_parent = g_file_get_parent(info->m_file);
    info->m_is_remote = !g_file_is_native(info->m_file);
    info->m_file_info = g_file_query_info(info->m_file, "standard::*," G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    return info;
}

Info *Info::fromGFile(GFile *file)
{
    Info *info = new Info;
    info->m_file = g_file_dup(file);
    info->m_parent = g_file_get_parent(info->m_file);
    info->m_is_remote = !g_file_is_native(info->m_file);
    info->m_file_info = g_file_query_info(info->m_file, "standard::*," G_FILE_ATTRIBUTE_ID_FILE,
                                          G_FILE_QUERY_INFO_NONE, nullptr,
                                          nullptr);
    return info;
}

Info *Info::fromGFileInfo(GFile *parent, GFileInfo *fileInfo)
{
    Info *info = new Info;
    info->m_parent = parent;
    info->m_file = g_file_get_child(parent, g_file_info_get_name(fileInfo));
    info->m_file_info = g_file_info_dup(fileInfo);
    return info;
}

void Info::querySync()
{
    if (m_is_loaded)
        return;
    if (m_file_info) {
        GFileType type = g_file_info_get_file_type (m_file_info);
        switch (type) {
        case G_FILE_TYPE_DIRECTORY:
            m_is_dir = true;
        case G_FILE_TYPE_MOUNTABLE:
            m_is_volume = true;
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

    m_is_loaded = true;
}
