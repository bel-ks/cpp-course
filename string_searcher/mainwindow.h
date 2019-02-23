#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtConcurrent/QtConcurrentRun>
#include <QtConcurrent/QtConcurrentMap>
#include <QFileSystemWatcher>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QMutexLocker>
#include <QMainWindow>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>
#include <QByteArray>
#include <QString>
#include <QMutex>
#include <QDebug>
#include <QMap>
#include <QSet>
#include <QDir>

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

    void on_index_button_clicked();

    void on_search_button_clicked();

    void files_were_found();

    void cancel_button_clicked();

    void changed_file_or_dir();

private:
    Ui::MainWindow *ui;
    QString str;
    QMutex abil;
    qint32 curSize;
    qint32 strSize;
    bool isCanceled;
    bool hasAllTris;
    QString curPath;
    qint32 filesSize;
    QVector<QString> allFiles;
    QFileSystemWatcher *watcher;
    QVector<QString> filesWith1;
    QVector<QString> filesWith2;
    QVector<QString> containsStr;
    QFutureWatcher<void> strHandler;
    QFutureWatcher<void> triHandler;
    QFutureWatcher<void> fileHandler;
    const qint64 bufSize = (1 << 16);
    QMap<QString, QSet<qint64> > trigrams;
    QVector<QVector<QPair<QString, qint64> > > blocks;

    void deleteAllItems();
    void index_files(QString path);
    void get_files(QVector<QPair<QString, qint64> > files);
    void get_trigrams(QVector<QPair<QString, qint64> > files);
};

#endif // MAINWINDOW_H
