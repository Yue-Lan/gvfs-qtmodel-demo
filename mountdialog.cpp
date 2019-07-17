#include "mountdialog.h"
#include "ui_mountdialog.h"

#include <QMessageBox>
#include <QPushButton>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include <QDebug>

MountDialog::MountDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MountDialog)
{
    ui->setupUi(this);
    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    connect(ui->checkBox, &QCheckBox::clicked, [=](bool checked){
        if (checked) {
            ui->pwd_edit->clear();
            ui->pwd_edit->setEnabled(false);
            ui->usrname_edit->clear();
            ui->usrname_edit->setEnabled(false);
        } else {
            ui->pwd_edit->setEnabled(true);
            ui->usrname_edit->setEnabled(true);
        }
    });
}

MountDialog::~MountDialog()
{
    g_object_unref(m_file);
    g_object_unref(m_op);
    delete ui;
}

GAsyncReadyCallback MountDialog::mount_callback(GObject *source_object,
                                   GAsyncResult *res,
                                   MountDialog *dlg)
{
    qDebug()<<"mount op finished";
    GFile *file = G_FILE (source_object);
    GError *err = nullptr;
    g_file_mount_enclosing_volume_finish (file, res, &err);

    if (err) {
        qDebug()<<err->code<<err->message<<err->domain;
        g_error_free(err);
    }

    Q_EMIT dlg->mountFinished(0);
    return nullptr;
}

void MountDialog::
ask_question_cb (GMountOperation *op,
                 char *message,
                 char **choices,
                 MountDialog* dialog)
{
    qDebug()<<"ask question cb:"<<message;
    Q_UNUSED(dialog);
    QMessageBox *msg_box = new QMessageBox;
    msg_box->setText(message);
    char **choice = choices;
    int i = 0;
    while (*choice) {
        qDebug()<<*choice;
        QPushButton *button = msg_box->addButton(QString(*choice), QMessageBox::ActionRole);
        connect(button, &QPushButton::clicked, [=](){
            g_mount_operation_set_choice(op, i);
        });
        *choice++;
        i++;
    }
    msg_box->exec();
    qDebug()<<"msg_box done";
    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

void MountDialog::
ask_password_cb (GMountOperation *op,
                 const char      *message,
                 const char      *default_user,
                 const char      *default_domain,
                 GAskPasswordFlags flags,
                 MountDialog* dlg)
{
    qDebug()<<"ask_password_cb:"<<flags;
    Q_UNUSED(message);
    Q_UNUSED(default_user);
    Q_UNUSED(default_domain);

    if (flags & G_ASK_PASSWORD_NEED_USERNAME)
    {
        qDebug()<<"G_ASK_PASSWORD_NEED_USERNAME";
        g_mount_operation_set_username (op, dlg->ui->usrname_edit->text().toUtf8());
    }

    if (flags & G_ASK_PASSWORD_NEED_PASSWORD)
    {
        qDebug()<<"G_ASK_PASSWORD_NEED_PASSWORD";
        g_mount_operation_set_password (op, dlg->ui->pwd_edit->text().toUtf8());
    }

    if (flags & G_ASK_PASSWORD_NEED_DOMAIN) {
        qDebug()<<"need domain";
    }

    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

void MountDialog::
aborted_cb (GMountOperation *op,
            MountDialog *dlg)
{
    qDebug()<<"aborted_cb";
    g_object_unref(op);
    dlg->deleteLater();
}

void MountDialog::startMount(GFile *file)
{
    qDebug()<<"dlg::startMount";
    m_file = G_FILE (g_object_ref(file));
    m_op = g_mount_operation_new();
    g_file_mount_enclosing_volume(m_file, G_MOUNT_MOUNT_NONE,
                                  m_op, nullptr, GAsyncReadyCallback(mount_callback), this);

    g_signal_connect (m_op, "ask-question", G_CALLBACK(ask_question_cb), this);
    g_signal_connect (m_op, "ask-password", G_CALLBACK (ask_password_cb), this);
    g_signal_connect (m_op, "aborted", G_CALLBACK (aborted_cb), this);

}
