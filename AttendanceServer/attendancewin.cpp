#include "attendancewin.h"
#include "ui_attendancewin.h"
#include <opencv2/opencv.hpp>
#include <QDateTime>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QTabBar>
using namespace cv;
AttendanceWin::AttendanceWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AttendanceWin)
{
    ui->setupUi(this);

    //QTcpServer当有客户端连接会发送newconnection，这个信号连接到accept_client槽
    connect(&mserver,&QTcpServer::newConnection,this,&AttendanceWin::accept_client);

    mserver.listen(QHostAddress::Any,9999);//监听，启动服务器，监听所有地址的9999端口

    bsize = 0;

    //给sql模型绑定表格
    model.setTable("student");

    //这里创建了一个新的线程thread，并将QFaceObject对象fobj移动到这个新创建的线程中执行。然后启动该线程。在多线程环境中，这有助于保持主线程（GUI线程）的响应性。
    //让fobj对象的操作（如槽函数）在新线程中运行，而不是在主线程（也称为GUI线程）中运行。

    //创建一个线程
    thread = new QThread();
    //把QFaceObject对象fobj移动到thread线程中执行，当 QFaceObject 涉及到耗时操作时，如图像处理，将其移动到线程中执行有利于节省时间
    //此时，与fobj相关的任何信号和槽操作都将在新线程中执行，而不是在创建fobj的原始线程（可能是主线程）中执行。
    fobj.moveToThread(thread);
    //启动线程
    thread->start();
    //把对象移动到线程上面去后，对象的函数不能直接调用，而是要通过信号来触发调用
    //由于fobj对象已经被移动到另一个线程中，所以不能在主线程中直接调用它的成员函数（除非这些函数是线程安全的或标记为Q_INVOKABLE以便通过Qt的信号和槽机制调用）。

    // 连接信号和槽
    connect(this,&AttendanceWin::query,&fobj,&QFaceObject::face_query);
    //关联QFaceObject对象里面的send_faceid信号
    connect(&fobj,&QFaceObject::send_faceid,this,&AttendanceWin::recv_faceid);
    connect(&timer,&QTimer::timeout,this,&AttendanceWin::updateTime);

    // 设置定时器间隔为1000毫秒（1秒）
    timer.start(1000);

    // 初始化时间显示
    updateTime();

    //对选项卡标签（QTabBar）的样式设置
//    QTabBar *tabBar = ui->tabWidget->tabBar();
//    tabBar->setStyleSheet("QTabBar::tab { font-family: 'Arial'; font-size: 12pt; color: blue; }");
    ui->tabWidget->setStyleSheet(
        "QTabBar::tab { "
        "    color: #333; "                        // 文本颜色
        "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "                                          stop: 0 #f0f0f0, stop: 1 #e0e0e0); " // 背景渐变
        "    border: 1px solid #c4c4c3; "           // 边框
        "    border-bottom-color: #c2c7cb; "        // 底部边框颜色，与 QTabWidget 边框合并
        "    border-top-left-radius: 4px; "         // 左上角圆角
        "    border-top-right-radius: 4px; "        // 右上角圆角
        "    min-width: 80px; "                     // 最小宽度
        "    padding: 6px; "                        // 内边距
        "    margin-right: 2px; "                   // 选项卡之间的间隔
        "}"
        "QTabBar::tab:selected { "
        "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "                                      stop: 0 #ffffff, stop: 1 #e0e0e0); " // 选中时的背景渐变
        "    border-color: #9B9B9B; "               // 选中时的边框颜色
        "    border-bottom-color: #C2C7CB; "        // 选中时底部边框颜色，与 QTabWidget 边框合并
        "}"
        "QTabBar::tab:hover { "
        "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "                                      stop: 0 #f5f5f5, stop: 1 #e5e5e5); " // 鼠标悬停时的背景渐变
        "}"
        );

}
void AttendanceWin::updateTime()
{
    // 获取当前时间并格式化为字符串
    QTime currentTime = QTime::currentTime();
    QString timeString = currentTime.toString("hh:mm:ss");

    // 用在界面的一个QLabel成员来显示时间
    if (ui->timeLabel)
    {
        ui->timeLabel->setText(timeString); // 设置标签的文本
    }
}

AttendanceWin::~AttendanceWin()
{
    delete ui;// 清理UI指针

    // 停止并等待线程结束
    if (thread->isRunning())
    {
        thread->quit(); // 请求线程退出
        thread->wait(); // 等待线程结束
    }

    // 如果msocket不是nullptr，断开连接并删除
    if (msocket)
    {
        msocket->disconnectFromHost();
        msocket->deleteLater(); // 在事件循环中删除msocket
        msocket = nullptr;
    }

    // 关闭服务器
    mserver.close();
}

void AttendanceWin::accept_client()
{
    //获取客户端通信的套接字
    msocket = mserver.nextPendingConnection();
    //当客户端有数据到达的时候会发送readyRead信号
    connect(msocket,&QTcpSocket::readyRead,this,&AttendanceWin::read_data);
}

