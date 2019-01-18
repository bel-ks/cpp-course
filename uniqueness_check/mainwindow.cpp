#include "mainwindow.h"
#include "ui_mainwindow.h"

QMutex MainWindow::abil;
qint32 MainWindow::curSize;
bool MainWindow::isCanceled;
qint32 MainWindow::filesSize;
QFutureWatcher<void> MainWindow::fileHandler;
QFutureWatcher<void> MainWindow::hashHandler;
QVector<QVector<QString> > MainWindow::blocks;
QMap<quint64, QVector<QString> > MainWindow::allFiles;
QMap<QString, QVector<QString> > MainWindow::fileHashes;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->file_table->setHorizontalHeaderLabels(tableHeaders);
    ui->file_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(&fileHandler, SIGNAL(finished()), this, SLOT(files_were_found()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_choose_button_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning", QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->path->setText(dir);
}

void MainWindow::get_files(QString dirPath) {
    QDir dir(dirPath);
    if (dir.exists()) {
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (auto d : dirs) {
            get_files(dirPath + "/" + d);
        }
        QStringList files = dir.entryList(QDir::Files | QDir::NoSymLinks);
        for (auto f : files) {
            qint64 size = QFile(dirPath + "/" + f).size();
            allFiles[size].push_back(dirPath + "/" + f);
            filesSize += (size >> 15);
        }
    }
}

void MainWindow::on_find_button_clicked() {
    curSize = 0;
    filesSize = 0;
    blocks.clear();
    allFiles.clear();
    fileHashes.clear();
    isCanceled = false;
    ui->file_table->setRowCount(0);
    ui->del_button->setEnabled(false);
    ui->find_button->setEnabled(false);
    ui->choose_button->setEnabled(false);
    ui->statusBar->showMessage("Started work");
    fileHandler.setFuture(QtConcurrent::run(&MainWindow::get_files, ui->path->text()));
}

void MainWindow::get_hashes(QVector<QString> paths) {
    QVector<QPair<QString, QString> > hashes;
    for (auto filePath : paths) {
        if (isCanceled) {
            return;
        }
        QFile file(filePath);
        qint64 size = file.size();
        if (file.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha512);
            quint64 ind = size / bufSize + 1;
            for (; ind > 0; --ind) {
                if (isCanceled) {
                    file.close();
                    return;
                }
                hash.addData(file.read(bufSize));
                abil.lock();
                curSize += ((bufSize < size ? bufSize : size) >> 15);
                emit hashHandler.progressValueChanged(curSize);
                abil.unlock();
                size -= bufSize;
            }
            file.close();
            hashes.push_back({ (QString)hash.result(), filePath });
        } else {
            if (isCanceled) {
                return;
            }
            abil.lock();
            curSize += (size >> 15);
            emit hashHandler.progressValueChanged(curSize);
            abil.unlock();
        }
    }
    if (isCanceled) {
        return;
    }
    abil.lock();
    for (auto h : hashes) {
        fileHashes[h.first].push_back(h.second);
    }
    abil.unlock();
}

void MainWindow::files_were_found()
{
    ui->statusBar->showMessage("All files were found");
    qint64 size = QThread::idealThreadCount();
    blocks.resize(size);
    for (auto files : allFiles) {
        if (files.size() > 1) {
            for (qint64 ind = files.size() - 1; ind > -1; --ind) {
                qint64 to = -1;
                for (qint64 j = 0; j < size; ++j) {
                    if (to == -1 || blocks[to].size() > blocks[j].size()) {
                        to = j;
                    }
                }
                blocks[to].push_back(files[ind]);
            }
        } else {
            filesSize -= (QFile(files[0]).size() >> 15);
        }
    }
    QProgressDialog progress("Getting hashes..", "Cancel", 0, filesSize);
    connect(&progress, SIGNAL(canceled()), &hashHandler, SLOT(cancel()));
    connect(&progress, SIGNAL(canceled()), this, SLOT(cancel_button_clicked()));
    connect(&hashHandler, SIGNAL(finished()), &progress, SLOT(reset()));
    connect(&hashHandler, SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));
    hashHandler.setFuture(QtConcurrent::map(blocks, &MainWindow::get_hashes));
    progress.exec();
    hashHandler.waitForFinished();
    if (!isCanceled) {
        hashes_were_found();
    }
}

void MainWindow::hashes_were_found()
{
    ui->statusBar->showMessage("All hashes were found");
    qint64 row = 0, block = 0;
    for (auto fh : fileHashes) {
        quint64 size = fh.size();
        if (size > 1) {
            for (auto f : fh) {
                ui->file_table->setRowCount(row + 1);
                f.remove(0, ui->path->text().length());
                ui->file_table->setItem(row, 0, new QTableWidgetItem(f));
                ui->file_table->item(row, 0)->setFlags(ui->file_table->item(row, 0)->flags() & ~Qt::ItemIsEditable);
                if (block % 2) {
                    ui->file_table->item(row, 0)->setBackgroundColor(Qt::GlobalColor::darkGray);
                }
                ++row;
            }
            ui->file_table->setSpan(row - size, 1, size, 1);
            ui->file_table->setSpan(row - size, 2, size, 1);
            ui->file_table->setItem(row - size, 1, new QTableWidgetItem(QString::number(size)));
            ui->file_table->setItem(row - size, 2, new QTableWidgetItem(QString::number(QFile(fh[0]).size()) + " bytes"));
            ui->file_table->item(row - size, 1)->setTextAlignment(Qt::AlignCenter);
            ui->file_table->item(row - size, 2)->setTextAlignment(Qt::AlignCenter);
            ui->file_table->item(row - size, 1)->setFlags(ui->file_table->item(row - size, 1)->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable);
            ui->file_table->item(row - size, 2)->setFlags(ui->file_table->item(row - size, 2)->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable);
            if (block % 2) {
                ui->file_table->item(row - size, 1)->setBackgroundColor(Qt::GlobalColor::darkGray);
                ui->file_table->item(row - size, 2)->setBackgroundColor(Qt::GlobalColor::darkGray);
            }
            ++block;
        }
    }
    ui->del_button->setEnabled(true);
    ui->find_button->setEnabled(true);
    ui->choose_button->setEnabled(true);
    ui->statusBar->showMessage("Finished");
}

void MainWindow::cancel_button_clicked()
{
    isCanceled = true;
    ui->del_button->setEnabled(true);
    ui->find_button->setEnabled(true);
    ui->choose_button->setEnabled(true);
    ui->statusBar->showMessage("Canceled");
}

void MainWindow::on_del_button_clicked() {
    auto items = ui->file_table->selectedItems();
    for (auto i : items) {
        if (QFile(ui->path->text() + i->text()).remove()) {
            i->setBackgroundColor(Qt::GlobalColor::darkGreen);
        } else {
            i->setBackgroundColor(Qt::GlobalColor::darkRed);
        }
        i->setFlags(i->flags() & ~Qt::ItemIsSelectable);
    }
}
