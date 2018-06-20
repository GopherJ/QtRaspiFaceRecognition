#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <iostream>
#include <fstream>

QByteArray block;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->udpSocket = new QUdpSocket(this);
    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(100));
    block_size=0;
    donneesBuffer=0;
    groupAddress = QHostAddress("239.255.43.54");
    udpSocket->bind(QHostAddress::AnyIPv4, 9998, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (udpSocket->joinMulticastGroup(groupAddress)){
        qDebug()<<"multicast OK";
    }
    else{
        qDebug()<<"multicast PROBLEME !!!";
    }
    //destAddress = QHostAddress("192.168.100.103");
    destAddress = QHostAddress("127.0.0.1");
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_ConnecterBt_clicked()
{
    connect(udpSocket, SIGNAL(readyRead()),this, SLOT(read_text()));
    /*QTimer *timer = new QTimer();
    timer->connect(timer, SIGNAL(timeout()),this,SLOT(on_EnvoyerBt_clicked()));
    timer->start(1000);*/
    fichier_csv.open("stat_latence.csv");
    fichier_csv << "temps serveur,temps client\n";
}

/*void MainWindow::on_EnvoyerBt_clicked()
{
    struct timeval date;
    gettimeofday(&date,NULL);
    QByteArray block_send;
    QDataStream out_block(&block_send,QIODevice::WriteOnly);
    out_block.setVersion(QDataStream::Qt_4_0);
    out_block << (long long int)date.tv_sec;
    out_block<< (long long int)date.tv_usec;
    udpSocket->writeDatagram(block_send,destAddress,9999);
    qDebug()<<"envoi !";
}*/

void MainWindow::on_QuitterBt_clicked()
{
    fichier_csv.close();
    udpSocket->leaveMulticastGroup(groupAddress);
    exit(0);
}

void MainWindow::read_text()
{
    while (udpSocket->hasPendingDatagrams()){

        QHostAddress sender;
        quint16 senderPort;
        quint32 tempSize = udpSocket->pendingDatagramSize();
        qDebug()<<"temp size : " << tempSize;
        block.resize(block.size() + tempSize);
        quint32 readedSize = udpSocket->readDatagram(block.data(), block.size(),&sender, &senderPort);
        donneesBuffer += readedSize;

    }
    if (block_size == 0){
        if (donneesBuffer < sizeof(quint32)){
            return;
        }
        QDataStream read_data(&block,QIODevice::ReadOnly);
        read_data.setVersion(QDataStream::Qt_4_0);
        read_data >> block_size;
        qDebug()<<"block size : " << block_size;
    }

    if (donneesBuffer < block_size){
        return;
    }

    QDataStream read_data(&block,QIODevice::ReadOnly);
    read_data.setVersion(QDataStream::Qt_4_0);
    read_data >> block_size;

    using namespace std;
    long long int seconds;
    read_data >> seconds;
    long long int micro_seconds;
    read_data >> micro_seconds;
    qreal temperature;
    read_data >> temperature;
    qreal pression;
    read_data >> pression;
    struct timeval date_client;
    gettimeofday(&date_client,NULL);
    QString line = tr("temps serveur : %1.%2\ntemps client :    %3.%4\ntemperature : %5 , pression : %6").arg(seconds).arg(micro_seconds,6,10,QChar('0')).arg((long long int)date_client.tv_sec).arg((long long int)date_client.tv_usec,6,10,QChar('0')).arg(temperature).arg(pression);
    fichier_csv<<tr("%1.%2,%3.%4\n").arg(seconds).arg(micro_seconds,6,10,QChar('0')).arg((long long int)date_client.tv_sec).arg((long long int)date_client.tv_usec,6,10,QChar('0')).toStdString();
    //line = ui->LogBox->toPlainText() + line;
    ui->LogBox->append(line);
    qDebug()<<"Reception image"<<block.size();
    image_final.load(read_data.device(),"JPEG");
    if (image_final.isNull()){
       qDebug()<<"image nulle";
    }
    QPixmap item = QPixmap::fromImage(image_final);
    ui->afficher_image->setPixmap(item);
    block_size = 0;
    donneesBuffer = 0;
    block.clear();
    //read_data.resetStatus();
}
