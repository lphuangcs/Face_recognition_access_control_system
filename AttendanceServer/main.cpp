#include "attendancewin.h"
#include "selectwin.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>
#include "registerwin.h"
int main(int argc, char *argv[])
{

    QString state;
    // 获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    QTime time = currentTime.time();

    QApplication a(argc, argv);

    qRegisterMetaType<cv::Mat>("cv::Mat&");
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<int64_t>("int64_t");

    //连接数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    //设置数据名称
    db.setDatabaseName("server.db");//服务器数据库，会包含两个表：员工表与考勤表
    //打开数据库
    if(!db.open())
    {
        //打开数据库失败
        qDebug()<<db.lastError().text();
        return -1;
    }

    //创建学生信息表格
    QString createsql = "create table if not exists student(studentID integer primary key autoincrement,name varchar(256),sex varchar(32),"
                        "number text, school text, dormitory_number text, phone_number text, faceID integer unique, headfile text)";
    QSqlQuery query;
    if(!query.exec(createsql))
    {
        // 员工信息表数据库表创建失败，打印错误信息后推出
        qDebug()<<query.lastError().text();
        return -1;
    }

    // 创建刷脸信息表格
    createsql = "create table if not exists attendance(attendanceID integer primary key autoincrement, studentID integer, name varchar(256),"
                "attendanceTime TimeStamp NOT NULL DEFAULT(datetime('now','localtime')),state varchar(32))";//时间设置默认事件为当前系统时间
    if(!query.exec(createsql))
    {
        // 考勤表数据库表创建失败，打印错误信息后推出
        qDebug()<<query.lastError().text();
        return -1;
    }

    AttendanceWin w;
    w.show();

    return a.exec();
}
