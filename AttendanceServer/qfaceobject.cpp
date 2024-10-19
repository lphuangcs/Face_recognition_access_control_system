#include "qfaceobject.h"
#include <FaceDatabase.h>
#include <QDebug>
using namespace seeta;
QFaceObject::QFaceObject(QObject *parent)
    : QObject{parent}
{
    //初始化引擎
    ModelSetting FDmodel("C:/SeetaFace/bin/model/fd_2_00.dat", ModelSetting::CPU, 0);      //人脸检测模型
    ModelSetting PDmodel("C:/SeetaFace/bin/model/pd_2_00_pts5.dat", ModelSetting::CPU, 0); //
    ModelSetting FRmodel("C:/SeetaFace/bin/model/fr_2_10.dat", ModelSetting::CPU, 0); //人脸识别
    //创建FaceEngine实例
    try
    {
        this->fengineptr = new seeta::FaceEngine(FDmodel,PDmodel,FRmodel);//引擎建好了，之后可以进行人脸注册
    }
    catch (const std::exception& e)
    {
        qDebug() << "创建FaceEngine实例失败:" << e.what();
    }
    //人脸引擎中有多种方法
    //导入已有的人脸数据库
    this->fengineptr->Load("./face.db");
}

QFaceObject::~QFaceObject()
{
    delete fengineptr;
    fengineptr = nullptr;

}

int64_t QFaceObject::face_register(Mat &faceImage)
{
    // 判断faceImage是否有效
    if (faceImage.empty())
    {
        return -1; // 或者抛出异常
    }
    //把opencv的Mat数据转为seetaface数据（为了用下面的register函数，该函数要求的参数类型是SeetaImageData）
    SeetaImageData simage;//变量 simage，用于存储转换后的人脸图像数据。
    simage.data = faceImage.data;
    simage.width = faceImage.cols;//宽度对应列
    simage.height = faceImage.rows;//高度对应行
    simage.channels = faceImage.channels();

    //转换完成，进行人脸注册
    int64_t faceid = this->fengineptr->Register(simage);//注册返回一个人脸id
    //this->fengineptr->Register返回的是整型数，类型是int64_t

    if(faceid >= 0)
    {
        if(!this->fengineptr->Save("./face.db"))
        {
            // 处理保存失败的情况
            faceid = -1;
        }//将当前在内存中的人脸特征数据、标签（如果有的话）等信息保存到指定的文件（在这里是./face.db）中
    }
    return faceid;
    //注册成功返回对应的id，失败就返回-1
}

int QFaceObject::face_query(Mat &faceImage)
{
    //把opencv的Mat数据转为seetaface数据（为了用下面的Query函数）
    SeetaImageData simage;

    // 确保faceImage非空
    if (faceImage.empty())
    {
        // 可以在这里发出一个信号或者返回一个错误代码
        emit send_faceid(-1); // 假设这是表示无效的ID
        return -1;
    }

    simage.data = faceImage.data;
    simage.width = faceImage.cols;
    simage.height = faceImage.rows;
    simage.channels = faceImage.channels();
    float similarity = 0;
    int64_t faceid = fengineptr->Query(simage, &similarity);//运算时间长，耗费资源多
    //Query函数运算时间长且耗费资源多。如果可能，考虑优化输入图像的大小或质量，或者查看是否有并行化或异步执行该函数的选项。

    //qDebug()<<"similarity = "<<similarity;
    //qDebug()<<"faceid = "<<faceid;
    if(similarity > 0.5)
    {
        emit send_faceid(faceid);// 发出带有有效faceid的信号
    }
    else
    {
        emit send_faceid(-1);// 发出表示未识别到人脸的信号
    }
    return faceid;
}
