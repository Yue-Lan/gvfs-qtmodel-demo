#ifndef INFO_H
#define INFO_H

#include <QString>
#include <gio/gio.h>

class Item;

class Info
{
    friend class Item;
public:
    Info();
    ~Info();

    bool operator==(const Info &i) {
        if (i.m_file_id == nullptr)
            return false;
        if (this->m_file_id == i.m_file_id)
            return true;
        return false;
    }

    static Info* fromPath(QString path);
    static Info* fromUri(QString uri);
    static Info* fromGFile(GFile *file);
    static Info* fromGFileInfo(GFile *parent, GFileInfo *fileInfo);

    bool isVaild() {return m_is_valid;}
    bool isDir() {return m_is_dir;}
    bool isVolume() {return m_is_volume;}
    bool isRemote() {return m_is_remote;}
    bool isLoaded() {return m_is_loaded;}
    QString displayName() {return m_display_name;}
    QString iconName() {return m_icon_name;}
    QString fileID() {return m_file_id;}

    void querySync();
    //void queryAsync();

private:
    bool m_is_valid = false;
    bool m_is_dir = false;
    bool m_is_volume = false;
    bool m_is_remote = false;

    bool m_is_loaded = false;

    QString m_display_name = nullptr;
    QString m_icon_name = nullptr;
    QString m_file_id = nullptr;

    GFile *m_file = nullptr;
    GFile *m_parent = nullptr;
    //I use id::file attribute string to compare between two infos.
    /*!
     * \brief m_target_file
     * <br>
     * This is a GFile handle for Item::findVolumeChildren.
     * </br>
     * \see Item::findVolumeChildren()
     */
    GFile *m_target_file = nullptr;
    GFileInfo *m_file_info = nullptr;
};

#endif // INFO_H
