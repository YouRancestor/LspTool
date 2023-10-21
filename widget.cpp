#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->tabWidget->addTab(new QPlainTextEdit, "0");
}

Widget::~Widget()
{
    delete ui;
}



void Widget::on_pushButton_AddTab_clicked()
{
    ui->tabWidget->addTab(new QPlainTextEdit, ui->lineEdit_NewTabName->text());
}


void Widget::on_pushButton_DelTab_clicked()
{
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
}


void Widget::on_pushButton_SendInput_clicked()
{
    QPlainTextEdit* textEdit = dynamic_cast<QPlainTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        qDebug("%s",textEdit->toPlainText().toStdString().c_str());
    }
}

