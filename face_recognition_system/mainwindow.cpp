#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp> // 确保包含OpenCV的头文件
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QMessageBox>
#include <QDate>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , chineseLocale(QLocale::Chinese, QLocale::China)
{
    ui->setupUi(this);
    // 打开默认摄像头（通常是第一个连接的摄像头，索引为0）
    cap.open(0);

    //startTimer(interval) 启动一个定时器，其中interval 是以毫秒为单位的间隔
    startTimer(100);// 启动一个每100毫秒（0.1秒）触发一次的定时器

    //导入级联分类器文件
    cascade.load("C:/opencv452/etc/haarcascades/haarcascade_frontalface_alt2.xml");

    // QTcpSocket当断开连接的时候disconnectd信号，连接成功就发送connected
    connect(&msocket, &QTcpSocket::disconnected,this,&MainWindow::start_connect);
    connect(&msocket, &QTcpSocket::connected,this,&MainWindow::stop_connect);

    //关联接受数据的槽函数，当socket发出“准备读”信号时，就可以接受数据了
    connect(&msocket, &QTcpSocket::readyRead, this, &MainWindow::recv_data);

    //连接错误处理情况
    connect(&msocket, &QTcpSocket::errorOccurred, this, &MainWindow::onError);

    //定时器连接服务器
    connect(&mtimer, &QTimer::timeout,this,&MainWindow::timer_connect);
    connect(&mtimer1, &QTimer::timeout, this, &MainWindow::updateTime);
    // 启动定时器
    mtimer.start(5000);//每5s钟连接一次，连接成功就不再连接
    // 设置定时器间隔为1000毫秒（1秒）
    mtimer1.start(1000);

    flag = 0;

    ui->widgetLb->hide();  //将“识别成功”标志隐藏
    ui->widgetLb_2->hide();//将“认证成功，晚归”标志隐藏
    ui->fault_Lb->hide();//将”识别失败“标志隐藏
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime()
{
    // 获取当前时间
    QDateTime now = QDateTime::currentDateTime();

    // 格式化日期、星期和时间
    QString date = now.toString("yyyy年MM月dd日");
    //QString dayOfWeek = now.toString("EEEE"); // EEEE 表示完整的星期几名称
    QLocale locale(QLocale::English, QLocale::UnitedStates);
    QString dayOfWeek = chineseLocale.dayName(now.date().dayOfWeek(), QLocale::LongFormat); // 使用成员变量来获取星期几名称
    QString time = now.toString("hh:mm:ss"); // hh 表示12小时制的小时，如需24小时制请使用HH

    // 组合HTML字符串
    QString html = QString("<p>%1</p><p>%2</p><p>%3</p>").arg(date, dayOfWeek, time);

    // 设置QLabel的文本为HTML字符串
    ui->label_7->setText(html);
}

//连接错误处理函数
void MainWindow::onError(QAbstractSocket::SocketError error)
{
    // 该函数当发生错误时调用
    qDebug() << "连接出现错误：" << msocket.errorString();
}

//timerEvent 是QObject类的一个受保护的虚函数，是一个特殊的成员函数，它用于接收和处理定时器事件。
//该函数的内容是当定时器触发时应该执行的操作。
void MainWindow::timerEvent(QTimerEvent *e)//QTimerEvent 是 QEvent 的一个子类，专门用于处理定时器事件。
{
//    ui->widgetLb->hide();  //将“识别成功”标志隐藏
//    ui->widgetLb_2->hide();//将“认证成功，晚归”标志隐藏
//    ui->fault_Lb->hide();//将”识别失败“标志隐藏

    if (!cap.isOpened())
    { // 检查摄像头是否成功打开
        std::cerr << "Error opening video capture" << std::endl;
        return;
    }

    /*
    在将OpenCV的Mat图像显示在Qt的QLabel、QGraphicsView或其他需要QPixmap的
    控件上时，通常需要进行两步转换：
    1、从Mat到QImage
    2、从QImage到QPixmap
    */

    //定义一个Mat对象用于存储从摄像头捕获的图像
    Mat srcImage;

    //读取一帧数据到srcImage
    cap.read(srcImage);

    // 检查是否成功读取了新的一帧
    if(srcImage.empty())
    {
        qDebug()<<"无法从视频捕获中读取帧";
        return;// 如果没有读取到新的帧，打印错误信息并退出函数
    }

    //调整图片大小，调整成与显示窗口一样大
    cv::resize(srcImage,srcImage,Size(ui->videoLb->size().width(),ui->videoLb->size().height()));

    Mat grayImage;
    // 转灰度图，是为了后续检测人脸的时候加快检测速度
    cv::cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);

    //检测人脸数据
    vector<Rect> faceRects;
    // 定义向量faceRects，是用来存储检测到的各个对象的矩形框（Rect对象）
    // Rect表示一个矩形区域，例如图像处理中用于表示人脸检测或物体检测中的矩形边界框。

    // 使用分类器检测人脸
    // detectMultiScale 是一个常用于对象检测（如人脸检测）的函数
    // 这个函数尝试在输入图像中定位多个对象，并将这些对象的位置存储在一个std::vector<Rect>类型的变量中。
    cascade.detectMultiScale(grayImage,faceRects);

    // 检测到人脸框的时候，移动人脸框
    if(faceRects.size() > 0 && flag >= 0)
    {
        Rect rect = faceRects.at(0);//第一个人脸的矩形框
        rectangle(srcImage,rect,Scalar(0,0,255));
        //在原始图像srcImage上绘制一个矩形框，该矩形框由rect指定
        // 移动人脸框
        ui->headpicLb->move(rect.x,rect.y);

        //if(flag > 2)
        if(flag == 2)
        {
            // 把Mat数据转换为QbyteArray  --》编码成jpg格式
            std::vector<uchar> buf;
            cv::imencode(".jpg",srcImage,buf);//输出的数据存在buf
            QByteArray byte((const char*)buf.data(),buf.size());
            // 准备发送
            quint64 backsize = byte.size();//图片大小
            QByteArray sendData;
            QDataStream stream(&sendData,QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_6_5);
            stream<<backsize<<byte;//先存入数据的大小，再发送数据
            //发送
            msocket.write(sendData);
            //flag = -1;

            faceMat = srcImage(rect);
            //保存
            imwrite("./face.jpg",faceMat);//把人脸数据保存在图片中
        }
        flag++;
    }
    if(faceRects.size() == 0)
    {
        ui->headLb->setStyleSheet("border-image: url(:/face.png);background-color: rgb(255, 255, 255);border-radius:75px;");
        ui->nameEdit->clear();
        ui->numberEdit->clear();
        ui->schoolEdit->clear();
        ui->dormitory_number_Edit->clear();
        ui->timeEdit->clear();
        //把人脸框移动到中心位置
        ui->headpicLb->move(100,60);
        ui->widgetLb->hide();//将“识别成功”标志隐藏
        ui->widgetLb_2->hide();//将“识别成功，晚归”标志隐藏
        ui->fault_Lb->hide();
        flag = 0;
    }

    if(srcImage.data == nullptr)return;
    //把Opencv里面的Mat格式数据（BGR格式）转为Qt里面的QImage（RGB格式）
    //输入数据和输出数据都是srcImage
    cvtColor(srcImage,srcImage,COLOR_BGR2RGB);
    // 创建一个QImage对象从srcImage的数据。
    // 1. srcImage.data 指向图像数据的起始地址
    // 2. srcImage.cols 是图像的列数（宽度）
    // 3. srcImage.rows 是图像的行数（高度）
    // 4. srcImage.step1() 通常是srcImage.cols * srcImage.elemSize()，但它直接给出了行之间的字节数
    // 5. QImage::Format_RGB888 指定了QImage的像素格式为24位RGB
    QImage image(srcImage.data, srcImage.cols, srcImage.rows, srcImage.step1(), QImage::Format_RGB888);
    // 将QImage对象转换为QPixmap对象，因为QLabel通常使用QPixmap来显示图像
    QPixmap mmp = QPixmap::fromImage(image);

    //mmp = mmp.scaled(ui->videoLb->size());
    // 使用 setPixmap() 方法将 mmp 这个QPixmap对象设置为 QLabel 的内容，显示在该标签上，从而在屏幕上显示图像。
    ui->videoLb->setPixmap(mmp);
}

