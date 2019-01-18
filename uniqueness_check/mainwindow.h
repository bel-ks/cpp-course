#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>
#include <QCryptographicHash>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QFileDialog>
#include <QStringList>
#include <QByteArray>
#include <QIODevice>
#include <QThread>
#include <QVector>
#include <QString>
#include <QMutex>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_choose_button_clicked();

    void on_find_button_clicked();

    void files_were_found();

    void on_del_button_clicked();

    void cancel_button_clicked();

private:
    Ui::MainWindow *ui;
    static QMutex abil;
    static qint32 curSize;
    static bool isCanceled;
    static qint32 filesSize;
    static QFutureWatcher<void> fileHandler;
    static QFutureWatcher<void> hashHandler;
    static const qint64 bufSize = (1 << 16);
    static QVector<QVector<QString> > blocks;
    static QMap<quint64, QVector<QString> > allFiles;
    static QMap<QString, QVector<QString> > fileHashes;
    const QStringList tableHeaders = {"File name", "Count", "Size"};

    void hashes_were_found();
    static void get_files(QString dirPath);
    static void get_hashes(QVector<QString> files);
};

#endif // MAINWINDOW_H
