#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    hasAllTris = false;
    ui->search_button->setEnabled(false);
    connect(&fileHandler, SIGNAL(finished()), this, SLOT(files_were_found()));
    ui->files_with_str->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(on_index_button_clicked()));
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(on_index_button_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_choose_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning", QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->dirPath->setText(dir);
}

void MainWindow::index_files(QString path) {
    watcher->addPath(path);
    QDir dir(path);
    if (dir.exists()) {
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (auto d : dirs) {
            index_files(path + "/" + d);
        }
        QStringList files = dir.entryList(QDir::Files | QDir::NoSymLinks);
        for (auto f : files) {
            watcher->addPath(path + "/" + f);
            qint64 size = QFile(path + "/" + f).size();
            if (size > 0) {
                if (size == 1) {
                    filesWith1.push_back(path + "/" + f);
                } else if (size == 2) {
                    filesWith2.push_back(path + "/" + f);
                } else {
                    allFiles.push_back(path + "/" + f);
                }
                filesSize += (size >> 15);
            }
        }
    }
}

void MainWindow::deleteAllItems()
{
    qint64 size = ui->files_with_str->rowCount();
    for (qint64 ind = 0; ind < size; ++ind) {
        delete ui->files_with_str->item(ind, 0);
    }
    ui->files_with_str->setRowCount(0);
}

void MainWindow::on_index_button_clicked()
{
    if (!QDir(ui->dirPath->text()).exists()) {
        QMessageBox::critical(this, "Warning", "Directory doesn't exist");
        ui->statusBar->showMessage("Choose another directory");
        return;
    }
    curSize = 0;
    filesSize = 0;
    blocks.clear();
    allFiles.clear();
    trigrams.clear();
    deleteAllItems();
    hasAllTris = true;
    filesWith1.clear();
    filesWith2.clear();
    isCanceled = false;
    containsStr.clear();
    curPath = ui->dirPath->text();
    ui->dirPath->setEnabled(false);
    ui->index_button->setEnabled(false);
    ui->search_button->setEnabled(false);
    ui->choose_button->setEnabled(false);
    ui->str_to_search->setEnabled(false);
    ui->statusBar->showMessage("Started work");
    fileHandler.setFuture(QtConcurrent::run(this, &MainWindow::index_files, ui->dirPath->text()));
}

void MainWindow::get_trigrams(QVector<QPair<QString, qint64> > paths)
{
    QSet<QPair<QString, qint64> > tris;
    for (auto filePath : paths) {
        if (isCanceled) {
            return;
        }
        QFile file(filePath.first);
        qint64 size = file.size();
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            abil.lock();
            curSize += (size >> 15);
            emit triHandler.progressValueChanged(curSize);
            abil.unlock();
            continue;
        }
        QTextStream input(&file);
        bool isText = true;
        QString tri = "   ";
        QChar cur;
        qint64 ind = 0;
        while(!input.atEnd()) {
            if(isCanceled) {
                file.close();
                return;
            }
            input >> cur;
            if(cur.isNonCharacter()) {
                isText = false;
                break;
            }
            tri.remove(0, 1);
            tri.push_back(cur);
            if(ind > 1) {
                tris.insert({ tri, filePath.second });
            }
            ++ind;
        }
        file.close();
        abil.lock();
        curSize += (size >> 15);
        emit triHandler.progressValueChanged(curSize);
        if (isText) {
            for (auto t : tris) {
                trigrams[t.first].insert(t.second);
            }
        }
        tris.clear();
        abil.unlock();
    }
    if (isCanceled) {
        return;
    }
}