void MainWindow::recv_data()
{

    //1、对客户端发过来的数据进行接收，接收到的数据是json格式
    QByteArray array = msocket.readAll();
    qDebug()<<array;

    //2、对json数据进行解析
    //声明 QJsonParseError 类型变量 err。这个对象将用于存储 fromJson 方法在解析 JSON 数据时可能遇到的错误。
    QJsonParseError err;
    //QJsonDocument 类是用于处理 JSON 数据的。
    //QJsonDocument::fromJson 方法尝试将一个字节数组（QByteArray）解析为一个 JSON 文档。如果解析成功，doc 将包含解析后的 JSON 文档。
    //这个方法接受一个 QJsonParseError 对象的引用，用于在解析过程中报告任何错误。
    QJsonDocument doc = QJsonDocument::fromJson(array, &err);

    if(err.error != QJsonParseError::NoError)
    {
        //不等于NoError说明接收数据有问题
        qDebug() << "json数据错误:" << err.errorString();
        //用err.errorString()来获取错误的描述。
        QMessageBox::critical(this, "错误", "JSON数据解析失败：" + err.errorString());
        return;
    }

    //3、从解析后的 JSON 对象中提取各个字段的值，并将它们存储在相应的 QString 变量中
    QJsonObject obj = doc.object();

    QString name = obj.value("name").toString();
    QString number = obj.value("number").toString();
    QString school = obj.value("school").toString();
    QString dormitory_number = obj.value("dormitory_number").toString();
    QString timestr = obj.value("time").toString();
    QString state = obj.value("state").toString();

    if(state.isEmpty())
    {
        ui->widgetLb->hide();
        ui->widgetLb_2->hide();
        ui->fault_Lb->show();
    }

    //4、将从JSON对象中提取出来的数据显示到刷脸界面(UI)的各个控件中
    ui->nameEdit->setText(name);
    ui->numberEdit->setText(number);
    ui->schoolEdit->setText(school);
    ui->dormitory_number_Edit->setText(dormitory_number);
    //ui->department->setText(department);
    ui->timeEdit->setText(timestr);

    //5、通过样式来显示图片
    ui->headLb->setStyleSheet("border-radius:75px;border-image: url(./face.jpg);");
    if(state == "正常")
    {
        ui->fault_Lb->hide();
        ui->widgetLb->show();
    }
    if(state == "晚归")
    {
        ui->fault_Lb->hide();
        ui->widgetLb_2->show();
    }
}

void MainWindow::timer_connect()
{
    //连接服务器
    qDebug()<<"正在连接服务器";
    msocket.connectToHost("127.0.0.1",9999);//127.0.0.1
}

void MainWindow::stop_connect()
{
    mtimer.stop();
    qDebug()<<"成功连接服务器";
}

void MainWindow::start_connect()
{
    mtimer.start(5000);//启动定时器
    qDebug()<<"断开连接";
}
