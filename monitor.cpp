#include "monitor.h"

#include <QDebug>

static void file_changed_cb (GFileMonitor *g_file_monitor,
                             GFile *file,
                             GFile *other_file,
                             GFileMonitorEvent event_type,
                             Monitor *monitor)
{
    Q_UNUSED(g_file_monitor);
    Q_UNUSED(other_file);
    gchar *uri = g_file_get_uri (file);
    QString file_uri = uri;
    g_free (uri);
    //emit signal by event_type
    qDebug()<<"type:"<<event_type;
    switch (event_type) {
    case G_FILE_MONITOR_EVENT_CREATED:
        qDebug()<<"G_FILE_MONITOR_EVENT_CREATED";
        Q_EMIT monitor->fileCreated(file_uri);
        break;
    case G_FILE_MONITOR_EVENT_DELETED:
        qDebug()<<"G_FILE_MONITOR_EVENT_DELETED";
        Q_EMIT monitor->fileDeleted(file_uri);
        break;
    case G_FILE_MONITOR_EVENT_CHANGED:
        qDebug()<<"G_FILE_MONITOR_EVENT_CHANGED";
        Q_EMIT monitor->fileChanged(file_uri);
        break;
    default:
        break;
    }
}

Monitor::Monitor(QString uri, QObject *parent) : QObject(parent)
{
    GError *err = nullptr;
    m_uri = uri;
    qDebug()<<uri;
    m_monitored_file = g_file_new_for_uri(m_uri.toUtf8());
    m_file_monitor = g_file_monitor_directory(m_monitored_file,
                                              G_FILE_MONITOR_NONE,
                                              nullptr,
                                              &err);
    if (err) {
        m_is_valid = false;
        g_error_free(err);
        err = nullptr;
    } else {
        m_is_valid = true;
        //connect file changed signal
        m_handler_id = g_signal_connect (m_file_monitor, "changed", G_CALLBACK(file_changed_cb), this);
    }

    m_ref_count++;
}

Monitor::~Monitor()
{
    qDebug()<<"~monitor";
    g_signal_handler_disconnect(m_file_monitor, m_handler_id);
    g_file_monitor_cancel (m_file_monitor);
    g_object_unref(m_file_monitor);
    m_file_monitor = nullptr;
    g_object_unref(m_monitored_file);
}

void Monitor::ref()
{
    m_ref_count++;
}

void Monitor::unref()
{
    m_ref_count--;
}


