#ifndef SELECTWIN_H
#define SELECTWIN_H

#include <QWidget>
#include <QSqlTableModel>
#include <QTimer>
namespace Ui {
class SelectWin;
}
class SelectWin : public QWidget
{
    Q_OBJECT

public:
    explicit SelectWin(QWidget *parent = nullptr);
    ~SelectWin();

private slots:
    void on_selectBt_clicked();
    void onRadioButtonToggled(bool checked);

private:
    Ui::SelectWin *ui;
    QSqlTableModel *model;
};

#endif // SELECTWIN_H
