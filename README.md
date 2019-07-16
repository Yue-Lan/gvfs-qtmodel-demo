# gvfs-qtmodel-demo
This demo shows how to combine gvfs with qt model.

# detials
## how to build
It is simply to build this project in qt-creator. Also you can just use qmake with commandline. If in debian or ubuntu, I recommend you install these packages.
- qt5-default
- qt-creator
- libglib2.0-dev

Then you can configure, build run this demo.

## what's the use of it?
Qt Model/View Programming is an advanced modern architecture. It is similar to MVC, which is often use in data control and showing of huge project. In Qt Model/View Programming, we have had an framework to implement our model/view-design project. It doesn't need spend a lot of time on the underlying design.

If you're familiar with Qt Programing, you should know that Qt has provided a model called QFileSystemModel for local file system. This model is very 'powerful', and you can find many example about how to use it. However, QFileSystemModel doesn't support some important function, just like trash and samba, etc. It is because trash:/// and samba:/// is not a real file system. They are provided by some progress, we usually call them and their provider as virtual file system (VFS).

The most common VFS now is a part of glib/gio, which was called gvfs before. kio also providing a VFS, but I think it is not very unified comparing with gio. I wrote this demo because I want to implement a model that is like QFileSystemModel and can support gvfs. We can not only use this model access local file system, but aslo remote and more...

There has been some projects use qt model/view program and gvfs together, one of them is [libfm-qt](https://github.com/lxqt/libfm-qt). I've been inspired by it a lot. But its design is different from mine. I don't want to undermine the portability of models and views. So I can just use one kind of model in all different views.

## screenshot
![screenshot](https://github.com/Yue-Lan/gvfs-qtmodel-demo/blob/master/screenshot/iconview_and_tree_view_of_computer.png)

## Licsence
LGPLv3
