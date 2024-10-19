#include "registerwin.h"
#include "ui_registerwin.h"
#include <QFileDialog>
#include <qfaceobject.h>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QRegularExpression>
RegisterWin::RegisterWin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterWin)
{
    ui->setupUi(this);
}

RegisterWin::~RegisterWin()
{
    delete ui;
}

void RegisterWin::timerEvent(QTimerEvent *e)
{
    //获取摄像头数据并且显示在界面上
    if(cap.isOpened())
    {
        cap>>image;
        if(image.data == nullptr)
        {
            return;
        }
    }

    //Mat转QImage
    cv::Mat rgbImage;
    cv::cvtColor(image,rgbImage,cv::COLOR_BGR2RGB); // 确保图像数据是连续的

    // 确保QImage的步长是正确的
    QImage qImg(rgbImage.data, rgbImage.cols,rgbImage.rows,rgbImage.step1(),QImage::Format_RGB888);

    //在Qt界面上显示
    QPixmap mmp = QPixmap::fromImage(qImg);
    mmp = mmp.scaledToWidth(ui->headpicLb->width());
    ui->headpicLb->setPixmap(mmp);
}

void RegisterWin::on_resetBt_clicked()
{
    // 清空文本编辑框
    ui->nameEdit->clear();

    // 选中索引为0的项，即为“请选择”
    ui->sex_comboBox->setCurrentIndex(0);
    ui->numberEdit->clear();
    ui->school_comboBox->setCurrentIndex(0);
    ui->dormitoryEdit->clear();
    ui->phone_number_Edit->clear();
    ui->picfileEdit->clear();

    // 清除图片显示区域
    ui->headpicLb->clear();
}


void RegisterWin::on_addpicBt_clicked()
{
    // 检查摄像头是否处于打开状态
    if(ui->videoswitchBt->text() == "关闭摄像头")
    {
        // 关闭摄像头和定时器
        killTimer(timerid);//关闭定时器事件
        timerid = -1; // 重置timerid为无效值
        ui->videoswitchBt->setText("打开摄像头");
        ui->headpicLb->clear();// 清除图片显示控件
        //关闭摄像头
        cap.release();// 释放摄像头对象,释放对象就相当于关闭了
    }

    //通过文件对话框选择图片路径
    QString filepath = QFileDialog::getOpenFileName(this);//保存路径
    if (!filepath.isEmpty())
    {
        ui->picfileEdit->setText(filepath);

        // 显示图片
        QPixmap mmp;
        if (!mmp.load(filepath))
        {
            // 图片加载失败，显示错误消息或进行其他错误处理
            QMessageBox::warning(this, tr("错误"), tr("无法加载图片文件: %1").arg(filepath));
            return;
        }
        mmp = mmp.scaled(ui->headpicLb->size());
        ui->headpicLb->setPixmap(mmp);
    }
}

