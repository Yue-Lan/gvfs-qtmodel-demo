#include "mainwindow.h"
#include <QApplication>
#include <QListView>
#include <QTreeView>
#include <QSplitter>

#include "info.h"
#include "model.h"

#include "item.h"

#include "globalwatcher.h"
#include "monitor.h"

#include <QLineEdit>
#include <QLayout>

#include <QDebug>

int main(int argc, char *argv[])
{
    QIcon::setThemeName("ukui-icon-theme");
    QApplication a(argc, argv);

    GlobalWatcher *watcher = GlobalWatcher::getInstance();
    watcher->registerUri("file:///home/lanyue");

    Monitor *monitor = watcher->get_monitor_by_uri("file:///home/lanyue");
    if (monitor->isValid()) {
        qDebug()<<"valid";
    }
    QObject::connect(monitor, &Monitor::fileCreated, [=](QString uri){
        qDebug()<<"file created"<<uri;
    });
    QObject::connect(&a, &QApplication::destroyed, [=](){
        delete monitor;
    });

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
    MainWindow w;
    QLineEdit *edit = new QLineEdit(&w);
    edit->setText("computer:///");
    edit->show();
    QObject::connect(edit, &QLineEdit::returnPressed, [=](){
        qDebug()<<"goto";
        auto info = Info::fromUri(edit->text());

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
        QObject::connect(v1, &QListView::doubleClicked, [=](const QModelIndex &index){
            Item *item = static_cast<Item*>(index.internalPointer());
            Model *model = item->model();
            auto info = model->infoFromIndex(index);
            model->setRoot(info);
        });
    });
    //delete i;
    w.setFixedSize(600, 480);
    w.layout()->addWidget(edit);
    w.show();

    return a.exec();
}
