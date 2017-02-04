#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QDialog>
#include <QGridLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QUdpSocket>
#include <QVector>
#include <QMap>
#include <QLabel>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QApplication>
#include <QDir>
#include <ctime>
#include <QCheckBox>
#include <QFile>

class UDPServer : public QDialog
{
    Q_OBJECT

public:
    UDPServer(QWidget *parent = 0);
    ~UDPServer();

private:
        QUdpSocket *server;
        QTextEdit *text;
        QLineEdit *line;
        QListWidget *list;
        QMap<QString, QHostAddress> addreses;
        QVector<QListWidgetItem*> items;
        QDir dir;
        QCheckBox *download;
        int old_size;

private:
        void Layout();
        QTextEdit *edit();
        QPushButton *button(QString name);
        void AddClient(QString &nickname);
        void DelClient(QString &nickname);
        void SendClientList(QString message);
        void Monitor();

private slots:
                void SendDatagram();
                void ProcessData();

signals:
                void Close();
};

#endif // UDPSERVER_H
