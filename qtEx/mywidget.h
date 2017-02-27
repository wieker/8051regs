#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QObject>
#include <QWidget>
 #include <QMutex>

class MyWidget : public QWidget
{
    int cpos = 0;
    unsigned char* buf = new unsigned char[1280 * 1024];
    int updated = 0;
    int color = 0;
    QMutex mutex;

    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = 0);
    void _update(unsigned char* b, int q_size);
    void paintEvent(QPaintEvent *);
    void reset();

signals:

public slots:
};

#endif // MYWIDGET_H