void MainWindow::files_were_found()
{
    ui->statusBar->showMessage("Files were found");
    qint64 size = QThread::idealThreadCount();
    blocks.resize(size);
    qint64 to = 0;
    for (auto f : allFiles) {
        blocks[to].push_back({ f, to });
        ++to;
        to %= size;
    }
    while (blocks.size() != 1 && !blocks.back().size()) {
        blocks.pop_back();
    }
    QProgressDialog progress("Getting trigrams..", "Cancel", 0, filesSize);
    connect(&progress, SIGNAL(canceled()), &triHandler, SLOT(cancel()));
    connect(&progress, SIGNAL(canceled()), this, SLOT(cancel_button_clicked()));
    connect(&triHandler, SIGNAL(finished()), &progress, SLOT(reset()));
    connect(&triHandler, SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));
    triHandler.setFuture(QtConcurrent::map(blocks, [this] (QVector<QPair<QString, qint64> > files){
                                                get_trigrams(files);
                                            }));
    progress.exec();
    triHandler.waitForFinished();
    ui->dirPath->setEnabled(true);
    ui->index_button->setEnabled(true);
    ui->search_button->setEnabled(true);
    ui->choose_button->setEnabled(true);
    ui->str_to_search->setEnabled(true);
    if (!isCanceled) {
        ui->statusBar->showMessage("Finished work");
    } else {
        hasAllTris = false;
    }
}

void MainWindow::get_files(QVector<QPair<QString, qint64> > paths)
{
    QVector<QString> files;
    for (auto filePath : paths) {
        if (isCanceled) {
            return;
        }
        QFile file(filePath.first);
        qint64 size = file.size();
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            abil.lock();
            curSize += (size >> 15);
            emit strHandler.progressValueChanged(curSize);
            abil.unlock();
            continue;
        }
        QTextStream input(&file);
        QString may;
        for (qint32 ind = 0; ind < strSize; ++ind) {
            may.push_back(" ");
        }
        QChar cur;
        qint64 ind = 0;
        while(!input.atEnd()) {
            if(isCanceled) {
                file.close();
                return;
            }
            input >> cur;
            may.remove(0, 1);
            may.push_back(cur);
            if(ind > strSize - 2 && may == str) {
                files.push_back(filePath.first);
                break;
            }
            ++ind;
        }
        file.close();
        abil.lock();
        curSize += (size >> 15);
        emit strHandler.progressValueChanged(curSize);
        abil.unlock();
    }
    if (isCanceled) {
        return;
    }
    if (strSize == 2) {
        for (auto filePath : filesWith2) {
            if (isCanceled) {
                return;
            }
            QFile file(filePath);
            abil.lock();
            curSize += (2 >> 15);
            emit strHandler.progressValueChanged(curSize);
            abil.unlock();
            if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }
            QTextStream input(&file);
            QChar first, second;
            input >> first;
            input >> second;
            if (str[0] == first && str[1] == second) {
                files.push_back(filePath);
            }
            file.close();
        }
    }
    if (strSize == 1) {
        for (auto filePath : filesWith1) {
            if (isCanceled) {
                return;
            }
            QFile file(filePath);
            abil.lock();
            curSize += (1 >> 15);
            emit strHandler.progressValueChanged(curSize);
            abil.unlock();
            if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }
            QTextStream input(&file);
            QChar cur;
            input >> cur;
            if (str == cur) {
                files.push_back(filePath);
            }
            file.close();
        }
        for (auto filePath : filesWith2) {
            if (isCanceled) {
                return;
            }
            QFile file(filePath);
            abil.lock();
            curSize += (2 >> 15);
            emit strHandler.progressValueChanged(curSize);
            abil.unlock();
            if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }
            QTextStream input(&file);
            QChar first, second;
            input >> first;
            input >> second;
            if (str == first || str == second) {
                files.push_back(filePath);
            }
            file.close();
        }
    }
    abil.lock();
    for (auto f : files) {
        containsStr.push_back(f);
    }
    abil.unlock();
}

