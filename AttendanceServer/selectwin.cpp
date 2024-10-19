#include "selectwin.h"
#include "ui_selectwin.h"
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

//查询的时候数据有两份，一份是学生信息表，另一份是刷脸信息表
SelectWin::SelectWin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectWin)
{
    ui->setupUi(this);
    model = new QSqlTableModel();

    ui->studentRb->setChecked(true);

    // 连接信号和槽
    connect(ui->studentRb, &QRadioButton::toggled, this, &SelectWin::onRadioButtonToggled);
    connect(ui->attRb, &QRadioButton::toggled, this, &SelectWin::onRadioButtonToggled);

    //connect(ui->tableView, &QTableView::doubleClicked, this, &SelectWin::onTableViewDoubleClicked);

    ui->selectBt->setStyleSheet("QPushButton { background-color: lightblue;border-radius:10px; }"
                          "QPushButton:hover { background-color: rgb(51, 122, 183); }");
}

void SelectWin::onRadioButtonToggled(bool checked)
{
    ui->tableView->setModel(nullptr);
    QRadioButton *button = qobject_cast<QRadioButton *>(sender());
    if (button)
    {
        if (button->text() == "学生信息查询" && checked)
        {
            // 显示第一页
            ui->stackedWidget->setCurrentIndex(1);
        }
        else if (button->text() == "刷脸信息查询" && checked)
        {
            // 显示第二页
            ui->stackedWidget->setCurrentIndex(0);
        }
    }
}

SelectWin::~SelectWin()
{
    delete ui;
}

void SelectWin::on_selectBt_clicked()
{
    qDebug()<<"进来了";
    QString tableName;
    if(ui->studentRb->isChecked())
    {
        tableName = "student";
    }
    else if(ui->attRb->isChecked())
    {
        tableName = "attendance";
    }

    else
    {
        // 没有选择任何表格，可以选择显示一个错误消息或者不做任何操作
        QMessageBox::information(this,"错误提示","请先选择要查询的信息");
        return;// 不继续执行后续代码
    }

    if(tableName == "student")
    {
        QString name = ui->nameEdit_2->text();
        QString school = ui->school_comboBox->currentText();
        QString dormitory = ui->dormitory_comboBox->currentText();

        QString filter;// 定义一个过滤器

        qDebug()<<name<<school<<dormitory;

        // 构建查询条件
        QStringList conditions;
        if (!name.isEmpty())
        {
            qDebug()<<"name = ";
            conditions << "name = '" + name + "'";
        }
        if (!school.isEmpty() && school != "全部")
        {
            qDebug()<<"学院没写";
            conditions << "school = '" + school + "'";

        }
        if (!dormitory.isEmpty() && dormitory != "全部")
        {
            qDebug()<<"宿舍没写";
            conditions << "dormitory_number LIKE '" + dormitory + "%'";
        }

        // 将条件列表转换为SQL语句的WHERE部分
        if (!conditions.isEmpty())
        {
            filter = "WHERE " + conditions.join(" AND ");
        }

        // 设置模型的查询条件（这里假设你有一个设置SQL语句的方法）
        // 如果没有输入，filter将是空的，这时应该显示整个表格
        //qDebug()<<QString("SELECT * FROM %1 %2").arg(tableName).arg(filter);
        model->setQuery(QString("SELECT * FROM %1 %2").arg(tableName).arg(filter));
    }

    if(tableName == "attendance")
    {
        QString name = ui->nameEdit_1->text();//获取名字
        QString state = ui->state_comboBox->currentText();//获取要查询的时“正常”还是“晚归”

        QDate riqi = ui->dateEdit->date(); // 使用 date() 方法获取日期
        QString date = riqi.toString("yyyy-MM-dd");

        QString filter;// 定义一个过滤器

        qDebug()<<name<<state<<date;

        // 构建查询条件
        QStringList conditions;
        if (!name.isEmpty())
        {
            conditions << "name = '" + name + "'";
        }
        if (!state.isEmpty() && state != "全部")
        {
            conditions << "state = '" + state + "'";

        }
        if (!date.isEmpty() && date != "2000-01-01")
        {
            conditions << "attendanceTime LIKE '" + date + "%'";
        }

        // 将条件列表转换为SQL语句的WHERE部分
        if (!conditions.isEmpty())
        {
            filter = "WHERE " + conditions.join(" AND ");
        }

        // 设置模型的查询条件（这里假设你有一个设置SQL语句的方法）
        // 如果没有输入，filter将是空的，这时应该显示整个表格
        model->setQuery(QString("SELECT * FROM %1 %2").arg(tableName).arg(filter));
    }

    //查询
    model->select();
    //qDebug()<<model->record(0).value("attendanceTime");
    if (model->rowCount() == 0)
    {
        // 没有数据
        QMessageBox::information(this, "查询结果", "没有找到符合条件的数据。");
    }

    //ui->tableView->setStyleSheet("QTableView::item { text-align: center; }");
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();//使用自动调整列宽
}

