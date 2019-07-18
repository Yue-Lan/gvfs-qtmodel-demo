#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <gio/gio.h>

class Monitor : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Monitor
     * <br>
     * Monitor is a wrapper of GFileMonitor. It will help connect the file changed signals,
     * and then 'translate' them to Q_SIGNALS. Proxy connects these signals for synchronizing.
     * </br>
     * \param uri
     * <br>
     * Target uri to monitor.
     * </br>
     * \param parent
     */
    explicit Monitor(QString uri, QObject *parent = nullptr);
    bool isValid() {return m_is_valid;}
    /*!
     * \brief ref
     * <br>
     * ref count is used for manage the globalwatcher hash map,
     * proxy use GlobalWatcher::registerUri() to tell watcher new a monitor,
     * or ref an existed one.
     * \see GlobalWatcher::registerUri()
     * </br>
     */
    void ref();
    /*!
     * \brief unref
     * <br>
     * ref count is used for manage the globalwatcher hash map,
     * proxy use GlobalWatcher::unregisterUri() to tell watcher it doesn't
     * subcribe this monitor now.
     * When all proxy unregister this monitor, it will be remove from global hash.
     * \see GlobalWatcher::unregisterUri()
     * </br>
     */
    void unref();
    int refCount() {return m_ref_count;}
    ~Monitor();

private:
    QString m_uri = nullptr;
    GFile *m_monitored_file = nullptr;
    GFileMonitor *m_file_monitor = nullptr;
    bool m_is_valid = false;

    int m_ref_count = 0;
    gulong m_handler_id = 0;

Q_SIGNALS:
    void fileCreated(QString file_uri);
    void fileDeleted(QString file_uri);
    void fileChanged(QString file_uri);
};

#endif // MONITOR_H