void MainWindow::on_search_button_clicked()
{
    if (!hasAllTris) {
        QMessageBox::critical(this, "Warning", "Directory is not passed completely");
        ui->statusBar->showMessage("Choose directory and wait until program finish work");
        return;
    }
    if (!ui->str_to_search->text().size()) {
        QMessageBox::critical(this, "Warning", "No string to search");
        ui->statusBar->showMessage("Write string");
        return;
    }
    curSize = 0;
    filesSize = 0;
    blocks.clear();
    deleteAllItems();
    isCanceled = false;
    containsStr.clear();
    ui->dirPath->setEnabled(false);
    ui->index_button->setEnabled(false);
    ui->search_button->setEnabled(false);
    ui->choose_button->setEnabled(false);
    ui->str_to_search->setEnabled(false);
    ui->statusBar->showMessage("Started work");
    str = ui->str_to_search->text();
    strSize = str.size();
    QSet<qint64> files;
    if (strSize > 2) {
        for (qint32 ind = 2; ind < strSize; ++ind) {
            QString tri = "" + str[ind - 2] + str[ind - 1] + str[ind];
            for (auto f : trigrams[tri]) {
                files.insert(f);
            }
        }
    } else if (strSize == 2) {
        filesSize += ((filesWith2.size() * 2) >> 15);
        for (qint32 ind = 1; ind < strSize; ++ind) {
            for (auto pair : trigrams.toStdMap()) {
                QString tri = pair.first;
                if ((tri[0] == str[ind - 1] && tri[1] == str[ind]) || (tri[1] == str[ind - 1] && tri[2] == str[ind])) {
                    for (auto f : pair.second) {
                        files.insert(f);
                    }
                }
            }
        }
    } else {
        filesSize += ((filesWith2.size() * 2) >> 15) + (filesWith1.size() >> 15);
        for (qint32 ind = 0; ind < strSize; ++ind) {
            for (auto pair : trigrams.toStdMap()) {
                QString tri = pair.first;
                if ((tri[0] == str[ind]) || (tri[1] == str[ind]) || (tri[2] == str[ind])) {
                    for (auto f : pair.second) {
                        files.insert(f);
                    }
                }
            }
        }
    }
    qint64 size = QThread::idealThreadCount();
    blocks.resize(size);
    qint64 to = 0;
    for (auto f : files) {
        blocks[to].push_back({ allFiles[f], f });
        ++to;
        to %= size;
        filesSize += (QFile(allFiles[f]).size() >> 15);
    }
    while (blocks.size() != 1 && !blocks.back().size()) {
        blocks.pop_back();
    }
    QProgressDialog progress("Finding string..", "Cancel", 0, filesSize);
    connect(&progress, SIGNAL(canceled()), &strHandler, SLOT(cancel()));
    connect(&progress, SIGNAL(canceled()), this, SLOT(cancel_button_clicked()));
    connect(&strHandler, SIGNAL(finished()), &progress, SLOT(reset()));
    connect(&strHandler, SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));
    strHandler.setFuture(QtConcurrent::map(blocks, [this] (QVector<QPair<QString, qint64> > files){
                                                get_files(files);
                                            }));
    progress.exec();
    strHandler.waitForFinished();
    if (!isCanceled) {
        qint64 row = 0;
        for (auto f : containsStr) {
            ui->files_with_str->setRowCount(row + 1);
            f.remove(0, curPath.length());
            ui->files_with_str->setItem(row, 0, new QTableWidgetItem(f));
            ui->files_with_str->item(row, 0)->setFlags(ui->files_with_str->item(row, 0)->flags() & ~Qt::ItemIsEditable);
            ++row;
        }
        ui->statusBar->showMessage("Finished work");
    }
    ui->dirPath->setEnabled(true);
    ui->index_button->setEnabled(true);
    ui->search_button->setEnabled(true);
    ui->choose_button->setEnabled(true);
    ui->str_to_search->setEnabled(true);
}

void MainWindow::cancel_button_clicked()
{
    isCanceled = true;
    ui->statusBar->showMessage("Canceled");
}
