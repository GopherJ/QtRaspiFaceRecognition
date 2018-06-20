#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <raspicam/raspicam_cv.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <QUdpSocket>
#include <QBuffer>
#include "RTIMULib.h"

/*namespace Ui {
class MainWindow;
}*/

class MainWindow : QObject
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    //void new_connection();
    void send_client();
    void capture_image();
    void raffraichir_valeur_capteur();

private:
    //Ui::MainWindow *ui;
    QUdpSocket *udpServer;
    //QTcpSocket* client;
    QImage image_final;
    raspicam::RaspiCam_Cv Camera;
    cv::Mat tmp;
    cv::CascadeClassifier face_cascade;
    //QByteArray block;
    quint32 donneesBuffer;
    //QDataStream read_data;
    QHostAddress groupAddress;
    QBuffer read_buffer;
    RTIMU *imu;
    RTPressure *pressure;
    float temperature;
    float pression;
};

#endif // MAINWINDOW_H
