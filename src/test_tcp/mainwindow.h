#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <iostream>
#include <fstream>
#include <QBuffer>

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
    void on_ConnecterBt_clicked();

    void on_QuitterBt_clicked();

    void read_text();

private:
    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;
    QImage image_final;
    quint32 block_size;
    std::ofstream fichier_csv;
    QHostAddress groupAddress;
    QHostAddress destAddress;
    quint32 donneesBuffer;
    //QByteArray block;
    //QDataStream read_data;
    QBuffer read_buffer;
};

#endif // MAINWINDOW_H
