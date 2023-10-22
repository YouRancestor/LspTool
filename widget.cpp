#include "widget.h"
#include "./ui_widget.h"
#include "lspserverproxy.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , proxy(NULL)
{
    ui->setupUi(this);
    ui->tabWidget->addTab(new QPlainTextEdit, "0");

    connect(this, &Widget::MessageReceived, ui->plainTextEdit_Output, &QPlainTextEdit::appendPlainText, Qt::QueuedConnection);
    connect(this, &Widget::ServerExited, this, [this](int exit_code){
        QString str = QString::asprintf("\nServer exit with code %d.\n", exit_code);
        ui->plainTextEdit_Output->appendPlainText(str);
        ui->checkBox_StartServer->setChecked(false);
    }, Qt::QueuedConnection);
}

Widget::~Widget()
{
    delete proxy;
    delete ui;
}



void Widget::on_pushButton_AddTab_clicked()
{
    // 添加输入标签页
    ui->tabWidget->addTab(new QPlainTextEdit, ui->lineEdit_NewTabName->text());
}


void Widget::on_pushButton_DelTab_clicked()
{
    // 删除输入标签页
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
}

// 发送输入到LSP Server
void Widget::on_pushButton_SendInput_clicked()
{
    QPlainTextEdit* textEdit = dynamic_cast<QPlainTextEdit*>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        auto body = textEdit->toPlainText().toStdString();
        std::string msg = "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n" + body;
        proxy->SendRaw(msg.c_str(), int(msg.length()));
    }
}

// 启动/结束LSP Server进程
void Widget::on_checkBox_StartServer_clicked(bool checked)
{
    if (checked)
    {
        // 启动LSP Server进程
        proxy = new LspServerProxy(ui->lineEdit_ServerCmd->text().toStdString().c_str());
        proxy->SetCallback(OnMsgRecv, this);
        proxy->SetCallback(OnSvrExit, this);
        int ret = proxy->Init();
        if (ret)
        {
            QString str = QString::asprintf("\nServer init failed with code %d.\n", ret);
            ui->plainTextEdit_Output->appendPlainText(str);
            proxy = NULL;
            ui->checkBox_StartServer->setChecked(false);
        }
    }
    else
    {
        // 结束LSP Server进程
        delete proxy;
        proxy = NULL;
    }
}

// 回调：接收到来自服务器的消息
void Widget::OnMsgRecv(const char *data, size_t len, void *opaque)
{
    Widget* self = (Widget*)opaque;
    emit self->MessageReceived(QByteArray(data, int(len)));
}

// 回调：LSP Server进程退出
void Widget::OnSvrExit(int exit_code, void *opaque)
{
    Widget* self = (Widget*)opaque;
    emit self->ServerExited(exit_code);
}

