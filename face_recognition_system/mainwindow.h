#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <QLocale>
#include <QTcpSocket>
#include <QTimer>
using namespace cv;
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //定时器事件
    void timerEvent(QTimerEvent *e);

private slots:
    void timer_connect();//用于定时连接服务器
    void stop_connect(); //连接成功，就可以停止连接
    void start_connect();//断开连接就可以用 start 去启动连接

    void onError(QAbstractSocket::SocketError error);
    void recv_data();
    void updateTime();
private:
    Ui::MainWindow *ui;

    //定义一个摄像头
    VideoCapture cap;//在OpenCV中，VideoCapture 类用于从视频文件、图像序列或摄像头捕获视频帧。

    //级联分类器
    cv::CascadeClassifier cascade;

    //创建网络套接字，定时器
    QTcpSocket msocket;
    QTimer mtimer; //用于连接服务器的定时器
    QTimer mtimer1;//用于显示时间的定时器

    QLocale chineseLocale;

    //标志是否是同一个人脸进入到识别区域
    int flag;

    //保存人脸的数据
    cv::Mat faceMat;

};
#endif // MAINWINDOW_H
