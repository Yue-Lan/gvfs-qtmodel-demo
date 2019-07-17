#ifndef MOUNTDIALOG_H
#define MOUNTDIALOG_H

#include <QDialog>
#include <gio/gio.h>

namespace Ui {
class MountDialog;
}

class MountHelper;

class MountDialog : public QDialog
{
    friend class MountHelper;
    Q_OBJECT

public:
    explicit MountDialog(QWidget *parent = nullptr);
    ~MountDialog();

    void startMount(GFile *file);

    static GAsyncReadyCallback mount_callback(GObject *source_object,
                                              GAsyncResult *res,
                                              MountDialog *dlg);

    static void
    aborted_cb (GMountOperation *op,
                MountDialog *dlg);

    static void
    ask_question_cb (GMountOperation *op,
                     char *message,
                     char **choices,
                     MountDialog* dialog);

    static void
    ask_password_cb (GMountOperation *op,
                     const char      *message,
                     const char      *default_user,
                     const char      *default_domain,
                     GAskPasswordFlags flags,
                     MountDialog* dialog);


Q_SIGNALS:
    void mountFinished(int state);

private:
    Ui::MountDialog *ui;
    GFile *m_file = nullptr;
    GMountOperation *m_op = nullptr;
};

#endif // MOUNTDIALOG_H
