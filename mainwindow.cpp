#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    parser = new fileParser();
    parser->moveToThread(&workerThread);
    workerThread.start();
    connect(parser, SIGNAL(progressResult(QString,int)), this, SLOT(progressResult_slot(QString,int)));
    connect(parser, SIGNAL(processingResult(QString,int)), this, SLOT(processResult_slot(QString,int)));
    connect(parser, SIGNAL(fetching_finished()), this, SLOT(onFinished()));
    connect(parser, SIGNAL(processing_finished()), this, SLOT(onFinished()));
    connect(this, SIGNAL(fetchCommand(QString)), parser, SLOT(sepratingFiles(QString)));
    connect(this, SIGNAL(processCommand()), parser, SLOT(processingFiles()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addr_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "C:\\Users\\Mohammad Reza\\Desktop",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    disableButton();
    emit fetchCommand(dir);
}

void MainWindow::on_process_button_clicked()
{
    disableButton();
    emit processCommand();
}

void MainWindow::on_clear_button_clicked()
{
    parser->clearList();
    ui->selected_textEdit->clear();
}

void MainWindow::disableButton()
{
    ui->addr_button->setDisabled(true);
    ui->clear_button->setDisabled(true);
    ui->process_button->setDisabled(true);
}

void MainWindow::progressResult_slot(const QString &status, const int &progress)
{
    ui->status_label->setText(status);
    ui->selected_textEdit->append(status);
    ui->progressBar->setValue(progress);
}

void MainWindow::processResult_slot(const QString &status, const int &progress)
{
    ui->status_label->setText(status);
    ui->progressBar->setValue(progress);
}

void MainWindow::onFinished()
{
    ui->addr_button->setEnabled(true);
    ui->process_button->setEnabled(true);
    ui->clear_button->setEnabled(true);
    ui->progressBar->setValue(100);
}