void RegisterWin::on_registerBt_clicked()
{
    // 1. 进行数据验证
    QString name = ui->nameEdit->text();
    QString sex = ui->sex_comboBox->currentText(); // comboBox是性别选择器
    QString number = ui->numberEdit->text(); // 假设是学号或编号
    QString school = ui->school_comboBox->currentText();
    QString dormitoryNumber = ui->dormitoryEdit->text();
    QString phoneNumber = ui->phone_number_Edit->text();
    QString path_of_pic = ui->picfileEdit->text();

    // 先确认信息是否填写好了
    // 检查每个字段是否为空
    if (name.isEmpty() || sex == "请选择" || number.isEmpty() || school == "请选择" || dormitoryNumber.isEmpty() || phoneNumber.isEmpty())
    {
        QMessageBox::warning(this, "错误", "信息不完整，请先完成所有信息的填写");
        return;
    }
    // 检查是否已经采集图片
    if(path_of_pic.isEmpty())
    {
        QMessageBox::warning(this, "错误", "请先完成图片的采集");
        return;
    }

    // 检查信息是否填对了
    // 检查学号有效性
    if(number.size() != 12)
    {
        QMessageBox::warning(this, "错误", "请输入有效的学号");
        return;
    }
    //检查宿舍号有效性
    //QRegularExpression regex("^C\\d+-");      // 正则表达式，\\d 表示数字，+ 表示一个或多个
    QRegularExpression regex("^C\\d+-\\d{3}$");
    QRegularExpressionMatch match = regex.match(dormitoryNumber);
    if(!(match.hasMatch() && match.capturedStart() == 0))
    {
        QMessageBox::warning(this, "错误", "请输入有效的宿舍号");
        return;
    }
    //检查电话号码有效性
    if(phoneNumber.size() != 11)
    {
        QMessageBox::warning(this, "错误", "请输入有效的电话号码");
        return;
    }

    //2、 通过照片，结合FaceObject模块得到faceid
    cv::Mat image = cv::imread(ui->picfileEdit->text().toUtf8().data());

    // 检查图片是否成功加载
    if (image.empty())
    {
        QMessageBox::warning(this, "错误", "无法加载图片文件");
        return;
    }

    // 给收集的人脸图片登记，定出一个id
    QFaceObject faceobj;
    int faceID = faceobj.face_register(image);
    qDebug()<<faceID;
    // 如果采集的照片不是人脸，就应该重新采集
    if(faceID < 0)
    {
        QMessageBox::warning(this, "错误", "未识别到人脸，请重新采集图片");
        return;
    }

    //把头像保存到一个固定路径下,把文件名改为姓名
    QString new_headfilename = QString("./data/%1.jpg").arg(QString(ui->nameEdit->text()));
    qDebug()<<new_headfilename;

    //接下来对图片改名
    QFile file(ui->picfileEdit->text());
    if (!file.exists())
    {
        qDebug() << "图片不存在:" << ui->picfileEdit->text();
        return;
    }
    // 使用QDir来重命名文件
    QDir dir;
    if (!dir.rename(ui->picfileEdit->text(), new_headfilename))
    {
        qDebug() << "重命名失败:" << ui->picfileEdit->text() << "to" << new_headfilename;
        return;
    }

    //3、 把个人信息存储到数据库表employee
    QSqlTableModel model;// QSqlTableModel 是 Qt 框架中用于表示数据库表内容的一个模型类。
    model.setTable("student");//绑定一下student表格
    //设置表名，通过 setTable 方法，你可以告诉 QSqlTableModel 你想要与哪个数据库表进行交互。
    //将 model 绑定到名为 "employee" 的数据库表，接着可使用 QSqlTableModel 的各种方法来查询、编辑和插入该表中的数据。
    QSqlRecord record = model.record();
    //设置记录record的数据
    record.setValue("name",ui->nameEdit->text());
    record.setValue("sex",ui->sex_comboBox->currentText());
    record.setValue("number",ui->numberEdit->text());
    record.setValue("school",ui->school_comboBox->currentText());
    record.setValue("dormitory_number",ui->dormitoryEdit->text());
    record.setValue("phone_number",ui->phone_number_Edit->text());
    record.setValue("faceID",faceID);
    //头像路径
    record.setValue("headfile",new_headfilename);
    //此时所有信息打包好了，将记录插入到数据库表格中
    bool ret = model.insertRecord(0,record);

    //4、 提示注册成功
    if(ret)
    {
        //QMessageBox消息盒子，来显示提示语
        QMessageBox::information(this,"注册提示","注册成功");
        //提交，把数据真正写到数据库中去
        model.submitAll();
        // 当你使用 QSqlTableModel 对数据库进行操作时，所有的更改最初都是在模型内部进行的，并不会立即反映到实际的数据库中。
        // model.submitAll()是用于将所有在模型中做的更改（例如添加、删除或修改记录）提交到数据库中
        on_resetBt_clicked();
    }
    else
    {
        QMessageBox::information(this,"注册提示","注册失败");
    }
}

void RegisterWin::on_videoswitchBt_clicked()
{
    if(ui->videoswitchBt->text() == "打开摄像头")
    {
        //打开摄像头
        if(cap.open(0))//打开默认摄像头
        {
            ui->videoswitchBt->setText("关闭摄像头");
            //启动定时器事件
            timerid = startTimer(100);
        }
        else
        {
            // 摄像头打开失败，给出提示
            QMessageBox::warning(this, "错误", "无法打开摄像头");
        }
    }
    else
    {
        // 关闭摄像头和定时器
        killTimer(timerid);//关闭定时器事件
        timerid = -1; // 重置timerid为无效值
        ui->videoswitchBt->setText("打开摄像头");
        ui->headpicLb->clear();// 清除图片显示控件
        //关闭摄像头
        cap.release();// 释放摄像头对象,释放对象就相当于关闭了
    }
}

void RegisterWin::on_cameraBt_clicked()
{
    //拍照的时候也应该要关闭定时器，但在关闭定时器之前应该要把数据先保存下来（拍张照）

    //保存数据
    //把头像保存到一个固定路径下
    QString headfile = QString("./data/%1.jpg").arg(QString(ui->nameEdit->text().toUtf8().toBase64()));
    ui->picfileEdit->setText(headfile);

    // 使用 OpenCV 的 imwrite 函数保存图像，cv::imwrite 是一个函数，用于将图像数据写入到文件中。
    // image是一个cv::Mat对象，它包含了要写入文件的图像数据。
    cv::imwrite(headfile.toUtf8().data(),image);

    // 关闭定时器、更新按钮文本、关闭摄像头
    killTimer(timerid);//关闭定时器事件
    ui->videoswitchBt->setText("打开摄像头");
    //关闭摄像头
    cap.release();//释放对象就相当于关闭了
}

