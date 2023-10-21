#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_AddTab_clicked();

    void on_pushButton_DelTab_clicked();

    void on_pushButton_SendInput_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
