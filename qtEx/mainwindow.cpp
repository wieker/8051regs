#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include "urbprocessor.h"

class WorkerThread;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    glWidget = new MyWidget(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_fwButton_clicked()
{
    p = new URBProcessor(glWidget);
    p->dir = -1;
    p->firmware_hex_path = "/home/wieker/Projects/linux/sensor//qtEx/fw.ihx";
    p->search_vid = 0x0547;
    p->search_pid = 0x1002;
    std::cout << "Connect: " + p->ConnectAndInitUSB() << std::endl;
    std::cout << p->SubmitInitialURBs() << std::endl;
    startWorkInAThread();
}

void MainWindow::startWorkInAThread()
{
    WorkerThread *workerThread = new WorkerThread();
    workerThread->pipe = p;
    //connect(workerThread, &WorkerThread::resultReady, this, &MyObject::handleResults);
    //connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->start();
}

void MainWindow::submitCommand(char *command) {
    p->dir = 1;
    p->no_stdio = 1;
    std::cout << "One URB: " + p->SubmitOneURB((unsigned char *) command, strlen(command)) << std::endl;
    p->dir = -1;
    p->no_stdio = 0;
}

void MainWindow::submitCommandWithLength(char *command, int length) {
    p->dir = 1;
    p->no_stdio = 1;
    std::cout << "One URB: " + p->SubmitOneURB((unsigned char *) command, length) << std::endl;
    p->dir = -1;
    p->no_stdio = 0;
}

unsigned char cvtToBin(char hex) {
    if (hex >= 'a') {
        return hex - 'a' + 0xa;
    } else if (hex >= 'A') {
        return hex - 'A' + 0xa;
    } {
        return hex - '0';
    }
}

void MainWindow::on_sendPktButton_clicked()
{
    submitCommandWithLength("PING", 4);
}

void MainWindow::on_sendOtherPkt_clicked()
{
    submitCommandWithLength("DPORTAA", 7);
}

void MainWindow::on_toggleSlave_clicked()
{
    submitCommandWithLength("SLAVE", 5);
}

void MainWindow::on_drawButton_clicked()
{
    glWidget->update();
}

void MainWindow::on_gpifButton_clicked()
{
    glWidget->reset();
    submitCommandWithLength("GPIF", 4);
}
