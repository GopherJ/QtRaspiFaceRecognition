#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <QUdpSocket>
#include <sys/time.h>
//#include <raspicam/raspicam_cv.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <unistd.h>
#include "RTIMULib.h"

MainWindow::MainWindow(QWidget *parent) :
    QObject(parent)//,
    //ui(new Ui::MainWindow)
{
    //ui->setupUi(this);
    this->udpServer = new QUdpSocket(this);
    udpServer->setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(100));
    this->udpServer->bind(QHostAddress::AnyIPv4, 9999);


    Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3);
    Camera.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    Camera.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
//    image_final = NULL;
    if (!Camera.open()) {std::cerr<<"Error opening the camera"<<std::endl;exit(1);}
    if (!face_cascade.load("/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml")){
        qDebug()<<"échec du chargement de la configuration pour la reconnaissance facial";
        exit(1);
    }



    RTIMUSettings *settings = new RTIMUSettings("RTIMULib");
    imu = RTIMU::createIMU(settings);
    pressure = RTPressure::createPressure(settings);
    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU found\n");
        exit(1);
    }
    imu->IMUInit();

    imu->setSlerpPower(0.02);
    /*imu->setGyroEnable(true);
    imu->setAccelEnable(true);
    imu->setCompassEnable(true);*/

    //  set up pressure sensor

        if (pressure != NULL)
            pressure->pressureInit();


    donneesBuffer = 0;
    groupAddress = QHostAddress("239.255.43.54");
    //groupAddress = QHostAddress("127.0.0.1");
    QTimer *timer = new QTimer();
    timer->connect(timer, SIGNAL(timeout()),this,SLOT(send_client()));
    timer->start(1000);
    //connect(udpServer, SIGNAL(readyRead()),this, SLOT(read_client()));
}

MainWindow::~MainWindow()
{
    //delete ui;
}

void MainWindow::send_client(){
    /*QHostAddress sender;
    quint16 senderPort;
    QByteArray block;
    QDataStream read_data(&block,QIODevice::ReadWrite);
    read_data.setVersion(QDataStream::Qt_4_0);
    while (udpServer->hasPendingDatagrams()){

        quint32 tempSize = udpServer->pendingDatagramSize();
        block.resize(block.size()+tempSize);
        udpServer->readDatagram(block.data(), block.size(),&sender, &senderPort);
        qDebug()<< "taille : " << tempSize;
        qDebug()<<"taille bloc : " << block.size();
        donneesBuffer += tempSize;
    }
    if (donneesBuffer >= 2*sizeof(long long int)){

    long long int seconds;
    read_data >> seconds;
    long long int micro_seconds;
    read_data >> micro_seconds;
    QString line = tr("%1.%2\n").arg(seconds).arg(micro_seconds,6,10,QChar('0'));
    line = ui->logBox->toPlainText() + line;
    ui->logBox->document()->setPlainText(line);*/

    raffraichir_valeur_capteur();

    // envoie heure microsecondes
    struct timeval date;
    gettimeofday(&date,NULL);
    QByteArray block_send;
    QDataStream out_block(&block_send,QIODevice::WriteOnly);
    out_block << (long long int)date.tv_sec;
    out_block<< (long long int)date.tv_usec;
    out_block << (qreal) temperature;
    out_block << (qreal) pression;


    capture_image();

    QBuffer buf_image(&block_send);
    buf_image.open(QIODevice::ReadWrite);

    //qDebug()<<"Avant save";
    this->image_final.save(out_block.device(), "JPEG",-1); // -1 qualité par défaut sinon entre 0 et 100
    //qDebug()<<"Apres save";

    QByteArray final_block_send;
    QDataStream out_final(&final_block_send,QIODevice::WriteOnly);
    //final_block_send.append(block_send.size() - sizeof(quint32));
    out_final << (quint32)(block_send.size() - sizeof(quint32));
    qDebug()<<"block size :" << (block_send.size() - sizeof(quint32));
    final_block_send.append(block_send);
    //udpServer->writeDatagram(final_block_send, sender, senderPort);
    //udpServer->writeDatagram(final_block_send, groupAddress, senderPort);
    udpServer->writeDatagram(final_block_send, groupAddress, 9998);
    donneesBuffer = 0;
    }
//}


void MainWindow::capture_image()
{
    cv::Mat image;
    cv::Mat image_flip;
    cv::Mat tmp_gray;

    Camera.grab();
    Camera.retrieve ( image);
    cv::flip(image,image_flip,0);
    switch (image_flip.type()) {
    case CV_8UC1:
        cvtColor(image_flip,tmp, CV_GRAY2RGB);
        break;
    case CV_8UC3:
        cvtColor(image_flip,tmp, CV_BGR2RGB);
        break;
    }
    cvtColor(tmp,tmp_gray,CV_RGB2GRAY);
    // code reconnaissance facial opencv
    cv::equalizeHist(tmp_gray,tmp_gray);
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(tmp_gray,faces,1.1,2,0| cv::CASCADE_SCALE_IMAGE, cv::Size(30,30));
    qDebug()<<"rectangle detecté " << faces.size();
    for (int ic = 0; ic < faces.size(); ic++){
        cv::Point pt1(faces[ic].x,faces[ic].y);
        cv::Point pt2(faces[ic].x + faces[ic].height,faces[ic].y + faces[ic].width);
        cv::rectangle(tmp,pt1,pt2,cv::Scalar(0,255,0),2,8,0);
    }
    //cvtColor(tmp_gray,tmp,CV_GRAY2RGB);
    //QImage a besoin que les données soit stocké de manière contigu
assert(tmp.isContinuous());
    image_final = QImage(tmp.data,tmp.cols,tmp.rows,tmp.cols*3,QImage::Format_RGB888);
        //this->image_final.save("/home/pi/testx.jpg", "JPEG");
    //QPixmap item = QPixmap::fromImage(image_final);
    //ui->afficherImage->setPixmap(item);
    //qDebug()<<"Taille image A ["<<image_final.size().height()<<","<<image_final.size().width()<<"]";
}

void MainWindow::raffraichir_valeur_capteur()
{
    usleep(imu->IMUGetPollInterval() * 1000);

    if(imu->IMURead()) {
                RTIMU_DATA imuData = imu->getIMUData();
                if (pressure != NULL)
                    pressure->pressureRead(imuData);

                //printf("Temperature : %s\n", sampleRate, RTMath::displayDegrees("", imuData.fusionPose));
                temperature = imuData.temperature;
                pression = imuData.pressure;
            }
    qDebug()<< "temperature : "<< temperature << " degrés Celsius , pression : "<<pression << " millibars";

}
