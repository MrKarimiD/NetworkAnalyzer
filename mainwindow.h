#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <cstdlib>
#include <QThread>
#include "fileparser.h"

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
    void on_addr_button_clicked();

    void on_process_button_clicked();

    void on_clear_button_clicked();

    void progressResult_slot(const QString &status, const int &progress);
    void processResult_slot(const QString &status, const int &progress);
    void onFinished();

private:
    Ui::MainWindow *ui;
    fileParser* parser;
    QThread workerThread;
    void disableButton();

signals:
    void fetchCommand(const QString &status);
    void processCommand();
};

#endif // MAINWINDOW_H
