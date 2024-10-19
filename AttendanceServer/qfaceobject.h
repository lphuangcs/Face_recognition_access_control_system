#ifndef QFACEOBJECT_H
#define QFACEOBJECT_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <seeta/FaceEngine.h>
#include <seeta/FaceDetector.h>
//#include <SeetaFace/include/seeta/FaceDetector.h>
using namespace cv;
//这个类的职责：人脸数据存储、人脸检测、人脸识别
class QFaceObject : public QObject
{
    Q_OBJECT
public:
    explicit QFaceObject(QObject *parent = nullptr);
    ~QFaceObject();
public slots://两个槽函数：注册与查询
    int64_t face_register(cv::Mat& faceImage);
    int face_query(cv::Mat& faceImage);//传进去是一个图像数据，得到的是图像数据对应的id号
signals:
    void send_faceid(int64_t faceid);

private:
    seeta::FaceEngine *fengineptr;

};

#endif // QFACEOBJECT_H
