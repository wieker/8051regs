#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "../fx2pipe-0.8/fx2pipe/fx2pipe.h"
#include <iostream>
#include "mywidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    MyWidget* glWidget;

private slots:

    void on_fwButton_clicked();

    void on_sendPktButton_clicked();

    void on_sendOtherPkt_clicked();

    void on_toggleSlave_clicked();

    void on_drawButton_clicked();

    void on_gpifButton_clicked();

private:
    FX2Pipe* p;
    Ui::MainWindow *ui;

    void startWorkInAThread();
    void submitCommand(char* command);
    void submitCommandWithLength(char *command, int length);
};

class WorkerThread : public QThread
{
    Q_OBJECT
    void run() {
        QString result;
        pipe->dir = -1;
        pipe->no_stdio = 0;
        for (;;) {
            std::cout << pipe->ProcessEvents(1000) << std::endl;
        }
        emit resultReady(result);
    }
signals:
    void resultReady(const QString &s);
public:
    FX2Pipe* pipe;
};

#endif // MAINWINDOW_H
