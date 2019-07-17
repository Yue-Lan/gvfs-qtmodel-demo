#include "mounthelper.h"

#include <QDialog>
#include <QDebug>

#include "mountdialog.h"

MountHelper::MountHelper(GFile *file, QObject *parent) : QObject(parent)
{
    m_dlg = new MountDialog;
    m_file = G_FILE(g_object_ref(file));
    //m_op = g_mount_operation_new();
    connect(m_dlg, &QDialog::accepted, this, &MountHelper::startMount);
    connect(m_dlg, &QDialog::rejected, this, &MountHelper::deleteLater);
    connect(m_dlg, &MountDialog::mountFinished, this, &MountHelper::mountFinished);
}

void MountHelper::showMountDialog()
{
    m_dlg->exec();
}

MountHelper::~MountHelper()
{
    delete m_dlg;
    //g_object_unref(m_op);
    g_object_unref(m_file);
}

void MountHelper::startMount()
{
    qDebug()<<"helper:: start mount";
    m_dlg->setEnabled(false);
    m_dlg->startMount(m_file);
    //start mount
}