void AttendanceWin::read_data()
{
//    //读取所有的数据
//    QString msg = msocket->readAll();
//    qDebug()<<msg;
    QDataStream stream(msocket);//把套接字绑定到数据流上
    stream.setVersion(QDataStream::Qt_6_5);//从流中把数据拿出来

    if(bsize == 0)
    {
        // 检查是否有足够的数据来读取bsize
        if(msocket->bytesAvailable()<(qint64)sizeof(bsize))
        {
            // 数据不足，返回并继续等待
            return;
        }
        //读取bsize，获得数据的长度
        stream>>bsize;
    }

    // 检查是否有足够的数据来读取整个数据块
    if(msocket->bytesAvailable() < bsize)//说明数据还没发送完成，返回继续等待
    {
        return;
    }

    QByteArray data;// data用于存放接收到的数据
    stream>>data;//此时得到是jpg格式数据，因为在客户端是jpg格式数据传过来的
    bsize = 0;
    if(data.size() == 0)//说明没有读取到数据
    {
        return;
    }

    //显示图片
    QPixmap mmp;
    if (!mmp.loadFromData(data, "jpg"))
    {
        // 图片加载失败，可能需要处理错误
        qDebug() << "图片加载失败";
        return;
    }
    //mmp.loadFromData(data,"jpg");
    mmp = mmp.scaled(ui->picLb->size());
    ui->picLb->setPixmap(mmp);

    //识别人脸
    cv::Mat faceImage;
    std::vector<uchar> decode;
    decode.resize(data.size());
    memcpy(decode.data(),data.data(),data.size());
    //拷贝data的数据，长度是data的大小,其实就是把data的数据拷贝到decode中去
    faceImage = cv::imdecode(decode,cv::IMREAD_COLOR);//此时字节流数据已经转化为cv::Mat类型(这是一个解码的过程)
    if (faceImage.empty())
    {
        // 解码失败，可能需要处理错误
        qDebug() << "解码失败";
        return;
    }
    //int faceid = fobj.face_query(faceImage);//消耗资源较多
    emit query(faceImage);//发出这个信号，就会自动执行QFaceObject中的face_query函数，因为在构造函数中query信号已经和该函数绑定
}

void AttendanceWin::recv_faceid(int64_t faceid)
{
    qDebug()<<"接收到了："<<faceid;
    if(faceid < 0)
    {
        // 发送空消息给客户端
        QString sdmsg = QString("{\"name\":\"\",\"number\":\"\",\"school\":\"\",\"dormitory_number\":\"\",\"time\":\"\",\"state\":\"\"}");
        msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
        return;
    }

    //从数据库中查询faceid对应的个人信息
    //给模型设置过滤器，设置过滤器相当于设置查询条件
    model.setFilter(QString("faceID=%1").arg(faceid));
    //开始查询
    model.select();

    //判断是否查询到数据
    if(model.rowCount() == 1)
    {
        //查到的行数是1，说明查到数据了，就把这行数据的结果打包成json/固定格式传递回到客户端

        QString state;//定义一个字符串来存储刷脸状态

        // 获取当前时间
        QDateTime currentTime = QDateTime::currentDateTime();
        QTime time = currentTime.time();

        // 判断时间并设置状态
        //if (time.hour() >= 23 || (time.hour() >= 0 && time.hour() < 6))
        if ((time.hour() >= 23 || time.hour() < 6))
        {
            state = "晚归";
        }
        else
        {
            state = "正常";
        }
        qDebug()<<state;

        //姓名、学号、学院、宿舍号、刷脸时间、刷脸状态
        //{name:%1,number:%2,school:%3,dormitory_number:%4,time:%5,state:6%}
        QSqlRecord record = model.record(0);//拿到查询的第一条记录
        QString sdmsg = QString("{\"name\":\"%1\",\"number\":\"%2\",\"school\":\"%3\",\"dormitory_number\":\"%4\",\"time\":\"%5\",\"state\":\"%6\"}")
                .arg(record.value("name").toString())
                .arg(record.value("number").toString())
                .arg(record.value("school").toString())
                .arg(record.value("dormitory_number").toString())
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(state);

        //读完客户端发送过来的人脸数据，进行人脸识别得到人脸id，并且获取了存在数据库中的信息后，就可以打包信息发送回客户端了
        //msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端

        //把数据写入数据库--刷脸信息表
        //QString insertSql = QString("insert into attendance(studentID, name) values('%1','%2')").arg(record.value("studentID").toString()).arg(record.value("name").toString());
        QString insertSql = QString("insert into attendance(studentID, name, state) values('%1','%2','%3')").arg(record.value("studentID").toString()).arg(record.value("name").toString()).arg(state);
        QSqlQuery query;
        if(!query.exec(insertSql))
        {
            qDebug()<<"插入失败, 发送空消息给客户端";
            QString sdmsg = QString("{\"name\":\"\",\"number\":\"\",\"school\":\"\",\"dormitory_number\":\"\",\"time\":\"\",\"state\":\"\"}");
            msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
            //qDebug()<<query.lastError().text();
            return;
        }
        else
        {
            // 插入成功，发送JSON消息给客户端
            msocket->write(sdmsg.toUtf8());
        }
    }
    else
    {
        qDebug() << "没有找到符合条件的学生" << faceid;
    }
}
