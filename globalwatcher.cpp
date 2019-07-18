#include "globalwatcher.h"
#include "monitor.h"

static GlobalWatcher *m_instance = nullptr;

GlobalWatcher::GlobalWatcher(QObject *parent) : QObject(parent)
{
    m_hash = new QHash<QString, Monitor*>();
}

GlobalWatcher::~GlobalWatcher()
{
    delete m_hash;
}

GlobalWatcher *GlobalWatcher::getInstance()
{
    if (m_instance == nullptr)
        m_instance = new GlobalWatcher;
    return m_instance;
}

void GlobalWatcher::destroyNow()
{
    /*
    m_instance->~GlobalWatcher();
    m_instance = nullptr;
    */
}

/*!
 * \brief GlobalWatcher::registerUri
 * \param uri
 * \return
 * \retval true if no err happend when g_file_monitor_directory
 * \retval false if can not monitor directory
 * \note
 * Even though Monitor was init, it doesn't mean we can monitor target uri all the time.
 * For example, a uncorrect uri will lead monitor get error when tring to monitor it.
 * some special uri, such as sftp://xx is aslo can not be monitor with gvfs and somehow limmted.
 * If monitor is not valid, we should not connect the signal to monitor at proxy instance.
 */
bool GlobalWatcher::registerUri(QString uri)
{
    m.lock();
    Monitor *monitor = m_hash->value(uri);
    if (monitor == nullptr) {
        monitor = new Monitor(uri);
        //ref = 1
        m_hash->insert(uri, monitor);
    } else {
        monitor->ref();
    }
    m.unlock();
    return monitor->isValid();
}

void GlobalWatcher::unregisterUri(QString uri)
{
    m.lock();
    Monitor *monitor = m_hash->value(uri);
    if (monitor) {
        monitor->unref();
        if (monitor->refCount() == 0)
            m_hash->remove(uri);
    }
    m.unlock();
}

/*!
 * \brief GlobalWatcher::get_monitor_by_uri
 * \param uri
 * \return monitor which monitor this uri.
 * \note You should use registerUri() before get monitor.
 * get_monitor_by_uri() won't change the ref count of monitor.
 */
Monitor *GlobalWatcher::get_monitor_by_uri(QString uri)
{
    return m_hash->value(uri);
}

