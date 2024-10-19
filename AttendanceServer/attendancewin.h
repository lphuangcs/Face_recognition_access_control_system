#ifndef ATTENDANCEWIN_H
#define ATTENDANCEWIN_H

#include <qfaceobject.h>

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class AttendanceWin; }
QT_END_NAMESPACE

class AttendanceWin : public QMainWindow
{
    Q_OBJECT

public:
    AttendanceWin(QWidget *parent = nullptr);
    ~AttendanceWin();

signals:
    void query(cv::Mat &image);

private slots:
    void accept_client();
    void read_data();
    void recv_faceid(int64_t faceid);
    void updateTime();
private:
    Ui::AttendanceWin *ui;

    QTcpServer mserver;
    QTcpSocket *msocket;

    quint64 bsize;

    QFaceObject fobj;
    QSqlTableModel model;

    QTimer timer;

    QThread *thread;//一个指向QThread对象的指针，用于在多线程环境中执行某些任务。

};
#endif // ATTENDANCEWIN_H
