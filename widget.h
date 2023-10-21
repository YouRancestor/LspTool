#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class LspServerProxy;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void MessageReceived(const QString& msg);
    void ServerExited(int exit_code);

private slots:
    void on_pushButton_AddTab_clicked();

    void on_pushButton_DelTab_clicked();

    void on_pushButton_SendInput_clicked();

    void on_checkBox_StartServer_clicked(bool checked);

private:
    static void OnMsgRecv(const char* data, size_t len, void* opaque);
    static void OnSvrExit(int exit_code, void* opaque);

    Ui::Widget *ui;
    LspServerProxy* proxy;
};
#endif // WIDGET_H
