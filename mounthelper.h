#ifndef MOUNTHELPER_H
#define MOUNTHELPER_H

#include <QObject>
#include <gio/gio.h>

class MountDialog;

class MountHelper : public QObject
{
    friend class MountDialog;
    Q_OBJECT
public:
    explicit MountHelper(GFile *file, QObject *parent = nullptr);
    ~MountHelper();

Q_SIGNALS:
    void mountFinished(int state);

public Q_SLOTS:
    void showMountDialog();
    void startMount();

private:
    GFile *m_file = nullptr;
    //GMountOperation *m_op = nullptr;
    MountDialog *m_dlg = nullptr;
};

#endif // MOUNTHELPER_H
