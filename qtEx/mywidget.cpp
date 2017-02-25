#include "mywidget.h"
#include <QPainter>
#include <iostream>

MyWidget::MyWidget(QWidget *parent) : QWidget(parent)
{
    setGeometry(0, 50, 960, 768);
}

void MyWidget::_update(unsigned char *b, int q_size) {
    mutex.lock();
    for (int i = 0; i < q_size; i ++) {
        if (cpos >= 1280 * 1024) {
            cpos = 0;
        }
        buf[cpos ++] = b[i] << 4;
    }
    updated ++;
    if (updated > 70) {
        std::cout << "70 updates done!" << std::endl;
        updated = 0;
    }
    mutex.unlock();
}

void MyWidget::paintEvent(QPaintEvent *) {
    mutex.lock();
    QPainter painter(this);

    QRectF rectangle(0.0, 0.0, 960.0, 768.0);
    QImage image = QImage(1280, 1024, QImage::Format_RGB32);
    int pos = 0;
    for (int i = 0; i < 1024; i ++) {
        for (int j = 0; j < 1280; j ++) {
            image.setPixel(j, i, qRgba(buf[pos], (int) buf[pos], (int) buf[pos], 0));
            if (pos < 100) (std::cout << std::hex) << (int) buf[pos];
            if (pos < 100) std::cout << " ";
            pos ++;
        }
    }

    (std::cout << std::dec) << std::endl;
    painter.drawImage(rectangle, image);
    int startAngle = 30 * 16;
    int spanAngle = 120 * 16;
    painter.drawArc(rectangle, startAngle, spanAngle);

    color += 100;
    mutex.unlock();
}

