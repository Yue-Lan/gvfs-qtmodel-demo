#include "mainwindow.h"
#include <QApplication>
#include <QListView>
#include <QTreeView>
#include <QSplitter>

#include "info.h"
#include "model.h"

#include <QLineEdit>
#include <QLayout>

#include <QDebug>

int main(int argc, char *argv[])
{
    QIcon::setThemeName("ukui-icon-theme");
    QApplication a(argc, argv);
    /*
    Info *i = Info::fromUri("file:///");
    i->querySync();

    QListView v;
    Model m(0);
    m.setRoot(i);
    v.setModel(&m);
    v.show();
    v.setViewMode(QListView::IconMode);
    v.setSelectionMode(QAbstractItemView::ExtendedSelection);
    v.setDragDropMode(QAbstractItemView::DragDrop);
    v.setDragEnabled(true);

    QTreeView tv;
    tv.setModel(&m);
    tv.show();
    qDebug()<<i->displayName()<<"icon:"<<i->iconName()<<"id:"<<i->fileID();
    */
    QLineEdit *edit = new QLineEdit;;
    edit->setText("computer:///");
    edit->show();
    QObject::connect(edit, &QLineEdit::returnPressed, [=](){
        qDebug()<<"goto";
        Info *info = Info::fromUri(edit->text());

        QSplitter *splitter = new QSplitter;
        splitter->setOrientation(Qt::Horizontal);
        QTreeView *v = new QTreeView;
        v->setSortingEnabled(true);
        QListView *v1 = new QListView;
        v1->setViewMode(QListView::IconMode);
        v1->setSelectionMode(QAbstractItemView::ExtendedSelection);
        v1->setDragDropMode(QAbstractItemView::DragDrop);
        v1->setDragEnabled(true);
        splitter->addWidget(v);
        splitter->addWidget(v1);
        Model *m = new Model(splitter);
        m->setRoot(info);
        v->setModel(m);
        v1->setModel(m);
        splitter->show();
    });

    //delete i;
    MainWindow w;
    w.setFixedSize(600, 480);
    w.layout()->addWidget(edit);
    w.show();

    return a.exec();
}
