#ifndef GLOBALWATCHER_H
#define GLOBALWATCHER_H

#include <QObject>
#include <QHash>
#include "monitor.h"

class GlobalWatcher : public QObject
{
    Q_OBJECT

public:
    static GlobalWatcher *getInstance();
    bool registerUri(QString uri);
    void unregisterUri(QString uri);

    Monitor *get_monitor_by_uri (QString uri);

    /*!
     * \brief destroyNow
     * <br>
     * delete all monitor, this method might be deprecated soon.
     * </br>
     */
    void destroyNow();

private:
    explicit GlobalWatcher(QObject *parent = nullptr);
    ~GlobalWatcher();
    QHash<QString, Monitor*> *m_hash = nullptr;
};

#endif // GLOBALWATCHER_H
